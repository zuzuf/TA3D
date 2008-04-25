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

#include <stdio.h>

extern bool	MultiTexturing;
extern bool g_useTextureCompression;
extern bool g_useStencilTwoSide;
extern bool g_useCopyDepthToColor;
extern bool g_useProgram;
extern bool g_useFBO;

void install_ext();
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

	inline SHADER()
	{
		succes=false;
	}

	void load(const char *fragment_file,const char *vertex_file);

	void load_memory(const char *fragment_data,int frag_len,const char *vertex_data,int vert_len);

	inline void destroy()
	{
		if(succes) {
			glDetachObjectARB(program,fragment);
			glDetachObjectARB(program,vertex);
			glDeleteObjectARB(program);
			glDeleteObjectARB(fragment);
			glDeleteObjectARB(vertex);
			}
		succes=false;
	}

	inline void on()
	{
		if(succes)
			glUseProgramObjectARB(program);
	}

	inline void off()
	{
		if(succes)
			glUseProgramObjectARB(0);
	}

	inline void setvar1f(const char *var_name,float v0)
	{
		if(succes)
			glUniform1fARB(glGetUniformLocationARB(program, var_name), v0);
	}

	inline void setvar2f(const char *var_name,float v0,float v1)
	{
		if(succes)
			glUniform2fARB(glGetUniformLocationARB(program, var_name), v0, v1);
	}

	inline void setvar3f(const char *var_name,float v0,float v1,float v2)
	{
		if(succes)
			glUniform3fARB(glGetUniformLocationARB(program, var_name), v0, v1, v2);
	}

	inline void setvar4f(const char *var_name,float v0,float v1,float v2,float v3)
	{
		if(succes)
			glUniform4fARB(glGetUniformLocationARB(program, var_name), v0, v1, v2, v3);
	}

	inline void setvar1i(const char *var_name,int v0)
	{
		if(succes)
			glUniform1iARB(glGetUniformLocationARB(program, var_name), v0);
	}

	inline void setvar2i(const char *var_name,int v0,int v1)
	{
		if(succes)
			glUniform2iARB(glGetUniformLocationARB(program, var_name), v0, v1);
	}

	inline void setvar3i(const char *var_name,int v0,int v1,int v2)
	{
		if(succes)
			glUniform3iARB(glGetUniformLocationARB(program, var_name), v0, v1, v2);
	}

	inline void setvar4i(const char *var_name,int v0,int v1,int v2,int v3)
	{
		if(succes)
			glUniform4iARB(glGetUniformLocationARB(program, var_name), v0, v1, v2, v3);
	}
};
