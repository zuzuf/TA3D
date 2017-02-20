#ifndef __SOCKET_BROADCAST_H__
#define __SOCKET_BROADCAST_H__

# include <misc/string.h>
# include "socket.udp.h"

namespace TA3D
{

    class SocketBroadCast : public SocketUDP
    {
    public:
        virtual void open(const QString &hostname, uint16 port);
        virtual void open(uint16 port);
    };
}
#endif
