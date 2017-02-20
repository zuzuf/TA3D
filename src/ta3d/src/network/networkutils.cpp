/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005  Roland BROCHARD

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include <zlib.h>
#include "network.h"
#include <threads/thread.h>
#include "networkutils.h"
#include "ta3dsock.h"
#include "TA3D_Network.h"
#include <logs/logs.h>
#include <misc/paths.h>
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;



namespace TA3D
{


	//need testing
	void ListenThread::proc(void* param)
	{
		TA3DSock* newsock;
		int v = 0;

		Network* network;
		network = ((struct net_thread_params*)param)->network;
		delete((struct net_thread_params*)param);
		param = NULL;

		struct event event;
		event.type = 45; //arbitrary number for "new player connected" event
		//fill in other info for new player connected event

		while (!pDead && network->listen_socket.isOpen() )
		{
			v = network->listen_socket.accept(&newsock,100);
			if(v < 0)
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



	void SocketThread::proc(void* param)
	{
		TA3DSock* sock;
		Network* network;
		int sockid,packtype;

		struct chat chat;
		struct sync sync;
		struct event event;

		network = ((struct net_thread_params*)param)->network;
		sockid = ((struct net_thread_params*)param)->sockid;
		delete((struct net_thread_params*)param);
		param = NULL;
		sock = network->players.getSock(sockid);

		while(!pDead && sock->isOpen())
		{
			//sleep until data is coming
			sock->check(10);
			if (pDead) break;

			//ready for reading, absorb some bytes
			sock->pumpIn();

			//see if there is a packet ready to process
			packtype = sock->getPacket();

			switch(packtype)
			{
			case 'P':		// ping
				if (sockid != -1)
					network->sendPong(sockid);
				sock->makePing();
				break;
			case 'p':		// pong
				network->processPong(sockid);
				sock->makePong();
				break;
			case 'T':       // Tick synchronization
				sock->makeTick(sockid);
				break;
			case 'A'://special (resend to all!!)
			case 'X'://special
				network->xqmutex.lock();
				if (pDead || sock->makeSpecial(&chat) == -1)
				{
					network->xqmutex.unlock();
					break;
				}
				if (packtype != 'A' && network->isServer())
					chat.from = (uint16)sockid;
				network->specialq.push_back(chat);
				network->xqmutex.unlock();
				if (packtype == 'A' && network->isServer())
					network->sendSpecial( &chat, sockid, -1, true );
				break;
			case 'C'://chat
				network->cqmutex.lock();
				if (pDead || sock->makeChat(&chat) == -1)
				{
					network->cqmutex.unlock();
					break;
				}
				network->chatq.push_back(chat);
				network->cqmutex.unlock();
				if( network->isServer() )
					network->sendChat(&chat, sockid);
				break;
			case 'S'://sync
				network->sqmutex.lock();
				if (pDead || sock->makeSync(&sync) == -1)
				{
					network->sqmutex.unlock();
					break;
				}
				network->syncq.push_back(sync);
				network->sqmutex.unlock();
				if( network->isServer() )
					network->sendSync(&sync, sockid);
				break;
			case 'E'://event
				network->eqmutex.lock();
				if (pDead || sock->makeEvent(&event) == -1)
				{
					network->eqmutex.unlock();
					break;
				}
				network->eventq.push_back(event);
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
						if ((*i)->port == port)
						{
						port = -1;
						while( !(*i)->ready && !(*i)->isDead() )	suspend(1);
						if (!(*i)->isDead())
						{
							(*i)->buffer_size = sock->getFileData( (*i)->buffer );
							(*i)->ready = false;
						}
						break;
					}
					if (port != -1)
						sock->getFileData( NULL );
				}
				break;
			case 'R':						// File response (send back the amount of data that has been received)
				{
					int port = sock->getFilePort();
					for(std::list< SendFileThread* >::iterator i = network->sendfile_thread.begin() ; i != network->sendfile_thread.end() ; i++ )
						if ((*i)->port == port && (*i)->player_id == sockid)
						{
						port = -1;
						sock->getFileData( (byte*)&((*i)->progress) );
						break;
					}
					if (port != -1)
						sock->getFileData( NULL );
				}
				break;

			default:
				sock->cleanPacket();
			}
		}

		pDead = 1;
		if (!sock->isOpen())
			network->setPlayerDirty();

		return;
	}


	void BroadCastThread::proc(void* param)
	{
		SocketBroadCast* sock;
		Network* network;

		network = ((struct net_thread_params*)param)->network;
		sock = &(network->broadcast_socket);

		delete((struct net_thread_params*)param);
		param = NULL;

		QString msg;

		pDead = 0;

		while(!pDead && sock->isOpen() )
		{
			//sleep until data is coming
			sock->check(100);
			if(pDead) break;

			msg = sock->getString();
            if (!msg.isEmpty())
			{
				network->mqmutex.lock();
				if (pDead)
				{
					network->mqmutex.unlock();
					break;
				}
				network->broadcastq.push_back( msg );
				network->broadcastaddressq.push_back( sock->getRemoteIPstr() );
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

		File* file;
		int length,n;
		byte *buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE ];
		QString filename;

		network = ((struct net_thread_params*)param)->network;
		sockid = ((struct net_thread_params*)param)->sockid;
		filename = ((struct net_thread_params*)param)->filename;
		file = VFS::Instance()->readFile( filename );

		delete((struct net_thread_params*)param);
		param = NULL;

		if (NULL == file)
		{
			DELETE_ARRAY(buffer);
			LOG_DEBUG( LOG_PREFIX_NET_FILE << "cannot open file '" << filename << "'" );
			pDead = 1;
			network->setFileDirty();
			return;
		}

		length = file->size();

		uint32 timer = msec_timer;

		int real_length = length;

		int pos = 0;
		progress = 0;

		network->sendFileData(sockid, (uint16)port, (const byte*)&length, 4);

		LOG_INFO(LOG_PREFIX_NET_FILE << "Starting...");
		while (!pDead)
		{
			n = file->read(buffer, FILE_TRANSFER_BUFFER_SIZE);            // Read data into the buffer
			network->sendFileData(sockid, (uint16)port, buffer, n);
			if (n > 0)
			{
				pos += n;
                network->updateFileTransferInformation( filename + QString::number(sockid), real_length, pos );

				const uint32 timer = msec_timer;
				while( progress < pos - 10 * FILE_TRANSFER_BUFFER_SIZE && !pDead && msec_timer - timer < 60000 )
					suspend(0);
				if (msec_timer - timer >= 60000)
				{
					DELETE_ARRAY(buffer);
					LOG_DEBUG( LOG_PREFIX_NET_FILE << "file transfert timed out");
					pDead = 1;
                    network->updateFileTransferInformation(filename + QString::number(sockid), 0, 0);
					network->setFileDirty();
					delete file;
					return;
				}
			}

			if (file->eof())
				break;

			suspend(1);
		}

		timer = msec_timer;
		while (progress < pos - FILE_TRANSFER_BUFFER_SIZE && !pDead && msec_timer - timer < 60000)
			suspend(1);		// Wait for client to say ok

		LOG_INFO(LOG_PREFIX_NET_FILE << "Done.");

        network->updateFileTransferInformation( filename + QString::number(sockid), 0, 0 );
		pDead = 1;
		delete file;
		network->setFileDirty();
		DELETE_ARRAY(buffer);
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
		QString filename;
		int length,n,sofar;

		buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE + 12 ];
		network = ((struct net_thread_params*)param)->network;

		//supposed sender
		sockid = ((struct net_thread_params*)param)->sockid;

		//blank file open for writing
        filename = Paths::Resources + ((struct net_thread_params*)param)->filename;
		const QString path = Paths::ExtractFilePath(filename);
        if (!path.isEmpty())
			Paths::MakeDir(path);
        QFile file( filename + ".part");
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);

		delete((struct net_thread_params*)param);
		param = NULL;

        if (!file.isOpen())
		{
			LOG_DEBUG( LOG_PREFIX_NET_FILE << "cannot open file '" << filename << ".part'");
			pDead = 1;
			network->setFileDirty();
			DELETE_ARRAY(buffer);
			return;
		}

		LOG_INFO(LOG_PREFIX_NET_FILE << "Starting...");

		int timer = msec_timer;

		ready = true;
		while (!pDead && ready && msec_timer - timer < 60000 )
			suspend(0);
		memcpy(&length,buffer,4);

		if (ready) // Time out
		{
			LOG_DEBUG(LOG_PREFIX_NET_FILE << "file transfert timed out (0)");
			pDead = 1;
			file.close();
            file.remove();
			network->setFileDirty();
			DELETE_ARRAY(buffer);
            network->updateFileTransferInformation( filename + QString::number(sockid), 0, 0 );
			return;
		}

		sofar = 0;
		if (pDead) length = 1;			// In order to delete the file
		while (!pDead)
		{
			ready = true;
			timer = msec_timer;
			while( !pDead && ready && msec_timer - timer < 60000 ) suspend( 0 );			// Get paquet data
			n = buffer_size;

			if (ready) // Time out
			{
				LOG_DEBUG(LOG_PREFIX_NET_FILE << "file transfert timed out (1)");
				pDead = 1;
				file.close();
                file.remove();
				network->setFileDirty();
				DELETE_ARRAY(buffer);
                network->updateFileTransferInformation( filename + QString::number(sockid), 0, 0 );
				return;
			}

			if (n > 0)
			{
				// First we must decompress the data

				sofar += buffer_size;
                network->updateFileTransferInformation( filename + QString::number(sockid), length, sofar );

				file.write((const char*)buffer, buffer_size);       // Write data

				const int pos = sofar;
				network->sendFileResponse(sockid, (uint16)port, (const byte*)&pos, 4);
			}
			if(sofar >= length)
				break;
			suspend(0);
		}

		LOG_INFO(LOG_PREFIX_NET_FILE << "Done.");

        network->updateFileTransferInformation( filename + QString::number(sockid), 0, 0 );

		file.close();
		if( pDead && sofar < length )				// Delete the file if transfer has been aborted
            file.remove();
		else
		{
            file.rename(filename);
			VFS::Instance()->reload();
		}

		pDead = 1;
		network->setFileDirty();
		DELETE_ARRAY(buffer);
		return;
	}



	//not finished
	void AdminThread::proc(void* param)
	{
		Network* network;
		network = ((struct net_thread_params*)param)->network;
		delete((struct net_thread_params*)param);
		param = NULL;
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
			suspend(1);//testing
		}
		return;
	}


} // namespace TA3D

