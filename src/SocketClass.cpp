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
#include "SocketClass.h"


Socket::Socket(){
	sockreports = 1;
	sockerrors = 1;
	//Console->AddEntry("default constructor\n");
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
}


Socket::~Socket(){
	if (stype != STYPE_BROKEN)
		close(fd);
}


Socket::Socket(char *hostname, char *port, int transport, int network){
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	sockreports = 1;
	sockerrors = 1;
	int x;
	x = Open(hostname,port,transport,network);
	if (x<0){
		//Console->AddEntry("Socket initialization for %s port %s failed!\n",hostname,port);
	}
}

Socket::Socket(char *hostname, char *port, int transport){
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	int x;
	x = Open(hostname,port,transport,AF_UNSPEC);
	if (x<0){
		//Console->AddEntry("Socket initialization for %s port %s failed!\n",hostname,port);
	}
}

Socket::Socket(char *hostname, char *port){
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	int x;
	x = Open(hostname,port,SOCK_STREAM,AF_UNSPEC);
	if (x<0){
		//Console->AddEntry("Socket initialization for %s port %s failed!\n",hostname,port);
	}
}

int Socket::Open(char *hostname, char *port){
	return Open(hostname,port,SOCK_STREAM,AF_UNSPEC);
}

int Socket::Open(char *hostname, char *port, int transport){
	return Open(hostname,port,transport,AF_UNSPEC);
}

int Socket::Open(char *hostname, char *port, int transport, int network){
	//create a socket
	if (stype != STYPE_BROKEN){
		sockError("Open: this socket is already open");
		return -1;
	}

	struct addrinfo *result,hints,*ptr,*cur;
	int err,s,v;


	//determine stype
	if(hostname == NULL && transport == SOCK_STREAM){
		sockReport("making tcp server");
		stype = STYPE_TCP_SERVER;
	}
	else if(transport == SOCK_STREAM){
		sockReport("making tcp client");
		stype = STYPE_TCP_CLIENT;
	}
	if(hostname == NULL && transport == SOCK_DGRAM){
		sockReport("making udp receiver");
		stype = STYPE_UDP_RECEIVER;
		//filtered = 0;
	}
	else if(transport == SOCK_DGRAM){
		sockReport("making udp sender");
		stype = STYPE_UDP_SENDER;
	}


	//set up the hints
	hints.ai_flags          = 0/*AI_ADDRCONFIG not available on winxp*/;
	if(hostname == NULL){
		hints.ai_flags |= AI_PASSIVE;
	}
	hints.ai_family         = network;
	hints.ai_socktype       = transport;
	hints.ai_addrlen        = 0;
	hints.ai_addr           = NULL;
	hints.ai_canonname      = NULL;
	hints.ai_next           = NULL;
	

	switch(transport){ //perhaps this could be better
		case SOCK_STREAM:
			hints.ai_protocol = IPPROTO_TCP;
			break;
		case SOCK_DGRAM:
			hints.ai_protocol = IPPROTO_UDP;
			break;
		default:
			sockError("Open: unsupported socket transport type");
			stype = STYPE_BROKEN;
			return -1;
	}

	
	
	//lookup all addresses
	err = getaddrinfo(hostname,port,&hints,&result);
	if(err){
		sockError("Open: getaddrinfo() error...");
		sockError((char *)gai_strerror(err));
		stype = STYPE_BROKEN;
		return -1;
	}


	//reorder addresses to put ipv6 first
	cur = result;
	ptr = result->ai_next;
	while(ptr){
		if(ptr->ai_family == AF_INET6){ //rearrange
			cur->ai_next = ptr->ai_next;
			ptr->ai_next = result;
			result = ptr;
		}

		ptr = cur->ai_next; //put ptr back to normal

		cur=ptr;
		if(ptr)
			ptr=ptr->ai_next;
		else
			break;
	}


	//try to connect to one of them
	for(ptr = result; ptr; ptr = ptr->ai_next){


		s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if (s < 0) {
        	sockError("Open: socket() error, trying another address...");
			continue;
		}
		
		if(stype == STYPE_UDP_SENDER)//outgoing udp socket
			break;

		if(hostname){//tcp client, so connect
			
			err = connect(s, ptr->ai_addr, ptr->ai_addrlen);
			if(err){
				sockError("Open: connect() error, trying another address...");
				continue;
			}
		}else{//tcp server or receiving udp socket so bind
			
			err = bind(s, ptr->ai_addr, ptr->ai_addrlen);
			if(err < 0){
				sockError("Open: bind() error, trying another address...");
				continue;
			}

			if(stype == STYPE_TCP_SERVER){//tcp server must listen
				err = listen(s, 1);
				if(err < 0){
					sockError("Open: listen() error, trying another address...");
					continue;
				}
			}
		}
			
		break;
			
	}


	if(!ptr){
		sockError("Open: no addresses left, unable to open socket");
		stype = STYPE_BROKEN;
		close(s);
		freeaddrinfo(result);
		return -1;
	}

	//set up socket details
	fd = s;
	memcpy(&address,ptr,sizeof(struct addrinfo));
	strncpy(service,port,NI_MAXSERV);
	v = platformSetNonBlock();
	if (v<0){
		Console->AddEntry("Open: SetNonBlock failed\n");
		stype = STYPE_BROKEN;
		close(s);
		freeaddrinfo(result);
		return -1;
	}

	//get ip address string
	getnameinfo(address.ai_addr, 
		address.ai_addrlen,
		number, 
		NI_MAXHOST,
		NULL, 
		0,
		NI_NUMERICHOST);
	
	freeaddrinfo(result);

	sockReport("socket opened");

	return 0;
	
}


int Socket::Close(){
	if (stype == STYPE_BROKEN)
		return 0;

	close(fd);
	sockReport("socket closed");
	strncpy(number,"closed socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;

	return 0;
}




int Socket::Accept(Socket& sock,int timeout){

	if(sock.stype != STYPE_BROKEN){
		sockError("Accept: please provide a broken socket to fill-in");
		return -1;
	}

	if(stype != STYPE_TCP_SERVER){
		sockError("Accept: only tcp server socket can accept!");
		return -1;
	}

	int s,v;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	struct timeval t;
	t.tv_sec = timeout/1000;
	t.tv_usec = 1000*(timeout%1000);

	select(fd+1,&fds,NULL,NULL,&t);
	
	if(!FD_ISSET(fd,&fds)){
		return -1;
	}
	s = accept(fd,address.ai_addr,&(address.ai_addrlen));//3rd parameter has problem on winsock?
	if (s < 0){
		sockError("Accept: accept() error");
		return -1;
	}

	sock.fd = s;
	sock.address = address;
	sock.stype = STYPE_TCP_CLIENT;
	v = sock.platformSetNonBlock();
	if (v<0){
		Console->AddEntry("Open: SetNonBlock failed\n");
		close(s);
		return -1;
	}

	getnameinfo(address.ai_addr, 
		address.ai_addrlen, 
		sock.number, 
		NI_MAXHOST,
		sock.service, 
		NI_MAXSERV,
		NI_NUMERICHOST||NI_NUMERICSERV);
	

	sockReport("new connection accepted");
	sock.sockReport("connection established");

	

	return 0;
}



int Socket::Accept(Socket& sock){

	if(sock.stype != STYPE_BROKEN){
		sockError("Accept: please provide a broken socket to fill-in");
		return -1;
	}

	if(stype != STYPE_TCP_SERVER){
		sockError("Accept: only tcp server socket can accept!");
		return -1;
	}

	int s,v;

	s = accept(fd,address.ai_addr,&(address.ai_addrlen));
	if (s < 0){
		sockError("Accept: accept() error");
		return -1;
	}

	sock.fd = s;
	sock.address = address;
	sock.stype = STYPE_TCP_CLIENT;
	v = sock.platformSetNonBlock();
	if (v<0){
		Console->AddEntry("Open: SetNonBlock failed\n");
		close(s);
		return -1;
	}

	getnameinfo(address.ai_addr, 
		address.ai_addrlen, 
		sock.number, 
		NI_MAXHOST,
		sock.service, 
		NI_MAXSERV,
		NI_NUMERICHOST||NI_NUMERICSERV);
	

	//Console->AddEntry("new connection accepted: fd %d addr %s on port %s\n",sock->fd,sock->number,sock->service);
	sockReport("new connection accepted");
	sock.sockReport("connection established");

	return 0;
}


char* Socket::getNumber(){
	return number;
}


char* Socket::getService(){
	return service;
}

int Socket::getAF(){
	return AF;
}



void Socket::sockError(char* message){
	if(sockreports){
		Console->AddEntry("sockError ([%s]:%s): %s\n",number,service,message);
	}
}


void Socket::sockReport(char* message){
	if(sockerrors){
		Console->AddEntry("sockReport ([%s]:%s): %s\n",number,service,message);
	}
}



int Socket::Send(void* data,int num){
	
	if(stype == STYPE_BROKEN){
		sockError("Send: socket must be open before sending\n");
		return -1;
	}
	if(stype == STYPE_TCP_SERVER){
		sockError("Send: tcp server socket can't send\n");
		return -1;
	}
	if(stype == STYPE_UDP_RECEIVER){
		sockError("Send: udp receiver socket can't send\n");
		return -1;
	}

	int v;

	if(stype == STYPE_UDP_SENDER)
		v = sendto(fd,data,num,0,address.ai_addr,address.ai_addrlen);
	else
		v = send(fd,data,num,0);
	if(v<0){
		if(errno==ECONNRESET){//THIS IS BROKEN, NEED TO USE platformGetError()
			sockError("Send: connection reset by peer\n");
			Close();
		}
		else{
			sockError("Send: socket error occured, closing socket\n");
			Close();
		}
	}
	
	
	return v;
}


int Socket::Recv(void* data, int num){
	
	if(stype == STYPE_BROKEN){
		sockError("Recv: socket must be open before receiving\n");
		return -1;
	}
	if(stype == STYPE_TCP_SERVER){
		sockError("Recv: tcp server socket can't recv\n");
		return -1;
	}
	if(stype == STYPE_UDP_SENDER){
		sockError("Recv: udp sender socket can't recv\n");
		return -1;
	}

	int v;
	struct sockaddr addr;
	socklen_t len;


	if(stype == STYPE_UDP_RECEIVER){
		v = recvfrom(fd,data,num,0,&addr,&len);
		if(!memcmp(&addr,address.ai_addr,len))//unexpected change of sender
			dgramUpdateAddress(&addr,&len);
	}
	else{
		v = recv(fd,data,num,0);
		if (v==0){
			sockReport("Recv: connection closed by remote host\n");
			Close();
		}
	}

	if(v<0){
			if (errno != EWOULDBLOCK){//this doesnt work on windows
				sockError("Recv: socket error occured, closing socket\n");
				Close();
			}
	}
	
	return v;
}



int Socket::SendString(char* data){
	return Send(data,strlen(data)+1);
}


int Socket::isOpen(){
	if(stype==STYPE_BROKEN)
		return 0;
	return 1;
}


void Socket::dgramUpdateAddress(struct sockaddr *from, socklen_t *fromlen){
	*(address.ai_addr) = *from;
	address.ai_addrlen = *fromlen;
	
	getnameinfo(address.ai_addr, 
		address.ai_addrlen,
		number, 
		NI_MAXHOST,
		NULL, 
		0,
		NI_NUMERICHOST);
}

#ifdef TA3D_PLATFORM_WINDOWS
void Socket::platformSetNonBlock(){
	ioctlsocket(fd,FIONBIO,1);
}

void Socket::platformGetError(){}
#endif


#ifdef TA3D_PLATFORM_LINUX
int Socket::platformSetNonBlock(){
	int flags;
	if (-1 == (flags = fcntl(fd, F_GETFL, 0))){
        flags = 0;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}     


void Socket::platformGetError(){}
#endif
