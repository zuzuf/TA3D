#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "netclient.h"
#include <algorithm>        // We need std::sort

#define BUFFER_SIZE     2048

namespace TA3D
{
    NetClient *NetClient::pInstance = NULL;

    NetClient *NetClient::instance()
    {
        if (!pInstance)
            pInstance = new NetClient;
        return pInstance;
    }

    void NetClient::destroyInstance()
    {
        if (pInstance)
            delete pInstance;
        pInstance = NULL;
    }

    NetClient::NetClient() : peerList(), login(), state( NetClient::DISCONNECTED ), messages(), sock()
    {
        buffer = new char[BUFFER_SIZE];
        buffer_pos = 0;
    }

    NetClient::~NetClient()
    {
        disconnect();
    }

    void NetClient::disconnect()
    {
        pMutex.lock();
        sendMessage("DISCONNECT");
        sock.close();
        state = DISCONNECTED;
        peerList.clear();
        buffer_pos = 0;

        pMutex.unlock();
    }

    NetClient::NetState NetClient::getState()
    {
        MutexLocker mLock(pMutex);
        return state;
    }

    String::Vector NetClient::getPeerList()
    {
        MutexLocker mLock(pMutex);
        return peerList;
    }

    bool NetClient::messageWaiting()
    {
        MutexLocker mLock(pMutex);
        return !messages.empty();
    }

    String NetClient::getNextMessage()
    {
        MutexLocker mLock(pMutex);
        String msg = messages.front();
        messages.pop_front();
        return msg;
    }

    void NetClient::sendMessage(const String &msg)
    {
        pMutex.lock();
        if (sock.isOpen())
            sock.send((msg + "\n").c_str(), msg.size() + 1);
        else
        {
            state = DISCONNECTED;
            peerList.clear();
        }
        pMutex.unlock();
    }

    void NetClient::clearMessageQueue()
    {
        pMutex.lock();
        messages.clear();
        pMutex.unlock();
    }

    void NetClient::reconnect()
    {
        connect(server, port, login, password);
    }

    void NetClient::connect(const String &server, const uint16 port, const String &login, const String &password, bool bRegister)
    {
        pMutex.lock();
        if (sock.isOpen())
            disconnect();

        messages.clear();

        sock.open(server, port);

        if (sock.isOpen())
        {
            this->password = password;
            this->login = login;
            this->port = port;
            this->server = server;

            sock.setNonBlockingMode(true);
            buffer_pos = 0;

            state = CONNECTING;
            sendMessage(format("CLIENT %d", TA3D_ENGINE_VERSION));
            if (bRegister)
                sendMessage("REGISTER " + login + " " + password);
            else
                sendMessage("LOGIN " + login + " " + password);
            uint32 timer = msec_timer;
            bool done = false;
            while(msec_timer - timer < 10000 && !done)   // 10s timeout
            {
                rest(1);
                receive();
                while(messageWaiting())
                {
                    String::Vector args;
                    String msg = getNextMessage();
                    msg.split(args, " ");

                    if (args.empty())   continue;

                    if (args[0] == "CONNECTED")     // Success !
                    {
                        state = CONNECTED;
                        done = true;
                    }
                    else if (args[0] == "ERROR")
                    {
                        done = true;
                        // Let the error message on top of the message queue
                        messages.push_front(msg);
                        break;
                    }
                }
            }

            if (state == CONNECTED)     // We want to know who is there
                sendMessage("GET CLIENT LIST");
        }

        pMutex.unlock();
    }

    void NetClient::receive()
    {
        MutexLocker mLock(pMutex);
        if (!sock.isOpen())     // Socket is closed, we can't get anything
        {
            state = DISCONNECTED;
            peerList.clear();
            return;
        }

        int n = sock.recv(buffer + buffer_pos, BUFFER_SIZE - buffer_pos);
        if (n > 0)
        {
            buffer_pos += n;
            int e = 0;
            for(int i = 0 ; i < buffer_pos ; i++)
            {
                if (buffer[i] == '\n')
                {
                    String msg(buffer + e, i - e);
                    e = i + 1;
                    messages.push_back(msg);
                    processMessage(msg);
                }
            }
            if (e > 0)
            {
                buffer_pos -= e;
                if (buffer_pos > 0)
                    memmove(buffer, buffer + e, buffer_pos);
            }
        }
    }

    void NetClient::processMessage(const String &msg)
    {
        String::Vector args;
        msg.split(args, " ");
        if (args.empty())   return;

        if (args[0] == "USER" && args.size() == 2)
        {
            bool found = false;
            for(int i = 0 ; i < peerList.size() && !found ; i++)
                if (peerList[i] == args[1])
                    found = true;
            if (!found)
            {
                peerList.push_back(args[1]);
                // Sort the list
                std::sort(peerList.begin(), peerList.end());
            }
        }
    }
}
