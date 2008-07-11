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

#ifdef TA3D_PLATFORM_WINDOWS
#  include <alleggl.h>
#endif
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

class SHADER
{
public:
	GLhandleARB		program;
	GLhandleARB		fragment;
	GLhandleARB		vertex;
	bool			succes;

	SHADER() :succes(false) {}

	void load(const char *fragment_file,const char *vertex_file);

	void load_memory(const char *fragment_data,int frag_len,const char *vertex_data,int vert_len);

	void destroy();

	void on();

	void off();

	void setvar1f(const char *var_name,float v0);

	void setvar2f(const char *var_name,float v0,float v1);

	void setvar3f(const char *var_name,float v0,float v1,float v2);

	void setvar4f(const char *var_name,float v0,float v1,float v2,float v3);

	void setvar1i(const char *var_name,int v0);

	void setvar2i(const char *var_name,int v0,int v1);

	void setvar3i(const char *var_name,int v0,int v1,int v2);

	void setvar4i(const char *var_name,int v0,int v1,int v2,int v3);

}; // class SHADER

#endif // __TA3D_GFX_GL_FUNCTIONS_H__
