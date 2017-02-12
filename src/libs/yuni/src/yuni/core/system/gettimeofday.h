#ifndef __YUNI_SYSTEM_WINDOWS_GETTIMEOFDAY_H__
# define __YUNI_SYSTEM_WINDOWS_GETTIMEOFDAY_H__

# include "../../yuni.h"
# include "windows.hdr.h"


# define YUNI_HAS_GETTIMEOFDAY

# ifndef YUNI_OS_MSVC
#  	include <sys/time.h>
# else // YUNI_OS_MSVC

#  	ifdef YUNI_OS_WINDOWS
#    	include <time.h>
#    	if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#		    define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#    	else
#		    define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#    	endif
#  	endif

#	ifdef YUNI_HAS_GETTIMEOFDAY
#		undef YUNI_HAS_GETTIMEOFDAY
#	endif

# endif // YUNI_OS_MSVC



namespace Yuni
{

# ifndef YUNI_HAS_GETTIMEOFDAY

	struct timezone
	{
		int  tz_minuteswest; // minutes W of Greenwich
		int  tz_dsttime;     // type of dst correction
	};

	struct timeval
	{
		sint64 tv_sec;
		sint64 tv_usec;
	};

	int gettimeofday(struct timeval *tv, struct timezone *tz);

#	define YUNI_SYSTEM_GETTIMEOFDAY  ::Yuni::gettimeofday

# else

	typedef struct timezone timezone;
	typedef struct timeval timeval;
#	ifndef YUNI_SYSTEM_GETTIMEOFDAY
#		define YUNI_SYSTEM_GETTIMEOFDAY  ::gettimeofday
#	endif

# endif

} // namespace Yuni



#endif // __YUNI_SYSTEM_WINDOWS_GETTIMEOFDAY_H__
