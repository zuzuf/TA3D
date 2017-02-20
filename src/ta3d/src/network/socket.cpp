#include <stdafx.h>
#include <logs/logs.h>
#include "socket.h"

namespace TA3D
{

	uint16 Socket::getPort() const
	{
		return IP.port;
	}

	IPaddress Socket::getIP_sdl() const
	{
		return IP;
	}

	QString Socket::getIPstr() const
	{
        return QString("%1.%2.%3.%4").arg(IP.host & 0xFF).arg((IP.host >> 8) & 0xFF).arg((IP.host >> 16) & 0xFF).arg(IP.host >> 24);
	}

	uint32 Socket::getIP() const
	{
		return IP.host;
	}

	QString Socket::getString()
	{
		MutexLocker locker(pMutex);
		char buf[2049];
		const int size = recv(buf, 2048);
		if (size > 0)
            return QString::fromUtf8(buf, (uint32)strlen(buf));
		return QString();
	}
}
