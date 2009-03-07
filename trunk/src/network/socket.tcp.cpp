#include "../stdafx.h"
#include "../logs/logs.h"
#include "socket.tcp.h"

namespace TA3D
{

    SocketTCP::SocketTCP()
    {
        sock = NULL;
        set = NULL;
        checked = false;
        nonBlockingMode = false;
    }

    SocketTCP::~SocketTCP()
    {
        close();
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
        if (mode)
            start();
    }


    bool SocketTCP::isOpen() const
    {
        return sock != NULL;
    }

    void SocketTCP::open(uint16 port)
    {
        open("", port);
    }

    void SocketTCP::open(const std::string &hostname, uint16 port)
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
    }

    void SocketTCP::close()
    {
        MutexLocker locker(pMutex);
        if (sock)
            SDLNet_TCP_Close(sock);
        if (set)
            SDLNet_FreeSocketSet(set);
        set = NULL;
        sock = NULL;
        checked = false;
    }

    SocketTCP *SocketTCP::accept()
    {
        TCPsocket child = SDLNet_TCP_Accept(sock);
        if (child == NULL)
            return NULL;
        SocketTCP *newSock = new SocketTCP();
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

    void SocketTCP::send(const char *data, int size)
    {
        if (!sock)  return;

        int sent = SDLNet_TCP_Send(sock, data, size);

        if (sent < size)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error sending data to TCP socket : " << SDLNet_GetError() << " (" << sent << " / " << size<< ")");
            close();
        }
    }

    int SocketTCP::recv(char *data, int size)
    {
        if (!sock)  return -1;

        if (nonBlockingMode)
        {
            mBuffer.lock();

            int pos = 0;
            while(pos < size && !buffer.empty())
            {
                *data++ = buffer.front();
                buffer.pop_front();
                pos++;
            }

            mBuffer.unlock();

            return pos;
        }

        int n = SDLNet_TCP_Recv(sock, data, size);
        if (n < 0)
        {
            LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << SDLNet_GetError());
            close();
            return -1;
        }

        return n;
    }

    void SocketTCP::check(uint32 msec)
    {
        if (set)
        {
            SDLNet_CheckSockets(set, msec);
            checked = true;
        }
        else
            SDL_Delay(msec);
    }

    bool SocketTCP::ready() const
    {
        if (set && sock && checked)
            return SDLNet_SocketReady(sock);
        return false;
    }

    void SocketTCP::proc(void* param)
    {
        if (!nonBlockingMode)   return;

        char c;

        while(isOpen())
        {
            int n = SDLNet_TCP_Recv(sock, &c, 1);
            if (n < 0)
            {
                LOG_ERROR(LOG_PREFIX_NET << "error receiving data from TCP socket : " << SDLNet_GetError());
                close();
                return;
            }

            if (n == 1)
            {
                mBuffer.lock();
                buffer.push_back(c);
                mBuffer.unlock();
            }
        }
    }

    void SocketTCP::signalExitThread()
    {
        close();
    }
}
