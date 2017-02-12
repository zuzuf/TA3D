
#include <iostream>
#include <yuni/thread/thread.h>
#include <yuni/thread/condition.h>
#include <yuni/core/system/sleep.h>





class MyOwnThread : public Yuni::Thread::IThread
{
public:
	//! \name Constructor & Destructor
	//@{
	//! Default constructor
	MyOwnThread() {}
	//! Destructor
	virtual ~MyOwnThread() {stop();} // Required for Code robustness
	//@}

public:
	//! The condition to wait
	Yuni::Thread::Condition::Ptr condition;

protected:

	virtual bool onStarting()
	{
		// Start by locking the condition.
		// At this time, the main thread is stuck in the ::start() method,
		// so you can be sure that it won't send notifications until
		// you unlock this condition.
		condition->lock();

		return true;
	}

	virtual bool onExecute()
	{
		std::cout << "[thread] Started. Waiting for notification." << std::endl;

		// Atomically unlock and block until a notification arrive.
		condition->waitUnlocked();
		// Here we re-acquired the lock, since we received a notification.
		{
			// Some stuff here
			std::cout << "[thread] Doing some stuff" << std::endl;
		}

		// Here you have two possibilities: either you unlock, because you are done with the condition.
		// Or, you keep the lock, and re-enter waitUnlocked() to wait the next notification.
		// Here, we unlock, cause we want to exit.
		condition->unlock();

		std::cout << "[thread] Stopped." << std::endl;
		return false;
	}

}; // class MyOwnThread





int main(void)
{
	// Our condition, which is shared between the two threads
	Yuni::Thread::Condition::Ptr condition = new Yuni::Thread::Condition();
	// A thread, which will wait for our signal
	MyOwnThread thread;
	thread.condition = condition;

	// Starting the thread
	if (!thread.start())
		return 1;

	// Here, we lock the condition, to be sure the thread has come to the point where it can
	// receive our notifications (ie, it has unlocked the condition. Indeed, it held a lock since
	// the call to thread.start() - via onStarting()).
	condition->lock();
	condition->unlock();

	// Performing a very long operation... (the thread still awaits our notification)
	std::cout << "[main] Performing a long operation (3 seconds)..." << std::endl;
	Yuni::Suspend(3); // seconds
	std::cout << "[main] Done" << std::endl;

	// Notifying our thread that it should do its work
	condition->notify();

	// Waiting for the end of the thread (timeout = default: 5s)
	thread.stop();

	return 0;
}

