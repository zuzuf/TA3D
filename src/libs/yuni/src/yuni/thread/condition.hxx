#ifndef __YUNI_THREADS_CONDITION_HXX__
# define __YUNI_THREADS_CONDITION_HXX__


namespace Yuni
{
namespace Thread
{

	# ifndef YUNI_NO_THREAD_SAFE
	# else
	inline Condition::Condition()
		:pOwnMutex(true)
	{
		pMutex = new Mutex();
	}


	inline Condition::Condition(Mutex& mutex)
		:pMutex(&mutex), pOwnMutex(false)
	{}


	inline Condition::~Condition()
	{
		if (pOwnMutex)
			delete pMutex;
	}
	# endif



	inline Mutex& Condition::mutex() const
	{
		return *pMutex;
	}


	inline void Condition::lock()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pMutex->lock();
		# endif
	}


	inline void Condition::unlock()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pMutex->unlock();
		# endif
	}


	inline void Condition::notify()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pSignalled = true;
		::pthread_cond_signal(&pCondition);
		# endif
	}


	inline void Condition::notifyAll()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pSignalled = true;
		::pthread_cond_broadcast(&pCondition);
		# endif
	}


	// ConditionLocker
	inline ConditionLocker::ConditionLocker(Condition& c)
		:pCondition(c)
	{
		c.lock();
	}

	inline ConditionLocker::~ConditionLocker()
	{
		pCondition.unlock();
	}




} // namespace Threads
} // namespace Yuni

#endif // __YUNI_THREADS_CONDITION_HXX__
