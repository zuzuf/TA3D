#ifndef __YUNI_JOB_ENUM_H__
# define __YUNI_JOB_ENUM_H__


namespace Yuni
{
namespace Job
{

	/*!
	** \brief Set of possible states for a single job
	**
	** \ingroup Jobs
	*/
	enum State
	{
		//! The job does nothing (can be already done)
		stateIdle = 0,
		//! The job is waiting for being executed
		stateWaiting,
		//! The job is currently running
		stateRunning,
	};



	enum Priority
	{
		//! Low priority
		priorityLow = 0,
		//! Default priority
		priorityDefault = 1,
		//! High priority
		priorityHigh = 2,

		priorityCount = 3,
	};




} // namespace Job
} // namespace Yuni

#endif // __YUNI_JOB_ENUM_H__
