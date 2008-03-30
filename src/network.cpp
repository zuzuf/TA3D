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


/******************************/
/**  methods for SockList  ****/
/******************************/


SockList::SockList(){
	maxid = 0;
	list = NULL;
}

SockList::~SockList(){
	Shutdown();
}

void SockList::Shutdown(){
	while(list){
		Remove(list->id);
	}
}

int SockList::Add(TA3DSock* sock){
	if (maxid > 10000) //arbitrary limit
		return -1;
	slnode *node,*ptr;
	node = list;

	while(node)
		node=node->next;

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

int SockList::Remove(int id){
	slnode *node,*prev;

	if(list==NULL)
		return -1;
	if(list->id == id){
		node = list->next;
		delete list;
		list = node;
		return 0;
	}
	
	node=list->next;
	prev=list;
	while(node){
		if(node->id == id){
			prev->next = node->next;
			delete node;
			return 0;
		}
		node=node->next;
		prev=prev->next;
	}
	
	return -1;
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
	

	while(!dead){
		
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
	
	while(!dead){
	
		//sleep until data is coming
		sock->takeFive(1000);
		rest(1);
		if(dead) break;

		//ready for reading, absorb some bytes	
		sock->pumpIn();

		//see if there is a packet ready to process
		packtype = sock->getPacket();

		switch(packtype){
			case 'X'://special
				network->xqmutex.Lock();
					if(dead){
						network->xqmutex.Unlock();
						break;
					}
					if( sock->makeSpecial(&chat) == -1 ) {
						network->xqmutex.Unlock();
						break;
						}
					network->specialq.enqueue(&chat);
				network->xqmutex.Unlock();
				break;
			case 'C'://chat
				sock->makeChat(&chat);
				network->cqmutex.Lock();
					if(dead){
						network->cqmutex.Unlock();
						break;
					}
					network->chatq.enqueue(&chat);
				network->cqmutex.Unlock();
				break;
			case 'O'://order
				sock->makeOrder(&order);
				network->oqmutex.Lock();
					if(dead){
						network->oqmutex.Unlock();
						break;
					}
					network->orderq.enqueue(&order);
				network->oqmutex.Unlock();
				break;
			case 'S'://sync
				sock->makeSync(&sync);
				network->sqmutex.Lock();
					if(dead){
						network->sqmutex.Unlock();
						break;
					}
					network->syncq.enqueue(&sync);
				network->sqmutex.Unlock();
				break;
			case 'E'://event
				sock->makeEvent(&event);
				network->eqmutex.Lock();
					if(dead){
						network->eqmutex.Unlock();
						break;
					}
					network->eventq.enqueue(&event);
				network->eqmutex.Unlock();
		}
		

	}

	return;
}


void MultiCastThread::proc(void* param){
	MulticastSock* sock;
	Network* network;

	network = ((struct net_thread_params*)param)->network;
	sock = &(network->multicast_socket);

	delete ((struct net_thread_params*)param);

	String msg;
	
	while(!dead){
	
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
				network->multicastq.push_back( msg );
				network->multicastaddressq.push_back( sock->getAddress() );
			network->mqmutex.Unlock();
			}
	}

	Console->AddEntry("Multicast thread closed!");

	return;
}

//NEED TESTING
void SendFileThread::proc(void* param){
	TA3DSock* destsock;
	Socket filesock;
	Network* network;
	int sockid;
	FILE* file;
	int length,n,i;
	char buffer[256];
	
	network = ((struct net_thread_params*)param)->network;
	sockid = ((struct net_thread_params*)param)->sockid;
	file = ((struct net_thread_params*)param)->file;

	delete ((struct net_thread_params*)param);

	fseek(file,0,SEEK_END);
	length = ftell(file);
	fseek(file,0,SEEK_SET);

	network->slmutex.Lock();
		destsock = network->players.getSock(sockid);
	network->slmutex.Unlock();
	for(i = 0;i<10;i++){
		//put the standard file port here
		filesock.Open(destsock->getAddress(),"7779",SOCK_STREAM,destsock->getAF());
		if (filesock.isOpen())
			break;
	}
	
	if (!filesock.isOpen()){
		Console->AddEntry("SendFile: error unable to connect to target\n");
		dead = 1;
		return;
	}
	
	while(!dead){
		n = fread(buffer,1,256,file);
		filesock.Send(buffer,n);
		if(feof(file)){
			break;
		}
		sleep(1);
	}

	dead = 1;
	return;
}



//NEEDS TESTING
//doesnt support resume after broken transfer
void GetFileThread::proc(void* param){
	TA3DSock* sourcesock;
	Socket filesock;
	Network* network;
	int sockid;
	FILE* file;
	int length,n,sofar;
	char buffer[256];
	
	network = ((struct net_thread_params*)param)->network;

	//supposed sender
	sockid = ((struct net_thread_params*)param)->sockid;

	//blank file open for writing
	file = ((struct net_thread_params*)param)->file;

	delete ((struct net_thread_params*)param);

	network->slmutex.Lock();
		sourcesock = network->players.getSock(sockid);
	network->slmutex.Unlock();

	//put the standard file port here
	filesock = Socket(NULL,"7778",SOCK_STREAM,sourcesock->getAF());
	
	if(!filesock.isOpen()){
		Console->AddEntry("GetFile: error couldn't open socket\n");
		dead = 1;
		return;
	}

	n=0;
	while(!dead && n<4){
		n += filesock.Recv(buffer+n,4);
	}
	memcpy(&length,buffer,4);
	length = ntohl(length);

	sofar = 0;	
	while(!dead){
		n = filesock.Recv(buffer,256);
		fwrite(buffer,1,n,file);
		sofar += n;
		if(sofar >= length){
			break;
		}
		sleep(1);
	}

	dead = 1;
	return;
}
	


//not finished
void AdminThread::proc(void* param){
	Network* network;
	network = ((struct net_thread_params*)param)->network;
	delete ((struct net_thread_params*)param);
	while(!dead){
		if(network->myMode == 1){
			//if you are the game 'server' then this thread
			//handles requests and delegations on the administrative
			//channel

			struct chat msg;
			if( network->getNextSpecial( &msg ) == 0 ) {
				printf("received '%s'\n", msg.message);
				}
		}
		else if(network->myMode == 2){
			//if you are a mere client then this thread responds to
			//stuff on the administrative channel such as change of host
			//and other things
			struct chat msg;
			network->sendSpecial( strtochat( &msg, "message" ) );
		}
		sleep(1);//testing
	}
	
	return;
}





/******************************/
/**  methods for Network  *****/
/******************************/

Network::Network() : 
multicastq(),
multicastaddressq(),
specialq(64,sizeof(struct chat)) , 
chatq(64,sizeof(struct chat)) , 
orderq(32,sizeof(struct order)) , 
syncq(128,sizeof(struct sync)) , 
eventq(32,sizeof(struct event)) {
	myMode = 0;
}

Network::~Network(){
	listen_thread.Join();
	admin_thread.Join();
	getfile_thread.Join();
	sendfile_thread.Join();
	multicast_thread.Join();

	tohost_socket.Close();//administrative channel
	listen_socket.Close();
	multicast_socket.Close();
	multicastq.clear();
	multicastaddressq.clear();
	players.Shutdown();
}

void Network::InitMulticast( char* target, char* port )
{
	multicast_socket.Open( target, port, 0 );
		//spawn broadcast thread
	net_thread_params *params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning multicast thread\n");
	multicast_thread.Spawn(params);
}



//name is the name that shows up in a game list
//port is the port the game listens on for connections
//proto 4=ipv4only 6=ipv6only 0=automatic
//not finished
int Network::HostGame(char* name,char* port,int network,int proto){

	if(myMode == 0){
		myMode = 1;
	}
	else{
		Console->AddEntry("HostGame: you can't host a game because you are already connected\n");
		return -1;
	}


	int P = num2af(proto);

	myID = 0;
	adminDir[0] = 1;

	//setup game
	strcpy(gamename,name);
	listen_socket.Open(NULL,port,P);
	if(!listen_socket.isOpen()){
		Console->AddEntry("Network: failed to host game on port %s\n",port);
		myMode = 0;
		return -1;
	}

	//advertise
	if (network == 2){
		Socket sock("gamelist.ta3d.net","7778");
		char buff[256];
		if(!sock.isOpen()){
			Console->AddEntry("Network: advertising game failed\n");
			sock.Close();
		}
		else{
			//magic protocol that gamelist server understands
			buff[0] = 'N';
			buff[1] = ';';
			buff[2] = '\0';
			strncat(buff,gamename,128);
			strcat(buff,";");
			strncat(buff,port,32);
			Console->AddEntry("NI_MAXSERV = %d\n",NI_MAXSERV);
			sock.SendString(buff);
			sock.Close();
			Console->AddEntry("Network: game advertised on the internet\n");
		}
	}
	
	//spawn listening thread
	net_thread_params *params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning listen thread\n");
	listen_thread.Spawn(params);
	
	//spawn listening thread
	params = new net_thread_params;
	params->network = this;
	Console->AddEntry("Network: spawning admin thread\n");
	admin_thread.Spawn(params);
	
	Console->AddEntry("Network: network game running\n");

	return 0;

}



//not finished
int Network::Connect(char* target,char* port,int proto){

	if(myMode == 0){
		myMode = 2;
	}
	else{
		Console->AddEntry("Connect: you can't connect to a game, you are already hosting one!\n");
		return -1;
	}

	int P = num2af(proto);	

	tohost_socket.Open(target,port,SOCK_STREAM,P);
	if(!tohost_socket.isOpen()){
		//error couldnt connect to game
		Console->AddEntry("Network: error connecting to game at [%s]:%s\n",target,port);
		myMode = 0;
		return -1;
	}

	//get game info or start admin thread here
	Console->AddEntry("Network: spawning admin thread\n");
	net_thread_params *params = new net_thread_params;
	params->network = this;
	admin_thread.Spawn(params);

	Console->AddEntry("Network: successfully connected to game at [%s]:%s\n",target,port);

	return 0;
}




//not completely finished
void Network::Disconnect(){
	
	listen_thread.Join();
	listen_socket.Close();

	tohost_socket.Close();

	multicast_thread.Join();
	multicast_socket.Close();

	slmutex.Lock();
		players.Shutdown();
	slmutex.Unlock();

	myMode = 0;

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
		Console->AddEntry("thread not found???\n");
		return -1;
	}

	net_thread_params *params = new net_thread_params;
	params->network = this;
	params->sockid = n;
	Console->AddEntry("spawning socket thread\n");
	thread->Spawn(params);

	//send a new player event
	//eventNewPlayer(n);

	return 0;
}

int Network::dropPlayer(int num){
	if(!adminDir[myID]){
		Console->AddEntry("you can't drop players because you aren't the game admin\n");
		return -1;
	}

	int v;

	slmutex.Lock();
		v = players.Remove(num);
	slmutex.Unlock();

	return v;
}




int Network::sendSpecial(struct chat* chat){
	if( !tohost_socket.isOpen() || chat == NULL )	return -1;
	if( myMode == 1 ) {				// Server mode
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock )
				v += sock->sendSpecial( chat );
			}
		return v;
		}
	else if( myMode == 2 ) {			// Client mode
		char tmp[256];
		int len;
		tmp[0] = len = 2 + strlen( chat->message );
		tmp[1] = 'X';
		tmp[2] = chat->from + 1;
		memcpy( tmp + 3, chat->message, 253 );
		return tohost_socket.Send( tmp, len );
		}
	return -1;						// Not connected, it shouldn't be possible to get here if we're not connected ...
}

int Network::sendChat(struct chat* chat){
	if( !tohost_socket.isOpen() || chat == NULL )	return -1;
	if( myMode == 1 ) {				// Server mode
		int v = 0;
		for( int i = 1 ; i <= players.getMaxId() ; i++ )  {
			TA3DSock *sock = players.getSock( i );
			if( sock )
				v += sock->sendSpecial( chat );
			}
		return v;
		}
	else if( myMode == 2 ) {			// Client mode
		char tmp[256];
		int len;
		tmp[0] = len = 2 + strlen( chat->message );
		tmp[1] = 'C';
		tmp[2] = chat->from + 1;
		memcpy( tmp + 3, chat->message, 253 );
		return tohost_socket.Send( tmp, len + 1 );
		}
}

int Network::sendOrder(struct order* order){
	//determine who to send the order to
	//send to all other players?
	//send to 'host'?
	return 0;
}

int Network::sendSync(struct sync* sync){
	return 0;
}

int Network::sendEvent(struct event* event){
	return 0;
}

int Network::sendFile(int player,FILE* file){
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


int Network::num2af(int proto){
	switch(proto){
		case 0:
			return AF_UNSPEC;
		case 4:
			return AF_INET;
		case 6:
			return AF_INET6;
	}
	return AF_UNSPEC;
}

int Network::broadcastMessage( const char *msg )
{
	if( !multicast_socket.isOpen() )
		return -1;

	return multicast_socket.sendMessage( msg );
}

std::string Network::getNextBroadcastedMessage()
{
	std::string msg;
	mqmutex.Lock();
		if( !multicastq.empty() ) {
			msg = multicastq.front();
			multicastq.pop_front();
			if( multicastq.size() + 1 < multicastaddressq.size() )
				multicastaddressq.pop_front();
			}
	mqmutex.Unlock();
	return msg;
}

uint32 Network::getLastMessageAddress()
{
	uint32 address;
	mqmutex.Lock();
		if( !multicastaddressq.empty() )
			address = multicastaddressq.front();
	mqmutex.Unlock();
	return address;
}


bool Network::BroadcastedMessages()
{
	mqmutex.Lock();
		bool result = !multicastq.empty();
	mqmutex.Unlock();
	return result;
}

std::string ip2str( uint32 ip ) {
	std::string address = "";
	for( int i = 0 ; i < 3 ; i++ )
		address += format( "%d.", ((unsigned char*)&ip)[i] );
	address += format( "%d", ((unsigned char*)&ip)[3] );
	return address;
}


