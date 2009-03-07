#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL/SDL_net.h>
#include "../threads/thread.h"

namespace TA3D
{

    class Socket : public ObjectSync
    {
    protected:
        IPaddress  IP;
    public:
        virtual ~Socket() {};

        virtual bool isOpen() const = 0;

        virtual void open(const std::string &hostname, uint16 port) = 0;
        virtual void close() = 0;

        virtual void send(const char *data, int size) = 0;
        virtual int recv(char *data, int size) = 0;
        String getString();

        virtual void check(uint32 msec) = 0;
        virtual bool ready() const = 0;

        virtual IPaddress getIP_sdl() const;
        virtual std::string getIPstr() const;
        virtual uint32 getIP() const;
        virtual uint16 getPort() const;
    };

}
#endif
