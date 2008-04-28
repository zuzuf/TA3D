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

/*****************************/
/***  methods for UDPSock  ***/
/*****************************/


int UDPSock::Open(const char* hostname,const char* port){

	udp_port = atoi( port );

	udpsock.Open(hostname,port,PROTOCOL_UDP);

	if(!udpsock.isOpen())
		return -1;

	return 0;

}

int UDPSock::isOpen(){
	return udpsock.isOpen();
}

void UDPSock::Close(){
	if( udpsock.isOpen() )
		udpsock.Close();
}


void UDPSock::send(const std::string &address){
	udpmutex.Lock();
	
	NLaddress addr;
	nlStringToAddr( address.c_str(), &addr );
	nlSetAddrPort( &addr, udp_port );

	nlSetRemoteAddr( udpsock.getFD(), &addr );

	int n = 0;
	while(n < obp) {
		int v = udpsock.Send(outbuf + n,obp - n);
		if( v <= 0 ) {
			Console->AddEntry("ERROR : could not send data over UDP!");
			break;
			}
		n += v;
		}
	obp = 0;

	udpmutex.Unlock();
}

void UDPSock::recv(){
	if(uiremain == 0)
		return;
	udpmutex.Lock();
	memset( udpinbuf, 0, UDPSOCK_BUFFER_SIZE );
	int p = udpsock.Recv(udpinbuf,UDPSOCK_BUFFER_SIZE);//get new number
	if( p <= 0 ) {
		rest(1);
		uiremain = -1;
		udpmutex.Unlock();
		return;
		}
	uibp = p;
	uiremain = 0;
	udpmutex.Unlock();
}


void UDPSock::pumpIn(){
	recv();
}

char UDPSock::getPacket(){
	if(uiremain>0)
		return 0;
	else
		return udpinbuf[0];
}

void UDPSock::cleanPacket(){
	if(uiremain<=0) {
		udpinbuf[uibp] = 0;
		printf("udpinbuf = '%s'\n", udpinbuf);
		uibp = 0;
		uiremain = -1;
	}
}



int UDPSock::takeFive(int time){
	return udpsock.takeFive( time );
}

//byte shuffling
void UDPSock::loadLong(uint32_t x){//uint32
	uint32_t temp;
	temp = nlSwapl(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void UDPSock::loadShort(uint16_t x){//uint16
	uint16_t temp;
	temp = nlSwaps(x);
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void UDPSock::loadByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void UDPSock::loadString(const char* x){//null terminated
	int n = strlen(x);
	if(n < UDPSOCK_BUFFER_SIZE - obp - 1 ){
		memcpy(outbuf+obp,x,n);
		obp+=n;
	}
	else{
		memcpy(outbuf+obp,x,UDPSOCK_BUFFER_SIZE - obp - 1);
		obp+=UDPSOCK_BUFFER_SIZE - obp - 1;
	}
	loadByte('\0');
}	

void UDPSock::loadFloat(float x){
	float temp;
	temp = nlSwapf(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}


int UDPSock::sendSpecial(struct chat* chat, const std::string &address){
	udpmutex.Lock();

	loadByte('X');
	loadShort(chat->from);
	loadString(chat->message);
	send( address );

	udpmutex.Unlock();
	return 0;
}

int UDPSock::sendSync(struct sync* sync, const std::string &address){
	udpmutex.Lock();

	loadByte('S');
	loadLong(sync->timestamp);
	loadShort(sync->unit);
	loadLong(sync->x);
	loadLong(sync->y);
	loadLong(sync->z);
	loadLong(sync->vx);
	loadLong(sync->vz);
	loadShort(sync->orientation);
	send( address );

	udpmutex.Unlock();
	return 0;
}

int UDPSock::makeSync(struct sync* sync){
	if(udpinbuf[0] != 'S'){
		Console->AddEntry("makeSync error: the data doesn't start with an 'S'");
		return -1;
	}
	if(uiremain == -1){
		return -1;
	}
	uint32 temp;
	uint16 stemp;

	memcpy(&temp,udpinbuf+1,4);
	sync->timestamp = nlSwapl(temp);

	memcpy(&stemp,udpinbuf+5,2);
	sync->unit = nlSwaps(stemp);

	memcpy(&temp,udpinbuf+7,4);
	sync->x = nlSwapl(temp);

	memcpy(&temp,udpinbuf+11,4);
	sync->y = nlSwapl(temp);

	memcpy(&temp,udpinbuf+15,4);
	sync->z = nlSwapl(temp);

	memcpy(&temp,udpinbuf+19,4);
	sync->vx = nlSwapl(temp);

	memcpy(&temp,udpinbuf+23,4);
	sync->vz = nlSwapl(temp);

	memcpy(&stemp,udpinbuf+27,2);
	sync->orientation = nlSwaps(stemp);
	
	uibp = 0;
	uiremain = -1;
	return 0;
}

int UDPSock::makeSpecial(struct chat* chat){
	if(udpinbuf[0] != 'X'){
		Console->AddEntry("makeSpecial error: the data doesn't start with a 'X'");
		return -1;
	}
	if(uiremain == -1){
		return -1;
	}
	chat->from = ((uint16*)(udpinbuf+1))[0];
	memcpy(chat->message,udpinbuf+3,253);
	(chat->message)[252] = '\0';
	uibp = 0;
	uiremain = -1;

	return 0;
}

