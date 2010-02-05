#include "mutex.h"

namespace TA3D
{
	Synchronizer::Synchronizer(int nbThreadsToSync)
			: nbThreadsToSync(nbThreadsToSync),
			nbThreadsWaiting(0),
			pSignalled(0)
	{
		// Note: The linux implementation currently does not support
		// any condition attr
		::pthread_cond_init(&pCondition, NULL);
	}

	Synchronizer::~Synchronizer()
	{
		nbThreadsToSync = 0;
		release();
		::pthread_cond_destroy(&pCondition);
	}

	void Synchronizer::sync()
	{
		pMutex.lock();
		++nbThreadsWaiting;

		if (nbThreadsToSync <= nbThreadsWaiting)
		{
			nbThreadsWaiting = 0;
			++pSignalled;
			pMutex.unlock();
			::pthread_cond_broadcast(&pCondition);
		}
		else
		{
			// The pthread_cond_wait will unlock the mutex and wait for
			// signalling.

			unsigned int curSignal = pSignalled;
			int pthread_cond_wait_error;
			do
			{
				// Spurious wakeups from this function can occur.
				// Therefore we must check out pSignalled variable to ensure we have
				// really been signalled.
				pthread_cond_wait_error = ::pthread_cond_wait(&pCondition, &pMutex.pthreadMutex());
			} while (pSignalled == curSignal);

			pMutex.unlock();
		}
	}

	void Synchronizer::release()
	{
		pMutex.lock();

		nbThreadsWaiting = 0;
		++pSignalled;

		::pthread_cond_broadcast(&pCondition);

		pMutex.unlock();
	}
}
