#ifndef __YUNI_NET_MESSAGE_QUEUESERVICE_H__
# define __YUNI_NET_MESSAGE_QUEUESERVICE_H__

# include "../../yuni.h"
# include "../net.h"
# include "../../thread/policy.h"
# include "../../core/event.h"
# include "fwd.h"
# include "connection.h"
# include "transport.h"
# include "worker.h"


namespace Yuni
{
namespace Net
{
namespace Message
{

	enum
	{
		//! The default value for the maximum size of a message
		messageDefaultMaxSize = 42 * 1024 * 1024,
	};


	class QueueService : public Policy::ObjectLevelLockable<QueueService>
	{
	public:
		//! The threading policy
		typedef Policy::ObjectLevelLockable<QueueService>  ThreadingPolicy;

		/*!
		** \brief Different states of a queue service
		*/
		enum State
		{
			//! The queue service is currently stopped
			stStopped = 0,
			//! The queue service is currently starting
			stStarting,
			//! The queue service is working
			stRunning,
			//! The queue service is current stopping its work
			stStopping
		};

		//! Prototype event: The queue service is starting
		typedef Event<void (Error&)> OnStarting;
		//! Prototype event: The queue service has started
		typedef Event<void ()> OnStarted;
		//! Prototype event: The queue service is stopping
		typedef Event<void ()> OnStopping;
		//! Prototype event: The queue service has been stopped
		typedef Event<void ()> OnStopped;
		//! Prototype event: An error has occured
		typedef Event<void (State, Error)> OnError;
		//! Prototype event: accepting a client
		typedef Event<void (bool&, const String&, Port, const String&, Port)> OnClientAccept;
		//! Prototype event: A client has connected
		typedef Event<void (const IConnection::Ptr&)> OnClientConnected;
		//! Prototype event: A client has disconnected
		typedef Event<void (const IConnection::Ptr&)> OnClientDisconnected;


	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		QueueService();
		//! Destructor
		~QueueService();
		//@}

		//! \name Addresses management
		//@{
		/*!
		**
		** This action will be effective the next time the server starts
		** \code
		** Net::Message::QueueService  server;
		** server.listen("::1", 4242);
		** server.listen("82.125.10.31", 4242);
		** server.start();
		** \endcode
		*/
		Error  listen(const StringAdapter& address, const Port& port, TransportLayer transport = tlTCP);

		/*!
		** \brief Add a new address where the server should listen for incoming connections
		**
		** This action will be effective the next time the server starts
		** \code
		** Net::Message::QueueService  server;
		** server.listen("::1", 4242);
		** server.listen("82.125.10.31", 4242);
		** server.start();
		** \endcode
		*/
		Error  listen(const StringAdapter& address, const Port& port, Transport::ITransport::Ptr transport);

		/*!
		** \brief Clear all addresses where the server should listen for incoming connections
		**
		** This action will be effective the next time the server starts
		*/
		void clear();
		//@}

		//! \name Service management
		//@{
		/*!
		** \brief Try to start the queue service
		*/
		Error start();
		/*!
		** \brief Stop the server
		*/
		Error stop();
		//@}


		//! \name Messages
		//@{
		/*!
		** \brief Send a message to all peers
		*/
		template<class StringT> Error  sendAll(const StringT& buffer);

		/*!
		** \brief Send a raw buffer as message to all peers
		*/
		Error sendAll(const char* const buffer, unsigned int length);

		//! Get the maximum size (in bytes) of a message
		unsigned int messageMaxSize() const;
		//! Set the maximum size (in bytes, > 0) of a message
		bool messageMaxSize(unsigned int size);
		//@}


	public:
		//! \name Events
		//@{
		//! Event: The queue service is starting
		OnStarting onStarting;
		//! Event: The queue service has started and is ready for incoming connections
		OnStarted  onStarted;
		//! Event: The queue service is shutting down
		OnStopping onStopping;
		//! Event: The queue service is stopped
		OnStopped  onStopped;
		//! Event: The queue service has encountered an error
		OnError  onError;
		//! Event: A client try to connect to the server
		OnClientAccept   onClientAccept();
		//! Event: A client has been successfully connected to the server
		//@}


	protected:
		//! A single worker
		typedef Yuni::Private::Net::Message::Worker  Worker;

	protected:
		//! All workers
		Thread::Array<Worker> pWorkers;
		//! All addresses to listen
		Transport::ITransport::Set  pListenInfos;
		//! The maximum size (in bytes) of a message
		unsigned int pMessageMaxSize;
		//! Flag to know the state of the server
		State pState;

		// Friends
		friend class Yuni::Private::Net::Message::Worker;

	}; // class QueueService






} // namespace Message
} // namespace Net
} // namespace Yuni

# include "queueservice.hxx"

#endif // __YUNI_NET_MESSAGE_QUEUESERVICE_H__
