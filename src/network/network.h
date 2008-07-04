/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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


#ifndef __TA3D_NET_NETWORK_H__
# define __TA3D_NET_NETWORK_H__

# include "../stdafx.h"
# include "ta3dsock.h"
# include "udpsock.h"
# include "broadcastsock.h"
# include "../misc/superqueue.h"
# include "../threads/thread.h"
# include "networkutils.h"
# include "socketlist.h"
# include <list>


namespace TA3D
{


    /***
    **
    **  Network - TA3D's network code
    **
    **  outstanding problems
    **  1. the accept connection thread new/deletes too much
    **  2. error messages in Socket do not work on windows (need to 
    **     abstract this with WSAGetLastError() )
    **  3. on windows, cant convert from int* to size_t*
    **  4. probably several minor things on windows, need to build on windows
    **  
    **  things to do
    **  1. platform specific errors in Socket
    **  2. fix the platform #ifdef directives
    **
    **  state of modules
    **  Socket - needs some windows work
    **  TA3DSock - finished, needs testing
    **  SuperQueue - needs to be rewritten to better accomodate overflow
    **  Thread - 100% working/completed (but needs windows testing)
    **  Network - thread procs unfinished, need high level policy for sends/connects
    **
    ** 
    **  note - there is some platform specific code in
    **  thread.h
    **  SocketClass.h
    **
    **  currently using the #ifdef TA3D_PLATFORM_XXX directives
    **
    **
    ***/



    /*! \class Network
    **
    ** \brief This is the main network interface for TA3D
    ** it handles all connections provides incoming events
    ** and allows high-level control over the network.
    */
    class Network
    {
    private:
        friend class ListenThread;
        friend class SocketThread;
        friend class SendFileThread;
        friend class GetFileThread;
        friend class AdminThread;
        friend class BroadCastThread;
        friend class UDPThread;

        TA3DSock listen_socket;
        ListenThread listen_thread;

        UDPSock	udp_socket;
        UDPThread	udp_thread;

        TA3DSock *tohost_socket;//administrative channel
        AdminThread admin_thread;

        std::list<GetFileThread*> getfile_thread;
        std::list<SendFileThread*> sendfile_thread;
        std::list<FileTransferProgress> transfer_progress;

        BroadcastSock broadcast_socket;	// Used to discover LAN servers
        BroadCastThread broadcast_thread;

        char gamename[128];//displays on the internet gamelist
        char creatorName[64];//name of the game owner
        int adminDir[10];//which players are admins
        int myMode;//0 off, 1 'server', 2 client
        int myID;//your id in everyone elses socklist

        //these 5 things need to be syncronized with a mutex
        SockList players;
        SuperQueue specialq;
        SuperQueue chatq;
        SuperQueue orderq;
        SuperQueue syncq;
        SuperQueue eventq;

        std::list< String > broadcastq;
        std::list< String > broadcastaddressq;

        Mutex slmutex;
        Mutex mqmutex;
        Mutex xqmutex;
        Mutex cqmutex;
        Mutex oqmutex;
        Mutex sqmutex;
        Mutex eqmutex;
        Mutex ftmutex;

        bool playerDirty;
        bool fileDirty;
        bool playerDropped;

        int addPlayer(TA3DSock* sock);
        void updateFileTransferInformation( String id, int size, int pos );

        //this stuff places specific events in the event queue
        void eventFileComing(int player);
        void eventFilePercent(int player,int percent);
        void eventFileTransferComplete(int player);
        void eventFileTransferError(int player);
        void eventPlayer(int player,int type);
        void eventTeam(int player1,int player2,int type);
        //...

    public:
        Network();
        ~Network();

        int HostGame(const char* name,const char* port,int network);
        int Connect(const char* target,const char* port);
        void Disconnect();

        void setPlayerDirty();
        void setFileDirty();
        bool getPlayerDropped();
        bool pollPlayer(int id);

        void InitBroadcast( const char* port);

        int listNetGames(std::list<SERVER_DATA> &list);
        int registerToNetServer( const String &name, const int Slots );
        String HttpRequest( const String &servername, const String &request );

        int sendSpecialUDP( String msg, int src_id = -1, int dst_id = -1);
        int sendSpecialUDP(struct chat* chat, int src_id = -1, int dst_id = -1);

        int sendPing( int src_id = -1, int dst_id = -1 );
        int sendAll( String msg );
        int sendSpecial( String msg, int src_id = -1, int dst_id = -1);
        int sendSpecial(struct chat* chat, int src_id = -1, int dst_id = -1, bool all = false);
        int sendChat(struct chat* chat, int src_id = -1);
        int sendOrder(struct order* order, int src_id = -1);
        int sendSync(struct sync* sync, int src_id = -1);
        int sendSyncTCP(struct sync* sync, int src_id = -1);
        int sendEvent(struct event* event, int src_id = -1);
        int sendEventUDP(struct event* event, int dst_id = -1);
        int sendFile(int player, const String &filename, const String &port);

        int dropPlayer(int num);
        int cleanPlayer();
        void cleanFileThread();
        void cleanQueues();

        int getNextSpecial(struct chat* chat);
        int getNextChat(struct chat* chat);
        int getNextOrder(struct order* order);
        int getNextSync(struct sync* sync);
        int getNextEvent(struct event* event);
        String getFile(int player, const String &filename);

        void stopFileTransfer( const String &port = "", int to_id = -1);
        bool isTransferFinished( const String &port );

        bool isConnected();
        bool isServer();
        int getMyID();

        float getFileTransferProgress();

        int broadcastMessage( const char *msg );
        String getNextBroadcastedMessage();
        String getLastMessageAddress();
        bool BroadcastedMessages();

        int sendFileData( int player, uint16 port, byte *data, int size );
        int sendFileResponse( int player, uint16 port, byte *data, int size );

    }; // class Network



    extern Network	network_manager;

    //needed by netthreads
    struct net_thread_params
    {
        Network* network;
        int sockid;
        String filename;
    };


} // namespace TA3D

#endif // __TA3D_NET_NETWORK_H__
