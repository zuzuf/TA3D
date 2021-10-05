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

    Shader::Shader()
    {
    }

    Shader::Shader(const char *fragmentFilename, const char *vertexFilename)
    {
        load(fragmentFilename, vertexFilename);
    }

    Shader::Shader(const QString& fragmentFilename, const QString& vertexFilename)
    {
        load(fragmentFilename, vertexFilename);
    }

    Shader::Shader(const QByteArray &fragment_data, const QByteArray &vertex_data)
    {
        load_memory(fragment_data, vertex_data);
    }

    void Shader::load_memory(const QByteArray &fragment_data, const QByteArray &vertex_data)
	{
        if(isLinked())
			return;

        if (!addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_data))
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Vertex shader compilation error: " << log());
            exit(1);
            return;
        }
        if (!addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_data))
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Fragment shader compilation error: " << log());
            exit(1);
            return;
        }

        if (!link())
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Shader program link error: " << log());
            exit(1);
            return;
        }
        LOG_DEBUG(LOG_PREFIX_SHADER << "Shader program link succes");
	}



	void Shader::load(const QString& fragmentFilename, const QString& vertexFilename)
	{
        if(isLinked())
            return;

        const QByteArray &fragment_shader_code = VFS::Instance()->readFileAsBuffer(fragmentFilename);
        const QByteArray &vertex_shader_code = VFS::Instance()->readFileAsBuffer(vertexFilename);
        load_memory(fragment_shader_code, vertex_shader_code);
    }

    void Shader::setvar1f(const char *var, const float v0)
	{
        setUniformValue(var, v0);
	}

    void Shader::setvar2f(const char *var, const float v0, const float v1)
	{
        setUniformValue(var, v0, v1);
    }

    void Shader::setvar3f(const char *var, const float v0, const float v1, const float v2)
	{
        setUniformValue(var, v0, v1, v2);
    }

    void Shader::setvar4f(const char *var, const float v0, const float v1, const float v2, const float v3)
	{
        setUniformValue(var, v0, v1, v2, v3);
    }

    void Shader::setvar1i(const char *var, const int v0)
	{
        setUniformValue(var, v0);
    }

    void Shader::setmat4f(const char *var, const GLfloat *mat)
	{
        setUniformValue(var, QMatrix4x4(mat).transposed());
	}

// } // namespace GFX
} // namespace TA3D
