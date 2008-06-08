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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "TA3D_hpi.h"
#include "TA3D_Network.h"

using namespace TA3D::UTILS::HPI;

Network	network_manager;




/******************************/
/**  methods for SockList  ****/
/******************************/


SockList::SockList()
{
	maxid = 0;
	list = NULL;
}

SockList::~SockList()
{
	Shutdown();
}

void
SockList::Shutdown()
{
	while(list)
    {
	    Remove(list->id);
    }
}

int
SockList::Add(TA3DSock* sock)
{
	if (maxid > 10000) //arbitrary limit
		return -1;
	slnode *node,*ptr;

	node = new slnode;
	maxid++;
	node->id = maxid;
	node->sock = sock;

	if(!list)
		list = node;
	else{
		ptr=list;
		while(ptr->next)
			ptr=ptr->next;
		ptr->next = node;
	}

	return node->id;
}

int
SockList::Remove(const int id)
{
	slnode *node,*prev;

	if(list==NULL)
		return -1;
	if(list->id == id)
    {
		node = list->next;
		delete list;
		list = node;
		return 0;
	}
	
	node=list->next;
	prev=list;
	while(node)
    {
		if(node->id == id)
        {
			prev->next = node->next;
			delete node;
			return 0;
		}
		node=node->next;
		prev=prev->next;
	}
	
	return -1;
}

SocketThread*
SockList::getThread(const int id) const
{
	for(slnode* node = list; node; node = node->next)
    {
        if (id == node->id)
			return &(node->thread);
	}
	return NULL;
}

TA3DSock*
SockList::getSock(const int id) const
{
    for(slnode* node = list; node; node = node->next)
    {
		if (node->id == id)
				return node->sock;
    }
	return NULL;
}
	



/******************************************/
/**  methods for Networking Threads  ******/
/******************************************/



//need testing
void ListenThread::proc(void* param){
	TA3DSock* newsock;
	int v = 0;
	
	Network* network;
	network = ((struct net_thread_params*)param)->network;
	delete ((struct net_thread_params*)param);
	
	struct event event;
	event.type = 45; //arbitrary number for "new player connected" event
	//fill in other info for new player connected event
	

	while(!dead && network->listen_socket.isOpen() ){
		
		v = network->listen_socket.Accept(&newsock,100);
		if(v<0){
			if(v==-1){}
		}
		else{
			//perhaps check address and take other actions here

			//add to list and spawn thread
			network->addPlayer(newsock);
		}
	}

	return;
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
	
	while(!dead && sock->isOpen()){
	
		//sleep until data is coming
		sock->takeFive(1000);
		if(dead) break;

		//ready for reading, absorb some bytes	
		sock->pumpIn();

		//see if there is a packet ready to process
		packtype = sock->getPacket();

		switch(packtype){
			case 'P':		// ping
				if( sockid != -1 )
					network->sendSpecial("PONG", -1, sockid);
				sock->makePing();
				break;
			case 'A'://special (resend to all!!)
			case 'X'://special
				network->xqmutex.Lock();
					if( dead || sock->makeSpecial(&chat) == -1 ){
						network->xqmutex.Unlock();
						break;
					}
					if( packtype != 'A' && network->isServer() )
						chat.from = sockid;
					network->specialq.enqueue(&chat);
				network->xqmutex.Unlock();
				if( packtype == 'A' && network->isServer() )
					network->sendSpecial( &chat, sockid, -1, true );
				break;
			case 'C'://chat
				network->cqmutex.Lock();
					if( dead || sock->makeChat(&chat) == -1 ){
						network->cqmutex.Unlock();
						break;
					}
					network->chatq.enqueue(&chat);
				network->cqmutex.Unlock();
				if( network->isServer() )
					network->sendChat(&chat, sockid);
				break;
			case 'O'://order
				network->oqmutex.Lock();
					if( dead || sock->makeOrder(&order) == -1 ){
						network->oqmutex.Unlock();
						break;
					}
					network->orderq.enqueue(&order);
				network->oqmutex.Unlock();
				if( network->isServer() )
					network->sendOrder(&order, sockid);
				break;
			case 'S'://sync
				network->sqmutex.Lock();
					if( dead || sock->makeSync(&sync) == -1 ){
						network->sqmutex.Unlock();
						break;
					}
					network->syncq.enqueue(&sync);
				network->sqmutex.Unlock();
				if( network->isServer() )
					network->sendSync(&sync, sockid);
				break;
			case 'E'://event
				network->eqmutex.Lock();
					if( dead || sock->makeEvent(&event) == -1 ){
						network->eqmutex.Unlock();
						break;
					}
					printf("received event\n");
					network->eventq.enqueue(&event);
				network->eqmutex.Unlock();
				if( network->isServer() )
					network->sendEvent(&event, sockid);
			case 0:
				break;

				// For file transfert
			case 'F':						// File data
				{
					int port = sock->getFilePort();
					for( List< GetFileThread* >::iterator i = network->getfile_thread.begin() ; i != network->getfile_thread.end() ; i++ )
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
					for( List< SendFileThread* >::iterator i = network->sendfile_thread.begin() ; i != network->sendfile_thread.end() ; i++ )
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

	dead = 1;
	if( !sock->isOpen() )
		network->setPlayerDirty();

	return;
}

void UDPThread::proc(void* param){
	Network* network;
	UDPSock *sock;
	int packtype;

	struct chat chat;
	struct order order;
	struct sync sync;
	struct event event;

	network = ((struct net_thread_params*)param)->network;
	sock = &(network->udp_socket);
	delete ((struct net_thread_params*)param);
	
	while(!dead && sock->isOpen()){
	
		//sleep until data is coming
		sock->takeFive(1000);
		if(dead) break;

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
				network->xqmutex.Lock();
					if( dead || sock->makeSpecial(&chat) == -1 ){
						network->xqmutex.Unlock();
						break;
					}
					if( network->isServer() )
						chat.from = player_id;
					network->specialq.enqueue(&chat);
				network->xqmutex.Unlock();
				break;
			case 'S'://sync
				network->sqmutex.Lock();
					if( dead || sock->makeSync(&sync) == -1 ){
						network->sqmutex.Unlock();
						break;
					}
					network->syncq.enqueue(&sync);
				network->sqmutex.Unlock();
				if( network->isServer() )
					network->sendSync(&sync, player_id);
				break;
			case 'E'://UDP event, used to tell someone we've synced a unit, so check destination and resend it if necessary
				{
					network->eqmutex.Lock();
						if( dead || sock->makeEvent(&event) == -1 ){
							network->eqmutex.Unlock();
							break;
						}
						if( event.type != EVENT_UNIT_SYNCED ) {		// We only accept EVENT_UNIT_SYNCED
							network->eqmutex.Unlock();
							break;
							}
						int dest = g_ta3d_network->getNetworkID( event.opt1 );
						if( dest == network_manager.getMyID() ) {
							network->eventq.enqueue(&event);
							dest = -1;
							}
					network->eqmutex.Unlock();
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

	dead = 1;
	if( !sock->isOpen() )
		network->setPlayerDirty();

	return;
}


void BroadCastThread::proc(void* param){
	BroadcastSock* sock;
	Network* network;

	network = ((struct net_thread_params*)param)->network;
	sock = &(network->broadcast_socket);

	delete ((struct net_thread_params*)param);

	String msg;
	
	dead = 0;
	
	while(!dead && sock->isOpen() ){
	
		//sleep until data is coming
		sock->takeFive(1000);
		if(dead) break;

		//ready for reading, absorb some bytes	
		sock->pumpIn();

		msg = sock->makeMessage();
		if( !msg.empty() ) {
			network->mqmutex.Lock();
				if(dead){
					network->mqmutex.Unlock();
					break;
				}
				network->broadcastq.push_back( msg );
				network->broadcastaddressq.push_back( sock->getAddress() );
			network->mqmutex.Unlock();
			}
	}

	dead = 1;
	Console->AddEntry("Broadcast thread closed!");

	return;
}

#define FILE_TRANSFER_BUFFER_SIZE		2048

//NEED TESTING
void SendFileThread::proc(void* param){
	TA3DSock* destsock;
	Network* network;
	int sockid;
	TA3D_FILE* file;
	int length,n,i;
	byte buffer[ FILE_TRANSFER_BUFFER_SIZE ];
	String filename;
	
	network = ((struct net_thread_params*)param)->network;
	sockid = ((struct net_thread_params*)param)->sockid;
	filename = ((struct net_thread_params*)param)->filename;
	file = ta3d_fopen( filename );

	delete ((struct net_thread_params*)param);

	if( file == NULL ) {
		dead = 1;
		network->setFileDirty();
		return;
		}

	length = ta3d_fsize(file);

	int timer = msec_timer;

	int real_length = length;

	int pos = 0;
	progress = 0;
	
	network->sendFileData(sockid,port,(byte*)&length,4);
	
	Console->AddEntry("starting file transfer...");
	while(!dead){
		n = ta3d_fread(buffer,1,FILE_TRANSFER_BUFFER_SIZE,file);

		network->sendFileData(sockid,port,buffer,n);

		if( n > 0 ) {
			pos += n;
			network->updateFileTransferInformation( filename + format("%d", sockid), real_length, pos );

			int timer = msec_timer;
			while( progress < pos - 10 * FILE_TRANSFER_BUFFER_SIZE && !dead && msec_timer - timer < 5000 )	rest(0);
			if( msec_timer - timer >= 5000 ) {
				dead = 1;
				network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
				network->setFileDirty();
				ta3d_fclose( file );
				return;
				}
			}
		
		if(ta3d_feof(file))		break;

		rest(1);
	}
	
	timer = msec_timer;
	while( progress < pos - FILE_TRANSFER_BUFFER_SIZE && !dead && msec_timer - timer < 5000 )	rest(1);		// Wait for client to say ok
	
	Console->AddEntry("file transfer finished...");

	network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
	dead = 1;
	ta3d_fclose( file );
	network->setFileDirty();
	return;
}



//NEEDS TESTING
//doesnt support resume after broken transfer
void GetFileThread::proc(void* param){
	TA3DSock* sourcesock;
	Network* network;
	int sockid;
	FILE* file;
	String filename;
	int length,n,sofar;
	buffer = new byte[ FILE_TRANSFER_BUFFER_SIZE ];
	
	network = ((struct net_thread_params*)param)->network;

	//supposed sender
	sockid = ((struct net_thread_params*)param)->sockid;

	//blank file open for writing
	filename = ((struct net_thread_params*)param)->filename;
	file = TA3D_OpenFile( filename + ".part", "wb" );
	
	delete ((struct net_thread_params*)param);

	if( file == NULL ) {
		dead = 1;
		network->setFileDirty();
		delete[] buffer;
		return;
		}

	Console->AddEntry("starting file transfer...");

	int timer = msec_timer;

	ready = true;
	while( !dead && ready && msec_timer - timer < 5000 ) rest( 0 );
	memcpy(&length,buffer,4);
	
	if( ready ) {				// Time out
		dead = 1;
		fclose( file );
		delete_file( (filename + ".part").c_str() );
		network->setFileDirty();
		delete[] buffer;
		network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
		return;
		}
	
	sofar = 0;
	if( dead ) length = 1;			// In order to delete the file
	while(!dead){
		ready = true;
		timer = msec_timer;
		while( !dead && ready && msec_timer - timer < 5000 ) rest( 0 );			// Get paquet data
		n = buffer_size;

		if( ready ) {				// Time out
			dead = 1;
			fclose( file );
			delete_file( (filename + ".part").c_str() );
			network->setFileDirty();
			delete[] buffer;
			network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );
			return;
			}

		if( n > 0 ) {
			sofar += n;
			network->updateFileTransferInformation( filename + format("%d", sockid), length, sofar );

			fwrite(buffer,1,n,file);

			int pos = sofar;
			network->sendFileResponse(sockid,port,(byte*)&pos,4);
			}
		if(sofar >= length)
			break;
		rest(0);
	}

	Console->AddEntry("file transfer finished...");

	network->updateFileTransferInformation( filename + format("%d", sockid), 0, 0 );

	fclose( file );
	if( dead && sofar < length )				// Delete the file if transfer has been aborted
		delete_file( (filename + ".part").c_str() );
	else
		rename( (filename + ".part").c_str(), filename.c_str() );

	dead = 1;
	network->setFileDirty();
	delete[] buffer;
	return;
}
	


//not finished
void AdminThread::proc(void* param){
	Network* network;
	network = ((struct net_thread_params*)param)->network;
	delete ((struct net_thread_params*)param);
	while(!dead){
		network->cleanPlayer();
		network->cleanFileThread();
		if(network->myMode == 1){
			//if you are the game 'server' then this thread
			//handles requests and delegations on the administrative
			//channel
		}
		else if(network->myMode == 2){
			//if you are a mere client then this thread responds to
			//stuff on the administrative channel such as change of host
			//and other things
		}
		sleep(1);//testing
	}
	
	return;
}





/******************************/
/**  methods for Network  *****/
/******************************/

Network::Network() : 
udp_socket(),
udp_thread(),
transfer_progress(),
getfile_thread(),
sendfile_thread(),
broadcastq(),
broadcastaddressq(),
specialq(64,sizeof(struct chat)) , 
chatq(64,sizeof(struct chat)) , 
orderq(32,sizeof(struct order)) , 
syncq(128,sizeof(struct sync)) , 
eventq(32,sizeof(struct event)) {
	myMode = 0;
	tohost_socket = NULL;
	playerDirty = false;
	fileDirty = false;
	
	nlInit();						// Start NL

	nlEnable( NL_SOCKET_STATS );	// Activates statistics

	nlEnable( NL_LITTLE_ENDIAN_DATA );	// Little endian because most copies of TA3D will probably run on little endian hardware
										// so don't waste CPU cycles doing useless conversions
	
	nlSelectNetwork( NL_IP );		// We want IP networking
}

Network::~Network(){
	listen_thread.Join();
	admin_thread.Join();
	broadcast_thread.Join();
	udp_thread.Join();

	for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; i++ ) {
		(*i)->Join();
		delete *i;
		}
	for( List< SendFileThread* >::iterator i = sendfile_thread.begin() ; i != sendfile_thread.end() ; i++ ) {
		(*i)->Join();
		delete *i;
		}
	getfile_thread.clear();
	sendfile_thread.clear();
	transfer_progress.clear();

	listen_socket.Close();
	broadcast_socket.Close();
	broadcastq.clear();
	broadcastaddressq.clear();
	udp_socket.Close();
	
	players.Shutdown();
	
	nlShutdown();
}

void Network::InitBroadcast( const char* port )
{
	broadcast_socket.Open( port );
		//spawn broadcast thread
	net_thread_params *params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning broadcast thread");
	broadcast_thread.Spawn(params);
}



//name is the name that shows up in a game list
//port is the port the game listens on for connections
//proto 4=ipv4only 6=ipv6only 0=automatic
//not finished
int Network::HostGame(const char* name,const char* port,int network){

	if(myMode == 0){
		myMode = 1;
	}
	else{
		Console->AddEntry("HostGame: you can't host a game because you are already connected\n");
		return -1;
	}


	myID = 0;
	adminDir[0] = 1;

	//setup game
	strcpy(gamename,name);
	listen_socket.Open(NULL,port);
	udp_socket.Open(NULL,port);
	if(!listen_socket.isOpen()){
		Console->AddEntry("Network: failed to host game on port %s",port);
		myMode = 0;
		return -1;
	}
	
	//spawn listening thread
	net_thread_params *params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning listen thread");
	listen_thread.Spawn(params);
	
	//spawn udp thread
	params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning udp thread");
	udp_thread.Spawn(params);

	//spawn admin thread
	params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning admin thread");
	admin_thread.Spawn(params);
	
	Console->AddEntry("Network: network game running");

	return 0;

}



//not finished
int Network::Connect(const char* target,const char* port){

	if(myMode == 0){
		myMode = 2;
	}
	else{
		Console->AddEntry("Connect: you can't connect to a game, you are already hosting one!");
		return -1;
	}

	tohost_socket = new TA3DSock();
	myID = -1;

	tohost_socket->Open(target,port);
	udp_socket.Open(NULL,port);
	if(!tohost_socket->isOpen()){
		//error couldnt connect to game
		Console->AddEntry("Network: error connecting to game at [%s]:%s",target,port);
		delete tohost_socket;
		tohost_socket = NULL;
		myMode = 0;
		return -1;
	}

	addPlayer( tohost_socket );

	//get game info or start admin thread here
	Console->AddEntry("Network: spawning admin thread");
	net_thread_params *params = new net_thread_params;
	params->network = this;
	admin_thread.Spawn(params);

	//get game info or start admin thread here
	Console->AddEntry("Network: spawning udp thread");
	params = new net_thread_params;
	params->network = this;
	udp_thread.Spawn(params);

	Console->AddEntry("Network: successfully connected to game at [%s]:%s",target,port);

	getMyID();

	return 0;
}




//not completely finished
void Network::Disconnect(){
	
	listen_thread.Join();
	listen_socket.Close();

	udp_thread.Join();
	udp_socket.Close();

	tohost_socket = NULL;

	broadcast_thread.Join();
	broadcast_socket.Close();

	ftmutex.Lock();

	for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; i++ ) {
		(*i)->Join();
		delete *i;
		}
	for( List< SendFileThread* >::iterator i = sendfile_thread.begin() ; i != sendfile_thread.end() ; i++ ) {
		(*i)->Join();
		delete *i;
		}
	getfile_thread.clear();
	sendfile_thread.clear();
	transfer_progress.clear();

	ftmutex.Unlock();

	slmutex.Lock();
		players.Shutdown();
	slmutex.Unlock();

	cleanQueues();

	Console->AddEntry("");
	Console->AddEntry("network statistics :" );
	Console->AddEntry("average bytes/sec. received : %d bytes/sec.", nlGetInteger( NL_AVE_BYTES_RECEIVED ) );
	Console->AddEntry("maximum bytes/sec. received : %d bytes/sec.", nlGetInteger( NL_HIGH_BYTES_RECEIVED ) );
	Console->AddEntry("total bytes received : %d bytes", nlGetInteger( NL_BYTES_RECEIVED ) );
	Console->AddEntry("average bytes/sec. sent : %d bytes/sec.", nlGetInteger( NL_AVE_BYTES_SENT ) );
	Console->AddEntry("maximum bytes/sec. sent : %d bytes/sec.", nlGetInteger( NL_HIGH_BYTES_SENT ) );
	Console->AddEntry("total bytes sent : %d bytes", nlGetInteger( NL_BYTES_SENT ) );

	nlClear( NL_ALL_STATS );

	myMode = 0;

}

void Network::stopFileTransfer( const String &port, int to_id )
{
	ftmutex.Lock();

	if( port.empty() ) {
		for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; i++ ) {
			(*i)->Join();
			delete *i;
			}
		for( List< SendFileThread* >::iterator i = sendfile_thread.begin() ; i != sendfile_thread.end() ; i++ ) {
			(*i)->Join();
			delete *i;
			}
		getfile_thread.clear();
		sendfile_thread.clear();
		transfer_progress.clear();
		}
	else {
		int nb_port = atoi( port.c_str() );
		for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; )
			if( (*i)->port == nb_port ) {
				GetFileThread *p = *i;
				getfile_thread.erase( i++ );

				ftmutex.Unlock();
				p->Join();
				delete p;
				ftmutex.Lock();

				break;
				}
			else
				i++;
		for( List< SendFileThread* >::iterator i = sendfile_thread.begin() ; i != sendfile_thread.end() ; )
			if( (*i)->port == nb_port && (to_id == -1 || to_id == (*i)->player_id ) ) {
				SendFileThread *p = *i;
				sendfile_thread.erase( i++ );

				ftmutex.Unlock();
				p->Join();
				delete p;
				ftmutex.Lock();
				
				break;
				}
			else
				i++;
		}

	setFileDirty();

	ftmutex.Unlock();
}

bool Network::isTransferFinished( const String &port )
{
	int nb_port = atoi( port.c_str() );
	for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; i++ )
		if( (*i)->port == nb_port )
			return false;
	for( List< SendFileThread* >::iterator i = sendfile_thread.begin() ; i != sendfile_thread.end() ; i++ )
		if( (*i)->port == nb_port )
			return false;
	return true;
}


//not completely finished
int Network::addPlayer(TA3DSock* sock){
	int n;
	SocketThread* thread;

	slmutex.Lock();
		n = players.Add(sock);
	slmutex.Unlock();

	thread = players.getThread(n);

	if(thread==NULL){
		Console->AddEntry("thread not found???");
		return -1;
	}

	net_thread_params *params = new net_thread_params;
	params->network = this;
	params->sockid = n;
	Console->AddEntry("spawning socket thread");
	thread->Spawn(params);

	//send a new player event
	//eventNewPlayer(n);

	return 0;
}

int Network::dropPlayer(int num){
//	if(!adminDir[myID]){
//		Console->AddEntry("you can't drop players because you aren't the game admin\n");
//		return -1;
//	}

	int v;

	slmutex.Lock();
		v = players.Remove(num);
		playerDropped = true;
	slmutex.Unlock();

	return v;
}

int Network::cleanPlayer()
{
	if( !playerDirty )	return 0;
	slmutex.Lock();
	int v = 0;
	for( int i = 1 ; i <= players.getMaxId() ; i++ ) {
		TA3DSock *sock = players.getSock( i );
		if( sock && !sock->isOpen() ) {
			v = players.Remove( i );
			if( sock == tohost_socket ) {
				broadcast_thread.Join();
				broadcast_socket.Close();

				tohost_socket = NULL;
				myMode = 0;
				}
			}
		}
	slmutex.Unlock();
	playerDirty = false;
	return v;
}

void Network::setPlayerDirty()
{
	playerDirty = true;
}

void Network::setFileDirty()
{
	fileDirty = true;
}

void Network::cleanFileThread()
{
	if( !fileDirty )	return;
	ftmutex.Lock();
	for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; ) {
		if( (*i)->isDead() ) {
			for( List< FileTransferProgress >::iterator e = transfer_progress.begin() ; e != transfer_progress.end() ; )
				if( e->size == 0 )
					transfer_progress.erase( e++ );
				else
					e++;

			(*i)->Join();
			delete *i;
			getfile_thread.erase( i++ );
			}
		else
			i++;
		}
	for( List< SendFileThread* >::iterator i = sendfile_thread.begin() ; i != sendfile_thread.end() ; ) {
		if( (*i)->isDead() ) {
			for( List< FileTransferProgress >::iterator e = transfer_progress.begin() ; e != transfer_progress.end() ; )
				if( e->size == 0 )
					transfer_progress.erase( e++ );
				else
					e++;

			(*i)->Join();
			delete *i;
			sendfile_thread.erase( i++ );
			}
		else
			i++;
		}
	ftmutex.Unlock();
	fileDirty = false;
}

int Network::getMyID()
{
	if( myID != -1 )	return myID;
	switch( myMode )
	{
	case 1:						// Server
		myID = 0;
		return myID;
	case 2:						// Client
		struct chat special_msg;
		if( sendSpecial( strtochat( &special_msg, "REQUEST PLAYER_ID" ) ) )
			return -1;
		else {
			int timeout = 5000;
			myID = -1;
			while( myID == -1 && timeout-- && myMode == 2 && tohost_socket && tohost_socket->isOpen() ) {
				rest(1);
				if( getNextSpecial( &special_msg ) == 0 ) {
					Vector< String > params = ReadVectorString( special_msg.message, " " );
					if( params.size() == 3 && params[0] == "RESPONSE" && params[1] == "PLAYER_ID" ) {
						myID = atoi( params[2].c_str() );
						break;
						}
					}
				if( (timeout % 1000) == 0 )				// Resend
					sendSpecial( strtochat( &special_msg, "REQUEST PLAYER_ID" ) );
				}
			if( timeout == 0 )				// Timeout reached
				return -1;
			return myID;
			}
		break;
	};
	return -1;					// Not connected
}

int Network::sendSpecialUDP( String msg, int src_id, int dst_id)
{
	struct chat chat;
	return sendSpecialUDP( strtochat( &chat, msg ), src_id, dst_id );
}

int Network::sendSpecialUDP(struct chat* chat, int src_id, int dst_id){
	chat->from = myID;
	if( myMode == 1 ) {				// Server mode
		if( chat == NULL )	return -1;
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock && i != src_id && ( dst_id == -1 || i == dst_id ) )
				v += udp_socket.sendSpecial( chat, sock->getAddress() );
			}
		return v;
		}
	else if( myMode == 2 && src_id == -1 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || chat == NULL )	return -1;
		return udp_socket.sendSpecial( chat, tohost_socket->getAddress() );
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendAll( std::string msg )
{
	struct chat chat;
	return sendSpecial( strtochat( &chat, msg ), -1, -1, true );
}

int Network::sendSpecial( String msg, int src_id, int dst_id)
{
	struct chat chat;
	return sendSpecial( strtochat( &chat, msg ), src_id, dst_id );
}

int Network::sendSpecial(struct chat* chat, int src_id, int dst_id, bool all){
	if( src_id == -1 )
		chat->from = myID;
	if( myMode == 1 ) {				// Server mode
		if( chat == NULL )	return -1;
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock && i != src_id && ( dst_id == -1 || i == dst_id ) )
				v += sock->sendSpecial( chat, all );
			}
		return v;
		}
	else if( myMode == 2 && src_id == -1 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || chat == NULL )	return -1;
		return tohost_socket->sendSpecial( chat, all );
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendPing( int src_id, int dst_id )
{
	if( myMode == 1 ) {				// Server mode
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock && i != src_id && ( dst_id == -1 || i == dst_id ) )
				v += sock->sendPing();
			}
		return v;
		}
	else if( myMode == 2 && src_id == -1 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() )	return -1;
		return tohost_socket->sendPing();
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendChat(struct chat* chat, int src_id){
	if( src_id == -1 )
		chat->from = myID;
	if( myMode == 1 ) {				// Server mode
		if( chat == NULL )	return -1;
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock && i != src_id )
				v += sock->sendChat( chat );
			}
		return v;
		}
	else if( myMode == 2 && src_id == -1 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || chat == NULL )	return -1;
		return tohost_socket->sendChat( chat );
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendFileData( int player, uint16 port, byte *data, int size )
{
	TA3DSock *sock = players.getSock( player );
	if( sock ) {
		size += 3;
		byte buffer[size];
		buffer[0] = 'F';
		memcpy( buffer+1, &port, sizeof( port ) );
		memcpy( buffer+3, data, size - 3 );
		sock->sendTCP( buffer, size );
		return 0;
		}
	return -1;
}

int Network::sendFileResponse( int player, uint16 port, byte *data, int size )
{
	TA3DSock *sock = players.getSock( player );
	if( sock ) {
		size += 3;
		byte buffer[size];
		buffer[0] = 'R';
		memcpy( buffer+1, &port, sizeof( port ) );
		memcpy( buffer+3, data, size - 3 );
		sock->sendTCP( buffer, size );
		return 0;
		}
	return -1;
}

int Network::sendOrder(struct order* order, int src_id){
	//determine who to send the order to
	//send to all other players?
	//send to 'host'?
	return 0;
}

int Network::sendSync(struct sync* sync, int src_id){
	if( myMode == 1 ) {				// Server mode
		if( sync == NULL )	return -1;
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock && i != src_id )
				v += udp_socket.sendSync( sync, sock->getAddress() );
			}
		return v;
		}
	else if( myMode == 2 && src_id == -1 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || sync == NULL )	return -1;
		return udp_socket.sendSync( sync, tohost_socket->getAddress() );
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendSyncTCP(struct sync* sync, int src_id){
	if( myMode == 1 ) {				// Server mode
		if( sync == NULL )	return -1;
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock && i != src_id )
				v += sock->sendSync( sync );
			}
		return v;
		}
	else if( myMode == 2 && src_id == -1 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || sync == NULL )	return -1;
		return tohost_socket->sendSync( sync );
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendEvent(struct event* event, int src_id){
	if( myMode == 1 ) {				// Server mode
		if( event == NULL )	return -1;
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			if( i == src_id )	continue;
			TA3DSock *sock = players.getSock( i );
			if( sock )
				v = sock->sendEvent( event );
			}
		return v;
		}
	else if( myMode == 2 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || event == NULL )	return -1;
		return tohost_socket->sendEvent( event );
		}
	return -1;
}

int Network::sendEventUDP(struct event* event, int dst_id){
	if( myMode == 1 ) {				// Server mode
		if( event == NULL )	return -1;
		int v = 0;
		TA3DSock *sock = players.getSock( dst_id );
		if( sock )
			v += udp_socket.sendEvent( event, sock->getAddress() );
		return v;
		}
	else if( myMode == 2 ) {			// Client mode
		if( tohost_socket == NULL || !tohost_socket->isOpen() || event == NULL )	return -1;
		return udp_socket.sendEvent( event, tohost_socket->getAddress() );
		}
	return -1;
}

int Network::sendFile(int player, const String &filename, const String &port){
	ftmutex.Lock();
	SendFileThread *thread = new SendFileThread();
	sendfile_thread.push_back( thread );
	thread->port = atoi( port.c_str() );
	thread->player_id = player;

	net_thread_params *params = new net_thread_params;
	params->network = this;
	params->sockid = player;
	params->filename = filename;
	Console->AddEntry("spawning sendFile thread");
	thread->Spawn(params);

	ftmutex.Unlock();
	return 0;
}


int Network::getNextSpecial(struct chat* chat){
	int v;
	xqmutex.Lock();
		v = specialq.dequeue(chat);
	xqmutex.Unlock();
	return v;
}

int Network::getNextChat(struct chat* chat){
	int v;
	cqmutex.Lock();
		v = chatq.dequeue(chat);
	cqmutex.Unlock();
	return v;
}

int Network::getNextOrder(struct order* order){
	int v;
	oqmutex.Lock();
		v = orderq.dequeue(order);
	oqmutex.Unlock();
	return v;
}

int Network::getNextSync(struct sync* sync){
	int v;
	sqmutex.Lock();
		v = syncq.dequeue(sync);
	sqmutex.Unlock();
	return v;
}

int Network::getNextEvent(struct event* event){
	int v;
	eqmutex.Lock();
		v = eventq.dequeue(event);
	eqmutex.Unlock();
	return v;
}

String Network::getFile(int player, const String &filename){
	ftmutex.Lock();

	int port = 7776;						// Take the next port not in use
	for( List< GetFileThread* >::iterator i = getfile_thread.begin() ; i != getfile_thread.end() ; i++ )
		port = max( (*i)->port, port ) ;
	port++;
	
	GetFileThread *thread = new GetFileThread();
	thread->port = port;
	getfile_thread.push_back( thread );

	net_thread_params *params = new net_thread_params;
	params->network = this;
	params->sockid = player;
	params->filename = filename;
	Console->AddEntry("spawning getFile thread");
	thread->Spawn(params);

	ftmutex.Unlock();
	return format( "%d", port );
}


int Network::broadcastMessage( const char *msg )
{
	if( !broadcast_socket.isOpen() )
		return -1;

	return broadcast_socket.sendMessage( msg );
}

std::string Network::getNextBroadcastedMessage()
{
	std::string msg;
	mqmutex.Lock();
		if( !broadcastq.empty() ) {
			msg = broadcastq.front();
			broadcastq.pop_front();
			if( broadcastq.size() + 1 < broadcastaddressq.size() )
				broadcastaddressq.pop_front();
			}
	mqmutex.Unlock();
	return msg;
}

String Network::getLastMessageAddress()
{
	String address;
	mqmutex.Lock();
		if( !broadcastaddressq.empty() )
			address = broadcastaddressq.front();
	mqmutex.Unlock();
	return address;
}

void Network::cleanQueues()
{
	struct chat		chat;
	struct order	order;
	struct sync		sync;
	struct event	event;

	while( getNextSpecial(&chat) == 0 )	{}
	while( getNextChat(&chat) == 0 )	{}
	while( getNextOrder(&order) == 0 )	{}
	while( getNextSync(&sync) == 0 )	{}
	while( getNextEvent(&event) == 0 )	{}

	mqmutex.Lock();
		broadcastq.clear();
		broadcastaddressq.clear();
	mqmutex.Unlock();
}


bool Network::BroadcastedMessages()
{
	mqmutex.Lock();
		bool result = !broadcastq.empty();
	mqmutex.Unlock();
	return result;
}

bool Network::isConnected()
{
	return myMode == 1 || ( myMode == 2 && tohost_socket != NULL && tohost_socket->isOpen() );
}

bool Network::isServer()
{
	return myMode == 1;
}

bool Network::getPlayerDropped()
{
	slmutex.Lock();

	bool result = playerDropped;
	playerDropped = false;

	slmutex.Unlock();

	return result;
}

bool Network::pollPlayer(int id)
{
	slmutex.Lock();
	
	bool result = (players.getSock( id ) != NULL);

	slmutex.Unlock();
	
	return result;
}

float Network::getFileTransferProgress()
{
	ftmutex.Lock();
		if( transfer_progress.empty() ) {
			ftmutex.Unlock();
			return 100.0f;
			}

		int pos = 0;
		int size = 0;
		for( List< FileTransferProgress >::iterator i = transfer_progress.begin() ; i != transfer_progress.end() ; i++ ) {
			pos += i->pos;
			size += i->size;
			}
	
	ftmutex.Unlock();
	return size ? 100.0f * pos / size : 100.0f;
}

void Network::updateFileTransferInformation( String id, int size, int pos )
{
	ftmutex.Lock();
	for( List< FileTransferProgress >::iterator i = transfer_progress.begin() ; i != transfer_progress.end() ; i++ )
		if( i->id == id ) {
			i->size = size;
			i->pos = pos;
			ftmutex.Unlock();
			return;
			}
	
	FileTransferProgress info;
	info.id = id;
	info.size = size;
	info.pos = pos;
	transfer_progress.push_back( info );

	ftmutex.Unlock();
}

String Network::HttpRequest( const String &servername, const String &request )
{
    NLsocket    sock;
    NLaddress   addr;
    NLbyte      buffer[4096];
    String      f;
    NLint       count;
    NLint       crfound = 0;
    NLint       lffound = 0;

    nlGetAddrFromName( servername.c_str(), &addr);

    /* use the standard HTTP port */
    nlSetAddrPort(&addr, 80);

    /* open the socket and connect to the server */
    sock = nlOpen(0, NL_RELIABLE);
    if(sock == NL_INVALID) {
    	Console->AddEntry("Network::HttpRequest : error : could not open socket!");
    	return "";
        }
    if(nlConnect(sock, &addr) == NL_FALSE)
    {
    	nlClose( sock );
    	Console->AddEntry("Network::HttpRequest : error : could not connect to server!");
    	return "";
    }

    f.clear();

    sprintf(buffer, "GET %s HTTP/1.0\r\nHost:%s\nAccept: */*\r\nUser-Agent: TA3D\r\n\r\n"
                    , request.c_str(), servername.c_str() );
    while(nlWrite(sock, (NLvoid *)buffer, (NLint)strlen(buffer)) < 0)
    {
        if(nlGetError() == NL_CON_PENDING)
        {
            nlThreadYield();
            continue;
        }
    	Console->AddEntry("Network::HttpRequest : error : could not send request to server!");
    	nlClose( sock );
    	return "";
    }

	while(true) {
		count = nlRead(sock, (NLvoid *)buffer, (NLint)sizeof(buffer) - 1);
		if(count < 0) {
			NLint err = nlGetError();

			/* is the connection closed? */
			if(err == NL_MESSAGE_END)
				break;
			else {
		    	nlClose( sock );
				return "";
				}
			}
		if(count > 0) {
			/* parse out the HTTP header */
			if(lffound < 2) {
				int i;

				for( i = 0 ; i < count ; i++ ) {
					if(buffer[i] == 0x0D)
						crfound++;
					else {
						if(buffer[i] == 0x0A)
							lffound++;
						else
							/* reset the CR and LF counters back to 0 */
							crfound = lffound = 0;
						}
					if(lffound == 2) {
						/* i points to the second LF */
						/* NUL terminate the string and put it in the buffer string */
						buffer[count] = 0x0;
						f += buffer+i+1;
						break;
						}
					}
				}
			else {
				buffer[ count ] = 0x0;
				f += buffer;
				}
			}
		}
   	nlClose( sock );
    return f;
}

int Network::listNetGames(List< SERVER_DATA > &list)
{
	String gamelist = HttpRequest( lp_CONFIG->net_server, "/getserverlist.php" );
	
	foreach_( list, i )								// Remove internet servers to get a clean list
		if( i->internet )	list.erase( i++ );
		else				i++;

	if( gamelist.empty() )
		return 0;

	Vector< String > line = ReadVectorString( gamelist, "\n" );

	int nb_servers = 0;
	int old = -1;
	SERVER_DATA cur_server;
	cur_server.internet = true;
	String server_version = "";
	String server_mod = "";
	foreach( line, entry )
    {
		Vector< String >	params = ReadVectorString( *entry, " " );
		if( params.size() < 2 )	continue;
		if( params.size() == 2 && params[1] == "servers" )
        {
			nb_servers = atoi( params[0].c_str() );
			continue;
		}
		int cur = atoi( params[0].c_str() );
		if( cur != old ) 						// We've all we need for this one
        {
			if( server_version != TA3D_ENGINE_VERSION || server_mod != TA3D_CURRENT_MOD )		// Not compatible!!
				nb_servers--;
			else
				list.push_back( cur_server );
		}

		if( params[1] == "name:" )
        {
			cur_server.name = "";
			for( int i = 2 ; i < params.size() ; i++ )	cur_server.name += i > 2 ? " " + params[i] : params[i];
		}
		else if( params[1] == "IP:" )		cur_server.host = params.size() >= 3 ? params[2] : "";
		else if( params[1] == "slots:" )	cur_server.nb_open = params.size() >= 3 ? atoi( params[2].c_str() ) : 0;
		else if( params[1] == "mod:" ) {
			server_mod = "";
			for( int i = 2 ; i < params.size() ; i++ ) server_mod += i > 2 ? " " + params[i] : params[i];
			}
		else if( params[1] == "version:" ) {
			server_version = "";
			for( int i = 2 ; i < params.size() ; i++ ) server_version += i > 2 ? " " + params[i] : params[i];
			}

		old = cur;
		}
	if( old != -1 ) {
		if( server_version != TA3D_ENGINE_VERSION || server_mod != TA3D_CURRENT_MOD )		// Not compatible!!
			nb_servers--;
		else
			list.push_back( cur_server );
		}
	return nb_servers;
}

int Network::registerToNetServer( const String &name, const int Slots )
{
	String request = format("/register.php?name=%s&mod=%s&version=%s&slots=%d", ReplaceString( name, " ", "%20", false ).c_str(), ReplaceString( TA3D_CURRENT_MOD, " ", "%20", false ).c_str(), ReplaceString( TA3D_ENGINE_VERSION, " ", "%20", false ).c_str(), Slots );
	String result = HttpRequest( lp_CONFIG->net_server, request );
}


