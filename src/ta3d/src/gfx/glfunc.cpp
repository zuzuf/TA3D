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

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "glfunc.h"
#include <logs/logs.h>



# define CHECK_OPENGL_FUNCTION( extension, function, var ) \
		if ((function) == NULL)\
		{\
			LOG_WARNING( LOG_PREFIX_OPENGL << "OpenGL reports supporting " #extension " but " #function " is lacking");\
			(var) = false;\
		}



namespace TA3D
{

	bool	MultiTexturing;
	bool	g_useTextureCompression = false;
	bool	g_useStencilTwoSide = false;
	bool	g_useProgram = false;
	bool	g_useFBO = false;
	bool    g_useGenMipMaps = false;
	bool    g_useNonPowerOfTwoTextures = false;


	static void checkOpenGLExtensionsPointers()
	{
		if (MultiTexturing)
		{
			CHECK_OPENGL_FUNCTION( MultiTexturing, glActiveTextureARB, MultiTexturing )
			CHECK_OPENGL_FUNCTION( MultiTexturing, glMultiTexCoord2fARB, MultiTexturing )
			CHECK_OPENGL_FUNCTION( MultiTexturing, glClientActiveTextureARB, MultiTexturing )
			if (!MultiTexturing)
				LOG_WARNING( LOG_PREFIX_OPENGL << "MultiTexturing support will be disbaled");
		}
		if (g_useFBO)
		{
			CHECK_OPENGL_FUNCTION( FBO, glDeleteFramebuffersEXT, g_useFBO)
			CHECK_OPENGL_FUNCTION( FBO, glDeleteRenderbuffersEXT, g_useFBO)
			CHECK_OPENGL_FUNCTION( FBO, glBindFramebufferEXT, g_useFBO)
			CHECK_OPENGL_FUNCTION( FBO, glFramebufferTexture2DEXT, g_useFBO)
			CHECK_OPENGL_FUNCTION( FBO, glFramebufferRenderbufferEXT, g_useFBO)
			if (!g_useFBO)
				LOG_WARNING( LOG_PREFIX_OPENGL << "FBO support will be disbaled");
		}
		if (g_useStencilTwoSide)
		{
			CHECK_OPENGL_FUNCTION( StencilTwoSide, glActiveStencilFaceEXT, g_useStencilTwoSide)
			if (!g_useStencilTwoSide)
				LOG_WARNING( LOG_PREFIX_OPENGL << "StencilTwoSide support will be disbaled");
		}
		if (g_useProgram)
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
			CHECK_OPENGL_FUNCTION( GLSL, glUniformMatrix4fv, g_useProgram )
			CHECK_OPENGL_FUNCTION( GLSL, glGetUniformLocationARB, g_useProgram )
			if (!g_useProgram)
				LOG_WARNING( LOG_PREFIX_OPENGL << "GLSL support will be disbaled");
		}
	}

	void installOpenGLExtensions()
	{
		GLenum err = glewInit();
		if (err == GLEW_OK)
			LOG_DEBUG(LOG_PREFIX_OPENGL << "GLEW initialization successful");
		else
		{
			LOG_WARNING(LOG_PREFIX_OPENGL << "GLEW initialization failed!");
			LOG_WARNING(LOG_PREFIX_OPENGL << "GLEW error: " << (const char*) glewGetErrorString(err));
		}
		LOG_DEBUG(LOG_PREFIX_OPENGL << "Using GLEW " << (const char*) glewGetString(GLEW_VERSION));

		MultiTexturing = GLEW_ARB_multitexture;

		g_useTextureCompression = GLEW_ARB_texture_compression;
		g_useStencilTwoSide = GLEW_EXT_stencil_two_side;
		g_useProgram = GLEW_ARB_shader_objects && GLEW_ARB_shading_language_100 && GLEW_ARB_vertex_program && GLEW_ARB_fragment_program;
		g_useFBO = GLEW_EXT_framebuffer_object;
		g_useGenMipMaps = GLEW_SGIS_generate_mipmap;
		g_useNonPowerOfTwoTextures = GLEW_ARB_texture_non_power_of_two;

		checkOpenGLExtensionsPointers();

		// Extension: multitexturing
		if (glActiveTextureARB != NULL && glMultiTexCoord2fARB != NULL && glClientActiveTextureARB != NULL)
			MultiTexturing = true;
	}


} // namespace TA3D

