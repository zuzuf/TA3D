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

/***********************************/
/**  methods for BroadcastSock  ****/
/***********************************/


int BroadcastSock::Open(char* hostname,char* port,int network){

	udpin.Open(NULL,port,SOCK_DGRAM,network);
	if(hostname)
		udpout.Open(hostname,port,SOCK_DGRAM,network,true);
	
	if( !( udpin.isOpen() && (udpout.isOpen() || hostname == NULL) ) ){
		if( hostname )
			Console->AddEntry("couldn't open %s for UDP", hostname );
		else
			Console->AddEntry("couldn't open UDP sockets" );
		//one of them didnt work... quit
		udpin.Close();
		udpout.Close();
		return -1;
	}

	return 0;

}



int BroadcastSock::isOpen(){
	return udpin.isOpen();
}

void BroadcastSock::Close(){
	udpin.Close();
	udpout.Close();
}



//byte shuffling
void BroadcastSock::loadLong(uint32_t x){//uint32
	uint32_t temp;
	temp = htonl(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void BroadcastSock::loadShort(uint16_t x){//uint16
	uint16_t temp;
	temp = htons(x);
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void BroadcastSock::loadByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void BroadcastSock::loadString(char* x){//null terminated
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

void BroadcastSock::loadFloat(float x){
	uint32_t temp;
	temp = htonl((uint32_t)x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}


void BroadcastSock::sendUDP(){
	if( !udpout.isOpen() ) {
		Console->AddEntry("WARNING: broadcast socket closed!!");
		}
	int n = 0;
	while(n!=obp) {
		int v = udpout.Send(outbuf,obp);
		if( v <= 0 ) {
			Console->AddEntry("ERROR : couldn't broadcast data");
			break;
			}
		n += v;
		}
	obp = 0;
}

void BroadcastSock::recvUDP(){
	if(uiremain == 0)
		return;
	else if(uiremain == -1){
		uint8_t remain;
		udpin.Recv(&remain,1);//get new number
		if(remain == 0){
			uint16_t remain2;
			udpin.Recv(&remain2,2);//get big number
			uiremain = ntohs(remain2);
		}
		else if(remain>0) uiremain = remain;
		else Console->AddEntry("udp packet error cannot determine size\n");
		return;
	}
	int n = 0;
	n = udpin.Recv(udpinbuf+uibp,uiremain);
	uibp += n;
	uiremain -= n;
}


void BroadcastSock::pumpIn(){
	recvUDP();
}

char BroadcastSock::getPacket(){
	if(uiremain>0)
		return 0;
	else
		return udpinbuf[0];
}



int BroadcastSock::takeFive(int time){
	return udpin.takeFive( time );
}


int BroadcastSock::sendMessage( const char* msg ){
	loadString((char*)msg);
	loadByte('\0');
	sendUDP();
	return 0;
}

std::string BroadcastSock::makeMessage(){
	if(uiremain = -1){
		return "";
	}
	std::string msg;
	msg.reserve( uibp + 1 );
	for( int i = 0 ; i < uibp && udpinbuf[ i ] != 0 ; i++ )
		msg += udpinbuf[ i ];
	uibp = 0;
	uiremain = -1;

	return msg;
}
