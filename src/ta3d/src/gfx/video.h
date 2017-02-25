#ifndef __TA3D_GFX_VIDEO_H__
#define __TA3D_GFX_VIDEO_H__

#include "gfx.h"

namespace TA3D
{
	class Video
	{
	private:
		static GLuint gltex;
		static QImage buf;
	public:
		static void play(const QString &filename);
	private:
		static void update(QImage img, sint32, sint32, uint32, uint32);
	};
}
#endif
