#ifndef __SOCKET_TCP_H__
#define __SOCKET_TCP_H__

# include <misc/string.h>
#include "socket.h"
#include <threads/thread.h>
#include <deque>
#include <zlib.h>

class QTcpSocket;
class QTcpServer;

namespace TA3D
{

    class SocketTCP : public Socket
    {
    protected:
        QTcpSocket          *sock;
        QTcpServer          *serv;
        bool                checked;
        bool                nonBlockingMode;
		// Do we have to use compression ? (disabled by default)
		bool				compression;
		z_streamp			zSend;
		z_streamp			zRecv;
		byte				*sendBuf;
		byte				*recvBuf;
		uint32				bytesSent;
		uint32				bytesProcessed;
	public:
		SocketTCP(bool enableCompression = false);
        virtual ~SocketTCP();

        void setNonBlockingMode(bool mode);

        void reset();

        /*virtual*/ bool isOpen() const;

        /*virtual*/ void open(const QString &hostname, uint16 port);
        /*virtual*/ void close();
        void open(uint16 port); // Server mode
        SocketTCP *accept();

        /*virtual*/ void check(uint32 msec);
		/*virtual*/ bool ready();

        /*virtual*/ void send(const QByteArray &str);
        /*virtual*/ void send(const char *data, int size);
        /*virtual*/ int recv(char *data, int size);

        /*virtual*/ QString getIPstr() const;
        /*virtual*/ uint32 getIP() const;
        /*virtual*/ uint16 getPort() const;

        QByteArray getLine();
	private:
		static volatile bool forceFlush;
	public:
		static void enableFlush();
		static void disableFlush();
    };
}

#endif
