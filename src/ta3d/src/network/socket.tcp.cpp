#include <stdafx.h>
#include <logs/logs.h>
#include "socket.tcp.h"

#define TCP_BUFFER_SIZE			32
#define TCP_COMPRESSION_LEVEL	6

namespace TA3D
{
	volatile bool SocketTCP::forceFlush = true;

	void SocketTCP::enableFlush()
	{
		forceFlush = true;
	}

	void SocketTCP::disableFlush()
	{
		forceFlush = false;
	}

	SocketTCP::SocketTCP(bool enableCompression)
    {
        sock = NULL;
        set = NULL;
        checked = false;
        nonBlockingMode = false;
		bytesSent = 0;
		bytesProcessed = 0;
		compression = enableCompression;

		if (compression)
		{
			zSend = new z_stream;
			zRecv = new z_stream;
			recvBuf = new byte[TCP_BUFFER_SIZE];
			sendBuf = new byte[TCP_BUFFER_SIZE];
			zSend->avail_out = TCP_BUFFER_SIZE;
			zSend->next_out = sendBuf;
			zSend->zalloc = Z_NULL;
			zSend->zfree = Z_NULL;
			zSend->opaque = Z_NULL;

			zRecv->avail_in = 0;
			zRecv->next_in = Z_NULL;
			zRecv->zalloc = Z_NULL;
			zRecv->zfree = Z_NULL;
			zRecv->opaque = Z_NULL;

			deflateInit(zSend, TCP_COMPRESSION_LEVEL);
			inflateInit(zRecv);

			zRecv->avail_in = 0;
			zRecv->next_in = recvBuf;
		}
		else
		{
			zSend = NULL;
			zRecv = NULL;
			sendBuf = NULL;
			recvBuf = NULL;
		}
    }

    SocketTCP::~SocketTCP()
    {
        close();
		if (compression)
		{
			deflateEnd(zSend);
			inflateEnd(zRecv);
			delete zSend;
			delete zRecv;
			zSend = zRecv = NULL;
			DELETE_ARRAY(sendBuf);
			DELETE_ARRAY(recvBuf);
		}
	}

    void SocketTCP::reset()
    {
        MutexLocker locker(pMutex);
        sock = NULL;
        set = NULL;
        checked = false;
        nonBlockingMode = false;
    }

    void SocketTCP::setNonBlockingMode(bool mode)
    {
        MutexLocker locker(pMutex);
        nonBlockingMode = mode;
    }


    bool SocketTCP::isOpen() const
    {
        return sock != NULL;
    }

    void SocketTCP::open(uint16 port)
    {
        open("", port);
    }

    void SocketTCP::open(const String &hostname, uint16 port)
    {
        MutexLocker locker(pMutex);
        close();

        SDLNet_ResolveHost( &IP, hostname.c_str(), port );
        sock = SDLNet_TCP_Open(&IP);
        if (sock == NULL)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error opening TCP socket : " << SDLNet_GetError());
            return;
        }
        set = SDLNet_AllocSocketSet(1);
        if (set == NULL)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error creating socket set : " << SDLNet_GetError());
            close();
            return;
        }
        if (SDLNet_TCP_AddSocket(set, sock) == -1)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error filling socket set : " << SDLNet_GetError());
            close();
            return;
        }

		if (compression)
		{
			zRecv->next_in = recvBuf;
			zRecv->avail_in = 0;
		}
	}

    void SocketTCP::close()
    {
		if (isOpen())
		{
			LOG_INFO(LOG_PREFIX_NET << "stats:");
			LOG_INFO(LOG_PREFIX_NET << "bytes sent: " << bytesSent);
			if (compression)
			{
				LOG_INFO(LOG_PREFIX_NET << "bytes processed: " << bytesProcessed);
				if (bytesProcessed > 0)
					LOG_INFO(LOG_PREFIX_NET << "ratio : " << bytesSent * 100 / bytesProcessed << "%");
			}
			bytesProcessed = bytesSent = 0;
		}

		MutexLocker locker(pMutex);
        if (set)
            SDLNet_FreeSocketSet(set);
        if (sock)
            SDLNet_TCP_Close(sock);
        set = NULL;
        sock = NULL;
        checked = false;

		if (compression)
		{
			deflateEnd(zSend);
			inflateEnd(zRecv);
			delete zSend;
			delete zRecv;
			DELETE_ARRAY(sendBuf);
			DELETE_ARRAY(recvBuf);

			zSend = new z_stream;
			zRecv = new z_stream;
			sendBuf = new byte[TCP_BUFFER_SIZE];
			recvBuf = new byte[TCP_BUFFER_SIZE];
			zSend->avail_out = TCP_BUFFER_SIZE;
			zSend->next_out = sendBuf;
			zSend->zalloc = Z_NULL;
			zSend->zfree = Z_NULL;
			zSend->opaque = Z_NULL;

			zRecv->avail_in = 0;
			zRecv->next_in = Z_NULL;
			zRecv->zalloc = Z_NULL;
			zRecv->zfree = Z_NULL;
			zRecv->opaque = Z_NULL;

			deflateInit(zSend, TCP_COMPRESSION_LEVEL);
			inflateInit(zRecv);

			zRecv->avail_in = 0;
			zRecv->next_in = recvBuf;
		}
		else
		{
			zSend = NULL;
			zRecv = NULL;
			sendBuf = NULL;
			recvBuf = NULL;
		}
	}

    SocketTCP *SocketTCP::accept()
    {
		try
		{
			TCPsocket child = SDLNet_TCP_Accept(sock);
			if (child == NULL)
				return NULL;
			SocketTCP *newSock = new SocketTCP(compression);
			newSock->sock = child;
			IPaddress *remote_addr = SDLNet_TCP_GetPeerAddress( child );
			if (remote_addr == NULL)
			{
				LOG_ERROR(LOG_PREFIX_NET << "error getting remote address : " << SDLNet_GetError());
				newSock->close();
				return NULL;
			}
			newSock->IP = *remote_addr;

			newSock->set = SDLNet_AllocSocketSet(1);
			if (newSock->set == NULL)
			{
				LOG_ERROR(LOG_PREFIX_NET << "error creating socket set : " << SDLNet_GetError());
				newSock->close();
				return NULL;
			}
			if (SDLNet_TCP_AddSocket(newSock->set, newSock->sock) == -1)
			{
				LOG_ERROR(LOG_PREFIX_NET << "error filling socket set : " << SDLNet_GetError());
				newSock->close();
				return NULL;
			}

			return newSock;
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
			return NULL;
		}
	}

    void SocketTCP::send(const String &str)
    {
		send(str.c_str(), int(str.size()));
    }

    void SocketTCP::send(const char *data, int size)
    {
        if (!sock)  return;

		try
		{
			if (compression)
			{
				zSend->next_in = (Bytef*)data;
				zSend->avail_in = size;
				zSend->avail_out = 0;
				bytesProcessed += size;
				const int flush = forceFlush ? Z_SYNC_FLUSH : Z_NO_FLUSH;
				while (zSend->avail_in > 0 || zSend->avail_out == 0)
				{
					zSend->next_out = sendBuf;
					zSend->avail_out = TCP_BUFFER_SIZE;
					deflate(zSend, flush);
					const int sent = SDLNet_TCP_Send(sock, sendBuf, TCP_BUFFER_SIZE - zSend->avail_out);
					bytesSent += sent;
					if (sent < TCP_BUFFER_SIZE - int(zSend->avail_out))
					{
						LOG_ERROR(LOG_PREFIX_NET << "error sending data to TCP socket : " << SDLNet_GetError() << " (" << sent << " / " << size << ")");
						close();
						return;
					}
				}
			}
			else
			{
				const int sent = SDLNet_TCP_Send(sock, data, size);
				bytesSent += sent;

				if (sent < size)
				{
					LOG_ERROR(LOG_PREFIX_NET << "error sending data to TCP socket : " << SDLNet_GetError() << " (" << sent << " / " << size<< ")");
					close();
				}
			}
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
		}
	}

	inline void zStreamError(int ret, char *msg)
	{
		if (ret != Z_OK)
		{
			if (msg)
				LOG_ERROR(LOG_PREFIX_NET << "error decompressing data from TCP socket : " << msg);
			switch(ret)
			{
			case Z_DATA_ERROR:
				LOG_ERROR(LOG_PREFIX_NET << "zlib: input data corrupt!");
				break;
			case Z_STREAM_ERROR:
				LOG_ERROR(LOG_PREFIX_NET << "zlib: stream structure inconsistent!");
				break;
			case Z_MEM_ERROR:
				LOG_ERROR(LOG_PREFIX_NET << "zlib: not enough memory!");
				break;
			case Z_BUF_ERROR:
//				LOG_ERROR(LOG_PREFIX_NET << "zlib: no progress possible!");
				break;
			};
		}
	}

    int SocketTCP::recv(char *data, int size)
    {
        if (!sock)  return -1;

		try
		{
			if (compression)
			{
				zRecv->avail_out = size;
				zRecv->next_out = (Bytef*)data;

				int ret = Z_OK;
				do
				{
					check(0);
					byte *pIn = zRecv->next_in + zRecv->avail_in;
					while(ready() && zRecv->avail_in < TCP_BUFFER_SIZE)
					{
						const int n = SDLNet_TCP_Recv(sock, pIn++, 1);
						if (n == 1)
							zRecv->avail_in++;
						else if (n == 0)
						{
							LOG_ERROR(LOG_PREFIX_NET << "connection closed by peer");
							close();
							return size - zRecv->avail_out;
						}
						else
						{
							ret = inflate(zRecv, Z_SYNC_FLUSH);
							zStreamError(ret, zRecv->msg);
							LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << SDLNet_GetError());
							close();
							return size - zRecv->avail_out;
						}
						check(0);
					}
					ret = inflate(zRecv, Z_NO_FLUSH);
					if (ret != Z_OK)
					{
						zStreamError(ret, zRecv->msg);
						if (ret != Z_BUF_ERROR)
						{
							LOG_DEBUG("in = " << zRecv->avail_in);
							LOG_DEBUG("out/size = " << zRecv->avail_out << "/" << size);
							LOG_ERROR(LOG_PREFIX_NET << "closing compressed socket because of decompression error");
							close();
							return size - zRecv->avail_out;
						}
					}

					if (zRecv->avail_in > 0)
						memmove(recvBuf, zRecv->next_in, zRecv->avail_in);
					zRecv->next_in = recvBuf;
				} while (!nonBlockingMode && zRecv->avail_out > 0 && ret != Z_BUF_ERROR);
				return size - zRecv->avail_out;
			}
			else
			{
				if (nonBlockingMode)
				{
					int pos = 0;
					check(0);
					while(ready() && pos < size)
					{
						const int n = SDLNet_TCP_Recv(sock, data, 1);
						if (n == 1)
						{
							data++;
							pos++;
						}
						else if (n == 0)
						{
							LOG_ERROR(LOG_PREFIX_NET << "connectio closed by peer");
							close();
							return pos;
						}
						else
						{
							LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << SDLNet_GetError());
							close();
							return pos;
						}
						check(0);
					}
					return pos;
				}

				const int n = SDLNet_TCP_Recv(sock, data, size);
				if (n < 0)
				{
					LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << SDLNet_GetError());
					close();
					return -1;
				}
				if (n == 0)
				{
					LOG_ERROR(LOG_PREFIX_NET << "connection closed by peer");
					close();
					return -1;
				}

				return n;
			}
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
			return -1;
		}
	}

    void SocketTCP::check(uint32 msec)
    {
        if (set)
        {
			try
			{
				SDLNet_CheckSockets(set, msec);
			}
			catch(std::exception &e)
			{
				LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
				close();
				return;
			}
			checked = true;
        }
        else
            SDL_Delay(msec);
    }

	bool SocketTCP::ready()
    {
		try
		{
			if (set && sock && checked)
				return SDLNet_SocketReady(sock);
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
		}

        return false;
    }

	String SocketTCP::getLine()
	{
		String line;
		if (!ready())
			return line;
		while(this->isOpen())
		{
			char c;
			recv(&c, 1);
			if (c == '\n')
				break;
			if (c != '\r')
				line << c;
		}

		return line;
	}
}
