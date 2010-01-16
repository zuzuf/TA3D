#include "mutex.h"

namespace TA3D
{
	Synchronizer::Synchronizer(int nbThreadsToSync)
			: nbThreadsToSync(nbThreadsToSync),
			nbThreadsWaiting(0)
	{
	}

	Synchronizer::~Synchronizer()
	{
		nbThreadsToSync = 0;
		release();
	}

	void Synchronizer::sync()
	{
		lock();
		++nbThreadsWaiting;

		if (nbThreadsToSync <= nbThreadsWaiting)
		{
			nbThreadsWaiting = 0;
			notifyAll();
			unlock();
		}
		else
		{
			waitUnlocked();
			unlock();
		}
	}

	void Synchronizer::release()
	{
		lock();

		nbThreadsWaiting = 0;
		notifyAll();

		unlock();
	}
}
