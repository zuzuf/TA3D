#include <yuni/yuni.h>
#include <yuni/thread/thread.h>
#include <iostream>
#include <yuni/core/system/sleep.h>
#include <yuni/core/system/cpu.h>



//! This mutex will serve to synchronize the standard output.
static Yuni::Mutex mutex;




/*!
** \brief This class represents a task that should be executed
** in a separate thread.
**
** This task is implemented in the onExecute() method, and consists
** here of a sample: counting beer bottles.
*/
class BottleTask : public Yuni::Thread::IThread
{
public:
	/*!
	** \brief Our sample constructor.
	** \param[in] identifier A thread identifier, used only for display
	**            purposes
	*/
	BottleTask(const int identifier) :Yuni::Thread::IThread(), x(identifier) {}

	virtual ~BottleTask() {stop();}

protected:
	//! The beer-bottle counting implementation itself
	virtual bool onExecute()
	{
		// Count from 99
		int i = 99;

		while (true)
		{
			mutex.lock();
			std::cout << " ["<< x <<"] " << i-- << " bottles of beer on a wall. Calculating a very complicated thing ..." << std::endl;
			mutex.unlock();

			// Simulate some work. The work of the thread is here simulated by
			// a call to sleep(). This is intended to simulate a period of
			// work during which the task cannot be interrupted by stop() methods
			// - for example a long calculation, for the purpose of this example.
			//
			// The bottom line is: DO NOT use sleep() to wait in threads.
			Yuni::Suspend(1 /* second */);

			// After our work, if the task is repetitive, we will want to check
			// if we have to stop doing it and will eventually pause for some
			// seconds (while listening to any stop signal).
			// This is the purpose of the suspend function. We give 100ms here as
			// the time to wait before returning if no stop() signal is received.
			//
			// suspend() will either return false - the specified time elapsed
			// without any events - or return true - a stop signal was received.
			//
			// In the latter case, we may have a timeout before being killed,
			// so if possible defer any time-consuming task to the onStopped()
			// methods.
			if (suspend(100))
				return false;
		}
		return false;
	}

	/*!
	** \brief Things to do when the thread is stopped
	**
	** In this, we just signal to the user that the thread
	** execution has stopped, but we can also free thread-specific
	** resources and do general cleanup.
	*/
	virtual void onStopped()
	{
		std::cout << " [b] I have been interrupted." << std::endl;
	}

private:
	//! The class can hold the variables of your choice.
	int x;

}; // class BottleTask





int main(void)
{
	// For thread-planning purposes, Yuni provides an indication of
	// how many CPUs the system has, so you can plan how you will
	// manage your resources.
	std::cout << "[M] Your system has " << Yuni::System::CPU::Count()
		<< " logical processor(s)." << std::endl;

	// Here we create a new BottleTask with identifier 0 - keep in
	// mind that this identifier is specific to our sample class.
	// We can use it as an abstract or specific class, depending
	// on the way we want to manage it.
	Yuni::Thread::IThread* t = new BottleTask(0);
	std::cout << "[M] Starting bottle counting..." << std::endl;

	// Start counting bottles.
	t->start();

	// Then we can do all the processing we want, knowing that the
	// bottle counting task is running in background. But, beware,
	// because of the mutual access to the standard output, we
	// should lock a mutex before printing anything on it.
	mutex.lock();
	std::cout << "[M] Doing some processing here too." << std::endl;
	mutex.unlock();

	// Simulate a long processing
	Yuni::Suspend(5); // 5 seconds

	mutex.lock();
	std::cout << "[M] Stopping bottle counting..." << std::endl;
	mutex.unlock();

	// We then stop the bottle-counting thread. It will exit as
	// soon as possible (on its next call to suspend() for example),
	// but will be killed anyway after a timeout, in this case 4 seconds.
	t->stop(4);

	// We do not have to use mutexes anymore.
	std::cout << "[M] Thread stopped." << std::endl;

	// Delete the thread object.
	delete t;

	return 0;
}

