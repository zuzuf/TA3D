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

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "glfunc.h"
#include "../logs/logs.h"



bool	MultiTexturing;
bool	g_useTextureCompression = false;
bool	g_useStencilTwoSide = false;
bool	g_useCopyDepthToColor = false;
bool	g_useProgram = false;
bool	g_useFBO = false;




#if (defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC) || defined TA3D_PLATFORM_LINUX
static void installOpenGLExtensionsPointers()
{
    if(MultiTexturing)
    {
		glActiveTextureARB = (void (*)(GLenum)) allegro_gl_get_proc_address("glActiveTextureARB");
		glMultiTexCoord2fARB = (void (*)(GLenum, GLfloat, GLfloat)) allegro_gl_get_proc_address("glMultiTexCoord2fARB");
		glClientActiveTextureARB = (void (*)(GLenum)) allegro_gl_get_proc_address("glClientActiveTextureARB");
	}
	if(g_useFBO)
    {
		glDeleteFramebuffersEXT = (void (*)(GLsizei, const GLuint*)) allegro_gl_get_proc_address("glDeleteFramebuffersEXT");
		glDeleteRenderbuffersEXT = (void (*)(GLsizei, const GLuint*)) allegro_gl_get_proc_address("glDeleteRenderbuffersEXT");
		glBindFramebufferEXT = (void (*)(GLenum, GLuint)) allegro_gl_get_proc_address("glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint)) allegro_gl_get_proc_address("glFramebufferTexture2DEXT");
		glFramebufferRenderbufferEXT = (void (*)(GLenum, GLenum, GLenum, GLuint)) allegro_gl_get_proc_address("glFramebufferRenderbufferEXT");
	}
	if(g_useStencilTwoSide)
    {
		glActiveStencilFaceEXT = (void (*)(GLenum)) allegro_gl_get_proc_address("glActiveStencilFaceEXT");
	}
	if(g_useProgram)
    {
		glCreateShaderObjectARB = (GLhandleARB (*)(GLenum)) allegro_gl_get_proc_address("glCreateShaderObjectARB");
		glShaderSourceARB = (void (*)(GLhandleARB, GLsizei, const GLcharARB**, const GLint*)) allegro_gl_get_proc_address("glShaderSourceARB");
		glCompileShaderARB = (void (*)(GLhandleARB)) allegro_gl_get_proc_address("glCompileShaderARB");
		glGetObjectParameterivARB = (void (*)(GLhandleARB, GLenum, GLint*)) allegro_gl_get_proc_address("glGetObjectParameterivARB");
		glGetInfoLogARB = (void (*)(GLhandleARB, GLsizei, GLsizei*, GLcharARB*)) allegro_gl_get_proc_address("glGetInfoLogARB");
		glGenFramebuffersEXT = (void (*)(GLsizei, GLuint*)) allegro_gl_get_proc_address("glGenFramebuffersEXT");
		glGenRenderbuffersEXT = (void (*)(GLsizei, GLuint*)) allegro_gl_get_proc_address("glGenRenderbuffersEXT");
		glBindRenderbufferEXT = (void (*)(GLenum, GLuint)) allegro_gl_get_proc_address("glBindRenderbufferEXT");
		glRenderbufferStorageEXT = (void (*)(GLenum, GLenum, GLsizei, GLsizei)) allegro_gl_get_proc_address("glRenderbufferStorageEXT");
		glCreateProgramObjectARB = (GLhandleARB (*)()) allegro_gl_get_proc_address("glCreateProgramObjectARB");
		glAttachObjectARB = (void (*)(GLhandleARB, GLhandleARB)) allegro_gl_get_proc_address("glAttachObjectARB");
		glLinkProgramARB = (void (*)(GLhandleARB)) allegro_gl_get_proc_address("glLinkProgramARB");
		glUseProgramObjectARB = (void (*)(GLhandleARB)) allegro_gl_get_proc_address("glUseProgramObjectARB");
		glDetachObjectARB = (void (*)(GLhandleARB, GLhandleARB)) allegro_gl_get_proc_address("glDetachObjectARB");
		glDeleteObjectARB = (void (*)(GLhandleARB)) allegro_gl_get_proc_address("glDeleteObjectARB");
		glUniform1fARB = (void (*)(GLint, GLfloat)) allegro_gl_get_proc_address("glUniform1fARB");
		glUniform2fARB = (void (*)(GLint, GLfloat, GLfloat)) allegro_gl_get_proc_address("glUniform2fARB");
		glUniform3fARB = (void (*)(GLint, GLfloat, GLfloat, GLfloat)) allegro_gl_get_proc_address("glUniform3fARB");
		glUniform4fARB = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat)) allegro_gl_get_proc_address("glUniform4fARB");
		glUniform1iARB = (void (*)(GLint, GLint)) allegro_gl_get_proc_address("glUniform1iARB");
		glUniform2iARB = (void (*)(GLint, GLint, GLint)) allegro_gl_get_proc_address("glUniform2iARB");
		glUniform3iARB = (void (*)(GLint, GLint, GLint, GLint)) allegro_gl_get_proc_address("glUniform3iARB");
		glUniform4iARB = (void (*)(GLint, GLint, GLint, GLint, GLint)) allegro_gl_get_proc_address("glUniform4iARB");
		glGetUniformLocationARB = (GLint (*)(GLhandleARB, const GLcharARB*)) allegro_gl_get_proc_address("glGetUniformLocationARB");
	}
}
#endif

void installOpenGLExtensions()
{
	MultiTexturing = allegro_gl_is_extension_supported("GL_ARB_multitexture");

    # ifdef TA3D_PLATFORM_DARWIN
    g_useTextureCompression = false;
    # else
	g_useTextureCompression = allegro_gl_is_extension_supported("GL_ARB_texture_compression");
    # endif
	g_useStencilTwoSide = allegro_gl_is_extension_supported("GL_EXT_stencil_two_side");
	g_useCopyDepthToColor = allegro_gl_is_extension_supported("GL_NV_copy_depth_to_color");
	g_useProgram = allegro_gl_is_extension_supported("GL_ARB_shader_objects") && allegro_gl_is_extension_supported("GL_ARB_shading_language_100") && allegro_gl_is_extension_supported("GL_ARB_vertex_shader") && allegro_gl_is_extension_supported("GL_ARB_fragment_shader");
	g_useFBO = allegro_gl_is_extension_supported("GL_EXT_framebuffer_object");
    #if (defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC) || defined TA3D_PLATFORM_LINUX
    installOpenGLExtensionsPointers();
    #endif
    // Extension: multitexturing
	if (glActiveTextureARB != NULL && glMultiTexCoord2fARB != NULL && glClientActiveTextureARB != NULL)
        MultiTexturing = true;
}








