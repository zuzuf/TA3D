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
|      contient les fonctions et les variables permettant d'    |
|  utiliser des extensions d'OpenGl. utilise AllegroGl          |
\--------------------------------------------------------------*/

#ifndef __TA3D_GFX_GL_FUNCTIONS_H__
# define __TA3D_GFX_GL_FUNCTIONS_H__

#include "../stdafx.h"
#include <stdio.h>

extern bool	MultiTexturing;
extern bool g_useTextureCompression;
extern bool g_useStencilTwoSide;
extern bool g_useCopyDepthToColor;
extern bool g_useProgram;
extern bool g_useFBO;

/*!
** \brief Try to enable some specific OpenGL extensions
*/
void installOpenGLExtensions();

GLhandleARB load_fragment_shader_memory(const char *data,int filesize);
GLhandleARB load_vertex_shader_memory(const char *data,int filesize);
GLhandleARB load_fragment_shader(const char *filename);
GLhandleARB load_vertex_shader(const char *filename);

#endif // __TA3D_GFX_GL_FUNCTIONS_H__
