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

#include <QtEndian>
#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include <misc/math.h>
#include <misc/paths.h>
#include <logs/logs.h>
#include <QFile>

namespace TA3D
{

    QFile &dump_file()
    {
        static QFile file(TA3D::Paths::Logs + "net.dump");
        static bool b_init = true;
        if (file.isOpen() && b_init)
        {
            file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            b_init = false;
        }
        return file;
    }

	chat* strtochat(struct chat *chat_msg, QString msg)
	{
		if (chat_msg == NULL)
			return chat_msg;
		memset( chat_msg->message, 0, 253 );
        memcpy( chat_msg->message, msg.toStdString().c_str(), Math::Min(253, (int)msg.size() + 1));
		return chat_msg;
	}

	QString chattostr(struct chat *chat_msg)
	{
		if (chat_msg == NULL)
            return QString();
        return QString::fromStdString(std::string(chat_msg->message, 253).c_str());   // Make sure it represents a null terminated string
	}

	TA3DSock::TA3DSock() : tcpsock(true)		// Enable compression
	{
		obp = 0;
		tibp = 0;
		tiremain = -1;
	}

	int TA3DSock::open(const QString &hostname, uint16 port)
	{
		tcpsock.open(hostname, port);
		if(!tcpsock.isOpen())
			return -1;
		return 0;
	}


	int TA3DSock::open(uint16 port)
	{
		tcpsock.open(port);
		if(!tcpsock.isOpen())
			return -1;
		return 0;
	}

	int TA3DSock::accept(TA3DSock** sock)
	{
		tcpsock.check(0);
		SocketTCP *newSock = tcpsock.accept();

		if (newSock)
		{
			(*sock) = new TA3DSock();

			(*sock)->tcpsock = *newSock;
			newSock->reset();

			if(!(*sock)->tcpsock.isOpen())
			{
				delete *sock;
				*sock = NULL;
				return -1;
			}
			return 0;
		}

		return -1;
	}

	int TA3DSock::accept(TA3DSock** sock,int timeout)
	{
		tcpsock.check(timeout);
		SocketTCP *newSock = tcpsock.accept();

		if (newSock)
		{
			(*sock) = new TA3DSock();

			(*sock)->tcpsock = *newSock;
			newSock->reset();

			if(!(*sock)->tcpsock.isOpen())
			{
				delete *sock;
				*sock = NULL;
				return -1;
			}
			return 0;
		}

		return -1;
	}

	int TA3DSock::isOpen()
	{
		return tcpsock.isOpen();
	}

	void TA3DSock::close()
	{
		if (tcpsock.isOpen())
			tcpsock.close();
	}



	//byte shuffling
	void TA3DSock::putLong(uint32_t x)
	{
        qToLittleEndian(x, outbuf + obp);
		obp += 4;
	}

	void TA3DSock::putShort(uint16_t x)
	{
        qToLittleEndian(x, outbuf + obp);
		obp += 2;
	}

	void TA3DSock::putByte(uint8_t x)
	{
		memcpy(outbuf + obp, &x, 1);
		obp += 1;
	}

    void TA3DSock::putString(const char* x)
	{
		const size_t n = strlen(x);
		if(n < size_t(TA3DSOCK_BUFFER_SIZE - obp - 1))
		{
			memcpy(outbuf + obp, x, n);
			obp += (int)n;
		}
		else
		{
			memcpy(outbuf + obp, x, TA3DSOCK_BUFFER_SIZE - obp - 1);
			obp += TA3DSOCK_BUFFER_SIZE - obp - 1;
		}
		putByte('\0');
	}

	void TA3DSock::putFloat(float x)
	{
        qToLittleEndian(x, outbuf + obp);
		obp += 4;
	}

	uint32 TA3DSock::getLong()	//uint32
	{
        const uint32 result = qFromLittleEndian<quint32>(tcpinbuf + tibrp);
		tibrp += 4;
		return result;
	}

	uint16 TA3DSock::getShort()
	{
        const uint16 result = qFromLittleEndian<quint16>(tcpinbuf + tibrp);
		tibrp += 2;
		return result;
	}

	byte TA3DSock::getByte()
	{
		byte result = *((byte*)(tcpinbuf+tibrp));
		tibrp ++;
		return result;
	}

	void TA3DSock::getQString(char* x)
	{
		while ((*x = *((char*)(tcpinbuf+tibrp))))
		{
			++tibrp;
			++x;
		}
		++tibrp;
	}

	void TA3DSock::getBuffer(char* x, int size)
	{
		memcpy( x, tcpinbuf + tibrp, size );
		tibrp += size;
	}

	float TA3DSock::getFloat()
	{
        const float result = qFromLittleEndian<float>(tcpinbuf + tibrp);
		tibrp += 4;
		return result;
	}


	void TA3DSock::send(const byte *data, int size)
	{
		tcpmutex.lock();

		const uint16 length = (uint16)size;
		tcpsock.send( (const char*)&length, 2 );
		tcpsock.send( (const char*)data, size );
        if (dump_file().isOpen())
		{
            dump_file().write((const char*)data, size);
            dump_file().flush();
		}

		tcpmutex.unlock();
	}

	void TA3DSock::send()
	{
		tcpmutex.lock();

		const uint16 length = (uint16)obp;
		tcpsock.send((const char*)&length, 2);
		tcpsock.send(outbuf, obp);
        if (dump_file().isOpen())
		{
            dump_file().write(outbuf, obp);
            dump_file().flush();
		}
		obp = 0;

		tcpmutex.unlock();
	}

	void TA3DSock::recv()
	{
		if(!tiremain)
			return;

		if (tiremain == -1)
			tibp = 0;

		int p = tcpsock.recv( tcpinbuf + tibp, tiremain == -1 ? 2 : tiremain );
		if( p <= 0 && tiremain <= 0 )
		{
            QThread::msleep(1);
			tiremain = -1;
			return;
		}
		if (tiremain == -1)
		{
			tiremain = *((uint16*)tcpinbuf);
			tibp = 0;
		}
		else
		{
			tiremain -= p;
			tibp += p;
		}
	}


	void TA3DSock::pumpIn()
	{
		recv();
	}

	char TA3DSock::getPacket()
	{
		if(tiremain != 0)
			return 0;
		return tcpinbuf[0];
	}

	void TA3DSock::cleanPacket()
	{
		if(tiremain<=0)
		{
			tcpinbuf[tibp] = 0;
			printf("tcpinbuf = '%s'\n", tcpinbuf);
			tibp = 0;
			tiremain = -1;
		}
	}



	void TA3DSock::check(int time)
	{
		tcpsock.check( time );
	}


	int TA3DSock::sendSpecial(struct chat* chat, bool all)
	{
		tcpmutex.lock();
		if( all )
			putByte('A');
		else
			putByte('X');
		putShort(chat->from);
        putString(chat->message);
		send();
		tcpmutex.unlock();
		return 0;
	}

	int TA3DSock::sendChat(struct chat* chat)
	{
		tcpmutex.lock();
		putByte('C');
		putShort(chat->from);
        putString(chat->message);
		send();
		tcpmutex.unlock();
		return 0;
	}

	int TA3DSock::sendSync(struct sync* sync)
	{
		tcpmutex.lock();

		putByte('S');
		putLong(sync->timestamp);
		putShort(sync->unit);
		putByte(sync->mask);
		if (sync->mask & SYNC_MASK_X)
			putFloat(sync->x);
		if (sync->mask & SYNC_MASK_Y)
			putFloat(sync->y);
		if (sync->mask & SYNC_MASK_Z)
			putFloat(sync->z);

		if (sync->mask & SYNC_MASK_HP)
			putShort(sync->hp);
		if (sync->mask & SYNC_MASK_VX)
			putFloat(sync->vx);
		if (sync->mask & SYNC_MASK_VZ)
			putFloat(sync->vz);
		if (sync->mask & SYNC_MASK_O)
			putShort(sync->orientation);
		if (sync->mask & SYNC_MASK_BPL)
			putByte(sync->build_percent_left);
		putByte(sync->flags);

		send();

		tcpmutex.unlock();
		return 0;
	}

	int TA3DSock::sendPing()
	{
		tcpmutex.lock();
		putByte('P');
		putByte(0);
		send();
		tcpmutex.unlock();
		return 1; // TODO Check this value is correct
	}

	int TA3DSock::sendPong()
	{
		tcpmutex.lock();
		putByte('p');
		putByte(0);
		send();
		tcpmutex.unlock();
		return 1; // TODO Check this value is correct
	}

	int TA3DSock::sendEvent(struct event* event)
	{
		tcpmutex.lock();
		putByte('E');
		putByte(event->type);
		switch( event->type )
		{
			case EVENT_UNIT_NANOLATHE:
				putShort(event->opt1);
				putShort(event->opt2);
				putLong(event->opt3);
				if( event->opt2 & 2 )			// It's a feature, so we send its coordinates, not its index since we cannot sync it
					putLong(event->opt4);
				break;
			case EVENT_FEATURE_CREATION:
				putLong(event->opt3);
				putLong(event->opt4);
				putFloat(event->x);
				putFloat(event->y);
				putFloat(event->z);
                putString((const char*)(event->str));
				break;
			case EVENT_FEATURE_DEATH:
			case EVENT_FEATURE_FIRE:
				putLong(event->opt3);
				putLong(event->opt4);
				break;
			case EVENT_SCRIPT_SIGNAL:
				putShort(event->opt1);
				putShort(event->opt2);
				break;
			case EVENT_UNIT_EXPLODE:
				putShort(event->opt1);
				putShort(event->opt2);
				putFloat(event->x);
				putFloat(event->y);
				putFloat(event->z);
				break;
			case EVENT_DRAW:
				putShort(event->opt1);
				putFloat(event->x);
				putFloat(event->y);
				putFloat(event->z);
				putLong(event->opt3);
                putString((const char*)(event->str));
				break;
			case EVENT_PRINT:
				putShort(event->opt1);
				putFloat(event->x);
				putFloat(event->y);
                putString((const char*)(event->str));
				break;
			case EVENT_PLAY:
				putShort(event->opt1);
                putString((const char*)(event->str));
				break;
			case EVENT_CLS:
				putShort(event->opt1);
			case EVENT_CLF:
			case EVENT_INIT_RES:
				break;
			case EVENT_CAMERA_POS:
				putShort(event->opt1);
				putFloat(event->x);
				putFloat(event->z);
				break;
			case EVENT_UNIT_PARALYZE:
			case EVENT_UNIT_DAMAGE:
				putShort(event->opt1);
				putShort(event->opt2);
				break;
			case EVENT_WEAPON_CREATION:
				putShort(event->opt1);
				putShort(event->opt2);
				putLong(event->opt3);
				putLong(event->opt4);
				putByte(event->opt5);
				putFloat(event->x);
				putFloat(event->y);
				putFloat(event->z);
				putFloat(event->vx);
				putFloat(event->vy);
				putFloat(event->vz);
				putShort(event->dx);
				putShort(event->dy);
				putShort(event->dz);
                putString((const char*)(event->str));
				break;
			case EVENT_UNIT_SCRIPT:
				putShort(event->opt1);
				putByte((uint8)event->opt2);
				putByte((uint8)event->opt3);
				for (unsigned int i = 0 ; i < event->opt3 ; ++i)
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
		};
		send();
		tcpmutex.unlock();
		return 0;
	}


	int TA3DSock::makeSpecial(struct chat* chat)
	{
		if(tcpinbuf[0] != 'X' && tcpinbuf[0] != 'A')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with a 'X' or a 'A'");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		tibrp = 1;
		chat->from = getShort();
		getBuffer(chat->message,253);
		(chat->message)[252] = '\0';
		tibp = 0;
		tiremain = -1;

		return 0;
	}

	int TA3DSock::makeChat(struct chat* chat)
	{
		if (tcpinbuf[0] != 'C')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with a 'C'");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		tibrp = 1;
		chat->from = getShort();
		getBuffer(chat->message,253);
		(chat->message)[252] = '\0';
		tibp = 0;
		tiremain = -1;

		return 0;
	}

	int TA3DSock::makePing()
	{
		if (tcpinbuf[0] != 'P')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with a 'P'");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		tibp = 0;
		tiremain = -1;

		return 0;
	}

	int TA3DSock::makePong()
	{
		if (tcpinbuf[0] != 'p')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with a 'p'");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		tibp = 0;
		tiremain = -1;

		return 0;
	}

	int TA3DSock::makeSync(struct sync* sync)
	{
		if (tcpinbuf[0] != 'S')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with an 'S'. Impossible to synchronize");
			return -1;
		}
		if(tiremain == -1)
			return -1;
		tibrp = 1;

		sync->timestamp = getLong();
		sync->unit = getShort();
		sync->mask = getByte();
		if (sync->mask & SYNC_MASK_X)
			sync->x = getFloat();
		if (sync->mask & SYNC_MASK_Y)
			sync->y = getFloat();
		if (sync->mask & SYNC_MASK_Z)
			sync->z = getFloat();

		if (sync->mask & SYNC_MASK_HP)
			sync->hp = getShort();
		if (sync->mask & SYNC_MASK_VX)
			sync->vx = getFloat();
		if (sync->mask & SYNC_MASK_VZ)
			sync->vz = getFloat();
		if (sync->mask & SYNC_MASK_O)
			sync->orientation = getShort();
		if (sync->mask & SYNC_MASK_BPL)
			sync->build_percent_left = getByte();
		sync->flags = getByte();

		tibp = 0;
		tiremain = -1;

		return 0;
	}

	int TA3DSock::makeEvent(struct event* event)
	{
		if (tcpinbuf[0] != 'E')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with an 'E'. Impossible to make the event");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		tibrp = 1;
		event->type = getByte();

		switch (event->type)
		{
			case EVENT_UNIT_NANOLATHE:
				event->opt1 = getShort();
				event->opt2 = getShort();
				event->opt3 = getLong();
				if( event->opt2 & 2 )			// It's a feature, so we send its coordinates, not its index since we cannot sync it
					event->opt4 = getLong();
				break;
			case EVENT_FEATURE_CREATION:
				event->opt3 = getLong();
				event->opt4 = getLong();
				event->x = getFloat();
				event->y = getFloat();
				event->z = getFloat();
				getBuffer((char*)(event->str),128);
				break;
			case EVENT_FEATURE_DEATH:
			case EVENT_FEATURE_FIRE:
				event->opt3 = getLong();
				event->opt4 = getLong();
				break;
			case EVENT_SCRIPT_SIGNAL:
				event->opt1 = getShort();
				event->opt2 = getShort();
				break;
			case EVENT_UNIT_EXPLODE:
				event->opt1 = getShort();
				event->opt2 = getShort();
				event->x = getFloat();
				event->y = getFloat();
				event->z = getFloat();
				break;
			case EVENT_DRAW:
				event->opt1 = getShort();
				event->x = getFloat();
				event->y = getFloat();
				event->z = getFloat();
				event->opt3 = getLong();
				getBuffer((char*)(event->str),128);
				break;
			case EVENT_PRINT:
				event->opt1 = getShort();
				event->x = getFloat();
				event->y = getFloat();
				getBuffer((char*)(event->str),128);
				break;
			case EVENT_PLAY:
				event->opt1 = getShort();
				getBuffer((char*)(event->str),128);
				break;
			case EVENT_CLS:
				event->opt1 = getShort();
			case EVENT_CLF:
			case EVENT_INIT_RES:
				break;
			case EVENT_CAMERA_POS:
				event->opt1 = getShort();
				event->x = getFloat();
				event->z = getFloat();
				break;
			case EVENT_UNIT_PARALYZE:
			case EVENT_UNIT_DAMAGE:
				event->opt1 = getShort();
				event->opt2 = getShort();
				break;
			case EVENT_WEAPON_CREATION:
				event->opt1 = getShort();
				event->opt2 = getShort();
				event->opt3 = getLong();
				event->opt4 = getLong();
				event->opt5 = getByte();
				event->x = getFloat();
				event->y = getFloat();
				event->z = getFloat();
				event->vx = getFloat();
				event->vy = getFloat();
				event->vz = getFloat();
				event->dx = getShort();
				event->dy = getShort();
				event->dz = getShort();
				getBuffer((char*)(event->str),128);
				break;
			case EVENT_UNIT_SCRIPT:
				event->opt1 = getShort();
				event->opt2 = getByte();
				event->opt3 = getByte();
				for (unsigned int i = 0 ; i < event->opt3 ; ++i)
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
				getBuffer((char*)(event->str),128);
				break;
		}

		tibp = 0;
		tiremain = -1;
		return 0;
	}

	int TA3DSock::sendTick(uint32 tick, uint16 speed)
	{
		tcpmutex.lock();
		putByte('T');
		putLong(tick);
		putShort(speed);
		send();
		tcpmutex.unlock();
		return 0;
	}

	int TA3DSock::getFilePort()				// For file transfer, first call this one to get the port which allows us to grab the right thread and buffer
	{
		if( tcpinbuf[0] != 'F' && tcpinbuf[0] != 'R' )
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with an 'F' or an 'R'. Impossible to start the file transfer");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		return *((uint16*)(tcpinbuf+1));
	}

	int TA3DSock::getFileData(byte *buffer)	// Fill the buffer with the data and returns the size of the paquet
	{
		if (tcpinbuf[0] != 'F' && tcpinbuf[0] != 'R')
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with an 'F' or an 'R'. Impossible to get data for the file transfer");
			return -1;
		}
		if (tiremain == -1)
			return -1;
		int size = tibp - 3;
		if( buffer )
			memcpy( buffer, tcpinbuf + 3, size );
		tibp = 0;
		tiremain = -1;

		return size;
	}



} // namespace TA3D

