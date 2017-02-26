#include <stdafx.h>
#include <logs/logs.h>
#include "socket.tcp.h"
#include <QTcpSocket>
#include <QTcpServer>

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
        serv = NULL;
        sock = NULL;
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
        if (sock)
            delete sock;
        if (serv)
            delete serv;
        sock = NULL;
        serv = NULL;
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
        MutexLocker locker(pMutex);
        close();

        serv = new QTcpServer;
        if (!serv->listen(QHostAddress::Any, port))
        {
            LOG_ERROR(LOG_PREFIX_NET_SOCKET << "error creating TCP server socket: " << serv->errorString());
            delete serv;
            serv = NULL;
        }
    }

    void SocketTCP::open(const QString &hostname, uint16 port)
    {
        MutexLocker locker(pMutex);
        close();

        sock = new QTcpSocket();
        sock->connectToHost(hostname, port);
        if (!sock->waitForConnected())
        {
            LOG_ERROR(LOG_PREFIX_NET << "error opening TCP socket : " << sock->errorString());
            delete sock;
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
        if (serv)
            delete serv;
        serv = NULL;
        if (sock)
            delete sock;
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
            if (!serv->waitForNewConnection())
                return NULL;
            QTcpSocket *child = serv->nextPendingConnection();
			if (child == NULL)
				return NULL;
			SocketTCP *newSock = new SocketTCP(compression);
			newSock->sock = child;

			return newSock;
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
			return NULL;
		}
	}

    void SocketTCP::send(const QByteArray &str)
    {
        send(str.data(), str.size());
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
                    const int sent = sock->write((const char*)sendBuf, TCP_BUFFER_SIZE - zSend->avail_out);
					bytesSent += sent;
					if (sent < TCP_BUFFER_SIZE - int(zSend->avail_out))
					{
                        LOG_ERROR(LOG_PREFIX_NET << "error sending data to TCP socket : " << sock->errorString() << " (" << sent << " / " << size << ")");
						close();
						return;
					}
				}
			}
			else
			{
                const int sent = sock->write((const char*)data, size);
				bytesSent += sent;

				if (sent < size)
				{
                    LOG_ERROR(LOG_PREFIX_NET << "error sending data to TCP socket : " << sock->error() << " (" << sent << " / " << size<< ")");
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
                        const int n = sock->read((char*)pIn++, 1);
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
                            LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << sock->error());
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
                    if (zRecv->avail_out > 0 && !nonBlockingMode)
                        sock->waitForReadyRead();
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
                        const int n = sock->read(data, 1);
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
                            LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << sock->errorString());
							close();
							return pos;
						}
						check(0);
					}
					return pos;
				}

                const int n = sock->read(data, size);
				if (n < 0)
				{
                    LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << sock->errorString());
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
        if (sock)
        {
            if (!sock->waitForReadyRead(msec) && !sock->isOpen())
			{
                LOG_ERROR(LOG_PREFIX_NET_SOCKET << "socket error, disconnecting");
				close();
				return;
			}
			checked = true;
        }
        else
            QThread::msleep(msec);
    }

	bool SocketTCP::ready()
    {
        if (sock && checked)
            return sock->bytesAvailable() > 0;

        return false;
    }

    QByteArray SocketTCP::getLine()
	{
        QByteArray line;
		if (!ready())
			return line;
		while(this->isOpen())
		{
			char c;
			recv(&c, 1);
			if (c == '\n')
				break;
			if (c != '\r')
                line += c;
		}

		return line;
	}

    QString SocketTCP::getIPstr() const
    {
        if (!sock)
            return QString();
        return sock->peerAddress().toString();
    }

    uint32 SocketTCP::getIP() const
    {
        if (!sock)
            return 0;
        return sock->peerAddress().toIPv4Address();
    }

    uint16 SocketTCP::getPort() const
    {
        if (!sock)
            return 0;
        return sock->peerPort();
    }
}
