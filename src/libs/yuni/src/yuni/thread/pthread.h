#ifndef __YUNI_THREADS_PTHREAD_H__
# define __YUNI_THREADS_PTHREAD_H__

# ifndef YUNI_NO_THREAD_SAFE
#	ifndef _MULTI_THREADED
# 		define _MULTI_THREADED
#	endif
#	include <pthread.h>
# endif


namespace Yuni
{
namespace Private
{
namespace Thread
{

	extern "C"
	{
		/*!
		** \brief This procedure will be run in a separate thread and will
		**   run IThreadModel::baseExecute()
		*/
		void* threadMethodForPThread(void* arg);
	}



} // namespace Thread
} // namespace Private
} // namespace Yuni

#endif // __YUNI_THREADS_PTHREAD_H__
