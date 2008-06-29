#ifndef __TA3D_THREADS_MUTEX_H__
# define __TA3D_THREADS_MUTEX_H__


#ifndef TA3D_PLATFORM_WINDOWS
#  include <pthread.h>
#else
#  include <windows.h>
#endif



namespace TA3D
{


    class Mutex 
    {
    public:
        Mutex();
        ~Mutex();

        /*!
        ** \brief Lock the mutex
        */
		void lock()
		{
            # ifdef TA3D_PLATFORM_WINDOWS
			::EnterCriticalSection(&pCritSection);
            # else
			pthread_mutex_lock(&pPthreadLock);
            # endif
            pLocked = true;
		}

        /*!
        ** \brief Release the lock
        */
		void unlock()
		{
            pLocked = false;
            # ifdef TA3D_PLATFORM_WINDOWS
			::LeaveCriticalSection(&pCritSection);
            # else
			pthread_mutex_unlock(&pPthreadLock);
            # endif
		}
	
        /*!
        ** \brief Get if the mutex locked
        ** \return True if the mutex is locked
        */
        bool isLocked() const {return pLocked;}

    private:
        bool pLocked;
        # ifdef TA3D_PLATFORM_WINDOWS
		CRITICAL_SECTION pCritSection;
        # else
		pthread_mutex_t pPthreadLock;
        # endif

    }; // class Mutex



    /*! \class MutexLocker
    **
    ** \code
    **      class Foo
    **      {
    **      public:
    **          Foo() : pValue(42) {}
    **          ~Foo() {}
    **          int getValue()
    **          {
    **              MutexLocker locker(pMutex);
    **              return pValue;
    **          }
    **          void setValue(const int i)
    **          {
    **              pMutex.lock();
    **              pValue = i;
    **              pMutex.unlock();
    **          }
    **      private:
    **          int pValue;
    **          Mutex pMutex;
    **      };
    ** \endcode
    */
    class MutexLocker
    {
    public:
        MutexLocker(Mutex& m) : pMutex(m) { m.lock(); }
        ~MutexLocker() { pMutex.unlock(); }
    private:
        Mutex& pMutex;

    }; // MutexLocker

} // namespace TA3D


#endif // __TA3D_THREADS_MUTEX_H__
