#ifndef __YUNI_JOB_SCHEDULER_HIGHEST_PRIORITY_FIRST_H__
# define  __YUNI_JOB_SCHEDULER_HIGHEST_PRIORITY_FIRST_H__

# include "../../yuni.h"
# include "../queue/waitingroom.h"
# include "../job.h"
# include "../queue/thread.h"
# include "../../thread/array.h"



namespace Yuni
{
namespace Job
{
namespace Scheduler
{


	/*!
	** \brief Basic Scheduler, where the job with the highest priority is executed in first
	*/
	class HighestPriorityFirst
	{
	public:
		//! The scheduler itself
		typedef HighestPriorityFirst SchedulerType;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		**
		** \param room Reference to the Waiting room
		*/
		explicit HighestPriorityFirst(Private::QueueService::WaitingRoom& room);

		//! Destructor
		~HighestPriorityFirst();
		//@}

		unsigned int minimumThreadCount() const
		{
			return pMaximumThreadCount;
		}

		bool minimumThreadCount(unsigned int)
		{
			return false;
		}

		unsigned int maximumThreadCount() const
		{
			return pMaximumThreadCount;
		}

		bool maximumThreadCount(unsigned int n)
		{
			if (n < 1 || n > 512)
				return false;
			pMaximumThreadCount = n;
			return true;
		}

		/*!
		** \brief Get if the scheduler is idle
		*/
		bool idle() const;

	protected:
		/*!
		** \brief Start all threads to execute the jobs
		*/
		bool schedulerStart()
		{
			if (!pStarted)
			{
				pStarted = 1;
				pServiceMutex.lock();

				// Creating all threads we need
				pThreads.clear();
				for (unsigned int i = 0; i != pMaximumThreadCount; ++i)
					pThreads += new Yuni::Private::Jobs::QueueThread<SchedulerType>(*this);

				// Starting all threads at once
				pThreads.start();
				pServiceMutex.unlock();
			}
			return true;
		}


		/*!
		** \brief Stop all working threads
		*/
		bool schedulerStop(unsigned int timeout)
		{
			if (pStarted)
			{
				pStarted = 0;
				pServiceMutex.lock();
				// Stopping all threads
				pThreads.stop(timeout);
				pThreads.clear();
				pServiceMutex.unlock();
			}
			return true;
		}

		/*!
		** \brief Event: A job has just been added into the waiting room
		**
		** \param priority The priority of this job
		*/
		void schedulerNotifyNewJobInWaitingRoom(Yuni::Job::Priority)
		{
			pThreads.wakeUp();
		}


		/*!
		** \brief Get the next job to execute
		*/
		bool nextJob(IJob::Ptr& out)
		{
			while (!pWaitingRoom.empty())
			{
				if (pWaitingRoom.hasJob[priorityHigh])
				{
					if (pWaitingRoom.pop(out, priorityHigh))
						return true;
					continue;
				}
				if (pWaitingRoom.hasJob[priorityDefault])
				{
					if (pWaitingRoom.pop(out, priorityDefault))
						return true;
					continue;
				}
				if (pWaitingRoom.hasJob[priorityLow])
				{
					if (pWaitingRoom.pop(out, priorityLow))
						return true;
					continue;
				}
			}
			return false;
		}


		/*!
		** \brief Get the number of threads currently in use
		*/
		unsigned int schedulerThreadCount() const
		{
			return pThreads.count();
		}

		template<class PredicateT>
		void schedulerForeachThread(PredicateT& predicate)
		{
			pThreads.foreachThread(predicate);
		}

		void schedulerIncrementWorkerCount()
		{
			++pWorkerCount;
		}

		void schedulerDecrementWorkerCount()
		{
			--pWorkerCount;
		}


	private:
		//! Index of the next thread to wake up
		Atomic::Int<32> pRunningThreads;

		//! Reference to the array of threads
		Yuni::Thread::Array<Yuni::Private::Jobs::QueueThread<SchedulerType> >  pThreads;
		//! Reference to the waiting room
		Private::QueueService::WaitingRoom& pWaitingRoom;
		//! Mutex, used for start/stop methods
		Mutex pServiceMutex;
		//! Flag to know if the scheduler is started
		Atomic::Int<32> pStarted;
		//! Total number of workers
		Atomic::Int<32> pWorkerCount;

		//! The maximum number of thread
		unsigned int pMaximumThreadCount;

		// friend
		template<class T> friend class Yuni::Private::Jobs::QueueThread;

	}; // class HighestPriorityFirst





} // namespace Scheduler
} // namespace Job
} // namespace Yuni

# include "highestpriorityfirst.hxx"

#endif // __YUNI_JOB_SCHEDULER_HIGHEST_PRIORITY_FIRST_H__
