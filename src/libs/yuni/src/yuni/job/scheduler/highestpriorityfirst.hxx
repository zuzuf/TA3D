#ifndef __YUNI_JOB_SCHEDULER_HIGHEST_PRIORITY_FIRST_HXX__
# define  __YUNI_JOB_SCHEDULER_HIGHEST_PRIORITY_FIRST_HXX__


namespace Yuni
{
namespace Job
{
namespace Scheduler
{


	inline HighestPriorityFirst::HighestPriorityFirst(Private::QueueService::WaitingRoom& room)
		:pWaitingRoom(room), pStarted(0), pMaximumThreadCount(2)
	{
	}


	inline HighestPriorityFirst::~HighestPriorityFirst()
	{
		if (pStarted)
		{
			pStarted = 0;
			pServiceMutex.lock();
			pThreads.stop();
			pServiceMutex.unlock();
		}
	}


	inline bool HighestPriorityFirst::idle() const
	{
		return (0 == pWorkerCount);
	}





} // namespace Scheduler
} // namespace Job
} // namespace Yuni

#endif //  __YUNI_JOB_SCHEDULER_HIGHEST_PRIORITY_FIRST_HXX__


