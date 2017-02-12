#ifndef SGUI_SDL_HEADERS_H
# define SGUI_SDL_HEADERS_H

# if defined(__APPLE__) || defined(__MACH__) || defined(_WIN32)
#	include <SDL.h>
# else
#	include <SDL/SDL.h>
# endif

#ifdef _WIN32
/* M_SQRT2*/
#define _USE_MATH_DEFINES 
#include <math.h>
#define putenv _putenv
#endif

#endif // SGUI_SDL_HEADERS_H
