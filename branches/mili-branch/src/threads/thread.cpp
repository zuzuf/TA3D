
#include "thread.h"


namespace TA3D
{

    BaseThread::BaseThread()
        :pDead(1)
    {}

    BaseThread::~BaseThread()
    {
        pDead = 1;
    }


#ifdef TA3D_PLATFORM_WINDOWS

	void
    Thread::spawn(void* param)
    {
	    pDead = 0;
		secondary.thisthread = this;
		secondary.more = param;
		thread = ::CreateThread(NULL, 0, run, &secondary, 0, &threadid);
	}

    void
    Thread::join()
    {
		pDead = 1;
		::WaitForSingleObject(thread, 2000);
	}

#else // ifdef TA3D_PLATFORM_WINDOWS

	void
    Thread::spawn(void* param)
    {
        pDead = 0;
		secondary.thisthread = this;
		secondary.more = param;
		pthread_create(&thread, NULL, run, &secondary);
	}

    void
    Thread::join()
    {
        if(!pDead)
        {
			pDead = 1;
			pthread_join(thread, NULL);
        }
	}


#endif // ifdef TA3D_PLATFORM_WINDOWS


} // namespace TA3D
