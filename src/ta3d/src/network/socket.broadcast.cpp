#include <stdafx.h>
#include "socket.broadcast.h"

namespace TA3D
{

    void SocketBroadCast::open(const QString& /*hostname*/, uint16 port)
    {
        open(port);
    }

    void SocketBroadCast::open(uint16 port)
    {
        SocketUDP::open( "255.255.255.255", port );
    }
}
