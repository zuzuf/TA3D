#ifndef __YUNI_JOB_FORWARD_H__
# define __YUNI_JOB_FORWARD_H__

# include "enum.h"


namespace Yuni
{
namespace Job
{

	// Forward declaration
	class IJob;

} // namespace Job
} // namespace Yuni



namespace Yuni
{
namespace Private
{
namespace QueueService
{

	// Forward declaration
	class WaitingRoom;


	template<class JobT>
	class JobAccessor
	{
	public:
		template<class ThreadT>
		static void Execute(JobT& job, ThreadT* thread)
		{
			job.execute(thread);
		}

		static void ThreadHasBeenKilled(JobT& job)
		{
			job.pThread = NULL;
			job.pState = Yuni::Job::stateWaiting;
		}

		static void AddedInTheWaitingRoom(JobT& job)
		{
			job.pState = Yuni::Job::stateWaiting;
			job.pProgression = 0;
		}
	};





} // namespace QueueService
} // namespace Private
} // namespace Yuni



#endif // __YUNI_JOB_FORWARD_H__
