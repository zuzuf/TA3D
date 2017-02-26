#include "timer.h"
#include <QElapsedTimer>

namespace TA3D
{
    namespace
    {
        QElapsedTimer reference_timer;

        struct __on_init
        {
            __on_init()
            {
                reference_timer.start();
            }
        } __instance;
    }

    uint64 msectimer()
    {
        return reference_timer.elapsed();
    }

	uint64 usectimer()
	{
        return reference_timer.nsecsElapsed() / 1000UL;
	}
}
