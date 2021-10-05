/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

/*--------------------------------------------------------------\
|                          glfunc.h                             |
|      contains functions and variables to set up an OpenGL     |
|  environnement using some OpenGL extensions.                  |
\--------------------------------------------------------------*/

#ifndef __TA3D_GFX_GL_FUNCTIONS_H__
# define __TA3D_GFX_GL_FUNCTIONS_H__

#include <stdafx.h>
#include "gl.extensions.h"
#include <cstdio>

namespace TA3D
{

	extern bool	MultiTexturing;
	extern bool g_useTextureCompression;
	extern bool g_useFBO;
	extern bool g_useGenMipMaps;
	extern bool g_useNonPowerOfTwoTextures;

	/*!
	** \brief Try to enable some specific OpenGL extensions
	*/
	void installOpenGLExtensions();


} // namespace TA3D

#endif // __TA3D_GFX_GL_FUNCTIONS_H__
