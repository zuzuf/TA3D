
#include "fps.h"
#include <ta3dbase.h>
#include <gfx/gfx.h>


namespace TA3D
{


	FPSInfos::FPSInfos()
		:countSinceLastTime(0), average(0), lastTime(0)
	{}



	void FPSInfos::statisticsAddFrame()
	{
		++countSinceLastTime;
		if (msec_timer - lastTime >= 1000 /* 1s */)
		{
			average = countSinceLastTime * 1000 / (msec_timer - lastTime);
			countSinceLastTime = 0;
			lastTime = msec_timer;
            toStr += QString("fps: %1").arg(average);
		}
	}


	void FPSInfos::draw() const
	{
		// Display
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		gfx->print(gfx->TA_font, 1.0f, 1.0f, 0.0f, 0xFF000000U, toStr);
		gfx->print(gfx->TA_font, 0.0f, 0.0f, 0.0f, 0xFFFFFFFFU, toStr);
		glDisable(GL_BLEND);
	}



} // namespace TA3D
