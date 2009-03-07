#include "../stdafx.h"
#include "socket.h"

namespace TA3D
{

    uint16 Socket::getPort() const
    {
        return IP.port;
    }

    IPaddress Socket::getIP_sdl() const
    {
        return IP;
    }

    std::string Socket::getIPstr() const
    {
        return TA3D::format("%d.%d.%d.%d", (IP.host >> 24), (IP.host >> 16) & 0xFF, (IP.host >> 8) & 0xFF, IP.host & 0xFF);
    }

    uint32 Socket::getIP() const
    {
        return IP.host;
    }

    String Socket::getString()
    {
        char buf[2049];
        int size = recv(buf, 2048);
        return String(buf, size);
    }
}
