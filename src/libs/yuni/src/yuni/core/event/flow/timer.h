#ifndef __YUNI_CORE_EVENT_FLOW_TIMER_H__
# define __YUNI_CORE_EVENT_FLOW_TIMER_H__

# include "../../../yuni.h"
# include "../../../thread/thread.h"


namespace Yuni
{
namespace Core
{
namespace EventLoop
{
namespace Flow
{


	template<class EventLoopT>
	class Timer
	{
	public:
		//! Type of the event loop
		typedef EventLoopT EventLoopType;

	public:
		//! \name Constructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Timer() :pTimeout(200), pEventLoop(NULL) {}
		//@}

		/*!
		** \brief Set the timeout to wait between each cycle
		**
		** \note Due to implementation limitations, the timeout is limited to 10s when
		** the loop is not in detached mode. There is no restriction in detached mode
		** (which is the default).
		*/
		void timeout(unsigned int t)
		{
			if (!EventLoopType::detached)
			{
				// Hard limit when not in detached mode.
				// When not in detached mode, there is no cancellation point
				// so it is impossible to properly the loop if the timeout is too high
				if (t > 10000)
					t = 10000;
			}
			// The timeout will be set from the loop to avoid continuous locking
			Bind<bool ()> b;
			b.bind(this, &Timer::internalSetLoopTimeout, t);
			pEventLoop->dispatch(b);
		}

	protected:
		//! \name Events triggered by the public interface of the event loop (from any thread)
		//@{
		/*!
		** \brief The event loop has just started
		**
		** The event loop is locked when this method is called
		*/
		static bool onStart() {return true;}

		/*!
		** \brief The event loop has just stopped
		**
		** The event loop is locked when this method is called
		*/
		static bool onStop() {return true;}

		/*!
		** \brief A new request has just been added into the queue
		**
		** The event loop is locked when this method is called
		** \param request The request (bind, see EventLoopType::RequestType)
		** \return True to allow the request to be posted
		*/
		template<class U> static bool onRequestPosted(const U& request)
		{ (void) request; return true; }
		//@}

		//! \name Events triggered from the main thread of the event loop
		//@{
		/*!
		** \brief The event loop has started a new cycle
		**
		** This method is called from the main thread of the event loop.
		** No lock is provided.
		*/
		bool onNewCycle()
		{
			pEventLoop->suspend(pTimeout);
			return true;
		}
		//@}

		/*!
		** \brief Event triggered from the constructor of the event loop
		** \param e Pointer to the original event loop
		*/
		void onInitialize(EventLoopType* e)
		{
			pEventLoop = e;
		}

	private:
		bool internalSetLoopTimeout(unsigned int t)
		{
			// We assume here that `t` is valid.
			pTimeout = t;
			return true;
		}

	private:
		//! Timeout (ms)
		unsigned int pTimeout;
		//! Pointer to the original event loop
		EventLoopType* pEventLoop;

	}; // class Timer<>





} // namespace Flow
} // namespace EventLoop
} // namespace Core
} // namespace Yuni

#endif // __YUNI_CORE_EVENT_FLOW_TIMER_H__
