#ifndef __YUNI_NET_MESSAGE_WORKER_H__
# define __YUNI_NET_MESSAGE_WORKER_H__

# include "../../yuni.h"
# include "../net.h"
# include "fwd.h"
# include "transport.h"
# include "../../thread/thread.h"
# include "../../thread/array.h"


namespace Yuni
{
namespace Private
{
namespace Net
{
namespace Message
{

	/*!
	** \brief Worker for Net queue service
	*/
	class Worker : public Yuni::Thread::IThread
	{
	public:
		//! The most suitable smart pointer
		typedef Yuni::Thread::IThread::Ptr  Ptr;
		//! Alias to the queue service
		typedef Yuni::Net::Message::QueueService  QueueService;
		//! Transport layer (abstract)
		typedef Yuni::Net::Message::Transport::ITransport ITransport;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		explicit Worker(QueueService& queueservice, ITransport::Ptr transport);
		//! Destructor
		virtual ~Worker();
		//@}

	protected:
		//! Thread execution
		virtual bool onExecute();

	private:
		//! The transport layer
		ITransport::Ptr pTransport;
		//! Pointer to the queue service
		QueueService& pQueueService;

	}; // class Worker






} // namespace Message
} // namespace Net
} // namespace Private
} // namespace Yuni

# include "worker.hxx"

#endif // __YUNI_NET_MESSAGE_QUEUESERVICE_WORKER_H__
