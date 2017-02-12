#ifndef __YUNI_NET_HOST_ADDRESS_PORT_HXX__
# define __YUNI_NET_HOST_ADDRESS_PORT_HXX__


namespace Yuni
{
namespace Net
{


	inline bool HostAddressPort::Compare::operator () (const HostAddressPort& a, const HostAddressPort& b) const
	{
		return (a.port < b.port) && (a.address < b.address);
	}


	inline void HostAddressPort::reset()
	{
		address.clear();
		port = nullptr;
	}





} // namespace Net
} // namespace Yuni

#endif // __YUNI_NET_HOST_ADDRESS_PORT_H__
