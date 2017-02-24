#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "netclient.h"
#include <mods/mods.h>
#include <algorithm>        // We need std::sort

#define BUFFER_SIZE     2048


namespace TA3D
{


	NetClient::Ptr NetClient::pInstance;



	void NetClient::destroyInstance()
	{
		pInstance = nullptr;
	}


	NetClient::NetClient()
		:port(0), state(NetClient::DISCONNECTED), buffer_pos(0), currentChan("*"),
		modListChanged(false), serverListChanged(false),
		hostAck(false)
	{
		buffer = new char[BUFFER_SIZE];
	}

	NetClient::~NetClient()
	{
		disconnect();
	}

	void NetClient::disconnect()
	{
        MutexLocker locker(mtx);
		sendMessage("DISCONNECT");
		sock.close();
		state = DISCONNECTED;
		peerList.clear();
		chanList.clear();
		serverList.clear();
		serverListChanged = true;
		serverJoined.clear();
		buffer_pos = 0;
		currentChan = "*";
		hostAck = false;
	}


	QString NetClient::getNextMessage()
	{
        MutexLocker locker(mtx);
		QString msg = messages.front();
		messages.pop_front();
		return msg;
	}


	void NetClient::sendMessage(const QString &msg)
	{
        MutexLocker locker(mtx);
		if (sock.isOpen())
            sock.send(msg + "\n");
		else
		{
			state = DISCONNECTED;
			peerList.clear();
			chanList.clear();
		}
	}


	void NetClient::clearMessageQueue()
	{
        MutexLocker locker(mtx);
		messages.clear();
	}


	void NetClient::reconnect()
	{
		connect(server, port, login, password);
	}


	void NetClient::connect(const QString &server, const uint16 port, const QString &login, const QString &password, bool bRegister)
	{
        MutexLocker locker(mtx);
		if (sock.isOpen())
			disconnect();

		messages.clear();
		serverJoined.clear();
		hostAck = false;

		sock.open(server, port);

		if (sock.isOpen())
		{
			this->password = password;
			this->login = login;
			this->port = port;
			this->server = server;

			sock.setNonBlockingMode(true);
			buffer_pos = 0;

            sendMessage(QString("CLIENT ") + TA3D_ENGINE_VERSION);
            if (password.isEmpty())
				return;
			state = CONNECTING;
			if (bRegister)
                sendMessage("REGISTER " + login + ' ' + password);
			else
                sendMessage("LOGIN " + login + ' ' + password);
			uint32 timer = msec_timer;
			bool done = false;
			while (msec_timer - timer < 10000 && !done)   // 10s timeout
			{
				rest(1);
				receive();
				uint32 i = 0;
				while (messageWaiting() && i < messages.size())
				{
					++i;
					QString msg = getNextMessage();
                    const QStringList &args = msg.split(' ', QString::SkipEmptyParts);

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
				serverList.clear();
				sendMessage("GET USER LIST");   // We want to know who is there
				sendMessage("GET CHAN LIST");   // and the chan list
                sendMessage("GET MOD LIST");    // and the mod list
				sendMessage("GET SERVER LIST");	// and the server list
            }
		}
	}


	void NetClient::receive()
	{
        MutexLocker locker(mtx);
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
			modListChanged = false;
			for(int i = 0 ; i < buffer_pos ; i++)
			{
				if (buffer[i] == '\n')
				{
                    QString msg = QString::fromLatin1(buffer + e, i - e);
					e = i + 1;
					messages.push_back(msg);
					processMessage(msg);
				}
			}
			if (modListChanged)
				Mods::instance()->update();
			if (e > 0)
			{
				buffer_pos -= e;
				if (buffer_pos > 0)
					memmove(buffer, buffer + e, buffer_pos);
			}
		}
	}


	void NetClient::processMessage(const QString& msg)
	{
        if (msg.isEmpty())
			return;

		// We need to split the server command into its parameters
        const QStringList &args = SplitCommand(msg);

        if (args.isEmpty())
			return;

		if (args[0] == "USER" && args.size() == 2)
		{
			bool found = false;
			for(uint32 i = 0 ; i < peerList.size() && !found ; ++i)
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
			for(uint32 i = 0 ; i < peerList.size() && !found ; ++i)
				if (peerList[i] == args[1])
				{
					found = true;
					if (i + 1 < peerList.size())
						peerList[i] = peerList.back();
                    peerList.pop_back();
				}
		}
		else if (args[0] == "CHAN" && args.size() == 2)
		{
			bool found = false;
			for(uint32 i = 0 ; i < chanList.size() && !found ; ++i)
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
			for(uint32 i = 0 ; i < chanList.size() && !found ; ++i)
				if (chanList[i] == args[1])
				{
					found = true;
					if (i + 1 < chanList.size())
						chanList[i] = chanList.back();
                    chanList.pop_back();
				}
		}
		else if (args[0] == "CLOSE")
		{
			disconnect();
		}
        else if (args[0] == "CLEAR" && args.size() == 3 && args[1] == "MOD" && args[2] == "LIST")
		{
            modList.clear();
			modListChanged = true;
		}
		else if (args[0] == "MOD")
        {
            ModInfo mod(msg);
            if (mod.getID() >= 0)
            {
                bool found = false;
                for(ModInfo::List::iterator i = modList.begin() ; i != modList.end() && !found ; ++i)
                {
                    found = i->getID() == mod.getID();
                    if (found)
                        *i = mod;
                }
                if (!found)
                    modList.push_back(mod);
				modListChanged = true;
            }
        }
		else if (args[0] == "SERVER")
		{
			GameServer gameServer;
			for(uint32 i = 1 ; i < args.size() - 1 ; ++i)
			{
				if (args[i] == "NAME")
					gameServer.name = args[i + 1];
				else if (args[i] == "MOD")
					gameServer.mod = args[i + 1];
				else if (args[i] == "VERSION")
					gameServer.version = args[i + 1];
				else if (args[i] == "HOST")
					gameServer.host = args[i + 1];
				else if (args[i] == "MAP")
					gameServer.map = args[i + 1];
				else if (args[i] == "SLOTS")
					gameServer.nb_open = args[i + 1];
			}
			serverList[gameServer.name] = gameServer;
			serverListChanged = true;
		}
		else if (args[0] == "UNSERVER" && args.size() == 2)
		{
			if (serverList.count(args[1]) == 1)
			{
				serverList.erase(args[1]);
				serverListChanged = true;
			}
			else	// We've a problem, server list lost sync OO!
				sendMessage("GET SERVER LIST");
		}
		else if (args[0] == "CLEAR" && args.size() == 3 && args[1] == "SERVER" && args[2] == "LIST")
		{
			serverList.clear();
			serverListChanged = true;
		}
		else if (args[0] == "JOIN" && args.size() == 2)
			serverJoined = args[1];
		else if (args[0] == "UNJOIN" && args.size() == 2 && serverJoined == args[1])
			serverJoined.clear();
		else if (args[0] == "HOST")		// Server acknowledged, no errors we're free to go
			hostAck = true;
	}


    ModInfo::List NetClient::getModList()
    {
        MutexLocker locker(mtx);
		modListChanged = false;
        return modList;
    }


	NetClient::GameServer::List NetClient::getServerList()
	{
        MutexLocker locker(mtx);
		serverListChanged = false;
		return serverList;
	}


	void NetClient::changeChan(const QString &chan)
	{
        MutexLocker locker(mtx);
        currentChan = chan.isEmpty() ? "*" : chan;
        sendMessage("CHAN " + chan);
		sendMessage("GET USER LIST");
		peerList.clear();
	}


	void NetClient::sendChan(const QString &msg)
	{
        MutexLocker locker(mtx);
        sendMessage("SENDALL " + msg);
	}


	void NetClient::clearServerJoined()
	{
        MutexLocker locker(mtx);
		serverJoined.clear();
	}


	bool NetClient::getHostAck()
	{
        MutexLocker locker(mtx);
		if (hostAck)
		{
			hostAck = false;
			return true;
		}
		return false;
	}



} // namespace TA3D
