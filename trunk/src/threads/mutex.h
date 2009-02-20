#ifndef __TA3D_THREADS_MUTEX_H__
# define __TA3D_THREADS_MUTEX_H__


#include <SDL_thread.h>


namespace TA3D
{


    /*! \class Mutex
    **
    ** \brief  Mechanism to avoid the simultaneous use of a common resource
    */
    class Mutex
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        Mutex();
        //! Destructor
        ~Mutex();
        //@}

        /*!
        ** \brief Lock the mutex
        */
		void lock()
		{
            SDL_LockMutex(pMutex);
		}

        /*!
        ** \brief Release the lock
        */
		void unlock()
		{
            SDL_UnlockMutex(pMutex);
		}


    private:
        SDL_mutex *pMutex;
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
