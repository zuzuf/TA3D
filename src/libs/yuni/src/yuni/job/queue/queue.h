#ifndef __YUNI_JOB_QUEUE_QUEUE_H__
# define __YUNI_JOB_QUEUE_QUEUE_H__

# include "../../yuni.h"
# include "../job.h"
# include "waitingroom.h"
# include "../scheduler/highestpriorityfirst.h"



namespace Yuni
{
namespace Job
{

	/*!
	** \brief A Multithreaded Job QueueService
	**
	** - Breve description du fonctionnement (file d'attente et mode d'execution)
	** - Exemple d'utilisation centré sur la file (comment on déclare un job: \see Job::IJob)
	**
	** \tparam SchedulerT The thread Scheduler policy
	*/
	template<
		class SchedulerT = Scheduler::HighestPriorityFirst // The Scheduler Policy
		>
	class QueueService
		:public Policy::ObjectLevelLockable<QueueService<SchedulerT> >
		,public SchedulerT
	{
	public:
		//! QueueService
		typedef QueueService<SchedulerT> QueueServiceType;
		//! The threading policy
		typedef Policy::ObjectLevelLockable<QueueServiceType> ThreadingPolicy;
		//! The most suitable smart pointer for the class
		typedef SmartPtr<QueueServiceType> Ptr;

		//! The Scheduler policy
		typedef SchedulerT  SchedulerPolicy;

		enum
		{
			//! A default timeout
			defaultTimeout = Yuni::Thread::IThread::defaultTimeout,
		};

		/*!
		** \brief Information about a single thread
		*/
		class ThreadInfo
		{
		public:
			//! The most suitable smart pointer for the class
			typedef SmartPtr<ThreadInfo> Ptr;
			//! Vector of ThreadInfo
			typedef std::vector<typename ThreadInfo::Ptr>  Vector;

		public:
			//! Reference to the working thread
			Thread::IThread::Ptr thread;
			//! Reference to the job currently in execution
			Job::IJob::Ptr job;

			//! Flag to know if the thread has a job currently in execution
			bool hasJob;

			//! State of the job (if any)
			Job::State state;
			//! Flag to know if the job is canceling its work
			bool canceling;
			//! Progression (in percent) of the job (if any, between 0 and 100)
			int progression;
			//! Name of the job
			String name;

		}; // class ThreadInfo


	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		QueueService();
		/*!
		** \brief Constructor, with an autostart mode
		**
		** \param autostart True to automatically start the service
		*/
		explicit QueueService(bool autostart);
		/*!
		** \brief Destructor
		*/
		~QueueService();
		//@}


		//! \name Service
		//@{
		/*!
		** \brief Start the service and execute the jobs
		*/
		bool start();

		/*!
		** \brief Wait until all jobs are finished
		** \param timeout Timeout
		** \param pollInterval Interval in milliseconds between each poll when waiting
		** \return True if no all jobs are finished, false if the timeout has been reached
		*/
		bool wait(unsigned int timeout = defaultTimeout * 2, unsigned int pollInterval = 70);

		/*!
		** \brief Stop the service
		**
		** All unfinished jobs will be kept and re-executed at the next start.
		** It is of their responsibility to properly resume if they have to.
		** All working threads will be destroyed at the very end of this method.
		*/
		bool stop(unsigned int timeout = defaultTimeout);

		/*!
		** \brief Stop then start the service
		**
		** All unfinished jobs will be kept and re-executed at the next start.
		** It is of their responsibility to properly resume if they have to.
		*/
		bool restart(unsigned int timeout = defaultTimeout);

		/*!
		** \brief Get if the service is started
		*/
		bool started() const;
		//@}


		//! \name Jobs handling
		//@{
		/*!
		** \brief Add a job into the queue
		**
		** \warning The job may already be into the queue. However it must ensure
		** its thread-safety in this case.
		**
		** \param job The job to add
		*/
		void add(IJob* job);

		/*!
		** \brief Add a job into the queue
		**
		** \warning The job may already be into the queue. However it must ensure
		** its thread-safety in this case.
		**
		** \param job The job to add
		*/
		void add(const IJob::Ptr& job);

		/*!
		** \brief Add a job into the queue
		**
		** \warning The job may already be into the queue. However it must ensure
		** its thread-safety in this case.
		**
		** \param job The job to add
		** \param priority Its priority execution
		*/
		void add(const IJob::Ptr& job, Priority priority);

		/*!
		** \brief Add a job into the queue
		**
		** \warning The job may already be into the queue. However it must ensure
		** its thread-safety in this case.
		**
		** \param job The job to add
		** \param priority Its priority execution
		*/
		void add(IJob* job, Priority priority);

		/*!
		** \brief Retrieve information about the activity of the queue manager
		**
		** \note Event if in the list, a job may already have finished
		**   its work at the end of this method.
		*/
		void activitySnapshot(typename ThreadInfo::Vector& out);

		/*!
		** \brief Get the number of jobs waiting to be executed
		**
		** This value does not take into account the number of jobs
		** currently running.
		*/
		unsigned int size() const;

		//! \see size()
		unsigned int count() const;
		//@}


		//! \name Threads
		//@{
		/*!
		** \brief Get the number of threads
		*/
		unsigned int threadCount() const;
		//@}


		//! \name Operators
		//@{
		//! The operator << (add a job)
		QueueService& operator += (IJob* job);
		//! The operator << (add a job)
		QueueService& operator << (IJob* job);
		//! The operator << (add a job)
		QueueService& operator += (const IJob::Ptr& job);
		//! The operator << (add a job)
		QueueService& operator << (const IJob::Ptr& job);
		//@}


	private:
		//! Flag to know if the service is started
		Atomic::Int<32> pStarted;
		//! The list of all remaining jobs
		Yuni::Private::QueueService::WaitingRoom pWaitingRoom;

	}; // class QueueService






} // namespace Job
} // namespace Yuni

# include "queue.hxx"

#endif // __YUNI_JOB_QUEUE_QUEUE_H__
