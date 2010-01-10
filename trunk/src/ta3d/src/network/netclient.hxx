#ifndef __TA3D_NETWORK_NETCLIENT_HXX__
# define __TA3D_NETWORK_NETCLIENT_HXX__


namespace TA3D
{


	inline NetClient::NetState NetClient::getState() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return state;
	}


	inline String::Vector NetClient::getPeerList() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return peerList;
	}

	
	inline bool NetClient::messageWaiting() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return !messages.empty();
	}


	inline String NetClient::getLogin() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return login;
	}


	inline String NetClient::getChan() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return currentChan;
	}


	inline String NetClient::getServerJoined() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return serverJoined;
	}


	inline String::Vector NetClient::getChanList() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
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
