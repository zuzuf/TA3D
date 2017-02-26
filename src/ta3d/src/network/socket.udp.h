#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

# include <misc/string.h>
# include "socket.h"
# include <QHostAddress>

class QUdpSocket;

namespace TA3D
{

    class SocketUDP : public Socket
    {
    protected:
        QUdpSocket          *sock;
        bool                checked;
        uint16              port;
        QHostAddress        hostname;
        QHostAddress        remote_peer;
    public:
        SocketUDP();
        virtual ~SocketUDP();

        virtual bool isOpen() const;

        virtual void open(const QString &hostname, uint16 port);
        virtual void close();

        virtual void check(uint32 msec);
		virtual bool ready();

        virtual void send(const QByteArray &str);
        virtual void send(const char *data, int size);
        virtual int recv(char *data, int size);

        QString getRemoteIPstr() const;
        uint32 getRemoteIP() const;

        virtual QString getIPstr() const;
        virtual uint32 getIP() const;
        virtual uint16 getPort() const;
    };
}
#endif
