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

char *Socket::getError()
{
	NLint error = nlGetError();
	
	if( error == NL_SYSTEM_ERROR )
		return (char*) nlGetSystemErrorStr( nlGetSystemError() );
	return (char*) nlGetErrorStr( nlGetError() );
}

Socket::Socket(){
	sockreports = 1;
	sockerrors = 1;
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	//Console->AddEntry("default constructor\n");
//	sockReport("default constructor");
}


Socket::~Socket(){
	if (stype != STYPE_BROKEN)
		if( nlClose(fd) == NL_FALSE )
			Console->AddEntry("error: ~Socket: %s", nlGetErrorStr( nlGetError() ));
}


Socket::Socket(const char *hostname, const char *port, int transport){
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	sockreports = 1;
	sockerrors = 1;
	int x;
	x = Open(hostname,port,transport);
	if (x<0){
		//Console->AddEntry("Socket initialization for %s port %s failed!\n",hostname,port);
	}
}

Socket::Socket(const char *hostname, const char *port){
	strncpy(number,"new socket",NI_MAXHOST);
	strncpy(service,"???",NI_MAXSERV);
	stype = STYPE_BROKEN;
	sockreports = 1;
	sockerrors = 1;
	int x;
	x = Open(hostname,port,PROTOCOL_TCPIP);
	if (x<0){
		//Console->AddEntry("Socket initialization for %s port %s failed!\n",hostname,port);
	}
}

int Socket::Open(const char *hostname, const char *port){
	return Open(hostname,port,PROTOCOL_TCPIP);
}

int Socket::Open(const char *hostname, const char *port, int transport){
	//create a socket
	if (stype != STYPE_BROKEN){
		sockError("Open: this socket is already open");
		return -1;
	}

	//determine stype
	if(hostname == NULL && transport == PROTOCOL_TCPIP){
		sockReport("making tcp server");
		stype = STYPE_TCP_SERVER;
	}
	else if(transport == PROTOCOL_TCPIP){
		sockReport("making tcp client");
		stype = STYPE_TCP_CLIENT;
	}
	if(hostname == NULL && transport == PROTOCOL_UDP ){
		sockReport("making udp receiver");
		stype = STYPE_UDP;
	}
	else if(transport == PROTOCOL_UDP){
		sockReport("making udp sender");
		stype = STYPE_UDP;
	}
	if( transport == PROTOCOL_BROADCAST ) {
		sockReport("making udp broadcaster");
		stype = STYPE_BROADCAST;
		}


	if( hostname ) {
		bool valid = nlGetAddrFromName( hostname, &address ) != NL_FALSE;		// Get the address
	 	if( !valid || address.valid == NL_FALSE ) {
			bool valid = nlGetAddrFromName( format( "%s:%s", hostname, port ).c_str(), &address ) != NL_FALSE;		// Retry adding port
	 		
		 	if( !valid || address.valid == NL_FALSE ) {
				stype = STYPE_BROKEN;
				sockReport( format( "failed getting address for '%s'", hostname ).c_str() );
				return -1;
				}
			}
		}

		// Try to connect

	int n_port = atoi( port );
	
	ta3d_socket s;
	if( stype == STYPE_TCP_CLIENT )
		s = nlOpen( 0, transport );
	else
		s = nlOpen( n_port, transport );
	
	if( s == NL_INVALID ) {
		stype = STYPE_BROKEN;
		sockReport( format( "cannot create socket : %s", getError() ).c_str() );
		return -1;
		}

	switch( stype )
	{
	case STYPE_UDP:
		if( hostname == NULL )	break;
		nlSetAddrPort(&address, n_port);
		nlSetRemoteAddr( s, &address );
		break;
	case STYPE_TCP_CLIENT:
		nlSetAddrPort(&address, n_port);
		if( nlConnect( s, &address ) == NL_FALSE ) {
			stype = STYPE_BROKEN;
			sockReport( format( "connect error : %s", getError() ).c_str() );
			return -1;
			}
		break;
	case STYPE_TCP_SERVER:
		if( nlListen( s ) == NL_FALSE ) {
			stype = STYPE_BROKEN;
			sockReport( format( "listen error : %s", getError() ).c_str() );
			return -1;
			}
		break;
	case STYPE_BROADCAST:			// We're ready to broadcast/receive
		break;
	};

	//set up socket details
	fd = s;
	strncpy(service,port,NI_MAXSERV);
	
	nlAddrToString(&address, number);
	for( char *f = number ; *f ; f++ )			// Get rid of the port
		if( *f == ':' ) {
			*f = 0;
			break;
			}
	
	sockReport("socket opened");

	return 0;
	
}


int Socket::Close(){
	if (stype == STYPE_BROKEN)
		return 0;

	nlClose(fd);
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

	NLint group = nlGroupCreate();
	nlGroupAddSocket( group, fd );
	
	ta3d_socket s;
	NLint v = nlPollGroup( group, NL_READ_STATUS, &s, 1, timeout);

	nlGroupDestroy( group );

	if( v == NL_INVALID || v == 0 )			// Nothing to do
		return -1;

	s = nlAcceptConnection(fd);

	if (s == NL_INVALID ){
		sockError("Accept: accept() error");
		sockError( getError() );
		return -1;
	}

	sock.fd = s;
	sock.address = address;
	sock.stype = STYPE_TCP_CLIENT;

	nlGetRemoteAddr( s, &sock.address );

	strncpy(sock.service,service,NI_MAXSERV);
	
	nlAddrToString(&sock.address, sock.number);
	for( char *f = sock.number ; *f ; f++ )			// Get rid of the port
		if( *f == ':' ) {
			*f = 0;
			break;
			}

	sockReport("new connection accepted");
	sock.sockReport("connection established");

	

	return 0;
}



int Socket::Accept(Socket& sock){
	return Accept( sock, 0 );
}


inline void Socket::sockError(const char* message){
	if(sockreports){
#if defined NETWORK_STANDALONE_UNIT_TEST
	  printf ("sockError ([%s]:%s): %s\n",number,service,message);
#else
	  const char *type = "broken";
	  switch( stype )
	  {
	  case STYPE_UDP:			type = "UDP";	break;
	  case STYPE_TCP_CLIENT:	type = "TCP";	break;
	  case STYPE_TCP_SERVER:	type = "TCP";	break;
	  case STYPE_BROADCAST:		type = "BROADCAST";	break;
	  };
	  Console->AddEntry("sockError (%s,[%s]:%s): %s",type,number,service,message);
#endif
	}
}


inline void Socket::sockReport(const char* message){
	if(sockerrors){
#if defined NETWORK_STANDALONE_UNIT_TEST
	  printf ("sockReport ([%s]:%s): %s\n",number,service,message);
#else
	  const char *type = "broken";
	  switch( stype )
	  {
	  case STYPE_UDP:			type = "UDP";	break;
	  case STYPE_TCP_CLIENT:	type = "TCP";	break;
	  case STYPE_TCP_SERVER:	type = "TCP";	break;
	  case STYPE_BROADCAST:		type = "BROADCAST";	break;
	  };
	  Console->AddEntry("sockReport (%s,[%s]:%s): %s",type,number,service,message);
#endif
	}
}



int Socket::Send(const void* data,int num){
	
	if(stype == STYPE_BROKEN){
		sockError("Send: socket must be open before sending");
		return -1;
	}
	if(stype == STYPE_TCP_SERVER){
		sockError("Send: tcp server socket can't send");
		return -1;
	}

	int count = 0;
	retry:
	int v = nlWrite( fd, data, num );

	if(v == NL_INVALID ){
		sockError( format("Send: %s", getError() ).c_str() );
		if( nlGetError() == NL_CON_PENDING ) {					// Don't close connection, it's not even opened yet !!
			count++;
			if( count < 1000 ) {
				rest(1);
				goto retry;
				}
			}
		Close();
		return -1;
		}
	return v;
}


int Socket::Recv(void* data, int num){
	
	if(stype == STYPE_BROKEN){
		sockError("Recv: socket must be open before receiving");
		return -1;
	}
	if(stype == STYPE_TCP_SERVER){
		sockError("Recv: tcp server socket can't recv");
		return -1;
	}

	int v = nlRead( fd, data, num );
	
#ifdef TA3D_PLATFORM_WINDOWS
	if( v == NL_INVALID && nlGetError() == 	NL_SYSTEM_ERROR && nlGetSystemError() == 10040 )		// Windows raise an error if you read only the beginning of the buffer !!
		v = num;
#endif
	if( v == NL_INVALID && nlGetError() != NL_BUFFER_SIZE ){		// If buffer is too small then return what we've read but don't close the connection
		sockError( format("Recv: %s", getError() ).c_str() );
		switch( nlGetError() )
		{
		case NL_SYSTEM_ERROR :
		case NL_INVALID_SOCKET :
		case NL_CON_REFUSED :
		case NL_MESSAGE_END :
			Close();
		};
		return -1;
		}

	if( v == NL_INVALID )	v = num;		// We've filled all the buffer
		
	if( stype == STYPE_UDP || stype == STYPE_BROADCAST ) {
		nlGetRemoteAddr( fd, &address );
		nlAddrToString(&address, number);
		for( char *f = number ; *f ; f++ )			// Get rid of the port
			if( *f == ':' ) {
				*f = 0;
				break;
				}
		}
	
	return v;
}



int Socket::SendString(const char* data){
	return Send(data,strlen(data)+1);
}


int Socket::isOpen(){
	if(stype==STYPE_BROKEN)
		return 0;
	return 1;
}

int Socket::takeFive(int time){
	NLint group = nlGroupCreate();
	nlGroupAddSocket( group, fd );
	
	ta3d_socket s;
	NLint v = nlPollGroup( group, NL_READ_STATUS, &s, 1, time);

	nlGroupDestroy( group );

	return v;
}
