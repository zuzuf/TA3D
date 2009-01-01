#include "mutex.h"

namespace TA3D
{


    Mutex::Mutex()
    {
        #ifdef TA3D_PLATFORM_WINDOWS
		::InitializeCriticalSection(&pCritSection);
        #else
		pthread_mutexattr_t mutexattr;
		pthread_mutexattr_init(&mutexattr);
        # ifdef TA3D_PLATFORM_DARWIN
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
        # else
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
        # endif
		pthread_mutex_init(&pPthreadLock, &mutexattr);
		pthread_mutexattr_destroy(&mutexattr);
        #endif
    }


	Mutex::~Mutex()
	{
        #ifdef TA3D_PLATFORM_WINDOWS
		::DeleteCriticalSection(&pCritSection);
        #else
		pthread_mutex_destroy(&pPthreadLock);
        #endif
	}

} // namespace TA3D
