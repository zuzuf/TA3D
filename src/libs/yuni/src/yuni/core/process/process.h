#ifndef __YUNI_CORE_PROCESS_PROCESS_H__
# define __YUNI_CORE_PROCESS_PROCESS_H__

# include "../../yuni.h"
# include "../../core/string.h"
# include "../atomic/int.h"
# include "fwd.h"


namespace Yuni
{

	/*!
	** The class is thread-safe
	*/
	class Process : public Policy::ObjectLevelLockable<Process>
	{
	public:
		//! The threading policy
		typedef Policy::ObjectLevelLockable<Process> ThreadingPolicy;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Process();
		//! Destructor
		virtual ~Process();
		//@}

		/*!
		** \brief Execute the process
		*/
		bool execute(unsigned int timeout = 0u);

		/*!
		** \brief Wait for the end of the sub-process
		*/
		void wait();

		/*!
		** \brief Cancel the execution of the sub-process
		*/
		void cancel();

		//! Get if the process is currently running
		bool running() const;

	private:
		typedef SmartPtr<Yuni::Private::Process::SubProcess>  ThreadPtr;;

	private:
		//! The command
		String::Vector pArguments;
		//! The working directory
		String pWorkingDirectory;
		//! Flag to know if the process is running
		Atomic::Int<> pRunning;
		//! PID
		unsigned int pProcessID;
		//! input file descriptors
		int pProcessInput;
		//! Thread
		ThreadPtr pThread;

		// friend !
		friend class Yuni::Private::Process::SubProcess;

	}; // class Process





} // namespace Yuni

# include "process.hxx"

#endif // __YUNI_CORE_PROCESS_PROCESS_H__
