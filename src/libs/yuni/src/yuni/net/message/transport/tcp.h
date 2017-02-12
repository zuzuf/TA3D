#ifndef __YUNI_NET_MESSAGE_TRANSPORT_TCP_H__
# define __YUNI_NET_MESSAGE_TRANSPORT_TCP_H__

# include "../../../yuni.h"
# include "../../../core/string.h"
# include "transport.h"


namespace Yuni
{
namespace Net
{
namespace Message
{
namespace Transport
{

	/*!
	** \brief Transport layer for messages (abstract)
	**
	** A transport layer is not thread-safe.
	*/
	class TCP : public ITransport
	{
	public:
		//! The most suitable smart pointer for the class
		typedef SmartPtr<TCP>  Ptr;
		//! Set
		typedef std::set<Ptr> Set;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		TCP(Mode m);
		//! Destructor
		virtual ~TCP();
		//@}

	protected:
		//! Execute the transport layer
		virtual Yuni::Net::Error  onExecute();

	private:
		//! Server mode
		Yuni::Net::Error  runServer();

	}; // class Transport






} // namespace Transport
} // namespace Server
} // namespace Net
} // namespace Yuni

#endif // __YUNI_NET_MESSAGE_TRANSPORT_TCP_H__
