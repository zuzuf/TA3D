#ifndef __TA3D_STD_SDL_H__
# define __TA3D_STD_SDL_H__

# include "stdafx.h"


/*
** The SDL library
*/
# if defined TA3D_PLATFORM_WINDOWS //&& defined TA3D_PLATFORM_MSVC
#	include <yuni/toolbox/system/windows.hdr.h>
#   include "tools/win32/mingw32/include/GL/glew.h"
#   include <SDL.h>
#   include <SDL_image.h>
# else
/*
** The OpenGL library
*/
#   include <GL/glew.h>
#   include <SDL.h>
#   include <SDL_image.h>
#endif



namespace TA3D
{

	void rest(uint32 msec);

} // namespace TA3D




#endif // __TA3D_STD_SDL_H__
