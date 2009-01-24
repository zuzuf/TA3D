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
bool    g_useGenMipMaps = false;

bool is_extension_supported(const String &name)
{
    String extensions = (char*)glGetString(GL_EXTENSIONS);
    return extensions.find(name) != std::string::npos;
}


#if (defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC) || defined TA3D_PLATFORM_LINUX
#define CHECK_OPENGL_FUNCTION( extension, function, var ) \
		if ((function) == NULL)\
		{\
            LOG_WARNING( LOG_PREFIX_OPENGL << "OpenGL reports supporting " #extension " but " #function " is lacking");\
            (var) = false;\
		}

static void installOpenGLExtensionsPointers()
{
#if not defined TA3D_PLATFORM_LINUX
    if(MultiTexturing)
    {
		glActiveTextureARB = (void (*)(GLenum)) SDL_GL_GetProcAddress("glActiveTextureARB");
		glMultiTexCoord2fARB = (void (*)(GLenum, GLfloat, GLfloat)) SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		glClientActiveTextureARB = (void (*)(GLenum)) SDL_GL_GetProcAddress("glClientActiveTextureARB");
		CHECK_OPENGL_FUNCTION( MultiTexturing, glActiveTextureARB, MultiTexturing )
		CHECK_OPENGL_FUNCTION( MultiTexturing, glMultiTexCoord2fARB, MultiTexturing )
		CHECK_OPENGL_FUNCTION( MultiTexturing, glClientActiveTextureARB, MultiTexturing )
		if (!MultiTexturing)
            LOG_WARNING( LOG_PREFIX_OPENGL << "MultiTexturing support will be disbaled");
	}
	if(g_useFBO)
    {
		glDeleteFramebuffersEXT = (void (*)(GLsizei, const GLuint*)) SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
		glDeleteRenderbuffersEXT = (void (*)(GLsizei, const GLuint*)) SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
		glBindFramebufferEXT = (void (*)(GLenum, GLuint)) SDL_GL_GetProcAddress("glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint)) SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
		glFramebufferRenderbufferEXT = (void (*)(GLenum, GLenum, GLenum, GLuint)) SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
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
		glActiveStencilFaceEXT = (void (*)(GLenum)) SDL_GL_GetProcAddress("glActiveStencilFaceEXT");
		CHECK_OPENGL_FUNCTION( StencilTwoSide, glActiveStencilFaceEXT, g_useStencilTwoSide)
		if(!g_useStencilTwoSide)
            LOG_WARNING( LOG_PREFIX_OPENGL << "StencilTwoSide support will be disbaled");
	}
	if(g_useProgram)
    {
		glCreateShaderObjectARB = (GLhandleARB (*)(GLenum)) SDL_GL_GetProcAddress("glCreateShaderObjectARB");
		glShaderSourceARB = (void (*)(GLhandleARB, GLsizei, const GLcharARB**, const GLint*)) SDL_GL_GetProcAddress("glShaderSourceARB");
		glCompileShaderARB = (void (*)(GLhandleARB)) SDL_GL_GetProcAddress("glCompileShaderARB");
		glGetObjectParameterivARB = (void (*)(GLhandleARB, GLenum, GLint*)) SDL_GL_GetProcAddress("glGetObjectParameterivARB");
		glGetInfoLogARB = (void (*)(GLhandleARB, GLsizei, GLsizei*, GLcharARB*)) SDL_GL_GetProcAddress("glGetInfoLogARB");
		glGenFramebuffersEXT = (void (*)(GLsizei, GLuint*)) SDL_GL_GetProcAddress("glGenFramebuffersEXT");
		glGenRenderbuffersEXT = (void (*)(GLsizei, GLuint*)) SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
		glBindRenderbufferEXT = (void (*)(GLenum, GLuint)) SDL_GL_GetProcAddress("glBindRenderbufferEXT");
		glRenderbufferStorageEXT = (void (*)(GLenum, GLenum, GLsizei, GLsizei)) SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
		glCreateProgramObjectARB = (GLhandleARB (*)()) SDL_GL_GetProcAddress("glCreateProgramObjectARB");
		glAttachObjectARB = (void (*)(GLhandleARB, GLhandleARB)) SDL_GL_GetProcAddress("glAttachObjectARB");
		glLinkProgramARB = (void (*)(GLhandleARB)) SDL_GL_GetProcAddress("glLinkProgramARB");
		glUseProgramObjectARB = (void (*)(GLhandleARB)) SDL_GL_GetProcAddress("glUseProgramObjectARB");
		glDetachObjectARB = (void (*)(GLhandleARB, GLhandleARB)) SDL_GL_GetProcAddress("glDetachObjectARB");
		glDeleteObjectARB = (void (*)(GLhandleARB)) SDL_GL_GetProcAddress("glDeleteObjectARB");
		glUniform1fARB = (void (*)(GLint, GLfloat)) SDL_GL_GetProcAddress("glUniform1fARB");
		glUniform2fARB = (void (*)(GLint, GLfloat, GLfloat)) SDL_GL_GetProcAddress("glUniform2fARB");
		glUniform3fARB = (void (*)(GLint, GLfloat, GLfloat, GLfloat)) SDL_GL_GetProcAddress("glUniform3fARB");
		glUniform4fARB = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat)) SDL_GL_GetProcAddress("glUniform4fARB");
		glUniform1iARB = (void (*)(GLint, GLint)) SDL_GL_GetProcAddress("glUniform1iARB");
		glUniform2iARB = (void (*)(GLint, GLint, GLint)) SDL_GL_GetProcAddress("glUniform2iARB");
		glUniform3iARB = (void (*)(GLint, GLint, GLint, GLint)) SDL_GL_GetProcAddress("glUniform3iARB");
		glUniform4iARB = (void (*)(GLint, GLint, GLint, GLint, GLint)) SDL_GL_GetProcAddress("glUniform4iARB");
		glGetUniformLocationARB = (GLint (*)(GLhandleARB, const GLcharARB*)) SDL_GL_GetProcAddress("glGetUniformLocationARB");

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
#endif
}
#endif

void installOpenGLExtensions()
{
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
    #if (defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC) || defined TA3D_PLATFORM_LINUX
    installOpenGLExtensionsPointers();
    #endif
    // Extension: multitexturing
	if (glActiveTextureARB != NULL && glMultiTexCoord2fARB != NULL && glClientActiveTextureARB != NULL)
        MultiTexturing = true;
}








