#include <zlib.h>
#include "network.h"
#include "../threads/thread.h"
#include "networkutils.h"
#include "ta3dsock.h"
#include "TA3D_Network.h"
#include "../logs/logs.h"




namespace TA3D
{


    //need testing
    void ListenThread::proc(void* param)
    {
        TA3DSock* newsock;
        int v = 0;

        Network* network;
        network = ((struct net_thread_params*)param)->network;
        delete ((struct net_thread_params*)param);

        struct event event;
        event.type = 45; //arbitrary number for "new player connected" event
        //fill in other info for new player connected event


        while(!pDead && network->listen_socket.isOpen() )
        {
            v = network->listen_socket.Accept(&newsock,100);
            if(v<0)
            {
                if(v==-1){}
            }
            else
            {
                //perhaps check address and take other actions here
                //add to list and spawn thread
                network->addPlayer(newsock);
            }
        }
    }



    void SocketThread::proc(void* param){
        TA3DSock* sock;
        Network* network;
        int sockid,packtype;

        struct chat chat;
        struct order order;
        struct sync sync;
        struct event event;

        network = ((struct net_thread_params*)param)->network;
        sockid = ((struct net_thread_params*)param)->sockid;
        delete ((struct net_thread_params*)param);
        sock = network->players.getSock(sockid);

        while(!pDead && sock->isOpen())
        {
            //sleep until data is coming
            sock->takeFive(1000);
            if(pDead) break;

            //ready for reading, absorb some bytes
            sock->pumpIn();

            //see if there is a packet ready to process
            packtype = sock->getPacket();

            switch(packtype)
            {
                case 'P':		// ping
                    if( sockid != -1 )
                        network->sendSpecial("PONG", -1, sockid);
                    sock->makePing();
                    break;
                case 'A'://special (resend to all!!)
                case 'X'://special
                    network->xqmutex.lock();
                    if( pDead || sock->makeSpecial(&chat) == -1 )
                    {
                        network->xqmutex.unlock();
                        break;
                    }
                    if( packtype != 'A' && network->isServer() )
                        chat.from = sockid;
                    network->specialq.enqueue(&chat);
                    network->xqmutex.unlock();
                    if( packtype == 'A' && network->isServer() )
                        network->sendSpecial( &chat, sockid, -1, true );
                    break;
                case 'C'://chat
                    network->cqmutex.lock();
                    if( pDead || sock->makeChat(&chat) == -1 ){
                        network->cqmutex.unlock();
                        break;
                    }
                    network->chatq.enqueue(&chat);
                    network->cqmutex.unlock();
                    if( network->isServer() )
                        network->sendChat(&chat, sockid);
                    break;
                case 'O'://order
                    network->oqmutex.lock();
                    if( pDead || sock->makeOrder(&order) == -1 ){
                        network->oqmutex.unlock();
                        break;
                    }
                    network->orderq.enqueue(&order);
                    network->oqmutex.unlock();
                    if( network->isServer() )
                        network->sendOrder(&order, sockid);
                    break;
                case 'S'://sync
                    network->sqmutex.lock();
                    if( pDead || sock->makeSync(&sync) == -1 ){
                        network->sqmutex.unlock();
                        break;
                    }
                    network->syncq.enqueue(&sync);
                    network->sqmutex.unlock();
                    if( network->isServer() )
                        network->sendSync(&sync, sockid);
                    break;
                case 'E'://event
                    network->eqmutex.lock();
                    if( pDead || sock->makeEvent(&event) == -1 ){
                        network->eqmutex.unlock();
                        break;
                    }
                    printf("received event\n");
                    network->eventq.enqueue(&event);
                    network->eqmutex.unlock();
                    if( network->isServer() )
                        network->sendEvent(&event, sockid);
                case 0:
                    break;

                    // For file transfert
                case 'F':						// File data
                    {
                        int port = sock->getFilePort();
                        for(std::list< GetFileThread* >::iterator i = network->getfile_thread.begin() ; i != network->getfile_thread.end() ; i++ )
                            if( (*i)->port == port ) {
                                port = -1;
                                while( !(*i)->ready && !(*i)->isDead() )	rest(1);
                                if( !(*i)->isDead() ) {
                                    (*i)->buffer_size = sock->getFileData( (*i)->buffer );
                                    (*i)->ready = false;
                                }
                                break;
                            }
                        if( port != -1 )
                            sock->getFileData( NULL );
                    }
                    break;
                case 'R':						// File response (send back the amount of data that has been received)
                    {
                        int port = sock->getFilePort();
                        for(std::list< SendFileThread* >::iterator i = network->sendfile_thread.begin() ; i != network->sendfile_thread.end() ; i++ )
                            if( (*i)->port == port && (*i)->player_id == sockid ) {
                                port = -1;
                                sock->getFileData( (byte*)&((*i)->progress) );
                                break;
                            }
                        if( port != -1 )
                            sock->getFileData( NULL );
                    }
                    break;

                default:
                    sock->cleanPacket();
            }


        }

        pDead = 1;
        if( !sock->isOpen() )
            network->setPlayerDirty();

        return;
    }

    void UDPThread::proc(void* param)
    {
        Network* network;
        UDPSock *sock;
        int packtype;

        struct chat chat;
        struct sync sync;
        struct event event;

        network = ((struct net_thread_params*)param)->network;
        sock = &(network->udp_socket);
        delete ((struct net_thread_params*)param);

        while(!pDead && sock->isOpen())
        {
            //sleep until data is coming
            sock->takeFive(1000);
            if(pDead) break;

            //ready for reading, absorb some bytes
            sock->pumpIn();

            //see if there is a packet ready to process
            packtype = sock->getPacket();

            int player_id = -1;

            if( packtype )
                for( int i = 1 ; i <= network->players.getMaxId() ; i++ ) {
                    TA3DSock *s = network->players.getSock( i );
                    if( s && strcmp( s->getAddress(), sock->getAddress() ) == 0 ) {
                        player_id = i;
                        break;
                    }
                }

            switch(packtype){
                case 'X'://special
                    network->xqmutex.lock();
                    if( pDead || sock->makeSpecial(&chat) == -1 ){
                        network->xqmutex.unlock();
                        break;
                    }
                    if( network->isServer() )
                        chat.from = player_id;
                    network->specialq.enqueue(&chat);
                    network->xqmutex.unlock();
                    break;
                case 'S'://sync
                    network->sqmutex.lock();
                    if( pDead || sock->makeSync(&sync) == -1 ){
                        network->sqmutex.unlock();
                        break;
                    }
                    network->syncq.enqueue(&sync);
                    network->sqmutex.unlock();
                    if( network->isServer() )
                        network->sendSync(&sync, player_id);
                    break;
                case 'E'://UDP event, used to tell someone we've synced a unit, so check destination and resend it if necessary
                    {
                        network->eqmutex.lock();
                        if( pDead || sock->makeEvent(&event) == -1 ){
                            network->eqmutex.unlock();
                            break;
                        }
                        if( event.type != EVENT_UNIT_SYNCED ) {		// We only accept EVENT_UNIT_SYNCED
                            network->eqmutex.unlock();
                            break;
                        }
                        int dest = g_ta3d_network->getNetworkID( event.opt1 );
                        if( dest == network_manager.getMyID() ) {
                            network->eventq.enqueue(&event);
                            dest = -1;
                        }
                        network->eqmutex.unlock();
                        if( network->isServer() && dest != -1 )
                            network->sendEventUDP(&event, dest);
                    }
                    break;
                case 0:
                    break;

                default:
                    sock->cleanPacket();
            }


        }

        pDead = 1;
        if( !sock->isOpen() )
            network->setPlayerDirty();

        return;
    }


    void BroadCastThread::proc(void* param)
    {
        BroadcastSock* sock;
        Network* network;

        network = ((struct net_thread_params*)param)->network;
        sock = &(network->broadcast_socket);

        delete ((struct net_thread_params*)param);

        String msg;

        pDead = 0;

        while(!pDead && sock->isOpen() )
        {
            //sleep until data is coming
            sock->takeFive(1000);
            if(pDead) break;

            //ready for reading, absorb some bytes
            sock->pumpIn();

            msg = sock->makeMessage();
            if (!msg.empty())
            {
                network->mqmutex.lock();
                if (pDead)
                {
                    network->mqmutex.unlock();
                    break;
                }
                network->broadcastq.push_back( msg );
                network->broadcastaddressq.push_back( sock->getAddress() );
                network->mqmutex.unlock();
            }
        }

        pDead = 1;
        LOG_DEBUG(LOG_PREFIX_NET_BROADCAST << "The thread has been closed");
        return;
    }

#define FILE_TRANSFER_BUFFER_SIZE		16384

    //NEED TESTING
    void SendFileThread::proc(void* param)
    {
        Network* network;
        int sockid;
        TA3D_FILE* file;
        int length,n;
        byte *buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE ];
        byte *compressed_buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE * 2 + 12 ];
        String filename;

        network = ((struct net_thread_params*)param)->network;
        sockid = ((struct net_thread_params*)param)->sockid;
        filename = ((struct net_thread_params*)param)->filename;
        file = ta3d_fopen( filename );

        delete ((struct net_thread_params*)param);

        if (NULL == file)
        {
            delete[] buffer;
            delete[] compressed_buffer;
            LOG_DEBUG( LOG_PREFIX_NET_FILE << "cannot open file '" << filename << "'" );
            pDead = 1;
            network->setFileDirty();
            return;
        }

        length = ta3d_fsize(file);

        int timer = msec_timer;

        int real_length = length;

        int pos = 0;
        progress = 0;

        network->sendFileData(sockid,port,(byte*)&length,4);

        LOG_INFO(LOG_PREFIX_NET_FILE << "Starting...");
        while (!pDead)
        {
            n = ta3d_fread(buffer,1,FILE_TRANSFER_BUFFER_SIZE,file);            // Read data into the buffer
            uLongf compressed_size = FILE_TRANSFER_BUFFER_SIZE * 2 + 12;
            int res = compress2(compressed_buffer, &compressed_size, buffer, n, 9);   // Compress the data
            switch(res)
            {
            case Z_OK:
                break;
            case Z_MEM_ERROR:
                LOG_ERROR(LOG_PREFIX_NET_FILE << "compress2 : Z_MEM_ERROR");
                break;
            case Z_BUF_ERROR:
                LOG_ERROR(LOG_PREFIX_NET_FILE << "compress2 : Z_BUF_ERROR");
                break;
            case Z_STREAM_ERROR:
                LOG_ERROR(LOG_PREFIX_NET_FILE << "compress2 : Z_STREAM_ERROR");
                break;
            default:
                LOG_ERROR(LOG_PREFIX_NET_FILE << "compress2 : unknown error");
            };
            std::cout << n << " ==> " << compressed_size << std::endl;
            network->sendFileData(sockid,port,compressed_buffer,compressed_size);
            if (n > 0)
            {
                pos += n;
                network->updateFileTransferInformation( filename + format("%d", sockid), real_length, pos );

                int timer = msec_timer;
                while( progress < pos - 10 * FILE_TRANSFER_BUFFER_SIZE && !pDead && msec_timer - timer < 60000 )
                    rest(0);
                if (msec_timer - timer >= 60000)
                {
                    delete[] buffer;
                    delete[] compressed_buffer;
                    LOG_DEBUG( LOG_PREFIX_NET_FILE << "file transfert timed out");
                    pDead = 1;
                    network->updateFileTransferInformation(filename + format("%d", sockid), 0, 0);
                    network->setFileDirty();
                    ta3d_fclose(file);
                    return;
                }
            }

            if (ta3d_feof(file))
                break;

            rest(1);
        }

        timer = msec_timer;
        while (progress < pos - FILE_TRANSFER_BUFFER_SIZE && !pDead && msec_timer - timer < 60000)
            rest(1);		// Wait for client to say ok

        LOG_INFO(LOG_PREFIX_NET_FILE << "Done.");

        network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
        pDead = 1;
        ta3d_fclose( file );
        network->setFileDirty();
        delete[] buffer;
        delete[] compressed_buffer;
        return;
    }


    GetFileThread::GetFileThread() : Thread()
    {
        ready = false;
    }

    //NEEDS TESTING
    //doesnt support resume after broken transfer
    void GetFileThread::proc(void* param)
    {
        Network* network;
        int sockid;
        FILE* file;
        String filename;
        int length,n,sofar;

        buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE * 2 + 12 ];
        byte *uncompressed_buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE + 12 ];
        network = ((struct net_thread_params*)param)->network;

        //supposed sender
        sockid = ((struct net_thread_params*)param)->sockid;

        //blank file open for writing
        filename = ((struct net_thread_params*)param)->filename;
        file = TA3D_OpenFile( filename + ".part", "wb" );

        delete ((struct net_thread_params*)param);

        if( file == NULL )
        {
            LOG_DEBUG( LOG_PREFIX_NET_FILE << "cannot open file '" << filename << ".part'");
            pDead = 1;
            network->setFileDirty();
            delete[] buffer;
            delete[] uncompressed_buffer;
            return;
        }

        LOG_INFO(LOG_PREFIX_NET_FILE << "Starting...");

        int timer = msec_timer;

        ready = true;
        while (!pDead && ready && msec_timer - timer < 60000 )
            rest(0);
        memcpy(&length,buffer,4);

        if( ready ) // Time out
        {
            LOG_DEBUG(LOG_PREFIX_NET_FILE << "file transfert timed out (0)");
            pDead = 1;
            fclose( file );
            delete_file( (filename + ".part").c_str() );
            network->setFileDirty();
            delete[] buffer;
            delete[] uncompressed_buffer;
            network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
            return;
        }

        sofar = 0;
        if( pDead ) length = 1;			// In order to delete the file
        while (!pDead)
        {
            ready = true;
            timer = msec_timer;
            while( !pDead && ready && msec_timer - timer < 60000 ) rest( 0 );			// Get paquet data
            n = buffer_size;

            if (ready) // Time out
            {
                LOG_DEBUG(LOG_PREFIX_NET_FILE << "file transfert timed out (1)");
                pDead = 1;
                fclose( file );
                delete_file( (filename + ".part").c_str() );
                network->setFileDirty();
                delete[] buffer;
                delete[] uncompressed_buffer;
                network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
                return;
            }

            if (n > 0)
            {
                        // First we must decompress the data
                uLongf uncompressed_size = FILE_TRANSFER_BUFFER_SIZE + 12;
                int res = uncompress(uncompressed_buffer, &uncompressed_size, buffer, n);
                switch(res)
                {
                case Z_OK:
                    break;
                case Z_MEM_ERROR:
                    LOG_ERROR(LOG_PREFIX_NET_FILE << "uncompress : Z_MEM_ERROR");
                    break;
                case Z_BUF_ERROR:
                    LOG_ERROR(LOG_PREFIX_NET_FILE << "uncompress : Z_BUF_ERROR");
                    break;
                case Z_DATA_ERROR:
                    LOG_ERROR(LOG_PREFIX_NET_FILE << "uncompress : Z_DATA_ERROR");
                    break;
                default:
                    LOG_ERROR(LOG_PREFIX_NET_FILE << "uncompress : unknown error");
                };

                sofar += uncompressed_size;
                network->updateFileTransferInformation( filename + format("%d", sockid), length, sofar );

                fwrite(uncompressed_buffer,1,uncompressed_size,file);       // Write uncompressed data

                int pos = sofar;
                network->sendFileResponse(sockid,port,(byte*)&pos,4);
            }
            if(sofar >= length)
                break;
            rest(0);
        }

        LOG_INFO(LOG_PREFIX_NET_FILE << "Done.");

        network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );

        fclose(file);
        if( pDead && sofar < length )				// Delete the file if transfer has been aborted
            delete_file( (filename + ".part").c_str() );
        else
            rename( (filename + ".part").c_str(), filename.c_str() );

        pDead = 1;
        network->setFileDirty();
        delete[] buffer;
        delete[] uncompressed_buffer;
        return;
    }



    //not finished
    void AdminThread::proc(void* param)
    {
        Network* network;
        network = ((struct net_thread_params*)param)->network;
        delete ((struct net_thread_params*)param);
        while(!pDead)
        {
            network->cleanPlayer();
            network->cleanFileThread();
            if(network->myMode == 1)
            {
                //if you are the game 'server' then this thread
                //handles requests and delegations on the administrative
                //channel
            }
            else
            {
                if (network->myMode == 2)
                {
                    //if you are a mere client then this thread responds to
                    //stuff on the administrative channel such as change of host
                    //and other things
                }
            }
            sleep(1);//testing
        }
        return;
    }


} // namespace TA3D

