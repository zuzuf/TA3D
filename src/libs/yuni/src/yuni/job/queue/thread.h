#ifndef __YUNI_JOB_QUEUE_THREAD_H__
# define __YUNI_JOB_QUEUE_THREAD_H__

# include "../../thread/thread.h"
# include "../job.h"


namespace Yuni
{
namespace Private
{
namespace Jobs
{


	/*!
	** \brief A single thread for a queue service
	*/
	template<class SchedulerT>
	class QueueThread : public Yuni::Thread::IThread
	{
	public:
		//! QueueThread
		typedef QueueThread<SchedulerT> QueueThreadType;
		//! The most suitable smart pointer for the class
		typedef SmartPtr<QueueThreadType> Ptr;
		//! Scheduler Policy
		typedef SchedulerT SchedulerType;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		explicit QueueThread(SchedulerType& scheduler);
		//! Destructor
		virtual ~QueueThread();
		//@}


		/*!
		** \brief Get the Job currently running
		*/
		Yuni::Job::IJob::Ptr currentJob() const;


	protected:
		/*!
		** \brief Implementation of the `onExecute` method to run the jobs from the waiting room
		*/
		virtual bool onExecute();

		/*!
		** \brief Implementation of the `onKill` method when the thread is killed without mercy
		*/
		virtual void onKill();

	private:
		//! The scheduler
		SchedulerType& pScheduler;
		//! The current job
		Yuni::Job::IJob::Ptr pJob;

	}; // class QueueThread






} // namespace Jobs
} // namespace Private
} // namespace Yuni

# include "thread.hxx"

#endif // __YUNI_JOB_QUEUE_THREAD_H__
