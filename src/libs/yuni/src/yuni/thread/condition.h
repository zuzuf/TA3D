#ifndef __YUNI_THREADS_CONDITION_H__
# define __YUNI_THREADS_CONDITION_H__

# include "mutex.h"
# include "../core/smartptr/smartptr.h"


namespace Yuni
{
namespace Thread
{

	/*!
	** \brief Condition Variable
	**
	** \ingroup Threads
	**
	** A condition variable is a synchronization object used in conjunction with a
	** mutex. It is a synchronization mechanism that allows a thread to suspend its
	** execution until the Condition is signalled by another thread.
	**
	** This can be used in many ways, from interruptible sleep to signal handlers,
	** but also as a thread startup synchronization mechanism.
	**
	** \see samples/threads/01.condition/main.cpp
	*/
	class Condition
	{
	public:
		//! The most suitable SmartPtr
		typedef SmartPtr<Condition> Ptr;

	public:
		//! \name Constructors & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Condition();
		/*!
		** \brief Constructor with an external mutex
		**
		** In this case, an external mutex will be used
		*/
		Condition(Mutex& mutex);
		//! Destructor
		~Condition();
		//@}


		//! \name Wait for a notification
		//@{
		/*!
		** \brief Atomic unlock() and wait for notification
		**
		** This method atomically releases the Condition's mutex and wait for any
		** notifications sent to this condition. This method will not return without
		** having received a notification.
		**
		** Upon receiving a notification, this method lock()s the internal mutex
		** and returns.
		**
		** IMPORTANT NOTES:
		**  - It is vital that the mutex shall be locked by the current thread when
		**	  you enter this method.
		**	- This method is a cancellation point for Threads.
		**
		** \see lock(), unlock()
		*/
		void waitUnlocked();

		/*!
		** \brief Atomic unlock() and wait for notification (with timeout)
		**
		** This version of the function accepts a timeout in milliseconds
		**
		** \param[in] msTimeout The timeout value (in milliseconds)
		** \return True if the timeout expired.
		** \see unlock()
		*/
		bool waitUnlocked(unsigned int msTimeout);
		//@}


		//! \name Manipulate the Condition's mutex
		//@{
		/*!
		** \brief Lock the inner mutex
		*/
		void lock();

		/*!
		** \brief Unlock the inner mutex
		*/
		void unlock();
		//@}


		//! \name Signal the condition
		//@{
		/*!
		** \brief Notify at least one thread
		**
		** If there's more than one thread waiting on the condition, at least
		** one of these shall be notified. If no threads are waiting, then this call has
		** no effect.
		*/
		void notifyLocked();

		/*!
		** \brief Notify at least one thread without locking the mutex.
		**
		** \see notify()
		*/
		void notify();

		/*!
		** \brief Notify all the threads
		**
		** Awaken all the threads waiting on the condition.
		** If no threads are currently waiting for the condition, this call has no effect.
		*/
		void notifyAllLocked();

		/*!
		** \brief Notify all the threads without locking the mutex.
		**
		** \see notify(),notifyAllLocked()
		*/
		void notifyAll();
		//@}


		//! \name Misc
		//@{
		/*!
		** \brief Get the inner mutex
		*/
		Mutex& mutex() const;
		//@}

	private:
		# ifndef YUNI_NO_THREAD_SAFE
		//! The PThread Condition
		pthread_cond_t  pCondition;
		# endif
		//! The mutex
		Mutex* pMutex;
		# ifndef YUNI_NO_THREAD_SAFE
		//! Have the condition been really signalled ?
		volatile bool pSignalled;
		# endif
		//! True if this class owns the mutex and must destroy it
		const bool pOwnMutex;

	}; // class Condition





	/*!
	** \brief Locks a condition's mutex in the constructor and unlocks it in the destructor.
	**
	** \see MutexLocker
	*/
	class ConditionLocker
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		**
		** \param c The condition to lock
		*/
		ConditionLocker(Condition& c);
		//! Destructor
		~ConditionLocker();
		//@}

	private:
		//! Reference to the real condition
		Condition& pCondition;

	}; // ConditionLocker





} // namespace Thread
} // namespace Yuni

# include "condition.hxx"

#endif // __YUNI_THREADS_CONDITION_H__
