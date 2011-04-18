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

	String Socket::getIPstr() const
	{
		return String() << (IP.host & 0xFF) << "." << ((IP.host >> 8) & 0xFF) << "." << ((IP.host >> 16) & 0xFF) << "."
			<< (IP.host >> 24);
	}

	uint32 Socket::getIP() const
	{
		return IP.host;
	}

	String Socket::getString()
	{
		MutexLocker locker(pMutex);
		char buf[2049];
		const int size = recv(buf, 2048);
		if (size > 0)
			return String(buf, (uint32)strlen(buf));
		return String();
	}
}
