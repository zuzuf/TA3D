
#include "timer.h"
#include <cassert>


namespace Yuni
{
namespace Thread
{


	Timer::Timer(const Timer& rhs)
		:IThread()
	{
		rhs.pTimerMutex.lock();
		pTimeInterval = rhs.pTimeInterval;
		pCycleCount = rhs.pCycleCount;
		rhs.pTimerMutex.unlock();
	}


	Timer& Timer::operator = (const Timer& rhs)
	{
		pTimerMutex.lock();
		rhs.pTimerMutex.lock();
		pTimeInterval = rhs.pTimeInterval;
		pCycleCount = rhs.pCycleCount;
		pTimerMutex.unlock();
		rhs.pTimerMutex.unlock();
		return *this;
	}


	bool Timer::internalRunInfiniteLoop()
	{
		const unsigned int nnTimeInterval(pTimeInterval);
		pTimerMutex.unlock();

		while (true)
		{
			if (IThread::suspend(nnTimeInterval))
				break;
			if (!onInterval(0))
				break;
			if (pShouldReload)
				return false;
		}
		return true;
	}


	bool Timer::internalRunFixedNumberOfCycles()
	{
		unsigned int cycleIndex = 0;
		const unsigned int nnTimeInterval = pTimeInterval;
		pTimerMutex.unlock();

		while (true)
		{
			// Wait then execute the timer
			if (suspend(nnTimeInterval) || !onInterval(cycleIndex))
				return true;
			if (++cycleIndex >= pCycleCount) // the maximum number of cycle is reached
				return true;
			if (pShouldReload)
				return false;
		}
		return true;
	}


	bool Timer::onExecute()
	{
		pShouldReload = 0;
		while (true)
		{
			// Lock
			pTimerMutex.lock();

			// No cycle to do, aborting now
			if (!pCycleCount)
			{
				pTimerMutex.unlock();
				return true;
			}

			// infinite loop
			if (infinite == pCycleCount)
			{
				if (internalRunInfiniteLoop())
					// Stopping the thread
					return false;
			}
			else
			{
				if (internalRunFixedNumberOfCycles())
					// Stopping the thread
					return false;
			}
		}
		return false;
	}


	void Timer::interval(const unsigned int t)
	{
		pTimerMutex.lock();
		pTimeInterval = t;
		pTimerMutex.unlock();
	}


	void Timer::cycleCount(const unsigned int n)
	{
		pTimerMutex.lock();
		pCycleCount = n;
		pTimerMutex.unlock();
	}


	void Timer::reload(unsigned int interval)
	{
		pTimerMutex.lock();
		pTimeInterval = interval;
		pTimerMutex.unlock();
		pShouldReload = 1;
	}


	void Timer::reload(unsigned int interval, unsigned int cycles)
	{
		pTimerMutex.lock();
		pTimeInterval = interval;
		pCycleCount = cycles;
		pTimerMutex.unlock();
		pShouldReload = 1;
	}




} // namespace Thread
} // namespace Yuni

