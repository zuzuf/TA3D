
#include "job.h"
#include "../core/math.h"



namespace Yuni
{
namespace Job
{


	bool IJob::suspend(const unsigned int delay) const
	{
		// This method must only be called from a thread
		assert(pThread != NULL && "Job: The pointer to the attached thread must not be NULL");

		// We can suspend the job only if it is running
		if (pState == stateRunning)
		{
			// It is important (for thread-safety reasons) that this method
			// does not modify the state.
			// This may lead to unwanted behaviors.
			// Sleeping for a while...
			const bool r = pThread->suspend(delay);
			// The state may have changed while we were sleeping
			return (pCanceling || r);
		}
		return true;
	}


	void IJob::execute(Thread::IThread* t)
	{
		// Reseting data
		// We will keep the state in `waiting` until we have properly set
		// all other values
		pThread = t;
		pCanceling = 0;
		pProgression = 0;

		// Here we go !
		pState = stateRunning;
		// Execute the specific implementation of the job
		onExecute();
		// The state must be reset to idle as soon as possible while the
		// other values are still valid.
		pState = stateIdle;

		// Other values
		pThread = NULL;
		pProgression = 100;
	}





} // namespace Job
} // namespace Yuni
