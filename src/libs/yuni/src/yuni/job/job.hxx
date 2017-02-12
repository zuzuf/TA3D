#ifndef __YUNI_JOB_AJOB_HXX__
# define __YUNI_JOB_AJOB_HXX__


namespace Yuni
{
namespace Job
{

	inline IJob::IJob()
		:pState(stateIdle), pProgression(0), pCanceling(), pThread(NULL)
	{}


	inline IJob::~IJob()
	{
		assert(this != NULL && "IJob: Destructor: Oo `this' is null !?");
		assert(pThread == NULL && "A job can not be attached to a thread when destroyed");
	}



	inline enum Job::State IJob::state() const
	{
		return (enum Job::State) ((sint32) pState);
	}


	inline bool IJob::idle() const
	{
		return ((pState & stateIdle) ? true : false);
	}

	inline bool IJob::waiting() const
	{
		return ((pState & stateWaiting) ? true : false);
	}

	inline bool IJob::running() const
	{
		return ((pState & stateRunning) ? true : false);
	}


	inline void IJob::cancel()
	{
		pCanceling = 1;
	}


	inline String IJob::name() const
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return pName;
	}


	template<class StringT> inline void IJob::name(const StringT& s)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		pName = s;
	}


	inline void IJob::progression(const int p)
	{
		pProgression = ((p < 0) ? 0 : (p > 100 ? 100 : p));
	}


	inline bool IJob::finished() const
	{
		// The state must be at the very end
		return (pProgression >= 100 && pState == stateIdle);
	}


	inline bool IJob::shouldAbort() const
	{
		assert(pThread != NULL && "Job: The pointer to the attached thread must not be NULL");
		return (pCanceling || pThread->shouldAbort());
	}


	template<class T>
	void IJob::fillInformation(T& info)
	{
		// The first important value is the state
		info.state = (Yuni::Job::State) ((int) (pState));
		// Then, if the job is canceling its work
		info.canceling = pCanceling;

		info.progression = pProgression;

		ThreadingPolicy::MutexLocker locker(*this);
		info.name = pName;
	}



	template<class StringT>
	inline void IJob::nameWL(const StringT& newName)
	{
		pName = newName;
	}




} // namespace Job
} // namespace Yuni

#endif // __YUNI_JOB_AJOB_HXX__
