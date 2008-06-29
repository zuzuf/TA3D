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
	if(uiremain != 0)
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
void UDPSock::putLong(uint32_t x){//uint32
	uint32_t temp;
	temp = nlSwapl( x );
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

void UDPSock::putShort(uint16_t x){//uint16
	uint16_t temp;
	temp = nlSwaps( x );
	memcpy(outbuf+obp,&temp,2);
	obp += 2;
}

void UDPSock::putByte(uint8_t x){//uint8
	memcpy(outbuf+obp,&x,1);
	obp += 1;
}

void UDPSock::putString(const char* x){//null terminated
	int n = strlen(x);
	if(n < UDPSOCK_BUFFER_SIZE - obp - 1 ){
		memcpy(outbuf+obp,x,n);
		obp+=n;
	}
	else{
		memcpy(outbuf+obp,x,UDPSOCK_BUFFER_SIZE - obp - 1);
		obp+=UDPSOCK_BUFFER_SIZE - obp - 1;
	}
	putByte('\0');
}	

void UDPSock::putFloat(float x){
	float temp;
	temp = nlSwapf(x);
	memcpy(outbuf+obp,&temp,4);
	obp += 4;
}

uint32 UDPSock::getLong()	//uint32
{
	uint32 result = nlSwapl( *((uint32*)(udpinbuf+uibrp)) );
	uibrp += 4;
	return result;
}

uint16 UDPSock::getShort()	//uint16
{
	uint16 result = nlSwaps( *((uint16*)(udpinbuf+uibrp)) );
	uibrp += 2;
	return result;
}

byte UDPSock::getByte()	//uint8
{
	byte result = *((byte*)(udpinbuf+uibrp));
	uibrp ++;
	return result;
}

void UDPSock::getString(char* x)	//null terminated
{
	while( *x = *((char*)(udpinbuf+uibrp)) ) {
		uibrp++;
		x++;
		}
	uibrp++;
}

void UDPSock::getBuffer(char* x, int size)
{
	memcpy( x, udpinbuf + uibrp, size );
	uibrp += size;
}

float UDPSock::getFloat()
{
	float result = nlSwapf( *((float*)(udpinbuf+uibrp)) );
	uibrp += 4;
	return result;
}


int UDPSock::sendSpecial(struct chat* chat, const std::string &address){
	udpmutex.Lock();

	putByte('X');
	putShort(chat->from);
	putString(chat->message);
	send( address );

	udpmutex.Unlock();
	return 0;
}

int UDPSock::sendEvent(struct event* event, const std::string &address){
	udpmutex.Lock();
	putByte('E');
	putByte(event->type);
	switch( event->type )
	{
	case EVENT_DRAW:
		putFloat(event->x);
		putFloat(event->y);
		putFloat(event->z);
		putLong(event->opt3);
		putString((const char*)(event->str));
		break;
	case EVENT_PRINT:
		putFloat(event->x);
		putFloat(event->y);
		putString((const char*)(event->str));
		break;
	case EVENT_PLAY:
		putString((const char*)(event->str));
		break;
	case EVENT_CLS:
	case EVENT_CLF:
	case EVENT_INIT_RES:
		break;
	case EVENT_CAMERA_POS:
		putShort(event->opt1);
		putFloat(event->x);
		putFloat(event->z);
		break;
	case EVENT_UNIT_SYNCED:
		putShort(event->opt1);
		putShort(event->opt2);
		putLong(event->opt3);
		break;
	case EVENT_UNIT_DAMAGE:
		putShort(event->opt1);
		putShort(event->opt2);
		break;
	case EVENT_WEAPON_CREATION:
		putShort(event->opt1);
		putShort(event->opt2);
		putFloat(event->x);
		putFloat(event->y);
		putFloat(event->z);
		putFloat(((real32*)(event->str))[0]);
		putFloat(((real32*)(event->str))[1]);
		putFloat(((real32*)(event->str))[2]);
		putLong(((sint16*)(event->str))[6]);
		putLong(((sint16*)(event->str))[7]);
		putLong(((sint16*)(event->str))[8]);
		putLong(((sint16*)(event->str))[9]);
		break;
	case EVENT_UNIT_SCRIPT:
		putShort(event->opt1);
		putShort(event->opt2);
		putLong(event->opt3);
		putLong(event->opt4);
		for( int i = 0 ; i < event->opt4 ; i++ )
			putLong(((sint32*)(event->str))[i]);
		break;
	case EVENT_UNIT_DEATH:
		putShort(event->opt1);
		break;
	case EVENT_UNIT_CREATION:
		printf("sending unit creation event (%s)\n", event->str);
		putShort(event->opt1);
		putShort(event->opt2);
		putFloat(event->x);
		putFloat(event->z);
		putString((const char*)(event->str));
		break;
	}
	send(address);
	udpmutex.Unlock();
	return 0;
}

int UDPSock::sendSync(struct sync* sync, const std::string &address){
	udpmutex.Lock();

	putByte('S');
	putLong(sync->timestamp);
	putShort(sync->unit);
	putFloat(sync->x);
	putFloat(sync->y);
	putFloat(sync->z);
	
	putShort(sync->hp);
	putFloat(sync->vx);
	putFloat(sync->vz);
	putShort(sync->orientation);
	putByte(sync->build_percent_left);
	putByte(sync->flags);

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

	uibrp = 1;
	
	sync->timestamp = getLong();
	sync->unit = getShort();
	sync->x = getFloat();
	sync->y = getFloat();
	sync->z = getFloat();

	sync->hp = getShort();
	sync->vx = getFloat();
	sync->vz = getFloat();
	sync->orientation = getShort();
	sync->build_percent_left = getByte();
	sync->flags = getByte();
	
	uibp = 0;
	uiremain = -1;
	return 0;
}

int UDPSock::makeEvent(struct event* event){
	if(udpinbuf[0] != 'E'){
		Console->AddEntry("makeEvent error: the data doesn't start with an 'E'");
		return -1;
	}
	if(uiremain == -1){
		return -1;
	}
	uibrp = 1;
	event->type = getByte();

	switch( event->type )
	{
	case EVENT_DRAW:
		event->x = getFloat();
		event->y = getFloat();
		event->z = getFloat();
		event->opt3 = getLong();
		getBuffer((char*)(event->str),24);
		break;
	case EVENT_PRINT:
		event->x = getFloat();
		event->y = getFloat();
		getBuffer((char*)(event->str),24);
		break;
	case EVENT_PLAY:
		getBuffer((char*)(event->str),24);
		break;
	case EVENT_CLS:
	case EVENT_CLF:
	case EVENT_INIT_RES:
		break;
	case EVENT_CAMERA_POS:
		event->opt1 = getShort();
		event->x = getFloat();
		event->z = getFloat();
		break;
	case EVENT_UNIT_SYNCED:
		event->opt1 = getShort();
		event->opt2 = getShort();
		event->opt3 = getLong();
		break;
	case EVENT_UNIT_DAMAGE:
		event->opt1 = getShort();
		event->opt2 = getShort();
		break;
	case EVENT_WEAPON_CREATION:
		event->opt1 = getShort();
		event->opt2 = getShort();
		event->x = getFloat();
		event->y = getFloat();
		event->z = getFloat();
		((real32*)(event->str))[0] = getFloat();
		((real32*)(event->str))[1] = getFloat();
		((real32*)(event->str))[2] = getFloat();
		((sint16*)(event->str))[6] = getLong();
		((sint16*)(event->str))[7] = getLong();
		((sint16*)(event->str))[8] = getLong();
		((sint16*)(event->str))[9] = getLong();
		break;
	case EVENT_UNIT_SCRIPT:
		event->opt1 = getShort();
		event->opt2 = getShort();
		event->opt3 = getLong();
		event->opt4 = getLong();
		for( int i = 0 ; i < event->opt4 ; i++ )
			((sint32*)(event->str))[i] = getLong();
		break;
	case EVENT_UNIT_DEATH:
		event->opt1 = getShort();
		break;
	case EVENT_UNIT_CREATION:
		event->opt1 = getShort();
		event->opt2 = getShort();
		event->x = getFloat();
		event->z = getFloat();
		getBuffer((char*)(event->str),24);
		break;
	}

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
	uibrp = 1;

	chat->from = getShort();
	getBuffer( chat->message, 253 );
	(chat->message)[252] = '\0';

	uibp = 0;
	uiremain = -1;

	return 0;
}

