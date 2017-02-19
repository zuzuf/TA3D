#ifndef __TA3D_NETWORK_NETCLIENT_H__
# define __TA3D_NETWORK_NETCLIENT_H__

# include <yuni/yuni.h>
# include <misc/string.h>
# include <mods/modinfo.h>
# include "socket.tcp.h"
# include <zuzuf/smartptr.h>


namespace TA3D
{


    class NetClient : public Yuni::Policy::SingleThreaded<NetClient>, public zuzuf::ref_count
    {
	public:
		//! The most suitable smartptr for the class
        typedef zuzuf::smartptr<NetClient>	Ptr;
		//! The threading policy
		typedef Yuni::Policy::SingleThreaded<NetClient>  ThreadingPolicy;

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
        NetState        getState() const;
        bool            messageWaiting() const;
        String          getNextMessage();
        String::Vector  getPeerList() const;
        String::Vector  getChanList() const;
		GameServer::List getServerList();
        ModInfo::List   getModList();
        void            clearMessageQueue();
        void            sendMessage(const String &msg);
        void            changeChan(const String &chan);
        void            receive();
        void            sendChan(const String &msg);
        String          getLogin() const;
        String          getChan() const;
		String			getServerJoined() const;
		void			clearServerJoined();
		bool			getHostAck();

    private:
        void            processMessage(const String &msg);

    public:
		static NetClient::Ptr instance();
        static void destroyInstance();

    private:
		static NetClient::Ptr pInstance;
    };




} // namespace TA3D

# include "netclient.hxx"

#endif // __TA3D_NETWORK_NETCLIENT_H__
