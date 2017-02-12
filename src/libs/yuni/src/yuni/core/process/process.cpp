
#include "process.h"
#include "../../thread/thread.h"
#ifndef YUNI_OS_WINDOWS
# include <unistd.h>
# include <stdio.h>
# include <sys/wait.h>
# include <signal.h>
#else
#endif

// http://msdn.microsoft.com/en-us/library/ms682499%28v=vs.85%29.aspx


namespace Yuni
{
namespace Private
{
namespace Process
{

	class SubProcess : public Yuni::Thread::IThread
	{
	public:
		SubProcess(Yuni::Process& process) :
			pProcess(process),
			pArguments(nullptr)
		{
		}

		virtual ~SubProcess()
		{
			if (pArguments)
				deleteAllArguments();
		}

		void arguments(const String::Vector& args)
		{
			if (pArguments)
				deleteAllArguments();
			const unsigned int count = args.size();
			if (!count)
			{
				pArguments = nullptr;
			}
			else
			{
				pArguments = new CString[count + 1];
				for (unsigned int i = 0; i != count; ++i)
				{
					const unsigned int csize = args[i].size();
					const char* source = args[i].c_str();
					char* target = new char[csize + 1];
					memcpy(target, source, csize);
					target[csize] = '\0';
					pArguments[i] = target;
				}
				pArguments[count] = nullptr;
			}
		}


	protected:
		virtual bool onExecute()
		{
			if (!pArguments)
			{
				pProcess.pMutex.unlock();
				return true;
			}

			# ifndef YUNI_OS_WINDOWS

			int outfd[2];
			int infd[2];
			int oldstdin, oldstdout;

			// The parent is going to write into
			pipe(outfd);
			// The parent is going to read from
			pipe(infd);

			// Save the current stdin
			oldstdin = dup(0);
			// Save the current stdout
			oldstdout = dup(1);

			close(0);
			close(1);

			// Make the read end of outfd pipe as stdin
			dup2(outfd[0], 0);
			// Make the write end of infd as stdout
			dup2(infd[1],1);

			const pid_t pid =fork();
			if (!pid)
			{
				close(outfd[0]); // Not required for the child
				close(outfd[1]);
				close(infd[0]);
				close(infd[1]);

				// Should never returns
				execve("/Users/milipili/projects/yuni/sources/current/src/yuni", pArguments, nullptr);
			}
			else
			{
				close(0); // Restore the original std fds of parent
				close(1);
				dup2(oldstdin, 0);
				dup2(oldstdout, 1);

				close(outfd[0]); // These are being used by the child
				close(infd[1]);

				// cleanup
				deleteAllArguments();
				pArguments = nullptr;

				//write(outfd[1], "2^32\n",5); // Write to child’s stdin

				// The mutex has already been locked
				// pProcess.pMutex.lock();
				pProcess.pProcessInput = outfd[1];
				pProcess.pProcessID = pid;
				pProcess.pMutex.unlock();

				char* buffer = new char[4096];
				do
				{
					const ssize_t size = read(infd[0], buffer, 4095);
					if (size <= 0)
					{
						std::cout << "FAILED\n";
						break;
					}
					buffer[size] = '\0';
					std::cout << "--" << size << "-- " << buffer;
				}
				while (true);
				delete[] buffer;
			}

			# else
			# endif

			theProcessHasStopped();
			return true;
		}

		virtual void onPause()
		{
			std::cout << "PAUSE\n";
		}

		virtual void onStop()
		{
			std::cout << "STOP\n";
		}

		virtual void onKill()
		{
			theProcessHasStopped();
		}


		void theProcessHasStopped()
		{
			// Making sure that the process ID is invalid
			pProcess.pMutex.lock();
			pProcess.pProcessID = 0;
			pProcess.pProcessInput = -1;
			pProcess.pRunning = false;
			pProcess.pMutex.unlock();
		}


		void deleteAllArguments()
		{
			for (unsigned int i = 0; pArguments[i]; ++i)
				delete[] pArguments[i];
			delete[] pArguments;
		}

	private:
		typedef char* CString;

	private:
		Yuni::Process& pProcess;
		CString* pArguments;

	}; // class SubProcess



} // namespace Process
} // namespace Private
} // namespace Yuni





namespace Yuni
{

	Process::Process()
		:pThread(nullptr)
	{}


	Process::~Process()
	{
		if (pRunning)
		{
			std::cout << "Destroying : running == true\n";
			cancel();
			wait();
		}
	}


	void Process::cancel()
	{
		if (!pRunning)
			return;
		# ifndef YUNI_OS_WINDOWS
		pMutex.lock();
		const pid_t pid = static_cast<pid_t>(pProcessID);
		pMutex.unlock();
		if (pid)
			kill(pid, SIGKILL);
		# else
		# endif
	}


	bool Process::execute(unsigned int timeout)
	{
		(void) timeout;
		if (pRunning)
			return false;

		// The mutex will be unlocked by the new thread
		std::cout << "locking\n";
		pMutex.lock();
		pRunning = true;
		pProcessID = 0u;
		pProcessInput = -1;

		if (!pThread)
			pThread = new Yuni::Private::Process::SubProcess(*this);
		else
			pThread->stop();

		pArguments.clear();
		pArguments.push_back("/Users/milipili/projects/yuni/sources/current/src/yuni/long");
		pArguments.push_back("-q");
		pThread->arguments(pArguments);

		std::cout << "starting................\n";
		pThread->start();

		return false;
	}


	void Process::wait()
	{
		if (pRunning)
		{
			pMutex.lock();
			std::cout << "Waiting...\n";
			ThreadPtr thread = pThread;
			pMutex.unlock();
			if (!(!thread))
				thread->stop();
		}
	}



} // namespace Yuni

