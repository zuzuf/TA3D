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

#if defined NETWORK_STANDALONE_UNIT_TEST
#include "SocketClass.h"
#else
#include "stdafx.h"
#include "TA3D_NameSpace.h"
#endif


#if defined TA3D_PLATFORM_WINDOWS

int Socket::initWinData()
{
  int result = WSAStartup(MAKEWORD(2,2), &init_win32);
  if (0 != result)
    {
      //problem!
      throw(result);
    }
  else
    {
      return 0;
    }
}
#endif

Socket::Socket(){
	sockreports = 1;
	sockerrors = 1;
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
#if defined TA3D_PLATFORM_WINDOWS
	initWinData();
#endif
	//Console->AddEntry("default constructor\n");
	sockReport("default constructor");
}


Socket::~Socket(){
	if (stype != STYPE_BROKEN)
		close(fd);
#if defined TA3D_PLATFORM_WINDOWS
	WSACleanup();
#endif	
}


Socket::Socket(char *hostname, char *port, int transport, int network, char *multicast){
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	sockreports = 1;
	sockerrors = 1;
#if defined TA3D_PLATFORM_WINDOWS
	initWinData();
#endif
	int x;
	x = Open(hostname,port,transport,network,multicast);
	if (x<0){
		//Console->AddEntry("Socket initialization for %s port %s failed!\n",hostname,port);
	}
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
	sockreports = 1;
	sockerrors = 1;
#if defined TA3D_PLATFORM_WINDOWS
	initWinData();
#endif
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
	sockreports = 1;
	sockerrors = 1;
#if defined TA3D_PLATFORM_WINDOWS
	initWinData();
#endif
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
	return Open(hostname,port,transport,network,false);
}

int Socket::Open(char *hostname, char *port, int transport, int network, char *multicast){
	//create a socket
	if (stype != STYPE_BROKEN){
		sockError("Open: this socket is already open");
		return -1;
	}

	struct addrinfo *result,hints,*ptr,*cur;
	int err,v;

	ta3d_socket s;


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
	//	hints.ai_flags          = 0/*AI_ADDRCONFIG not available on winxp*/;
	memset(&hints, 0, sizeof(hints));
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
#if !defined TA3D_PLATFORM_WINDOWS
		sockError((char *)gai_strerror(err));
#endif
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

	if( multicast && stype == STYPE_UDP_SENDER ) {
		int optReUseAddr = 1;
#if defined TA3D_PLATFORM_LINUX
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optReUseAddr, sizeof(int));
#elif defined TA3D_PLATFORM_WINDOWS
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optReUseAddr, sizeof(int));
#endif
		bind(s, ptr->ai_addr, ptr->ai_addrlen);
		}
		
	if( multicast ) {
		struct ip_mreq mreq;
		//		int i=0;
#if defined TA3D_PLATFORM_LINUX
		if (inet_aton( multicast, &mreq.imr_multiaddr) < 0)
		  {
		    sockError("inet_aton mreq");
		    stype = STYPE_BROKEN;
		    close(s);
		    freeaddrinfo(result);
		    return -1;
		  }
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
		{
		  sockError("setsockopt IP_ADD_MEMBERSHIP ");
		  return -1;
		}
#elif defined TA3D_PLATFORM_WINDOWS
		// on Vista, it is necessary to use RtlIpv4StringToAddress function!
		mreq.imr_multiaddr.s_addr = inet_addr ( multicast);
		if (INADDR_NONE == inet_addr ( multicast))
		  {
		    sockError("inet_addr mreq");
		    stype = STYPE_BROKEN;
		    close(s);
		    freeaddrinfo(result);
		    return -1;
		  }
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
		  {
		    sockError("setsockopt IP_ADD_MEMBERSHIP ");
		    stype = STYPE_BROKEN;
		    close(s);
		    freeaddrinfo(result);
		    return -1;
		  }
#endif
	}
	//set up socket details
	fd = s;
	memcpy(&address,ptr,sizeof(struct addrinfo));
	strncpy(service,port,NI_MAXSERV);
	v = platformSetNonBlock();
	if (v<0){
	  //Console->AddEntry("Open: SetNonBlock failed\n");
		sockError("Open: SetNonBlock failed");
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

//	freeaddrinfo(result);			// FIXME : frees used addresses ...

	sockReport("socket opened");

	return 0;
	
}


int Socket::Close(){
	if (stype == STYPE_BROKEN)
		return 0;

#if defined TA3D_PLATFORM_WINDOWS
	if((STYPE_TCP_SERVER == stype)
	   ||
	   (STYPE_TCP_CLIENT == stype))
	  {
	    shutdown(fd, 2);
	  }
#endif
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

	int v;
	ta3d_socket s;
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
#if defined TA3D_PLATFORM_LINUX
	s = accept(fd,address.ai_addr,&(address.ai_addrlen));
#elif defined TA3D_PLATFORM_WINDOWS
	s = accept(fd, address.ai_addr, ((int *)&(address.ai_addrlen)));
#endif

	if (s < 0){
		sockError("Accept: accept() error");
		sockError(strerror(errno));
		return -1;
	}

	sock.fd = s;
	sock.address = address;
	sock.stype = STYPE_TCP_CLIENT;
	v = sock.platformSetNonBlock();
	if (v<0){
	  //Console->AddEntry("Open: SetNonBlock failed\n");
		sockError("Open: SetNonBlock failed");
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

	int v;
	ta3d_socket s;

#if defined TA3D_PLATFORM_LINUX
	s = accept(fd,address.ai_addr,&(address.ai_addrlen));
#elif defined TA3D_PLATFORM_WINDOWS
	s = accept(fd, address.ai_addr, ((int *)&(address.ai_addrlen)));
#endif
	if (s < 0){
		sockError("Accept: accept() error");
		sockError(strerror(errno));
		return -1;
	}

	sock.fd = s;
	sock.address = address;
	sock.stype = STYPE_TCP_CLIENT;
	v = sock.platformSetNonBlock();
	if (v<0){
	  //Console->AddEntry("Open: SetNonBlock failed\n");
		sockError("Open: SetNonBlock failed");
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


inline void Socket::sockError(char* message){
	if(sockreports){
#if defined NETWORK_STANDALONE_UNIT_TEST
	  printf ("sockError ([%s]:%s): %s\n",number,service,message);
#else
	  Console->AddEntry("sockError ([%s]:%s): %s",number,service,message);
#endif
	}
}


inline void Socket::sockReport(char* message){
	if(sockerrors){
#if defined NETWORK_STANDALONE_UNIT_TEST
	  printf ("sockReport ([%s]:%s): %s\n",number,service,message);
#else
	  Console->AddEntry("sockReport ([%s]:%s): %s",number,service,message);
#endif
	}
}



int Socket::Send(void* data,int num){
	
	if(stype == STYPE_BROKEN){
		sockError("Send: socket must be open before sending");
		return -1;
	}
	if(stype == STYPE_TCP_SERVER){
		sockError("Send: tcp server socket can't send");
		return -1;
	}
	if(stype == STYPE_UDP_RECEIVER){
		sockError("Send: udp receiver socket can't send");
		return -1;
	}

	int v;
#if defined TA3D_PLATFORM_LINUX
	if(stype == STYPE_UDP_SENDER) {
//		printf("C: sending to %d.%d.%d.%d\n", ((unsigned char*)address.ai_addr->sa_data)[0],((unsigned char*)address.ai_addr->sa_data)[1],((unsigned char*)address.ai_addr->sa_data)[2],((unsigned char*)address.ai_addr->sa_data)[3]);
		v = sendto(fd,data,num,0,address.ai_addr,address.ai_addrlen);
		}
	else
		v = send(fd,data,num,0);
	if(v<0){
		if(errno==ECONNRESET){//THIS IS BROKEN, NEED TO USE platformGetError()
			sockError("Send: connection reset by peer");
			Close();
		}
		else{
			sockError("Send: socket error occured, closing socket");
			Close();
		}
	}
#elif defined TA3D_PLATFORM_WINDOWS
	if(stype == STYPE_UDP_SENDER)
		v = sendto(fd,(const char*)data,num,0,address.ai_addr,address.ai_addrlen);
	else
		v = send(fd,(const char*)data,num,0);
	if( SOCKET_ERROR == v )
	  {
	    sockError("Send: socket error occured, closing socket\n");
	    Close();
	    return(-1);
	  }
#endif


	return v;
}


int Socket::Recv(void* data, int num, uint32 *address){
	
	if(stype == STYPE_BROKEN){
		sockError("Recv: socket must be open before receiving");
		return -1;
	}
	if(stype == STYPE_TCP_SERVER){
		sockError("Recv: tcp server socket can't recv");
		return -1;
	}
//	if(stype == STYPE_UDP_SENDER){
//		sockError("Recv: udp sender socket can't recv");
//		return -1;
//	}

	int v;
	struct sockaddr_in addr;
	socklen_t len = sizeof( addr );


	if(stype == STYPE_UDP_RECEIVER || stype == STYPE_UDP_SENDER){
#if defined TA3D_PLATFORM_LINUX
		v = recvfrom(fd,data,num,0,(struct sockaddr*)&addr,&len);
#elif defined TA3D_PLATFORM_WINDOWS
		v = recvfrom(fd, (char*)data,num,0,(struct sockaddr*)&addr, &len);
#endif
		if( address && v > 0 )
			*address = addr.sin_addr.s_addr;
//		if(stype == STYPE_UDP_RECEIVER && !memcmp(&addr,address.ai_addr,len))//unexpected change of sender
//			dgramUpdateAddress(&addr,&len);
	}
	else{
#if defined TA3D_PLATFORM_LINUX
		v = recv(fd,data,num,0);
		if(v<0){
		  if (errno != EWOULDBLOCK){
		    sockError("Recv: socket error occured, closing socket\n");
		    Close();
		  }
		}

#elif defined TA3D_PLATFORM_WINDOWS
		v = recv(fd,(char*)data,num,0);

#endif
		if (v==0){
		  sockReport("Recv: connection closed by remote host\n");
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

#if defined TA3D_PLATFORM_LINUX
int Socket::platformSetNonBlock(){
	int flags;
	if (-1 == (flags = fcntl(fd, F_GETFL, 0))){
        flags = 0;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}     


inline int Socket::platformGetError(){}
#elif defined TA3D_PLATFORM_WINDOWS
int Socket::platformSetNonBlock(){
  u_long mode = 1;
  ioctlsocket(fd, FIONBIO, &mode);
  return(0);

  //return (ioctlsocket(fd,FIONBIO,1));
}

inline int Socket::platformGetError(){}
#endif


int Socket::takeFive(int time){
  //#if defined TA3D_PLATFORM_LINUX

	fd_set set;

	FD_ZERO(&set);
	FD_SET(fd,&set);
	
	struct timeval t;
	t.tv_sec = time/1000;
	t.tv_usec = 1000*(time%1000);

	select(fd+1,&set,NULL,NULL,&t);

	//#endif
		
	return 0;
}
