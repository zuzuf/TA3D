#include <yuni/yuni.h>
#include <yuni/thread/thread.h>
#include <yuni/job/job.h>
#include <yuni/job/queue.h>
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
class MyJob : public Yuni::Job::IJob
{
public:
	MyJob(const int identifier)
		:x(identifier)
	{
		nameWL("Bottle counting");
	}
	virtual ~MyJob() { }

private:
	virtual void onExecute()
	{
		mutex.lock();
		std::cout << " ["<< x <<"] Starting..." << std::endl;
		mutex.unlock();

		// Count from 10
		int i = 5;

		while (i > 0)
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
			Yuni::SuspendMilliSeconds(1200); // seconds
		}
		mutex.lock();
		std::cout << " ["<< x <<"] Finished." << std::endl;
		mutex.unlock();
	}

private:
	//! The class can hold the variables of your choice.
	int x;

}; // class MyJob
 





int main(void)
{
	// Here we create a new MyJob with identifier 0 - keep in
	// mind that this identifier is specific to our sample class.
	// We can use it as an abstract or specific class, depending
	// on the way we want to manage it.

	Yuni::Job::QueueService<> qs;

	for (int job = 1; job <= 5; ++job)
		qs.add(new MyJob(job));

	std::cout << "[M] Starting bottle counting..." << std::endl;
	qs.start();

	// Then we can do all the processing we want, knowing that the
	// bottle counting task is running in background. But, beware,
	// because of the mutual access to the standard output, we
	// should lock a mutex before printing anything on it.
	
//	mutex.lock();
//	std::cout << "[M] Doing some processing here too." << std::endl;
//	mutex.unlock();

	// Simulate a long processing
//	Yuni::Suspend(5 /* seconds */);

	// Waiting for our tasks to complete.
//	mutex.lock();
//	std::cout << "[M] Main thread processing is over." << std::endl;
//	std::cout << "[M] Waiting bottle counting tasks..." << std::endl;
//	mutex.unlock();

	mutex.lock();
	std::cout << "[M] Wait #1" << std::endl;
	mutex.unlock();

	qs.wait();

	mutex.lock();
	std::cout << "[M] Wait #2" << std::endl;
	mutex.unlock();

	qs.wait();

	mutex.lock();
	std::cout << "[M] Stop #1" << std::endl;
	mutex.unlock();

	qs.stop();


	// We do not have to use mutexes anymore.
	std::cout << "[M] Thread stopped." << std::endl;

	return 0;
}

