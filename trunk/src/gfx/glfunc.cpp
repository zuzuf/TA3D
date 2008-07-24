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




#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
static void installOpenGLExtensionsForWindows()
{
    if(MultiTexturing)
    {
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) allegro_gl_get_proc_address("glActiveTextureARB");
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) allegro_gl_get_proc_address("glMultiTexCoord2fARB");
		glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) allegro_gl_get_proc_address("glClientActiveTextureARB");
	}
	if(g_useFBO)
    {
		glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) allegro_gl_get_proc_address("glDeleteFramebuffersEXT");
		glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) allegro_gl_get_proc_address("glDeleteRenderbuffersEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) allegro_gl_get_proc_address("glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) allegro_gl_get_proc_address("glFramebufferTexture2DEXT");
		glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) allegro_gl_get_proc_address("glFramebufferRenderbufferEXT");
	}
	if(g_useStencilTwoSide)
    {
		glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC) allegro_gl_get_proc_address("glActiveStencilFaceEXT");
	}
	if(g_useProgram)
    {
		glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC) allegro_gl_get_proc_address("glCreateShaderObjectARB");
		glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) allegro_gl_get_proc_address("glShaderSourceARB");
		glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC) allegro_gl_get_proc_address("glCompileShaderARB"); 
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) allegro_gl_get_proc_address("glGetObjectParameterivARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC) allegro_gl_get_proc_address("glGetInfoLogARB");
		glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) allegro_gl_get_proc_address("glGenFramebuffersEXT");
		glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) allegro_gl_get_proc_address("glGenRenderbuffersEXT");
		glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) allegro_gl_get_proc_address("glBindRenderbufferEXT");
		glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) allegro_gl_get_proc_address("glRenderbufferStorageEXT");
		glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC) allegro_gl_get_proc_address("glCreateProgramObjectARB");
		glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC) allegro_gl_get_proc_address("glAttachObjectARB");
		glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) allegro_gl_get_proc_address("glLinkProgramARB");
		glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC) allegro_gl_get_proc_address("glUseProgramObjectARB");
		glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC) allegro_gl_get_proc_address("glDetachObjectARB");
		glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC) allegro_gl_get_proc_address("glDeleteObjectARB");
		glUniform1fARB = (PFNGLUNIFORM1FARBPROC) allegro_gl_get_proc_address("glUniform1fARB");
		glUniform2fARB = (PFNGLUNIFORM2FARBPROC) allegro_gl_get_proc_address("glUniform2fARB");
		glUniform3fARB = (PFNGLUNIFORM3FARBPROC) allegro_gl_get_proc_address("glUniform3fARB");
		glUniform4fARB = (PFNGLUNIFORM4FARBPROC) allegro_gl_get_proc_address("glUniform4fARB");
		glUniform1iARB = (PFNGLUNIFORM1IARBPROC) allegro_gl_get_proc_address("glUniform1iARB");
		glUniform2iARB = (PFNGLUNIFORM2IARBPROC) allegro_gl_get_proc_address("glUniform2iARB");
		glUniform3iARB = (PFNGLUNIFORM3IARBPROC) allegro_gl_get_proc_address("glUniform3iARB");
		glUniform4iARB = (PFNGLUNIFORM4IARBPROC) allegro_gl_get_proc_address("glUniform4iARB");
		glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC) allegro_gl_get_proc_address("glGetUniformLocationARB");
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
    #if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
    installOpenGLExtensionsForWindows();
    #endif
    // Extension: multitexturing
	if (glActiveTextureARB != NULL && glMultiTexCoord2fARB != NULL && glClientActiveTextureARB != NULL)
        MultiTexturing = true;
}








