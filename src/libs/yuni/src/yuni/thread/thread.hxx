#ifndef __YUNI_THREAD_THREAD_HXX__
# define __YUNI_THREAD_THREAD_HXX__

# include <cassert>


namespace Yuni
{
namespace Thread
{



	inline IThread::~IThread()
	{
		assert(pStarted == false && "A thread can not be destroyed while being still started");
	}


	inline bool IThread::started() const
	{
		return pStarted;
	}


	inline bool IThread::shouldAbort() const
	{
		ConditionLocker locker(pMustStopCond);
		return (pShouldStop || !pStarted);
	}


	inline bool IThread::operator ! () const
	{
		return !started();
	}




} // namespace Thread
} // namespace Yuni

#endif // __YUNI_THREAD_THREAD_HXX__
