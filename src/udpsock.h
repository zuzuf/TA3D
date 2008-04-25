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

#include "SocketClass.h"
#include "thread.h"


#define		UDPSOCK_BUFFER_SIZE		2560

struct chat;

//UDPSock- specialized low-level networking
//used internally by Network to send the data
//to the players. It is basically an UDP socket
class UDPSock{

	Socket udpsock;
	Mutex udpmutex;
	
	NLushort	udp_port;

	//only touched by main thread
	char outbuf[UDPSOCK_BUFFER_SIZE];
	int obp;

	//only touched by socket thread
	char udpinbuf[UDPSOCK_BUFFER_SIZE];
	int uibp;
	int uiremain;//how much is left to recv
	
	//byte shuffling
	void loadLong(uint32_t x);//uint32
	void loadShort(uint16_t x);//uint16
	void loadByte(uint8_t x);//uint8
	void loadString(const char* x);//null terminated
	void loadFloat(float x);

	void send( const std::string &address );
	void recv();

	int max(int a, int b) {return (a>b ? a : b);}

	public:
		UDPSock() {obp=0;uibp=0;uiremain=-1;}
		~UDPSock() {}

		int Open(const char* hostname,const char* port);
		void Close();

		char* getAddress() {return udpsock.getNumber();}
		char* getPort() {return udpsock.getService();}
		Socket& getSock() {return udpsock;}
		int isOpen();

		//these are for outgoing packets
		int sendSpecial(struct chat* chat, const std::string &address);
		void cleanPacket();

		//these are for incoming packets
		int makeSpecial(struct chat* chat);

		char getPacket();//if packet is ready return the type, else return -1
		void pumpIn();//get input from sockets non-blocking
		int takeFive(int time);//sleep until sockets will not block
};
