
#include "../yuni.h"
#include "condition.h"

# ifndef YUNI_NO_THREAD_SAFE
#include <time.h>
#include <cassert>
#if defined(YUNI_OS_MSVC)
# include <winsock2.h>
#else
# include <sys/time.h>
#endif
#include <errno.h>
#include "../core/system/gettimeofday.h"


namespace Yuni
{
namespace Thread
{


	// Note: when YUNI_NO_THREAD_SAFE is defined, the constructor and destructor
	//   are defined in the .hxx file
	Condition::Condition()
		:pSignalled(false), pOwnMutex(true)
	{
		// Note: The linux implementation currently does not support
		// any condition attr
		::pthread_cond_init(&pCondition, NULL);
		pMutex = new Mutex();
	}


	Condition::Condition(Mutex& mutex)
		:pMutex(&mutex), pSignalled(false), pOwnMutex(false)
	{
		// Note: The linux implementation currently does not support
		// any condition attr
		::pthread_cond_init(&pCondition, NULL);
	}


	Condition::~Condition()
	{
		::pthread_cond_destroy(&pCondition);
		if (pOwnMutex)
			delete pMutex;
	}



	void Condition::waitUnlocked()
	{
		// The pthread_cond_wait will unlock the mutex and wait for
		// signalling.

		int pthread_cond_wait_error;
		do
		{
			// Spurious wakeups from this function can occur.
			// Therefore we must check out pSignalled variable to ensure we have
			// really been signalled.
			pthread_cond_wait_error = ::pthread_cond_wait(&pCondition, &pMutex->pthreadMutex());
		} while (pSignalled != true);

		// The condition was signalled: the mutex is now locked again.
	}


	bool Condition::waitUnlocked(unsigned int timeout)
	{
		Yuni::timeval now;
		struct timespec t;

		// Set the timespec t at [timeout] milliseconds in the future.
		YUNI_SYSTEM_GETTIMEOFDAY(&now, NULL);
		time_t timeout_long = static_cast<time_t>(timeout);
		t.tv_nsec  =  static_cast<long>  (now.tv_usec * 1000 + ((static_cast<int>(timeout_long) % 1000) * 1000000));
		t.tv_sec   =  static_cast<time_t>(now.tv_sec + timeout_long / 1000 + (t.tv_nsec / 1000000000L));
		t.tv_nsec  %= 1000000000L;

		int pthread_cond_timedwait_error;
		do
		{
			// Avoid spurious wakeups (see waitUnlocked() above for explanations)
			pthread_cond_timedwait_error = ::pthread_cond_timedwait(&pCondition, &pMutex->pthreadMutex(), &t);
		}
		while (pSignalled != true // Condition not verified
			&& pthread_cond_timedwait_error != ETIMEDOUT // We have not timedout
			&& pthread_cond_timedwait_error != EINVAL);  // When t is in the past, we got EINVAL. We consider this as a timeout.

		// The condition was signalled or has timeoutted:
		return (pSignalled == false);
	}


	void Condition::notifyLocked()
	{
		pMutex->lock();
		pSignalled = true;
		::pthread_cond_signal(&pCondition);
		pMutex->unlock();
	}


	void Condition::notifyAllLocked()
	{
		pMutex->lock();
		pSignalled = true;
		::pthread_cond_broadcast(&pCondition);
		pMutex->unlock();
	}




} // namespace Thread
} // namespace Yuni

# endif // ifndef YUNI_NO_THREAD_SAFE

