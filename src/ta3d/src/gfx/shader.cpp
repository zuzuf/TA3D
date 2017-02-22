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
#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "shader.h"
#include "glfunc.h"
#include <logs/logs.h>
#include <misc/files.h>
#include <vfs/vfs.h>



namespace TA3D
{

	namespace
	{

		/*!
		* \brief Load an ARB fragment from a buffer
		*
		* \param data The buffer
		* \param size Size of the buffer
		*/
		GLhandleARB loadFragmentFromMemory(const char* data, const int size)
		{
			if (lp_CONFIG->disable_GLSL)    return 0;

			GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			int compiled = 0;

			glShaderSourceARB(shader, 1, (const GLcharARB **)&data, &size);
			glCompileShaderARB(shader);
			glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

			if (compiled)
			{
				// compilation successful!
				LOG_DEBUG(LOG_PREFIX_SHADER << "Pixel shader: successfully compiled");
			}
			else
			{
				// compilation error! Check compiler log!
				LOG_ERROR(LOG_PREFIX_SHADER << "Pixel shader: the compilation has failed");
#define GET_LOG(X)\
				do {\
					char log[10240];\
					memset(log, 0, 10240);\
					GLsizei len = 0;\
					glGetInfoLogARB(X, 10240, &len, log);\
					log[std::min(len, 10239)] = 0;\
					LOG_ERROR(LOG_PREFIX_SHADER << (const char*)log);\
				} while(false)
				GET_LOG(shader);
			}
			return shader;
		}


		GLhandleARB loadVertexShaderFromMemory(const char *data, const int size)
		{
			if (lp_CONFIG->disable_GLSL)    return 0;

			GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			int compiled = 0;

			glShaderSourceARB(shader, 1, (const GLcharARB **)&data, &size);
			glCompileShaderARB(shader);
			glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

			if (compiled)
			{
				// compilation successful!
				LOG_DEBUG(LOG_PREFIX_SHADER << "Vertex shader: successfully compiled");
			}
			else
			{
				// compilation error! Check compiler log!
				LOG_ERROR(LOG_PREFIX_SHADER << "Vertex shader: the compilation has failed");
				GET_LOG(shader);
			}
			return shader;
		}


		GLhandleARB loadFragmentShader(const QString& filename)
		{
			if (lp_CONFIG->disable_GLSL)    return 0;

            QIODevice *file = VFS::Instance()->readFile(filename);
			if (!file)
            {
				LOG_ERROR(LOG_PREFIX_SHADER << "`" << filename << "` could not be opened");
				return 0;
			}

			GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

            const QByteArray &buffer = file->readAll();
            GLint filesizeGL = (GLint)buffer.size();
            const char *buf = buffer.data();
			glShaderSourceARB(shader, 1, (const GLcharARB **)&buf, &filesizeGL);
			glCompileShaderARB(shader);

			int compiled = 0;
			glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

			if (compiled)
			{
				// compilation successful!
				LOG_DEBUG(LOG_PREFIX_SHADER << "Fragment shader:` " << filename << "` compiled");
			}
			else
			{
				// compilation error! Check compiler log!
				LOG_ERROR(LOG_PREFIX_SHADER << "Fragment shader: `" << filename << "` failed to compile");
				GET_LOG(shader);
			}
			delete file;
			return shader;
		}



		GLhandleARB loadVertexShader(const QString& filename)
		{
			if (lp_CONFIG->disable_GLSL)
				return 0;

            QIODevice *file = VFS::Instance()->readFile(filename);
			if (!file)
			{
				LOG_ERROR(LOG_PREFIX_SHADER << "`" << filename << "` could not be opened");
				return 0;
			}

			GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

			int compiled = 0;

            const QByteArray &buffer = file->readAll();
            GLint filesizeGL = (GLint)buffer.size();
            const char *buf = buffer.data();
			glShaderSourceARB(shader, 1, (const GLcharARB **)&buf, &filesizeGL);
			glCompileShaderARB(shader);
			glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

			if (compiled)
			{
				// compilation successful!
				LOG_DEBUG(LOG_PREFIX_SHADER << "Vertex shader: `" << filename << "` compiled");
			}
			else
			{
				// compilation error! Check compiler log!
				LOG_ERROR(LOG_PREFIX_SHADER << "Vertex sharder: `" << filename << "` failed to compile");
				GET_LOG(shader);
			}
			delete file;
			return shader;
		}



	} // anonymous namespace






	void Shader::destroy()
	{
		if (pLoaded)
		{
			glDetachObjectARB(pShaderProgram, pShaderFragment);
			glDetachObjectARB(pShaderProgram, pShaderVertex);
			glDeleteObjectARB(pShaderProgram);
			glDeleteObjectARB(pShaderFragment);
			glDeleteObjectARB(pShaderVertex);
			pLoaded = false;
		}
	}



	void Shader::load_memory(const char *fragment_data, const int frag_len,const char *vertex_data, const int vert_len)
	{
		if(!g_useProgram || lp_CONFIG->disable_GLSL)
			return;

		pShaderProgram  = glCreateProgramObjectARB();
		pShaderVertex   = loadVertexShaderFromMemory(vertex_data, vert_len);
		pShaderFragment = loadFragmentFromMemory(fragment_data, frag_len);

		glAttachObjectARB(pShaderProgram, pShaderVertex);
		glAttachObjectARB(pShaderProgram, pShaderFragment);
		glLinkProgramARB(pShaderProgram);

		GLint link = 0;
		glGetObjectParameterivARB(pShaderProgram, GL_OBJECT_LINK_STATUS_ARB, &link);
		if (link)
		{
			LOG_DEBUG(LOG_PREFIX_SHADER << "Object Link (ARB): Succes.");
			pLoaded = true;
		}
		else
		{
			LOG_ERROR(LOG_PREFIX_SHADER << "Object Link (ARB): Failure");
			GET_LOG(pShaderProgram);
			pLoaded = false;
		}
	}



	void Shader::load(const QString& fragmentFilename, const QString& vertexFilename)
	{
		if (!g_useProgram || lp_CONFIG->disable_GLSL)
			return;

		// Reset
		pShaderProgram  = glCreateProgramObjectARB();
		pShaderVertex   = loadVertexShader(vertexFilename);
		pShaderFragment = loadFragmentShader(fragmentFilename);

		// OpenGL  - attach objects
		glAttachObjectARB(pShaderProgram, pShaderVertex);
		glAttachObjectARB(pShaderProgram, pShaderFragment);
		glLinkProgramARB(pShaderProgram);
		GLint link = 0;
		glGetObjectParameterivARB(pShaderProgram, GL_OBJECT_LINK_STATUS_ARB, &link);
		if(link)
		{
			// LOG_DEBUG(LOG_PREFIX_SHADER << "Successfully loaded shader: `" << fragmentFilename << "`");
			pLoaded = true;
		}
		else
		{
			LOG_ERROR(LOG_PREFIX_SHADER << "Failed to load shader: `" << fragmentFilename << "`");
			GET_LOG(pShaderProgram);
			pLoaded = false;
		}
	}

	void Shader::on()
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
			glUseProgramObjectARB(pShaderProgram);
	}

	void Shader::off()
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
			glUseProgramObjectARB(0);
	}

	void Shader::setvar1f(const QString &var, const float v0)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform1fARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0);
	}

	void Shader::setvar2f(const QString &var, const float v0, const float v1)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform2fARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0, v1);
	}

	void Shader::setvar3f(const QString &var, const float v0, const float v1, const float v2)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform3fARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0, v1, v2);
	}

	void Shader::setvar4f(const QString &var, const float v0, const float v1, const float v2, const float v3)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform4fARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0, v1, v2, v3);
	}

	void Shader::setvar1i(const QString &var, const int v0)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform1iARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0);
	}

	void Shader::setvar2i(const QString &var, const int v0, const int v1)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform2iARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0, v1);
	}

	void Shader::setvar3i(const QString &var, const int v0, const int v1, const int v2)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform3iARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0, v1, v2);
	}

	void Shader::setvar4i(const QString &var, const int v0, const int v1, const int v2, const int v3)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform4iARB(glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), v0, v1, v2, v3);
	}

	void Shader::setmat4f(const QString &var, const GLfloat *mat)
	{
		if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniformMatrix4fv(	glGetUniformLocationARB(pShaderProgram, var.toStdString().c_str()), 1, GL_FALSE, mat);
	}





// } // namespace GFX
} // namespace TA3D
