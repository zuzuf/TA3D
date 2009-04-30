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
        String          server;
        uint16          port;
        String          login;
        String          password;
        NetState        state;
        String::List    messages;
        String::Vector  peerList;
        String::Vector  chanList;
        SocketTCP       sock;
        char            *buffer;
        int             buffer_pos;
        String          currentChan;
    public:
        NetClient();
        ~NetClient();

        void            disconnect();
        void            connect(const String &server, const uint16 port, const String &login, const String &password, bool bRegister = false);
        void            reconnect();
        NetState        getState();
        bool            messageWaiting();
        String          getNextMessage();
        String::Vector  getPeerList();
        String::Vector  getChanList();
        void            clearMessageQueue();
        void            sendMessage(const String &msg);
        void            changeChan(const String &chan);
        void            receive();
        void            sendChan(const String &msg);
        String          getLogin();
        String          getChan();

    private:
        void            processMessage(const String &msg);

    public:
        static NetClient *instance();
        static void destroyInstance();

    private:
        static NetClient *pInstance;
    };
}

#endif // NETCLIENT_H
