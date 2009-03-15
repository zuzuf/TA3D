#ifndef NETCLIENT_H
#define NETCLIENT_H

#include "../misc/string.h"
#include "socket.tcp.h"

namespace TA3D
{
    class NetClient : ObjectSync
    {
    public:
        enum NetState { CONNECTING,
                        CONNECTED,
                        DISCONNECTED };
    private:
        String          login;
        NetState        state;
        String::List    messages;
        String::Vector  peerList;
        SocketTCP       sock;
        char            *buffer;
        int             buffer_pos;
    public:
        NetClient();
        ~NetClient();

        void            disconnect();
        void            connect(const String &server, const uint16 port, const String &login, const String &password, bool bRegister = false);
        NetState        getState();
        bool            messageWaiting();
        String          getNextMessage();
        String::Vector  getPeerList();
        void            clearMessageQueue();
        void            sendMessage(const String &msg);
        void            receive();

    private:
        void            processMessage(const String &msg);
    };
}

#endif // NETCLIENT_H
