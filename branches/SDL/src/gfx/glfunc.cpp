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
bool    g_useGenMipMaps = false;

bool is_extension_supported(const String &name)
{
    String extensions = (char*)glGetString(GL_EXTENSIONS);
    return extensions.find(name) != std::string::npos;
}


#define CHECK_OPENGL_FUNCTION( extension, function, var ) \
		if ((function) == NULL)\
		{\
            LOG_WARNING( LOG_PREFIX_OPENGL << "OpenGL reports supporting " #extension " but " #function " is lacking");\
            (var) = false;\
		}

static void checkOpenGLExtensionsPointers()
{
    if(MultiTexturing)
    {
		CHECK_OPENGL_FUNCTION( MultiTexturing, glActiveTextureARB, MultiTexturing )
		CHECK_OPENGL_FUNCTION( MultiTexturing, glMultiTexCoord2fARB, MultiTexturing )
		CHECK_OPENGL_FUNCTION( MultiTexturing, glClientActiveTextureARB, MultiTexturing )
		if (!MultiTexturing)
            LOG_WARNING( LOG_PREFIX_OPENGL << "MultiTexturing support will be disbaled");
	}
	if(g_useFBO)
    {
		CHECK_OPENGL_FUNCTION( FBO, glDeleteFramebuffersEXT, g_useFBO)
		CHECK_OPENGL_FUNCTION( FBO, glDeleteRenderbuffersEXT, g_useFBO)
		CHECK_OPENGL_FUNCTION( FBO, glBindFramebufferEXT, g_useFBO)
		CHECK_OPENGL_FUNCTION( FBO, glFramebufferTexture2DEXT, g_useFBO)
		CHECK_OPENGL_FUNCTION( FBO, glFramebufferRenderbufferEXT, g_useFBO)
		if (!g_useFBO)
            LOG_WARNING( LOG_PREFIX_OPENGL << "FBO support will be disbaled");
	}
	if(g_useStencilTwoSide)
    {
		CHECK_OPENGL_FUNCTION( StencilTwoSide, glActiveStencilFaceEXT, g_useStencilTwoSide)
		if(!g_useStencilTwoSide)
            LOG_WARNING( LOG_PREFIX_OPENGL << "StencilTwoSide support will be disbaled");
	}
	if(g_useProgram)
    {
        CHECK_OPENGL_FUNCTION( GLSL, glCreateShaderObjectARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glShaderSourceARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glCompileShaderARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glGetObjectParameterivARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glGetInfoLogARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glGenFramebuffersEXT, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glGenRenderbuffersEXT, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glBindRenderbufferEXT, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glRenderbufferStorageEXT, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glCreateProgramObjectARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glAttachObjectARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glLinkProgramARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUseProgramObjectARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glDetachObjectARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glDeleteObjectARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform1fARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform2fARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform3fARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform4fARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform1iARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform2iARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform3iARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glUniform4iARB, g_useProgram )
		CHECK_OPENGL_FUNCTION( GLSL, glGetUniformLocationARB, g_useProgram )
		if (!g_useProgram)
            LOG_WARNING( LOG_PREFIX_OPENGL << "GLSL support will be disbaled");
	}
}

void installOpenGLExtensions()
{
    #if defined TA3D_PLATFORM_WINDOWS
    glewInit();
    #endif

	MultiTexturing = is_extension_supported("GL_ARB_multitexture");

    # ifdef TA3D_PLATFORM_DARWIN
    g_useTextureCompression = false;
    # else
	g_useTextureCompression = is_extension_supported("GL_ARB_texture_compression");
    # endif
	g_useStencilTwoSide = is_extension_supported("GL_EXT_stencil_two_side");
	g_useCopyDepthToColor = is_extension_supported("GL_NV_copy_depth_to_color");
	g_useProgram = is_extension_supported("GL_ARB_shader_objects") && is_extension_supported("GL_ARB_shading_language_100") && is_extension_supported("GL_ARB_vertex_shader") && is_extension_supported("GL_ARB_fragment_shader");
	g_useFBO = is_extension_supported("GL_EXT_framebuffer_object");
	g_useGenMipMaps = is_extension_supported("GL_SGIS_generate_mipmap");

    checkOpenGLExtensionsPointers();

    // Extension: multitexturing
	if (glActiveTextureARB != NULL && glMultiTexCoord2fARB != NULL && glClientActiveTextureARB != NULL)
        MultiTexturing = true;
}








