#include "mutex.h"

namespace TA3D
{
	Synchronizer::Synchronizer(int nbThreadsToSync)
			: nbThreadsToSync(nbThreadsToSync),
			nbThreadsWaiting(0),
			pSignalled(0)
	{
	}

	Synchronizer::~Synchronizer()
	{
		nbThreadsToSync = 0;
		release();
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
            pCondition.wakeAll();
		}
		else
		{
			// The pthread_cond_wait will unlock the mutex and wait for
			// signalling.

			unsigned int curSignal = pSignalled;
			do
			{
				// Spurious wakeups from this function can occur.
				// Therefore we must check out pSignalled variable to ensure we have
				// really been signalled.
                pCondition.wait(&pMutex);
			} while (pSignalled == curSignal);

			pMutex.unlock();
		}
	}

	void Synchronizer::release()
	{
		pMutex.lock();

		nbThreadsWaiting = 0;
		++pSignalled;

        pCondition.wakeAll();

		pMutex.unlock();
	}
}
