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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef TA3D_PLATFORM_LINUX
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h> /*andrewF says that this is bad during multithreading*/
#include <fcntl.h>
#endif

#ifdef TA3D_PLATFORM_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define STYPE_BROKEN -1
#define STYPE_TCP_SERVER 0
#define STYPE_TCP_CLIENT 1
#define STYPE_UDP_SENDER 2
#define STYPE_UDP_RECEIVER 3



/****
**
** Socket
**
** This particular socket class is a fairly basic layer
** above bsd sockets api. TCP/UDP and ipv6 are supported.
** There is no fancy support for sending packets, you may
** only use this class to send your pre-prepared bytes.
** That kind of stuff can go into a derived class.
**
**
**
** instruction manual
** when opening a socket you must specify four things:
**
** hostname - a string containing either
** a hostname: socket looks up connects to it OR
** an ipv4/ipv6 address: socket connects to it OR 
** NULL or nothing: socket will become a tcp server or udp receiver
**
** port - a string containing the port number to connect to or to listen on
**
** transport - a defined constant like
** SOCK_STREAM - use tcp transport proto / stream socket
** SOCK_DGRAM - use udp transport proto / datagram socket
** default transport = SOCK_STREAM
**
** network - a defined constant like
** AF_INET - use ipv4 connection or create a normal server socket
** AF_INET6 - use ipv6 connection or create a server socket that accepts both connections
** AF_UNSPEC - use this to let the socket decide which network proto to use (prefers ipv6)
** other protocols may work but for the general internet we only need these.
** default network = AF_UNSPEC
**
** examples
** some common ways to do common things
**
** //connect to a server somewhere on port 12345 (ipv4/ipv6)
** Socket mysock("foo.example.net","12345",SOCK_STREAM,AF_UNSPEC); or just
** Socket mysock("foo.example.net","12345");
**
** //connect to a server forcing a ipv4 connection
** Socket mysock("foo.example.net","12345",SOCK_STREAM,AF_INET);
**
** //set up a listening server on port 12345 (accepts both ipv4/ipv6 connections)
** Socket mysock(NULL,"12345");
** Socket remote;
** mysock.Accept(remote);
**
** //send UDP datagram to someone on port 12345 (auto determines ipv4/6)
** Socket mysock("foo.example.net","12345",SOCK_DATAGRAM);
** mysock.SendString("HELLO 12345");
**
** //receive UDP datagrams on port 12345
** Socket mysock(NULL,"12345",SOCK_DATAGRAM);
**
**
***/

#ifndef __TA3D_SOCKET_CLASS__
#define __TA3D_SOCKET_CLASS__

class Socket{

	//int ready; //perhaps unnecessary
	int fd; 							//system socket number, file descriptor
	addrinfo address; 	//new connection info struct supports ipv6
	char number[NI_MAXHOST]; 	//ip address
	char service[NI_MAXSERV];  //service/port number
	int stype;               	//what kind of socket
	int AF;							//ipv4, ipv6, unspec, or something completely different

	int sockreports;
	int sockerrors;
	
	void dgramUpdateAddress(struct sockaddr *from, socklen_t *fromlen);
	void sockError(char* message);
	void sockReport(char* message);

	int platformSetNonBlock();
	void platformGetError();

	public:
		Socket();
		Socket(char *hostname, char *port);
		Socket(char *hostname, char *port, int transport);
		Socket(char *hostname, char *port, int transport, int network);
		Socket(char *hostname, char *port, int transport, int network, bool multicast);
		~Socket();

		//utilities
		int Accept(Socket& sock); 
		int Accept(Socket& sock,int timeout);    	//wait for incoming connections
		int getFD() {return fd;}
		char* getNumber();   			//human readable ip address
		char* getService();  			//human readable port number
		int getAF();
		int isOpen();						//broken or not
		//int Select();					//block until ready for reading
		
		//open and close manually
		int Open(char *hostname, char *port);
		int Open(char *hostname, char *port, int transport);
		int Open(char* hostname, char *port, int transport, int network);
		int Open(char* hostname, char *port, int transport, int network, bool multicast);
		int Close();

		//communication
		int Send(void* data,int num);//send num bytes of data
		int Recv(void* data,int num);//recv a packet or num bytes if thats smaller
		
		int SendString(char* data);//if data is null terminated

		int takeFive(int time);
};
#endif
