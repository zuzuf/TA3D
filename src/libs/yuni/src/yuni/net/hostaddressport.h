#ifndef __YUNI_NET_HOST_ADDRESS_PORT_H__
# define __YUNI_NET_HOST_ADDRESS_PORT_H__

# include "../yuni.h"
# include "port.h"
# include "../core/string.h"
# include <vector>
# include <set>


namespace Yuni
{
namespace Net
{

	//! Raw Address for a host (IP/DNS)
	typedef CustomString<256, false>  HostAddress;



	class HostAddressPort
	{
	public:
		//! Predicate for comparing two host addresses
		struct Compare
		{
			bool operator () (const HostAddressPort& a, const HostAddressPort& b) const;
		};
		//! Vector
		typedef std::vector<HostAddressPort>  Vector;
		//! Set
		typedef std::set<HostAddressPort, Compare>  Set;


	public:
		//! Reset the host address and port
		void reset();

	public:
		//! Raw host address (IP/DNS)
		HostAddress  address;
		//! Socket port number
		Port port;

	}; // class HostAddressPort





} // namespace Net
} // namespace Yuni

# include "hostaddressport.hxx"

#endif // __YUNI_NET_HOST_ADDRESS_PORT_H__
