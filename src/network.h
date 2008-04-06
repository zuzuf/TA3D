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


#ifndef __TA3D__NETWORK__H
#define __TA3D__NETWORK__H

#include "ta3dsock.h"
#include "multicastsock.h"
#include "superqueue.h"
#include "thread.h"


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




//accepts new connections
class ListenThread : public Thread{
	virtual void proc(void* param);
};

//receives events, syncs, orders, etc
class SocketThread : public Thread{
	virtual void proc(void* param);
};

//sends a large file in the background
class SendFileThread : public Thread{
	virtual void proc(void* param);
};

//gets a large file in the background and writes to disk
class GetFileThread : public Thread{
	virtual void proc(void* param);
};

class AdminThread : public Thread{
	virtual void proc(void* param);
};

class MultiCastThread : public Thread{
	virtual void proc(void* param);
};


struct SERVER_DATA
{
	String		name;				// Its name
	int			timer;				// Allow it to timeout
	int			nb_open;			// How many player slots are open ?
	String		host;				// The host name of this server
};



//SockList - a C-style linked list of socket/thread pairs
class slnode{
	public:
		slnode() {next=NULL;}
		~slnode() {sock->Close(); thread.Join(); delete sock;}
		int id;
		TA3DSock* sock;
		SocketThread thread;
		slnode* next;
};

class SockList{
	slnode* list;
	int maxid;

	public:
		SockList();
		~SockList();

		int getMaxId() {return maxid;}
		int Add(TA3DSock* sock);
		int Remove(int id);
		void Shutdown();//close all sockets

		TA3DSock* getSock(int id){
			slnode *node;
			for(node=list;node;node=node->next)
				if (node->id == id)
					return node->sock;
			return NULL;
		}
		SocketThread* getThread(int id){
			slnode *node;
			for(node=list;node;node=node->next){
				if (node->id == id){
					return &(node->thread);
				}
			}
			return NULL;
		}
		
};


struct FileTransferProgress
{
	void	*id;			// Pointer to transfer thread
	int		size;
	int		pos;
};

/****
**
** This is the main network interface for TA3D
** it handles all connections provides incoming events
** and allows high-level control over the network.
**
***/
class Network{
	
	friend class ListenThread;
	friend class SocketThread;
	friend class SendFileThread;
	friend class GetFileThread;
	friend class AdminThread;
	friend class MultiCastThread;

	TA3DSock listen_socket;
	ListenThread listen_thread;

	TA3DSock *tohost_socket;//administrative channel
	AdminThread admin_thread;

	List< GetFileThread* > getfile_thread;
	List< SendFileThread* > sendfile_thread;
	List< FileTransferProgress > transfer_progress;

	MulticastSock multicast_socket;	// Used to discover LAN servers
	MultiCastThread multicast_thread;

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

	std::list< std::string > multicastq;
	std::list< uint32 > multicastaddressq;

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
	
	//0=dontcare 4=ipv4 6=ipv6
	int num2af(int proto);
	
	int addPlayer(TA3DSock* sock);
	void updateFileTransferInformation( void *id, int size, int pos );

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

		int HostGame(char* name,char* port,int network,int proto);
		int Connect(char* target,char* port,int proto);
		void Disconnect();

		void setPlayerDirty();
		void setFileDirty();
		bool getPlayerDropped();
		bool pollPlayer(int id);

		void InitMulticast( char* target, char* port);

		//int listNetGames(GamesList& list);
		//int listLanGames(GamesList& list);

		int sendSpecial( std::string msg, int src_id = -1, int dst_id = -1);
		int sendSpecial(struct chat* chat, int src_id = -1, int dst_id = -1);
		int sendChat(struct chat* chat, int src_id = -1);
		int sendOrder(struct order* order, int src_id = -1);
		int sendSync(struct sync* sync, int src_id = -1);
		int sendEvent(struct event* event, int src_id = -1);
		int sendFile(int player, const String &filename);

		int dropPlayer(int num);
		int cleanPlayer();
		void cleanFileThread();

		int getNextSpecial(struct chat* chat);
		int getNextChat(struct chat* chat);
		int getNextOrder(struct order* order);
		int getNextSync(struct sync* sync);
		int getNextEvent(struct event* event);
		int getFile(int player, const String &filename);
		
		bool isConnected();
		int getMyID();
		
		float getFileTransferProgress();
		
		int broadcastMessage( const char *msg );
		std::string getNextBroadcastedMessage();
		uint32 getLastMessageAddress();
		bool BroadcastedMessages();
};


//needed by netthreads
struct net_thread_params{
	Network* network;
	int sockid;
	String filename;
};

std::string ip2str( uint32 ip );

#endif
