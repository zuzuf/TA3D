#ifndef __YUNI_JOB_QUEUE_THREAD_HXX__
# define __YUNI_JOB_QUEUE_THREAD_HXX__


namespace Yuni
{
namespace Private
{
namespace Jobs
{

	template<class SchedulerT>
	inline QueueThread<SchedulerT>::QueueThread(SchedulerType& scheduler)
		:pScheduler(scheduler)
	{}


	template<class SchedulerT>
	inline QueueThread<SchedulerT>::~QueueThread()
	{
		// Ensure that the thread is really stopped
		stop();
	}


	template<class SchedulerT>
	inline Yuni::Job::IJob::Ptr QueueThread<SchedulerT>::currentJob() const
	{
		return pJob;
	}


	template<class SchedulerT>
	bool QueueThread<SchedulerT>::onExecute()
	{
		// assert
		assert(this != NULL && "Queue: Thread: Oo `this' is null !?");

		// Notify the scheduler that this thread begins its work
		pScheduler.schedulerIncrementWorkerCount();

		// Asking for the next job
		while (pScheduler.nextJob(pJob))
		{
			// Cancellation point
			// It is actually better to perform the test before executing the job
			if (shouldAbort())
			{
				// Notify the scheduler that this thread does no longer work
				pScheduler.schedulerDecrementWorkerCount();
				// We have to stop, no need for hibernating
				return false;
			}

			// Execute the job, via a wrapper for visibility issues
			Yuni::Private::QueueService::JobAccessor<Yuni::Job::IJob>::Execute(*pJob, this);

			// We must release our pointer to the job here to avoid its destruction
			// in `pScheduler.nextJob()` (when `pJob` is re-assigned).
			// This method uses a lock and the destruction of the job may take some
			// time.
			// However, there is absolutely no guarantee that the job will be
			// destroyed in this thread.
			pJob = nullptr;
		}

		// Notify the scheduler that this thread does no longer work
		pScheduler.schedulerDecrementWorkerCount();

		// Returning true, for hibernation
		return true;
	}



	template<class SchedulerT>
	void QueueThread<SchedulerT>::onKill()
	{
		if (!(!(pJob)))
		{
			Yuni::Private::QueueService::JobAccessor<Yuni::Job::IJob>::ThreadHasBeenKilled(*pJob);
			pJob = nullptr;
		}
	}





} // namespace Jobs
} // namespace Private
} // namespace Yuni

#endif // __YUNI_JOB_QUEUE_THREAD_HXX__
