#ifndef __SOCKET_TCP_H__
#define __SOCKET_TCP_H__

# include <misc/string.h>
#include "socket.h"
#include <threads/thread.h>
#include <deque>
#include <zlib.h>

namespace TA3D
{

    class SocketTCP : public Socket
    {
    protected:
        TCPsocket           sock;
        SDLNet_SocketSet    set;
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
        void open(uint16 port);
        SocketTCP *accept();

        /*virtual*/ void check(uint32 msec);
		/*virtual*/ bool ready();

        /*virtual*/ void send(const QString &str);
        /*virtual*/ void send(const char *data, int size);
        /*virtual*/ int recv(char *data, int size);

		QString getLine();
	private:
		static volatile bool forceFlush;
	public:
		static void enableFlush();
		static void disableFlush();
    };
}

#endif
