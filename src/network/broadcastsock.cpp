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

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"

/***********************************/
/**  methods for BroadcastSock  ****/
/***********************************/


int BroadcastSock::Open(const char* port)
{
	udpsocket.Open(NULL,port,PROTOCOL_BROADCAST);
	
	if( !udpsocket.isOpen()  )
    {
		Console->AddEntry("couldn't open UDP socket for broadcasting" );
		//one of them didnt work... quit
		udpsocket.Close();
		return -1;
	}
	
	obp = 0;
	uibp = 0;
	uiremain = -1;
	return 0;
}



int BroadcastSock::isOpen()
{
	return udpsocket.isOpen();
}

void BroadcastSock::Close()
{
	udpsocket.Close();
}



//byte shuffling
void BroadcastSock::loadLong(uint32_t x)//uint32
{
	uint32_t temp;
	temp = nlSwapl(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void BroadcastSock::loadShort(uint16_t x)//uint16
{
	uint16_t temp;
	temp = nlSwaps(x);
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void BroadcastSock::loadByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void BroadcastSock::loadString(const char* x)//null terminated
{
	int n = strlen(x);
	if(n < 256){
		memcpy(outbuf+obp,x,n);
		obp+=n;
	}
	else{
		memcpy(outbuf+obp,x,256);
		obp+=256;
	}
	loadByte('\0');
}	

void BroadcastSock::loadFloat(float x)
{
	float temp;
	temp = nlSwapf(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}


void BroadcastSock::sendUDP()
{
    if( !udpsocket.isOpen() )
    {
        Console->AddEntry("WARNING: broadcast socket closed!!");
    }
    int n = 0;
    while(n!=obp)
    {
        int v = udpsocket.Send(outbuf,obp);
        if( v <= 0 )
        {
            Console->AddEntry("ERROR : couldn't broadcast data");
            break;
        }
        n += v;
    }
    obp = 0;
}

void BroadcastSock::recvUDP()
{
    if(uiremain == 0)
        return;
    memset( udpinbuf, 0, MULTICAST_BUFFER_SIZE );
    int p = udpsocket.Recv(udpinbuf,MULTICAST_BUFFER_SIZE);//get new number
    if( p <= 0 ) {
        uiremain = -1;
        return;
    }
    uibp = p;
    uiremain = 0;
}


void BroadcastSock::pumpIn()
{
    recvUDP();
}

char BroadcastSock::getPacket()
{
    if(uiremain>0)
        return 0;
    return udpinbuf[0];
}



int BroadcastSock::takeFive(const int time)
{
    return udpsocket.takeFive(time);
}


String BroadcastSock::getAddress()
{
    return udpsocket.getNumber();
}


int BroadcastSock::sendMessage( const char* msg )
{
    loadString((char*)msg);
    sendUDP();
    return 0;
}

std::string BroadcastSock::makeMessage()
{
    if(uiremain != 0)
        return "";
    std::string msg;
    msg.reserve( uibp + 1 );
    for( int i = 0 ; i < uibp && udpinbuf[ i ] != 0 ; i++ )
        msg += udpinbuf[ i ];
    uibp = 0;
    uiremain = -1;

    return msg;
}


