
#include "gettimeofday.h"
#ifdef YUNI_OS_WINDOWS
# include <stdio.h>
# include <sys/timeb.h>
# include <time.h>
#endif



namespace Yuni
{


	int gettimeofday(struct timeval *tv, struct timezone *tz)
	{
		if (NULL != tv)
		{
			struct _timeb timebuffer;
			_ftime64_s(&timebuffer);
			tv->tv_sec  = (sint64)(timebuffer.time);
			tv->tv_usec = (sint64)(timebuffer.millitm * 1000);
		}

		if (NULL != tz)
		{
			static int tzflag = 0;
			if (!tzflag)
			{
				_tzset();
				tzflag++;
			}
			long tzone(0);
			_get_timezone(&tzone);
			tz->tz_minuteswest = tzone / 60;
			int dlight(0);
			_get_daylight(&dlight);
			tz->tz_dsttime = dlight;
		}

		return 0;
	}



} // namespace Yuni
