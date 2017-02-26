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

	void Shader::destroy()
	{
    }

	void Shader::load_memory(const char *fragment_data, const int frag_len,const char *vertex_data, const int vert_len)
	{
		if(!g_useProgram || lp_CONFIG->disable_GLSL)
			return;

        if (!pShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, QByteArray::fromRawData(vertex_data, vert_len)))
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Vertex shader compilation error: " << pShaderProgram.log());
            return;
        }
        if (!pShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, QByteArray::fromRawData(fragment_data, frag_len)))
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Fragment shader compilation error: " << pShaderProgram.log());
            return;
        }

        if (!pShaderProgram.link())
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Shader program link error: " << pShaderProgram.log());
            return;
        }
        LOG_DEBUG(LOG_PREFIX_SHADER << "Shader program link succes");
	}



	void Shader::load(const QString& fragmentFilename, const QString& vertexFilename)
	{
        if(!g_useProgram || lp_CONFIG->disable_GLSL)
            return;

        if (!pShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, VFS::Instance()->readFileAsBuffer(vertexFilename)))
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Vertex shader compilation error: " << pShaderProgram.log());
            return;
        }
        if (!pShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, VFS::Instance()->readFileAsBuffer(fragmentFilename)))
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Fragment shader compilation error: " << pShaderProgram.log());
            return;
        }

        if (!pShaderProgram.link())
        {
            LOG_DEBUG(LOG_PREFIX_SHADER << "Shader program link error: " << pShaderProgram.log());
            return;
        }
        LOG_DEBUG(LOG_PREFIX_SHADER << "Shader program link succes");
    }

	void Shader::on()
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.bind();
	}

	void Shader::off()
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.release();
	}

    void Shader::setvar1f(const char *var, const float v0)
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.setUniformValue(var, v0);
	}

    void Shader::setvar2f(const char *var, const float v0, const float v1)
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.setUniformValue(var, v0, v1);
    }

    void Shader::setvar3f(const char *var, const float v0, const float v1, const float v2)
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.setUniformValue(var, v0, v1, v2);
    }

    void Shader::setvar4f(const char *var, const float v0, const float v1, const float v2, const float v3)
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.setUniformValue(var, v0, v1, v2, v3);
    }

    void Shader::setvar1i(const char *var, const int v0)
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.setUniformValue(var, v0);
    }

    void Shader::setmat4f(const char *var, const GLfloat *mat)
	{
        if (!lp_CONFIG->disable_GLSL)
            pShaderProgram.setUniformValue(var, QMatrix4x4(mat));
	}

    bool Shader::isLoaded() const
    {
        return pShaderProgram.isLinked();
    }



// } // namespace GFX
} // namespace TA3D
