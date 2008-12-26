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

#include "hawknl/include/nl.h"			// Our low-level network layer, OS abstraction layer

#if defined TA3D_PLATFORM_WINDOWS

#define sleep rest

#endif


#define STYPE_BROKEN -1
#define STYPE_TCP_SERVER 0
#define STYPE_TCP_CLIENT 1
#define STYPE_UDP 2
#define STYPE_BROADCAST 3

#define PROTOCOL_TCPIP		NL_RELIABLE_PACKETS
#define PROTOCOL_UDP		NL_UNRELIABLE
#define PROTOCOL_BROADCAST	NL_BROADCAST

/*!
 * Windows and Unix do not use the same type for socket file descriptor
 * Then an abstraction is needed
 */
#define ta3d_socket NLsocket

/*!
 * This value is defined in netbd.h but it is not available on Windows
 */
#ifndef NI_MAXHOST
# define NI_MAXHOST      1025
#endif
#ifndef NI_MAXSERV
# define NI_MAXSERV      32
#endif

#if defined NETWORK_STANDALONE_UNIT_TEST
#if !defined uint32
#define uint32 unsigned long int
#endif
#endif

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

	ta3d_socket fd;

	NLaddress	address;
	char number[NI_MAXHOST]; 	//ip address
	char service[NI_MAXSERV];  //service/port number
	int stype;               	//what kind of socket

	int sockreports;
	int sockerrors;

	void sockError(const char* message);
	void sockReport(const char* message);

	int platformSetNonBlock();
	int platformGetError();
	
	char *getError();

	public:
		Socket();
		Socket(const char *hostname, const char *port);
		Socket(const char *hostname, const char *port, int transport);
		~Socket();

		//utilities
		int Accept(Socket& sock); 
		int Accept(Socket& sock,int timeout);    	//wait for incoming connections
		inline ta3d_socket getFD() {return fd;}
		inline /*const*/ char* getNumber()	{	return number;	}   			//human readable ip address
		inline /*const*/ char* getService(){return service;}  			//human readable port number
		inline int getstype(){return stype;}
		int isOpen();						//broken or not
		
		//open and close manually
		int Open(const char *hostname, const char *port);
		int Open(const char *hostname, const char *port, int transport);
		int Close();

		//communication
		int Send(const void* data,int num);//send num bytes of data
		int Recv(void* data,int num);//recv a packet or num bytes if thats smaller
		
		int SendString(const char* data);//if data is null terminated

		int takeFive(int time);
};
#endif
