
#include "suspend.h"
#ifdef YUNI_OS_WINDOWS
# include "windows.hdr.h"
#else
# include <unistd.h>
#endif



namespace Yuni
{

	void Suspend(unsigned int seconds)
	{
		# if defined(YUNI_OS_WINDOWS)
		Sleep(1000 * seconds);
		# else
		::sleep(seconds);
		# endif
	}


	void SuspendMilliSeconds(unsigned int milliseconds)
	{
		# if defined(YUNI_OS_WINDOWS)
		Sleep(milliseconds);
		# else
		::usleep(1000 * milliseconds);
		# endif
	}



} // namespace Yuni

