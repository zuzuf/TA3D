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


int TA3DSock::Open(char* hostname,char* port,int network){

	tcpsock.Open(hostname,port,SOCK_STREAM,network);

	if(!tcpsock.isOpen() ){
		if( tcpsock.isOpen() )
			printf("tcp open\n");
		if( udpin.isOpen() )
			printf("udpin open\n");
		if( udpout.isOpen() )
			printf("udpout open\n");
		//one of them didnt work... quit
		if( tcpsock.isOpen() )
			tcpsock.Close();
		if( udpin.isOpen() )
			udpin.Close();
		if( udpout.isOpen() )
			udpout.Close();
		return -1;
	}

//	udpin.Open(NULL,port,SOCK_DGRAM,network);
//	if(hostname)
//		udpout.Open(hostname,port,SOCK_DGRAM,network);
//	
//	if( !( tcpsock.isOpen() && udpin.isOpen() && (udpout.isOpen() || hostname == NULL) ) ){
//		//one of them didnt work... quit
//		tcpsock.Close();
//		udpin.Close();
//		udpout.Close();
//		return -1;
//	}

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
		if( (*sock)->tcpsock.isOpen() )
			printf("tcp open\n");
		if( (*sock)->udpin.isOpen() )
			printf("udpin open\n");
		if( (*sock)->udpout.isOpen() )
			printf("udpout open\n");
		//one of them didnt work... quit
		if( (*sock)->tcpsock.isOpen() )
			(*sock)->tcpsock.Close();
		if( (*sock)->udpin.isOpen() )
			(*sock)->udpin.Close();
		if( (*sock)->udpout.isOpen() )
			(*sock)->udpout.Close();
		delete sock;
		return -1;
	}

	//this is fishy....maybe not
//	(*sock)->udpin.Open(NULL,(*sock)->tcpsock.getService(),SOCK_DGRAM,(*sock)->tcpsock.getAF());
//	(*sock)->udpout.Open((*sock)->tcpsock.getNumber(),(*sock)->tcpsock.getService(),SOCK_DGRAM,(*sock)->tcpsock.getAF());

//	if(!((*sock)->tcpsock.isOpen() && (*sock)->udpin.isOpen() && (*sock)->udpout.isOpen()) ){
//		//one of them didnt work... quit
//		(*sock)->tcpsock.Close();
//		(*sock)->udpin.Close();
//		(*sock)->udpout.Close();
//		delete sock;
//		return -1;
//	}

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

	

	//this is fishy....maybe not
//	(*sock)->udpin.Open(NULL,(*sock)->tcpsock.getService(),SOCK_DGRAM,(*sock)->tcpsock.getAF());
//	(*sock)->udpout.Open((*sock)->tcpsock.getNumber(),(*sock)->tcpsock.getService(),SOCK_DGRAM,(*sock)->tcpsock.getAF());

//	if(!((*sock)->tcpsock.isOpen() && (*sock)->udpin.isOpen() && (*sock)->udpout.isOpen()) ){
	if(!(*sock)->tcpsock.isOpen() ){
		if( (*sock)->tcpsock.isOpen() )
			printf("tcp open\n");
		if( (*sock)->udpin.isOpen() )
			printf("udpin open\n");
		if( (*sock)->udpout.isOpen() )
			printf("udpout open\n");
		//one of them didnt work... quit
		if( (*sock)->tcpsock.isOpen() )
			(*sock)->tcpsock.Close();
		if( (*sock)->udpin.isOpen() )
			(*sock)->udpin.Close();
		if( (*sock)->udpout.isOpen() )
			(*sock)->udpout.Close();
		delete sock;
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
	if( udpin.isOpen() )
		udpin.Close();
	if( udpout.isOpen() )
		udpout.Close();
}



//byte shuffling
void TA3DSock::loadLong(uint32_t x){//uint32
	uint32_t temp;
	temp = htonl(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void TA3DSock::loadShort(uint16_t x){//uint16
	uint16_t temp;
	temp = htons(x);
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void TA3DSock::loadByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void TA3DSock::loadString(char* x){//null terminated
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

void TA3DSock::loadFloat(float x){
	uint32_t temp;
	temp = htonl((uint32_t)x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}


void TA3DSock::sendTCP(){
	int n = 0;
	while(n!=obp);
		n += tcpsock.Send(outbuf,obp);
	obp = 0;
}

void TA3DSock::sendUDP(){
	int n = 0;
	while(n!=obp);
		n += udpout.Send(outbuf,obp);
	obp = 0;
}

void TA3DSock::recvTCP(){
	if(tiremain == 0)
		return;
	else if(tiremain == -1){
		uint8_t remain;
		int p = tcpsock.Recv(&remain,1);//get new number
		if( p <= 0 ) {
			tiremain = -1;
			return;
			}
		if(remain == 0){
			uint16_t remain2;
			p = tcpsock.Recv(&remain2,2);//get big number
			if( p <= 0 )
				tiremain = -1;
			else
				tiremain = ntohs(remain2);
		}
		else if(remain>0) tiremain = remain;
		else Console->AddEntry("tcp packet error cannot determine size");
		return;
	}
	int n = 0;
	n = tcpsock.Recv(tcpinbuf+tibp,tiremain);
	if( n > 0 ) {
		tibp += n;
		tiremain -= n;
		}
}

void TA3DSock::recvUDP(){
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
		else Console->AddEntry("udp packet error cannot determine size");
		return;
	}
	int n = 0;
	n = udpin.Recv(udpinbuf+uibp,uiremain);
	if( n > 0 ) {
		uibp += n;
		uiremain -= n;
		}
}


void TA3DSock::pumpIn(){
	recvTCP();
//	recvUDP();
}

char TA3DSock::getPacket(){
	if(tiremain>0){
//		if(uiremain>0){
			return 0;
/*		}
		else{
			return udpinbuf[0];
		}*/
	}
	else{
		return tcpinbuf[0];
	}
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
	int tcp,udp;
	fd_set set;

	tcp = tcpsock.getFD();
//	udp = udpin.getFD();

	FD_ZERO(&set);
	FD_SET(tcp,&set);
//	FD_SET(udp,&set);
	
	struct timeval t;
	t.tv_sec = time/1000;
	t.tv_usec = 1000*(time%1000);

//	select(max(tcp,udp)+1,&set,NULL,NULL,&t);
	select(tcp+1,&set,NULL,NULL,&t);
	
	return 0;
}


int TA3DSock::sendSpecial(struct chat* chat){
	loadByte(3 + strlen( chat->message ) );
	loadByte('X');
	loadByte(chat->from + 1);
	loadString(chat->message);
//	loadByte('\0');
	sendTCP();
	return 0;
}

int TA3DSock::sendChat(struct chat* chat){
	loadByte(3 + strlen( chat->message ) );
	loadByte('C');
	loadByte(chat->from + 1);
	loadString(chat->message);
//	loadByte('\0');
	sendTCP();
	return 0;
}

int TA3DSock::sendOrder(struct order* order){
	loadByte(23);
	loadByte('O');
	loadLong(order->timestamp);
	loadLong(order->unit);
	loadByte(order->command);
	loadFloat(order->x);
	loadFloat(order->y);
	loadLong(order->target);
	loadByte(order->additional);
	sendTCP();
	return 0;
}

int TA3DSock::sendSync(struct sync* sync){
	loadByte(27);
	loadByte('S');
	loadLong(sync->timestamp);
	loadLong(sync->unit);
	loadFloat(sync->x);
	loadFloat(sync->y);
	loadFloat(sync->vx);
	loadFloat(sync->vy);
	loadShort(sync->orientation);
	sendUDP();
	return 0;
}

int TA3DSock::sendEvent(struct event* event){
	loadByte(4);
	loadByte('E');
	loadByte(event->type);
	loadByte(event->player1);
	loadByte(event->player2);
	sendTCP();
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
	chat->from = tcpinbuf[1] - 1;
//	printf("message = '%s'\n", tcpinbuf+2);
	memcpy(chat->message,tcpinbuf+2,253);
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
	chat->from = tcpinbuf[1] - 1;
	memcpy(chat->message,tcpinbuf+2,253);
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
	order->timestamp = ntohl(temp);

	memcpy(&temp,tcpinbuf+5,4);
	order->unit = ntohl(temp);

	order->command = tcpinbuf[9];

	memcpy(&temp,tcpinbuf+10,4);
	order->x = (float)ntohl(temp);

	memcpy(&temp,tcpinbuf+14,4);
	order->y = (float)ntohl(temp);

	memcpy(&temp,tcpinbuf+18,4);
	order->target = ntohl(temp);

	order->additional = tcpinbuf[19];
	
	tibp = 0;
	tiremain = -1;

	return 0;
}

int TA3DSock::makeSync(struct sync* sync){
	if(udpinbuf[0] != 'S'){
		Console->AddEntry("makeSync error: the data doesn't start with an 'S'");
		return -1;
	}
	if(uiremain == -1){
		return -1;
	}
	uint32_t temp;
	uint16_t stemp;

	memcpy(&temp,udpinbuf+1,4);
	sync->timestamp = ntohl(temp);

	memcpy(&temp,udpinbuf+5,4);
	sync->unit = ntohl(temp);

	memcpy(&temp,udpinbuf+9,4);
	sync->x = (float)ntohl(temp);

	memcpy(&temp,udpinbuf+13,4);
	sync->y = (float)ntohl(temp);

	memcpy(&temp,udpinbuf+17,4);
	sync->vx = (float)ntohl(temp);

	memcpy(&temp,udpinbuf+21,4);
	sync->vy = (float)ntohl(temp);

	memcpy(&stemp,udpinbuf+25,2);
	sync->orientation = ntohl(stemp);
	
	uibp = 0;
	uiremain = -1;

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
