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




/*
** notes on packets
**
** chat packet
** 1 byte  = N+1
** 1 byte  = 'C' (or 'X' if it's a special packet, for example for LAN servers discovery)
** N bytes = null terminated message string
** total = N+2
**
** order packet
**    1 byte  = 23  (not buffered)
** 0  1 byte  = 'O'
** 1  4 bytes = time of execution
** 5  4 bytes = unit number
** 9  1 byte  = order type
** 10 4 bytes = floating data 1
** 14 4 bytes = floating data 2
** 18 4 bytes = target unit
** 22 1 byte  = this is an additional order
** total = 24
**
** sync packet
**    1 byte  = 27  (not buffered)
** 0  1 byte  = 'S'
** 1  4 bytes = time of sync
** 5  4 bytes = unit number
** 9  4 bytes = floating x
** 13 4 bytes = floating y
** 17 4 bytes = floating vx
** 21 4 bytes = floating vy
** 25 2 bytes = orientation
** total = 28
**
** event packet
** 1 byte  = 4
** 1 byte  = 'E'
** 1 byte  = event type
** 1 byte  = player 1
** 1 byte  = player 2
** total = 5
*/


#define MULTICAST_BUFFER_SIZE	512

//MulticastSock- specialized low-level networking
//used internally by Network to discover servers over
//a LAN network
class BroadcastSock
{
private:
	Socket udpsocket;

	//only touched by main thread
	char outbuf[MULTICAST_BUFFER_SIZE];
	int obp;

	//only touched by socket thread
	char udpinbuf[MULTICAST_BUFFER_SIZE];
	int uibp;
	int uiremain;
	
	//byte shuffling
	void loadLong(uint32_t x);//uint32
	void loadShort(uint16_t x);//uint16
	void loadByte(uint8_t x);//uint8
	void loadString(const char* x);//null terminated
	void loadFloat(float x);

	void sendUDP();
	void recvUDP();

	int max(int a, int b) {return (a>b ? a : b);}

	public:
		BroadcastSock() {obp=0;uibp=0;uiremain=-1;}
		~BroadcastSock() {}

		int Open(const char* port);
		void Close();

		int isOpen();

		int sendMessage( const char* msg );

		//these are for incoming packets
		String makeMessage();

		String getAddress();

		char getPacket();//if packet is ready return the type, else return -1
		void pumpIn();//get input from sockets non-blocking
		int takeFive(const int time);//sleep until sockets will not block
};
