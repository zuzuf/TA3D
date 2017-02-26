#include <stdafx.h>
#include <logs/logs.h>
#include "socket.udp.h"
#include <QUdpSocket>

namespace TA3D
{

    SocketUDP::SocketUDP()
    {
        sock = NULL;
        checked = false;
    }

    SocketUDP::~SocketUDP()
    {
        close();
    }

    bool SocketUDP::isOpen() const
    {
        return sock != NULL;
    }

    void SocketUDP::open(const QString &hostname, uint16 port)
    {
        MutexLocker locker(pMutex);
        close();

        this->hostname = hostname;
        this->port = port;
        sock = new QUdpSocket();
        sock->bind(QHostAddress(hostname), port);
        if (!sock->isOpen())
        {
            LOG_ERROR(LOG_PREFIX_NET << "error opening UDP socket : " << sock->errorString());
            return;
        }
    }

    void SocketUDP::close()
    {
        MutexLocker locker(pMutex);
        if (sock)
            delete sock;
        sock = NULL;
        checked = false;
    }

    void SocketUDP::send(const QByteArray &str)
    {
        send(str.data(), str.size());
    }

    void SocketUDP::send(const char *data, int size)
    {
        if (!sock)  return;

        if (sock->writeDatagram(data, size, hostname, port) == -1)
        {
            LOG_ERROR(LOG_PREFIX_NET_SOCKET << "error writing UDP datagram : " << sock->errorString());
            close();
        }
	}

    int SocketUDP::recv(char *data, int size)
    {
        if (!sock)  return 0;

		try
		{

            size = sock->readDatagram(data, size, &remote_peer);
            if (size == -1)
			{
                LOG_ERROR(LOG_PREFIX_NET << "error receiving data from UDP socket : " << sock->errorString());
				close();
				return 0;
			}

            return size;
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
			return 0;
		}
	}

    void SocketUDP::check(uint32 msec)
    {
		try
		{
            if (sock)
			{
                sock->waitForReadyRead(msec);
				checked = true;
			}
			else
                QThread::msleep(msec);
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
		}
	}

	bool SocketUDP::ready()
    {
		try
		{
            if (sock && checked)
                return sock->hasPendingDatagrams();
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
		}
		return false;
	}

    QString SocketUDP::getRemoteIPstr() const
    {
        return remote_peer.toString();
    }

    uint32 SocketUDP::getRemoteIP() const
    {
        return remote_peer.toIPv4Address();
    }

    QString SocketUDP::getIPstr() const
    {
        return hostname.toString();
    }

    uint32 SocketUDP::getIP() const
    {
        return hostname.toIPv4Address();
    }

    uint16 SocketUDP::getPort() const
    {
        return port;
    }
}
