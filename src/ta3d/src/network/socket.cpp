#include <stdafx.h>
#include <logs/logs.h>
#include "socket.h"

namespace TA3D
{
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
