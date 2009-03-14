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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "glfunc.h"

bool	MultiTexturing;
bool	g_useTextureCompression = false;
bool	g_useStencilTwoSide = false;
bool	g_useCopyDepthToColor = false;
bool	g_useProgram = false;
bool	g_useFBO = false;

void install_ext()
{
	MultiTexturing = allegro_gl_is_extension_supported("GL_ARB_multitexture");

	g_useTextureCompression = allegro_gl_is_extension_supported("GL_ARB_texture_compression");
	g_useStencilTwoSide = allegro_gl_is_extension_supported("GL_EXT_stencil_two_side");
	g_useCopyDepthToColor = allegro_gl_is_extension_supported("GL_NV_copy_depth_to_color");
	g_useProgram = allegro_gl_is_extension_supported("GL_ARB_shader_objects") && allegro_gl_is_extension_supported("GL_ARB_shading_language_100") && allegro_gl_is_extension_supported("GL_ARB_vertex_shader") && allegro_gl_is_extension_supported("GL_ARB_fragment_shader");
	g_useFBO = allegro_gl_is_extension_supported("GL_EXT_framebuffer_object");

#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
	if(MultiTexturing) {
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) allegro_gl_get_proc_address("glActiveTextureARB");
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) allegro_gl_get_proc_address("glMultiTexCoord2fARB");
		glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) allegro_gl_get_proc_address("glClientActiveTextureARB");
		}
	if(g_useFBO) {
		glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) allegro_gl_get_proc_address("glDeleteFramebuffersEXT");
		glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) allegro_gl_get_proc_address("glDeleteRenderbuffersEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) allegro_gl_get_proc_address("glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) allegro_gl_get_proc_address("glFramebufferTexture2DEXT");
		glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) allegro_gl_get_proc_address("glFramebufferRenderbufferEXT");
		}
	if(g_useStencilTwoSide) {
		glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC) allegro_gl_get_proc_address("glActiveStencilFaceEXT");
		}
	if(g_useProgram) {
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
#endif
			// Extension pour le multitexturing
	if(glActiveTextureARB!=NULL && glMultiTexCoord2fARB!=NULL && glClientActiveTextureARB!=NULL) MultiTexturing=true;
}

GLhandleARB load_fragment_shader_memory(const char *data,int filesize)
{
	GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	int compiled = 0;

	glShaderSourceARB(shader, 1, (const GLcharARB **)&data, &filesize);
	glCompileShaderARB(shader); 
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	if (compiled) 
	{ // compilation successful!
		Console->AddEntry("pixel shader successfully loaded");
	}
	else 
	{ // compilation error! Check compiler log! 
		Console->AddEntry("pixel shader : compilation failed");
		char log[10000];
		GLsizei len=0;
		glGetInfoLogARB(shader, 10000, &len, log);
		Console->AddEntry("%s",log);
	}

	return shader;
}

GLhandleARB load_vertex_shader_memory(const char *data,int filesize)
{
	GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	int compiled = 0;

	glShaderSourceARB(shader, 1, (const GLcharARB **)&data, &filesize);
	glCompileShaderARB(shader);
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	if (compiled) 
	{ // compilation successful!
		Console->AddEntry("vertex shader successfully loaded");
	}
	else 
	{ // compilation error! Check compiler log! 
		Console->AddEntry("vertex shader : compilation failed");
		char log[10000];
		GLsizei len=0;
		glGetInfoLogARB(shader, 10000, &len, log);
		Console->AddEntry("%s",log);
	}

	return shader;
}

GLhandleARB load_fragment_shader(const char *filename)
{
	FILE *file=TA3D_OpenFile(filename,"rt");
	GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	if(file==NULL)	{
		Console->AddEntry("error: file %s doesn't exist!", filename);
		return shader;
		}
	int filesize=FILE_SIZE(filename);

	char *buf=(char*) malloc(filesize+1);
	fread(buf,filesize,1,file);
	buf[filesize]=0;
	fclose(file);

	int compiled = 0;

	glShaderSourceARB(shader, 1, (const GLcharARB **)&buf, &filesize);
	glCompileShaderARB(shader); 
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	if (compiled) 
	{ // compilation successful!
		Console->AddEntry("%s successfully loaded",filename);
	}
	else 
	{ // compilation error! Check compiler log! 
		Console->AddEntry("%s : compilation failed",filename);
		char log[10000];
		GLsizei len=0;
		glGetInfoLogARB(shader, 10000, &len, log);
		Console->AddEntry("%s",log);
	}

	free(buf);
	return shader;
}

GLhandleARB load_vertex_shader(const char *filename)
{
	FILE *file=TA3D_OpenFile(filename,"rt");
	GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	if(file==NULL)	{
		Console->AddEntry("error: file %s doesn't exist!", filename);
		return shader;
		}
	int filesize=FILE_SIZE(filename);

	char *buf=(char*) malloc(filesize+1);
	fread(buf,filesize,1,file);
	buf[filesize]=0;
	fclose(file);

	int compiled = 0;

	glShaderSourceARB(shader, 1, (const GLcharARB **)&buf, &filesize);
	glCompileShaderARB(shader);
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	if (compiled) 
	{ // compilation successful!
		Console->AddEntry("%s successfully loaded",filename);
	}
	else 
	{ // compilation error! Check compiler log! 
		Console->AddEntry("%s : compilation failed",filename);
		char log[10000];
		GLsizei len=0;
		glGetInfoLogARB(shader, 10000, &len, log);
		Console->AddEntry("%s",log);
	}

	free(buf);
	return shader;
}

void SHADER::load_memory(const char *fragment_data,int frag_len,const char *vertex_data,int vert_len)
{
	if(!g_useProgram)	return;

	program=glCreateProgramObjectARB();
	vertex=load_vertex_shader_memory(vertex_data,vert_len);
	fragment=load_fragment_shader_memory(fragment_data,frag_len);
	glAttachObjectARB(program,vertex);
	glAttachObjectARB(program,fragment);
	glLinkProgramARB(program);
	GLint link=0;
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &link);
	if(link) {
		Console->AddEntry("succes");
		succes=true;
		}
	else {
		Console->AddEntry("failure");
		char log[10000];
		GLsizei len=0;
		glGetInfoLogARB(program, 10000, &len, log);
		Console->AddEntry("%s",log);
		succes=false;
		}
}

void SHADER::load(const char *fragment_file,const char *vertex_file)
{
	if(!g_useProgram)	return;

	program=glCreateProgramObjectARB();
	vertex=load_vertex_shader(vertex_file);
	fragment=load_fragment_shader(fragment_file);
	glAttachObjectARB(program,vertex);
	glAttachObjectARB(program,fragment);
	glLinkProgramARB(program);
	GLint link=0;
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &link);
	if(link) {
		Console->AddEntry("succes");
		succes=true;
		}
	else {
		Console->AddEntry("failure");
		char log[10000];
		GLsizei len=0;
		glGetInfoLogARB(program, 10000, &len, log);
		Console->AddEntry("%s",log);
		succes=false;
		}
}