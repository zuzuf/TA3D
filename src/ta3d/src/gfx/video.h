#ifndef __TA3D_GFX_VIDEO_H__
#define __TA3D_GFX_VIDEO_H__

#include "gfx.h"

namespace TA3D
{
	class Video
	{
	private:
		static GLuint gltex;
		static SDL_Surface *buf;
	public:
		static void play(const QString &filename);
	private:
		static void update(SDL_Surface *img, sint32, sint32, uint32, uint32);
	};
}
#endif
