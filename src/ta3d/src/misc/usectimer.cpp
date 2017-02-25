#include "usectimer.h"
#include <sys/time.h>

namespace TA3D
{
	uint64 usectimer()
	{
		struct timeval tv;
        gettimeofday(&tv, nullptr);
        return uint64(tv.tv_sec) * 1000000UL + tv.tv_usec;
	}
}
