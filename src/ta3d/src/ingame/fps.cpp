
#include "fps.h"
#include <ta3dbase.h>
#include <gfx/gfx.h>
#include <misc/timer.h>


namespace TA3D
{


	FPSInfos::FPSInfos()
		:countSinceLastTime(0), average(0), lastTime(0)
	{}



	void FPSInfos::statisticsAddFrame()
	{
		++countSinceLastTime;
		if (msectimer() - lastTime >= 1000 /* 1s */)
		{
			average = countSinceLastTime * 1000 / (msectimer() - lastTime);
			countSinceLastTime = 0;
			lastTime = msectimer();
            toStr = QString("fps: %1").arg(average);
		}
	}


	void FPSInfos::draw() const
	{
		// Display
        gfx->glEnable(GL_BLEND);
        CHECK_GL();
        gfx->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        CHECK_GL();
        gfx->TA_font->print(1.0f, 1.0f, 0xFF000000U, toStr);
        gfx->TA_font->print(0.0f, 0.0f, 0xFFFFFFFFU, toStr);
        gfx->glDisable(GL_BLEND);
        CHECK_GL();
    }



} // namespace TA3D
