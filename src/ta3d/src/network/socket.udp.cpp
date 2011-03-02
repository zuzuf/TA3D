#include <stdafx.h>
#include <logs/logs.h>
#include "socket.udp.h"

namespace TA3D
{

    SocketUDP::SocketUDP()
    {
        sock = NULL;
        set = NULL;
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

    void SocketUDP::open(const String &hostname, uint16 port)
    {
        MutexLocker locker(pMutex);
        close();

        SDLNet_ResolveHost( &IP, hostname.c_str(), port );
        sock = SDLNet_UDP_Open(port);
        if (sock == NULL)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error opening UDP socket : " << SDLNet_GetError());
            return;
        }
        set = SDLNet_AllocSocketSet(1);
        if (set == NULL)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error creating socket set : " << SDLNet_GetError());
            close();
            return;
        }
        if (SDLNet_UDP_AddSocket(set, sock) == -1)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error filling socket set : " << SDLNet_GetError());
            close();
            return;
        }
    }

    void SocketUDP::close()
    {
        MutexLocker locker(pMutex);
        if (set)
            SDLNet_FreeSocketSet(set);
        if (sock)
            SDLNet_UDP_Close(sock);
        set = NULL;
        sock = NULL;
        checked = false;
    }

    void SocketUDP::send(const String &str)
    {
        send(str.c_str(), str.size());
    }

    void SocketUDP::send(const char *data, int size)
    {
        if (!sock)  return;

		try
		{
			UDPpacket packet;

			packet.channel = -1;
			packet.data = (Uint8*) data;
			packet.len = size;
			packet.maxlen = size;
			packet.address = IP;

			SDLNet_UDP_Send(sock, packet.channel, &packet);
		}
		catch(std::exception &e)
		{
			LOG_ERROR(LOG_PREFIX_NET_SOCKET << "exception caught : " << e.what());
			close();
		}
	}

    int SocketUDP::recv(char *data, int size)
    {
        if (!sock)  return 0;

		try
		{
			UDPpacket packet;
			packet.data = (Uint8*) data;
			packet.len = 0;
			packet.maxlen = size;

			if (SDLNet_UDP_Recv(sock, &packet) == -1)
			{
				LOG_ERROR(LOG_PREFIX_NET << "error receiving data from UDP socket : " << SDLNet_GetError());
				close();
				return 0;
			}

			remoteIP = packet.address;

			return packet.len;
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
			if (set)
			{
				SDLNet_CheckSockets(set, msec);
				checked = true;
			}
			else
				SDL_Delay(msec);
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

    IPaddress SocketUDP::getRemoteIP_sdl() const
    {
        return remoteIP;
    }

    String SocketUDP::getRemoteIPstr() const
    {
		return String() << (remoteIP.host & 0xFF) << '.' << ((remoteIP.host >> 8) & 0xFF) << '.' << ((remoteIP.host >> 16) & 0xFF) << '.' << (remoteIP.host >> 24);
    }

    uint32 SocketUDP::getRemoteIP() const
    {
        return remoteIP.host;
    }
}
