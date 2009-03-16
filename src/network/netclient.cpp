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
        currentChan = "*";
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
        chanList.clear();
        buffer_pos = 0;
        currentChan = "*";

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
            chanList.clear();
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
            sendMessage(format("CLIENT %s", TA3D_ENGINE_VERSION));
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
                int i = 0;
                while(messageWaiting() && i < messages.size())
                {
                    i++;
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
                    else
                        messages.push_back(msg);     // Don't remove other messages
                }
            }

            if (state == CONNECTED)
            {
                peerList.clear();
                chanList.clear();
                chanList.push_back("*");
                sendMessage("GET USER LIST");   // We want to know who is there
                sendMessage("GET CHAN LIST");   // and the chan list
            }
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
            chanList.clear();
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
        if (msg.empty())
            return;

        String::Vector args;
        msg.split(args, " ");

        if (args.empty())
            return;

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
        else if (args[0] == "LEAVE" && args.size() == 2)
        {
            bool found = false;
            for(int i = 0 ; i < peerList.size() && !found ; i++)
                if (peerList[i] == args[1])
                {
                    found = true;
                    if (i + 1 < peerList.size())
                        peerList[i] = peerList.back();
                    peerList.resize(peerList.size() - 1);
                }
        }
        else if (args[0] == "CHAN" && args.size() == 2)
        {
            bool found = false;
            for(int i = 0 ; i < chanList.size() && !found ; i++)
                if (chanList[i] == args[1])
                    found = true;
            if (!found)
            {
                chanList.push_back(args[1]);
                // Sort the list
                std::sort(chanList.begin(), chanList.end());
            }
        }
        else if (args[0] == "DECHAN" && args.size() == 2)
        {
            bool found = false;
            for(int i = 0 ; i < chanList.size() && !found ; i++)
                if (chanList[i] == args[1])
                {
                    found = true;
                    if (i + 1 < chanList.size())
                        chanList[i] = chanList.back();
                    chanList.resize(chanList.size() - 1);
                }
        }
    }

    String::Vector NetClient::getChanList()
    {
        MutexLocker mLock(pMutex);
        return chanList;
    }

    void NetClient::changeChan(const String &chan)
    {
        pMutex.lock();
        currentChan = chan.empty() ? "*" : chan;
        sendMessage("CHAN " + chan);
        sendMessage("GET USER LIST");
        peerList.clear();
        pMutex.unlock();
    }

    void NetClient::sendChan(const String &msg)
    {
        pMutex.lock();
        for(int i = 0 ; i < peerList.size() ; i++)
            if (peerList[i] != login)               // No self sending (server will block it, so it's useless)
                sendMessage("SEND " + peerList[i] + " " + msg);
        pMutex.unlock();
    }

    String NetClient::getLogin()
    {
        MutexLocker mLocker(pMutex);
        return login;
    }

    String NetClient::getChan()
    {
        MutexLocker mLocker(pMutex);
        return currentChan;
    }
}
