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
/**  methods for MulticastSock  ****/
/***********************************/


int MulticastSock::Open(char* port){

	udpsocket.Open(NULL,port,PROTOCOL_BROADCAST);
	
	if( !udpsocket.isOpen()  ){
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



int MulticastSock::isOpen(){
	return udpsocket.isOpen();
}

void MulticastSock::Close(){
	udpsocket.Close();
}



//byte shuffling
void MulticastSock::loadLong(uint32_t x){//uint32
	uint32_t temp;
	temp = nlSwapl(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void MulticastSock::loadShort(uint16_t x){//uint16
	uint16_t temp;
	temp = nlSwaps(x);
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void MulticastSock::loadByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void MulticastSock::loadString(char* x){//null terminated
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

void MulticastSock::loadFloat(float x){
	float temp;
	temp = nlSwapf(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}


void MulticastSock::sendUDP(){
	if( !udpsocket.isOpen() ) {
		Console->AddEntry("WARNING: broadcast socket closed!!");
		}
	int n = 0;
	while(n!=obp) {
		int v = udpsocket.Send(outbuf,obp);
		if( v <= 0 ) {
			Console->AddEntry("ERROR : couldn't broadcast data");
			break;
			}
		n += v;
		}
	obp = 0;
}

void MulticastSock::recvUDP(){
	if(uiremain == 0)
		return;
	else if(uiremain == -1){
		uint8_t remain;
		int p = udpsocket.Recv(&remain,1);//get new number
		if( p < 0 ) {
			uiremain = -1;
			return;
			}
		if(remain == 0){
			uint16_t remain2;
			p = udpsocket.Recv(&remain2,2);//get big number
			if( p < 0 ) {
				uiremain = -1;
				return;
				}
			uiremain = nlSwaps(remain2);
		}
		else if(remain>0) uiremain = remain;
		else Console->AddEntry("udp packet error cannot determine size");
		return;
	}
	int n = 0;
	n = udpsocket.Recv(udpinbuf+uibp,uiremain);
	if( n > 0 ) {
		uibp += n;
		uiremain -= n;
		}
}


void MulticastSock::pumpIn(){
	recvUDP();
}

char MulticastSock::getPacket(){
	if(uiremain>0)
		return 0;
	else
		return udpinbuf[0];
}



int MulticastSock::takeFive(int time){
	return udpsocket.takeFive( time );
}


String MulticastSock::getAddress() {
	return udpsocket.getNumber();
}


int MulticastSock::sendMessage( const char* msg ){
	loadString((char*)msg);
	loadByte('\0');
	sendUDP();
	return 0;
}

std::string MulticastSock::makeMessage(){
	if(uiremain != 0){
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
