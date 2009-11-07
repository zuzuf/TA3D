#ifndef NETCLIENT_H
#define NETCLIENT_H

#include "../misc/string.h"
#include "../mods/modinfo.h"
#include "socket.tcp.h"

namespace TA3D
{
    class NetClient : ObjectSync
    {
    public:
        enum NetState { CONNECTING,
                        CONNECTED,
                        DISCONNECTED };
		struct GameServer
		{
			//! Name of the server
			String name;
			//! Game version being run by the server
			String version;
			//! Mod being run by the server
			String mod;
			//! Current map on the server
			String map;
			//! Count of opened player slots
			String nb_open;
			//! Host name of this server
			String host;

			typedef std::map<String, GameServer> List;
		};

	private:
		String				server;
		uint16				port;
		String				login;
		String				password;
		NetState			state;
		String::List		messages;
		String::Vector		peerList;
		String::Vector		chanList;
		GameServer::List	serverList;
		ModInfo::List		modList;
		SocketTCP			sock;
		char				*buffer;
		int					buffer_pos;
		String				currentChan;
		bool				modListChanged;
		bool				serverListChanged;
		String				serverJoined;
		bool				hostAck;
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
		GameServer::List getServerList();
        ModInfo::List   getModList();
        void            clearMessageQueue();
        void            sendMessage(const String &msg);
        void            changeChan(const String &chan);
        void            receive();
        void            sendChan(const String &msg);
        String          getLogin();
        String          getChan();
		String			getServerJoined();
		void			clearServerJoined();
		bool			getHostAck();

    private:
        void            processMessage(const String &msg);

    public:
		static SmartPtr<NetClient> instance();
        static void destroyInstance();

    private:
		static SmartPtr<NetClient> pInstance;
    };
}

#endif // NETCLIENT_H
