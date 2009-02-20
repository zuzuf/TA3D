
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


	void
    Thread::spawn(void* param)
    {
	    pDead = 0;
		secondary.thisthread = this;
		secondary.more = param;
		thread = SDL_CreateThread(run, &secondary);
	}

    void
    Thread::join()
    {
        if (pDead == 0)
        {
            signalExitThread();
            pDead = 1;
            SDL_WaitThread(thread, NULL);
        }
	}

} // namespace TA3D
