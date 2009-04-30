#ifndef __SOCKET_BROADCAST_H__
#define __SOCKET_BROADCAST_H__

#include "socket.udp.h"

namespace TA3D
{

    class SocketBroadCast : public SocketUDP
    {
    public:
        /*virtual*/ void open(const std::string &hostname, uint16 port);
        /*virtual*/ void open(uint16 port);
    };
}
#endif
