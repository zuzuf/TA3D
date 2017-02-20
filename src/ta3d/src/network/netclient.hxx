#ifndef __TA3D_NETWORK_NETCLIENT_HXX__
# define __TA3D_NETWORK_NETCLIENT_HXX__

#include "netclient.h"

namespace TA3D
{


	inline NetClient::NetState NetClient::getState() const
	{
		return state;
	}


    inline const QStringList &NetClient::getPeerList() const
	{
		return peerList;
	}

	
	inline bool NetClient::messageWaiting() const
	{
		return !messages.empty();
	}


	inline QString NetClient::getLogin() const
	{
		return login;
	}


	inline QString NetClient::getChan() const
	{
		return currentChan;
	}


	inline QString NetClient::getServerJoined() const
	{
		return serverJoined;
	}


    inline const QStringList &NetClient::getChanList() const
	{
		return chanList;
	}


	inline NetClient::Ptr NetClient::instance()
	{
		if (!pInstance)
			pInstance = new NetClient;
		return pInstance;
	}




} // namespace TA3D

#endif // __TA3D_NETWORK_NETCLIENT_HXX__
