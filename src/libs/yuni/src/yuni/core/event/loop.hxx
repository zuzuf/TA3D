#ifndef __YUNI_CORE_EVENT_LOOP_HXX__
# define __YUNI_CORE_EVENT_LOOP_HXX__


namespace Yuni
{
namespace Private
{
namespace Core
{
namespace EventLoop
{

	template<class EventLoopT>
	class Thread : public Yuni::Thread::IThread
	{
	public:
		//! The type of the calling event loop
		typedef EventLoopT EventLoopType;

	public:
		Thread(EventLoopType& loop)
			:pEventLoop(loop)
		{
		}

		virtual ~Thread()
		{
			// Needed for code robustness and to prevent against corruption
			// of the v-table
			stop();
		}

		void suspendTheThread(unsigned int timeout)
		{
			(void) suspend(timeout);
		}

	protected:
		virtual bool onExecute()
		{
			// Starting an infinite loop
			// We don't have to check for the thread termination, since a request
			// will be dispatched especially for that. This request will return
			// `false`, which will abort the loop.
			pEventLoop.runInfiniteLoopWL();
			return false;
		}

		virtual void onStop()
		{
			// Notifying the event loop that our work is done
			typename EventLoopType::ThreadingPolicy::MutexLocker locker(pEventLoop);
			pEventLoop.pIsRunning = false;
		}

		virtual void onKill()
		{
			// Notifying the event loop that our work is done
			typename EventLoopType::ThreadingPolicy::MutexLocker locker(pEventLoop);
			pEventLoop.pIsRunning = false;
		}

	private:
		EventLoopType& pEventLoop;

	}; // class Thread<>



} // namespace EventLoop
} // namespace Core
} // namespace Private
} // namespace Yuni







namespace Yuni
{
namespace Core
{
namespace EventLoop
{


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	inline IEventLoop<ParentT,FlowT,StatsT,DetachedT>::IEventLoop()
		:pHasRequests(0), pRequests(NULL), pIsRunning(false), pThread(NULL)
	{
		// Note: Visual Studio does not like `this` in the initialization section
		// Broadcast the pointer of the event loop to the policies
		FlowPolicy::onInitialize(this);

		// Initialize the thread if in detached mode
		if (detached)
			pThread = new ThreadType(*this);
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	IEventLoop<ParentT,FlowT,StatsT,DetachedT>::~IEventLoop()
	{
		// Stopping gracefully the loop if not already done
		stop();

		typename ThreadingPolicy::MutexLocker locker(*this);
		{
			// Destroying the thread
			if (DetachedT)
				delete pThread;
			// Destroying the request list
			if (pRequests)
				delete pRequests;
		}
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	void IEventLoop<ParentT,FlowT,StatsT,DetachedT>::start()
	{
		{
			// Locking for inserting the new request
			typename ThreadingPolicy::MutexLocker locker(*this);
			// Aborting if the event loop is already running
			if (pIsRunning)
				return;
			// Flow
			if (FlowPolicy::onStart())
			{
				// The event loop is running
				pIsRunning = true;
			}
			// Initializing the request list
// 			if (pRequests)
// 				delete pRequests;
// 			pRequests = new RequestListType();
			if (!pRequests)
				pRequests = new RequestListType();
		}
		if (detached)
		{
			// The loop is launched from another thread
			pThread->start();
		}
		else
		{
			// Launching the event loop from the calling thread
			this->runInfiniteLoopWL();
			// Resetting internal status
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				pIsRunning = false;
				// unlocking the inner mutex
			}
		}
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	void IEventLoop<ParentT,FlowT,StatsT,DetachedT>::stop(const unsigned int timeout)
	{
		// Locking for checking internal status and inserting a 'stop' request
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			// Aborting if the event loop is already stopped
			if (!pIsRunning || !FlowPolicy::onStop())
				return;

			// Posting a request that will fail (return false) in order to stop
			// the event loop.
			// The object is still locked and we directly inject the request into
			// the request list.
			RequestType request;
			request.bind(this, &EventLoopType::requestStop);
			if (pRequests)
				pRequests->push_back(request);
			// unlocking the inner mutex
		}

		// Informing the event loop that a new request is available
		// We perform the reset after that the mutex has been unlocked to
		// reduce lock contention.
		pHasRequests = 1;


		if (detached)
		{
			// The event loop is running is detached mode
			// Waiting for the end of the execution of the external thread
			assert(NULL != pThread && "Event loop: Invalid thread pointer");
			pThread->stop(timeout);
		}
		else
		{
			// Trying to wait for the end of the event loop
			// Spinning lock, since we don't have better ways here.
			unsigned int elapsed = 0;
			do
			{
				// Checking for the thread termination
				// This check is performed first. With luck the loop is already stopped.
				{
					typename ThreadingPolicy::MutexLocker locker(*this);
					if (!pIsRunning)
						break;
					// unlocking the inner mutex
				}

				// Sleeping a bit...
				Yuni::SuspendMilliSeconds(80u); // 50ms
				elapsed += 80u;
			}
			while (elapsed < timeout);
		}
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	void IEventLoop<ParentT,FlowT,StatsT,DetachedT>::dispatch(const typename IEventLoop<ParentT,FlowT,StatsT,DetachedT>::RequestType& request)
	{
		// Locking for inserting the new request
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			// Initializing pRequests allows for dispatching a request before calling start()
			if (!pRequests)
				pRequests = new RequestListType();
			// Flow
			if (!FlowPolicy::onRequestPosted(request))
				return;
			// Inserting the new request
			pRequests->push_back(request);
			// Statistics
			StatisticsPolicy::onRequestPosted(request);
		}
		// Informing the event loop that a new request is available
		pHasRequests = 1;
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	bool IEventLoop<ParentT,FlowT,StatsT,DetachedT>::requestStop()
	{
		// Returning false to stop the event loop
		return false;
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	void IEventLoop<ParentT,FlowT,StatsT,DetachedT>::runInfiniteLoopWL()
	{
		// Statistics
		StatisticsPolicy::onStart();

		while (true)
		{
			// Run the cycle
			if (!FlowPolicy::onNewCycle() || !runCycleWL())
				break;
		}

		// Statistics
		StatisticsPolicy::onStop();
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	bool IEventLoop<ParentT,FlowT,StatsT,DetachedT>::runCycleWL()
	{
		// Statistics
		StatisticsPolicy::onNewCycle();

		// Performing requests, if any
		if (pHasRequests)
		{
			if (!performAllRequestsWL())
			{
				// At least one request has failed. Aborting
				return false;
			}
		}
		// Execute the parent loop
		if (static_cast<ParentType*>(this)->onLoop())
		{
			// Statistics
			StatisticsPolicy::onEndOfCycle();
			return true;
		}
		return false;
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	bool IEventLoop<ParentT,FlowT,StatsT,DetachedT>::performAllRequestsWL()
	{
		// The request list. The pointer will be retrieved as soon as the
		// inner mutex is locked
		const RequestListType* requests;

		// Lock and retrieve the request list
		// We will keep a pointer to the request list, and will create a new one
		// to release the mutex as soon as possible, to allow other threads to
		// dispatch new requests while the event loop is working.
		{
			// Locking
			typename ThreadingPolicy::MutexLocker locker(*this);
			// No request will remain into the queue after this method
			pHasRequests = 0;

			// This method may sometimes be called even if there is no request
			// in the list.
			if (pRequests->empty())
				return true;

			// We will take the ownership on the list, and will create a new one
			// to release the mutex as soon as possible and to process the requests
			// asynchronously.
			requests = pRequests;
			pRequests = new RequestListType();

			// The mutex is unlocked here
		}

		// Assert
		assert(requests != NULL && "IEventLoop: the request list is NULL");

		// Executing all requests
		const typename RequestListType::const_iterator end = requests->end();
		for (typename RequestListType::const_iterator i = requests->begin(); i != end; ++i)
		{
			// Statistics
			StatisticsPolicy::onProcessRequest(*i);

			// Processing the request
			if (! (*i)())
			{
				// The request has failed. Aborting now.
				delete requests;
				return false;
			}
		}

		// The request list has been processed and can be destroyed
		delete requests;
		return true;
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	bool IEventLoop<ParentT,FlowT,StatsT,DetachedT>::onLoop()
	{
		// The impossible happened !
		YUNI_STATIC_ASSERT(false, IEventLoop_TheParentMustImplementTheMethod_onLoop);
		return false;
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	inline void
	IEventLoop<ParentT,FlowT,StatsT,DetachedT>::suspend(unsigned int timeout)
	{
		if (detached)
		{
			// In detached mode, the thread pointer is valid. We will use the
			// method suspend which is far better (cancellation point) than a mere
			// sleep.
			pThread->suspendTheThread(timeout);
		}
		else
		{
			// However, when not in detached mode, we don't have any thread instance
			// to use. The only way to achieve a pause is to use a sleep
			SuspendMilliSeconds(timeout);
		}
	}



	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	inline IEventLoop<ParentT,FlowT,StatsT,DetachedT>&
	IEventLoop<ParentT,FlowT,StatsT,DetachedT>::operator += (const typename IEventLoop<ParentT,FlowT,StatsT,DetachedT>::RequestType& request)
	{
		dispatch(request);
		return *this;
	}


	template<class ParentT, template<class> class FlowT, template<class> class StatsT,
		bool DetachedT>
	inline IEventLoop<ParentT,FlowT,StatsT,DetachedT>&
	IEventLoop<ParentT,FlowT,StatsT,DetachedT>::operator << (const typename IEventLoop<ParentT,FlowT,StatsT,DetachedT>::RequestType& request)
	{
		dispatch(request);
		return *this;
	}





} // namespace EventLoop
} // namespace Core
} // namespace Yuni

#endif // __YUNI_CORE_EVENT_LOOP_HXX__
