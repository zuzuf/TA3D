#ifndef __YUNI_THREAD_TIMER_HXX__
# define __YUNI_THREAD_TIMER_HXX__


namespace Yuni
{
namespace Thread
{

	inline Timer::Timer()
		:IThread(), pTimeInterval(defaultInterval), pCycleCount(infinite)
	{}


	inline Timer::Timer(const unsigned int interval)
		:IThread(), pTimeInterval(interval), pCycleCount(infinite)
	{}


	inline Timer::Timer(const unsigned int interval, const unsigned int cycles)
		:IThread(), pTimeInterval(interval), pCycleCount(cycles)
	{}


	inline Timer::~Timer()
	{
		assert(started() == false);
	}


	inline unsigned int Timer::interval() const
	{
		Yuni::MutexLocker locker(pTimerMutex);
		return pTimeInterval;
	}


	inline unsigned int Timer::cycleCount() const
	{
		Yuni::MutexLocker locker(pTimerMutex);
		return pCycleCount;
	}


	inline void Timer::reload()
	{
		pShouldReload = 1;
	}




} // namespace Thread
} // namespace Yuni

#endif // __YUNI_THREAD_TIMER_HXX__
