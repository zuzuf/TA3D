
# include "../../yuni.h"
#include "waitingroom.h"


namespace Yuni
{
namespace Private
{
namespace QueueService
{


	void WaitingRoom::add(const Yuni::Job::IJob::Ptr& job)
	{
		// Locking the priority queue
		// We should avoid ThreadingPolicy::MutexLocker since it may not be
		// the good threading policy for these mutexes
		pMutexes[priorityDefault].lock();

		// Resetting some internal variables of the job
		Yuni::Private::QueueService::JobAccessor<Yuni::Job::IJob>::AddedInTheWaitingRoom(*job);
		// Adding it into the good priority queue
		pJobs[priorityDefault].push_back(job);

		// Resetting our internal state
		hasJob[priorityDefault] = 1;
		++pJobCount;

		// Unlocking
		pMutexes[priorityDefault].unlock();
	}


	void WaitingRoom::add(const Yuni::Job::IJob::Ptr& job, Yuni::Job::Priority priority)
	{
		// Locking the priority queue
		// We should avoid ThreadingPolicy::MutexLocker since it may not be
		// the good threading policy for these mutexes
		pMutexes[priority].lock();

		// Resetting some internal variables of the job
		Yuni::Private::QueueService::JobAccessor<Yuni::Job::IJob>::AddedInTheWaitingRoom(*job);
		// Adding it into the good priority queue
		pJobs[priority].push_back(job);

		// Resetting our internal state
		hasJob[priority] = 1;
		++pJobCount;

		// Unlocking
		pMutexes[priority].unlock();
	}


	bool WaitingRoom::pop(Yuni::Job::IJob::Ptr& out, const Yuni::Job::Priority priority)
	{
		// We should avoid ThreadingPolicy::MutexLocker since it may not be
		// the good threading policy for these mutexes
		pMutexes[priority].lock();

		if (!pJobs[priority].empty())
		{
			// It remains at least one job to run !
			out = pJobs[priority].front();
			// Removing it from the list of waiting jobs
			pJobs[priority].pop_front();
			// Resetting atomic variables about the internal status
			hasJob[priority] = pJobs[priority].empty() ? 0 : 1;

			--pJobCount;

			pMutexes[priority].unlock();
			return true;
		}

		// It does not remain any job for this priority. Aborting.
		// Resetting some variable
		hasJob[priority] = 0;

		// Global status
		pMutexes[priority].unlock();
		return false;
	}




} // namespace QueueService
} // namespace Private
} // namespace Yuni


