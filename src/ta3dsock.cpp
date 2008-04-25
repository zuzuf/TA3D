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

/******************************/
/**  methods for TA3DSock  ****/
/******************************/


int TA3DSock::Open(const char* hostname,const char* port){

	tcpsock.Open(hostname,port,PROTOCOL_TCPIP);

	if(!tcpsock.isOpen())
		return -1;

	return 0;

}



int TA3DSock::Accept(TA3DSock** sock){
	int v;
	(*sock) = new TA3DSock;
	v = tcpsock.Accept((*sock)->tcpsock);
	if(v<0){
		//accept error
		delete (*sock);
		return -1;
	}

	if(!(*sock)->tcpsock.isOpen() ){
		delete *sock;
		return -1;
	}

	return 0;
}

int TA3DSock::Accept(TA3DSock** sock,int timeout){
	int v;
	*sock = new TA3DSock;
	v = tcpsock.Accept((*sock)->tcpsock,timeout);
	
	if(v<0){
		//accept error
		delete (*sock);
		return -1;
	}

	
	if(!(*sock)->tcpsock.isOpen() ){
		delete *sock;
		return -1;
	}

	return 0;
}

int TA3DSock::isOpen(){
	return tcpsock.isOpen();
}

void TA3DSock::Close(){
	if( tcpsock.isOpen() )
		tcpsock.Close();
}



//byte shuffling
void TA3DSock::loadLong(uint32_t x){//uint32
	uint32_t temp;
	temp = x;
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void TA3DSock::loadShort(uint16_t x){//uint16
	uint16_t temp;
	temp = x;
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void TA3DSock::loadByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void TA3DSock::loadString(const char* x){//null terminated
	int n = strlen(x);
	if(n < TA3DSOCK_BUFFER_SIZE - obp - 1){
		memcpy(outbuf+obp,x,n);
		obp+=n;
	}
	else{
		memcpy(outbuf+obp,x,TA3DSOCK_BUFFER_SIZE - obp - 1);
		obp+=TA3DSOCK_BUFFER_SIZE - obp - 1;
	}
	loadByte('\0');
}	

void TA3DSock::loadFloat(float x){
	float temp;
	temp = nlSwapf(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}


void TA3DSock::sendTCP(byte *data, int size)
{
	tcpmutex.Lock();

	int n = 0;
	int count = 0;
	while( !n && count < 100 && isOpen() ) {
		n = tcpsock.Send(data,size);
		count++;
		}

	tcpmutex.Unlock();
}

void TA3DSock::sendTCP(){
	tcpmutex.Lock();

	int n = 0;
	int count = 0;
	while( !n && count < 100 && isOpen() ) {
		n = tcpsock.Send(outbuf,obp);
		count++;
		}
	obp = 0;

	tcpmutex.Unlock();
}

void TA3DSock::recvTCP(){
	if( tiremain == 0 )	return;

	int p = tcpsock.Recv( tcpinbuf, TA3DSOCK_BUFFER_SIZE );
	if( p <= 0 ) {
		rest(1);
		tiremain = -1;
		return;
		}
	tiremain = 0;
	tibp = p;
}


void TA3DSock::pumpIn(){
	recvTCP();
}

char TA3DSock::getPacket(){
	if(tiremain>0)
		return 0;
	else
		return tcpinbuf[0];
}

void TA3DSock::cleanPacket(){
	if(tiremain<=0) {
		tcpinbuf[tibp] = 0;
		printf("tcpinbuf = '%s'\n", tcpinbuf);
		tibp = 0;
		tiremain = -1;
	}
}



int TA3DSock::takeFive(int time){
	return tcpsock.takeFive( time );
}


int TA3DSock::sendSpecial(struct chat* chat){
	tcpmutex.Lock();
	loadByte('X');
	loadShort(chat->from);
	loadString(chat->message);
	sendTCP();
	tcpmutex.Unlock();
	return 0;
}

int TA3DSock::sendChat(struct chat* chat){
	tcpmutex.Lock();
	loadByte('C');
	loadShort(chat->from);
	loadString(chat->message);
	sendTCP();
	tcpmutex.Unlock();
	return 0;
}

int TA3DSock::sendOrder(struct order* order){
	tcpmutex.Lock();
	loadByte('O');
	loadLong(order->timestamp);
	loadLong(order->unit);
	loadByte(order->command);
	loadFloat(order->x);
	loadFloat(order->y);
	loadLong(order->target);
	loadByte(order->additional);
	sendTCP();
	tcpmutex.Unlock();
	return 0;
}

int TA3DSock::sendSync(struct sync* sync){
	tcpmutex.Lock();
//	loadByte('S');
//	loadLong(sync->timestamp);
//	loadLong(sync->unit);
//	loadFloat(sync->x);
//	loadFloat(sync->y);
//	loadFloat(sync->vx);
//	loadFloat(sync->vy);
//	loadShort(sync->orientation);
//	sendUDP();
	tcpmutex.Unlock();
	return 0;
}

int TA3DSock::sendEvent(struct event* event){
	tcpmutex.Lock();
	loadByte('E');
	loadByte(event->type);
	loadByte(event->player1);
	loadByte(event->player2);
	sendTCP();
	tcpmutex.Unlock();
	return 0;
}


int TA3DSock::makeSpecial(struct chat* chat){
	if(tcpinbuf[0] != 'X'){
		Console->AddEntry("makeSpecial error: the data doesn't start with a 'X'");
		return -1;
	}
	if(tiremain == -1){
		return -1;
	}
	chat->from = ((uint16*)(tcpinbuf+1))[0];
	memcpy(chat->message,tcpinbuf+3,253);
	(chat->message)[252] = '\0';
	tibp = 0;
	tiremain = -1;

	return 0;
}

int TA3DSock::makeChat(struct chat* chat){
	if(tcpinbuf[0] != 'C'){
		Console->AddEntry("makeChat error: the data doesn't start with a 'C'");
		return -1;
	}
	if(tiremain == -1){
		return -1;
	}
	chat->from = ((uint16*)(tcpinbuf+1))[0];
	memcpy(chat->message,tcpinbuf+3,253);
	(chat->message)[252] = '\0';
	tibp = 0;
	tiremain = -1;

	return 0;
}

int TA3DSock::makeOrder(struct order* order){
	if(tcpinbuf[0] != 'O'){
		Console->AddEntry("makeOrder error: the data doesn't start with an 'O'");
		return -1;
	}
	if(tiremain == -1){
		return -1;
	}
	uint32_t temp;

	memcpy(&temp,tcpinbuf+1,4);
	order->timestamp = temp;

	memcpy(&temp,tcpinbuf+5,4);
	order->unit = nlSwapl(temp);

	order->command = tcpinbuf[9];

	memcpy(&temp,tcpinbuf+10,4);
	order->x = (float)nlSwapl(temp);

	memcpy(&temp,tcpinbuf+14,4);
	order->y = (float)nlSwapl(temp);

	memcpy(&temp,tcpinbuf+18,4);
	order->target = nlSwapl(temp);

	order->additional = tcpinbuf[19];
	
	tibp = 0;
	tiremain = -1;

	return 0;
}

int TA3DSock::makeSync(struct sync* sync){
//	if(udpinbuf[0] != 'S'){
//		Console->AddEntry("makeSync error: the data doesn't start with an 'S'");
//		return -1;
//	}
//	if(uiremain == -1){
//		return -1;
//	}
//	uint32_t temp;
//	uint16_t stemp;

//	memcpy(&temp,udpinbuf+1,4);
//	sync->timestamp = nlSwapl(temp);

//	memcpy(&temp,udpinbuf+5,4);
//	sync->unit = nlSwapl(temp);

//	memcpy(&temp,udpinbuf+9,4);
//	sync->x = (float)nlSwapl(temp);

//	memcpy(&temp,udpinbuf+13,4);
//	sync->y = (float)nlSwapl(temp);

//	memcpy(&temp,udpinbuf+17,4);
//	sync->vx = (float)nlSwapl(temp);

//	memcpy(&temp,udpinbuf+21,4);
//	sync->vy = (float)nlSwapl(temp);

//	memcpy(&stemp,udpinbuf+25,2);
//	sync->orientation = nlSwapl(stemp);
//	
//	uibp = 0;
//	uiremain = -1;

	return 0;
}

int TA3DSock::makeEvent(struct event* event){
	if(tcpinbuf[0] != 'E'){
		Console->AddEntry("makeEvent error: the data doesn't start with an 'E'");
		return -1;
	}
	if(tiremain == -1){
		return -1;
	}
	event->type = tcpinbuf[1];
	event->player1 = tcpinbuf[2];
	event->player2 = tcpinbuf[3];
	tibp = 0;
	tiremain = -1;
	return 0;
}

int TA3DSock::getFilePort()				// For file transfer, first call this one to get the port which allows us to grab the right thread and buffer
{
	if( tcpinbuf[0] != 'F' && tcpinbuf[0] != 'R' ) {
		Console->AddEntry("getFilePort error: the data doesn't start with an 'F' or an 'R'");
		return -1;
		}
	if(tiremain == -1)
		return -1;
	return *((uint16*)(tcpinbuf+1));
}

int TA3DSock::getFileData(byte *buffer)	// Fill the buffer with the data and returns the size of the paquet
{
	if( tcpinbuf[0] != 'F' && tcpinbuf[0] != 'R' ) {
		Console->AddEntry("getFilePort error: the data doesn't start with an 'F' or an 'R'");
		return -1;
		}
	if(tiremain == -1)
		return -1;
	int size = tibp - 3;
	if( buffer )
		memcpy( buffer, tcpinbuf + 3, size );

	tibp = 0;
	tiremain = -1;
	
	return size;
}

