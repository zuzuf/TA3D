#ifndef __YUNI_THREADS_MUTEX_HXX__
# define __YUNI_THREADS_MUTEX_HXX__


namespace Yuni
{

	inline Mutex::Mutex()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		::pthread_mutexattr_t mutexattr;
		::pthread_mutexattr_init(&mutexattr);
		# if defined(YUNI_OS_DARWIN) || defined(YUNI_OS_FREEBSD) || defined(YUNI_OS_SOLARIS) || defined(YUNI_OS_SUNOS) || defined(YUNI_OS_HAIKU) || defined(YUNI_OS_CYGWIN)
		::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
		# else
		::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
		# endif
		::pthread_mutex_init(&pPthreadLock, &mutexattr);
		::pthread_mutexattr_destroy(&mutexattr);
		# endif
	}


	inline Mutex::Mutex(const bool recursive)
	{
		# ifndef YUNI_NO_THREAD_SAFE
		if (recursive)
		{
			::pthread_mutexattr_t mutexattr;
			::pthread_mutexattr_init(&mutexattr);
			# if defined(YUNI_OS_DARWIN) || defined(YUNI_OS_FREEBSD) || defined(YUNI_OS_SOLARIS) || defined(YUNI_OS_SUNOS) || defined(YUNI_OS_HAIKU) || defined(YUNI_OS_CYGWIN)
			::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
			# else
			::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
			# endif
			::pthread_mutex_init(&pPthreadLock, &mutexattr);
			::pthread_mutexattr_destroy(&mutexattr);
		}
		else
			::pthread_mutex_init(&pPthreadLock, NULL);
		# else
		(void) recursive;
		# endif
	}


	inline Mutex::~Mutex()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pthread_mutex_destroy(&pPthreadLock);
		# endif
	}


	inline void Mutex::lock()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pthread_mutex_lock(&pPthreadLock);
		# endif
	}


	inline void Mutex::unlock()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		pthread_mutex_unlock(&pPthreadLock);
		# endif
	}


	# ifndef YUNI_NO_THREAD_SAFE
	inline pthread_mutex_t& Mutex::pthreadMutex()
	{
		return pPthreadLock;
	}
	# endif



	inline MutexLocker::MutexLocker(Mutex& m)
		:pMutex(m)
	{
		m.lock();
	}


	inline MutexLocker::~MutexLocker()
	{
		pMutex.unlock();
	}




} // namespace Yuni

#endif // __YUNI_THREADS_MUTEX_HXX__
