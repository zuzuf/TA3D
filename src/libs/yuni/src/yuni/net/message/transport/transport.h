#ifndef __YUNI_NET_MESSAGE_TRANSPORT_TRANSPORT_H__
# define __YUNI_NET_MESSAGE_TRANSPORT_TRANSPORT_H__

# include "../../../yuni.h"
# include "../../../core/string.h"
# include "../../../core/noncopyable.h"
# include "../../../thread/thread.h"
# include "../fwd.h"
# include "../../errors.h"
# include "../../hostaddressport.h"
# include "../../port.h"
# include "layer.h"
# include <set>


namespace Yuni
{
namespace Net
{
namespace Message
{
namespace Transport
{

	enum Mode
	{
		tmNone = 0,
		//! Server mode
		tmServer,
		//! Client mode
		tmClient
	};


	/*!
	** \brief Transport layer for messages (abstract)
	**
	** A transport layer is not thread-safe.
	*/
	class ITransport : private NonCopyable<ITransport>
	{
	public:
		//! The most suitable smart pointer for the class
		typedef SmartPtr<ITransport>  Ptr;
		//! Set
		typedef std::set<Ptr> Set;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		ITransport(Mode m);
		//! Destructor
		virtual ~ITransport();
		//@}

		//! \name Attached thread
		//@{
		//! Get the attached thread
		Thread::IThread*  attachedThread();
		//! Get the attached thread (const)
		const Thread::IThread*  attachedThread() const;
		//! Set the attached thread
		void attachedThread(Thread::IThread* thread);
		//@}


		//! Service
		//@{
		//! Execute the transport layer
		Yuni::Net::Error execute();
		//! Execute the transport layer
		Yuni::Net::Error operator () ();
		//@}

	public:
		//! Address to listen
		HostAddress  address;
		//! Port
		Port port;
		//! Mode (server/client)
		const Mode mode;

	protected:
		//! Execute the transport layer
		virtual Yuni::Net::Error  onExecute() = 0;

	protected:
		//! The attached thread
		Thread::IThread* pAttachedThread;

	}; // class Transport






} // namespace Transport
} // namespace Server
} // namespace Net
} // namespace Yuni

# include "transport.hxx"

#endif // __YUNI_NET_MESSAGE_TRANSPORT_TRANSPORT_H__
