#ifndef __TA3D_NETWORK_NETCLIENT_H__
# define __TA3D_NETWORK_NETCLIENT_H__

# include <misc/string.h>
# include <mods/modinfo.h>
# include "socket.tcp.h"
# include <zuzuf/smartptr.h>
# include <threads/mutex.h>

namespace TA3D
{


    class NetClient : public zuzuf::ref_count
    {
	public:
		//! The most suitable smartptr for the class
        typedef zuzuf::smartptr<NetClient>	Ptr;

    public:
        enum NetState { CONNECTING,
                        CONNECTED,
                        DISCONNECTED };
		struct GameServer
		{
			//! Name of the server
			QString name;
			//! Game version being run by the server
			QString version;
			//! Mod being run by the server
			QString mod;
			//! Current map on the server
			QString map;
			//! Count of opened player slots
			QString nb_open;
			//! Host name of this server
			QString host;

			typedef std::map<QString, GameServer> List;
		};

	private:
		QString				server;
		uint16				port;
		QString				login;
		QString				password;
		NetState			state;
        QStringList         messages;
        QStringList         peerList;
        QStringList         chanList;
		GameServer::List	serverList;
		ModInfo::List		modList;
		SocketTCP			sock;
		char				*buffer;
		int					buffer_pos;
		QString				currentChan;
		bool				modListChanged;
		bool				serverListChanged;
		QString				serverJoined;
		bool				hostAck;
        Mutex               mtx;
	public:
        NetClient();
        ~NetClient();

        void            disconnect();
        void            connect(const QString &server, const uint16 port, const QString &login, const QString &password, bool bRegister = false);
        void            reconnect();
        NetState        getState() const;
        bool            messageWaiting() const;
        QString          getNextMessage();
        const QStringList  &getPeerList() const;
        const QStringList  &getChanList() const;
        GameServer::List getServerList();
        ModInfo::List   getModList();
        void            clearMessageQueue();
        void            sendMessage(const QString &msg);
        void            changeChan(const QString &chan);
        void            receive();
        void            sendChan(const QString &msg);
        QString          getLogin() const;
        QString          getChan() const;
		QString			getServerJoined() const;
		void			clearServerJoined();
		bool			getHostAck();

    private:
        void            processMessage(const QString &msg);

    public:
		static NetClient::Ptr instance();
        static void destroyInstance();

    private:
		static NetClient::Ptr pInstance;
    };




} // namespace TA3D

# include "netclient.hxx"

#endif // __TA3D_NETWORK_NETCLIENT_H__
