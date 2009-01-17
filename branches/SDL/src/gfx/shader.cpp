#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "shader.h"
#include "glfunc.h"
#include "../logs/logs.h"
#include "../misc/files.h"
#include "../TA3D_hpi.h"



namespace TA3D
{
// namespace GFX
// {


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
                char log[10000];
                GLsizei len = 0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_SHADER << log);
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
                char log[10000];
                GLsizei len=0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_SHADER << log);
            }
            return shader;
        }


        GLhandleARB loadFragmentShader(const String& filename)
        {
            if (lp_CONFIG->disable_GLSL)    return 0;

            uint64 filesize;
            char* buf = NULL;
            if (HPIManager)
            {
                uint32 fs(0);
                buf = (char*)HPIManager->PullFromHPI(filename, &fs);
                filesize = fs;
            }
            else
                buf = Paths::Files::LoadContentInMemory(filename, filesize, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
            if (!buf)
            {
                LOG_ERROR(LOG_PREFIX_SHADER << "`" << filename << "` could not be opened");
                return 0;
            }

            GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

            GLint filesizeGL = (GLint)filesize;
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
                char log[10000];
                GLsizei len = 0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_SHADER << log);
            }
            delete[] buf;
            return shader;
        }



        GLhandleARB loadVertexShader(const String& filename)
        {
            if (lp_CONFIG->disable_GLSL)    return 0;

            uint64 filesize;
            char* buf = NULL;
            if (HPIManager)
            {
                uint32 fs(0);
                buf = (char*)HPIManager->PullFromHPI(filename, &fs);
                filesize = fs;
            }
            else
                buf = Paths::Files::LoadContentInMemory(filename, filesize, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
            if (!buf)
            {
                LOG_ERROR(LOG_PREFIX_SHADER << "`" << filename << "` could not be opened");
                return 0;
            }

            GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

            int compiled = 0;

            GLint filesizeGL = (GLint)filesize;
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
                char log[10000];
                GLsizei len = 0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_SHADER << log);
            }
            delete[] buf;
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
        }
        pLoaded = false;
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
            char log[10000];
            GLsizei len = 0;
            glGetInfoLogARB(pShaderProgram, 10000, &len, log);
            LOG_ERROR(LOG_PREFIX_SHADER << log);
            pLoaded = false;
        }
    }


    void Shader::load(const String& fragmentFilename, const String& vertexFilename)
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
            char log[10000];
            GLsizei len = 0;
            glGetInfoLogARB(pShaderProgram, 10000, &len, log);
            LOG_ERROR(LOG_PREFIX_SHADER << log);
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

    void Shader::setvar1f(const char* var, const float v0)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform1fARB(glGetUniformLocationARB(pShaderProgram, var), v0);
    }

    void Shader::setvar2f(const char* var, const float v0, const float v1)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform2fARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1);
    }

    void Shader::setvar3f(const char* var, const float v0, const float v1, const float v2)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform3fARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2);
    }

    void Shader::setvar4f(const char* var, const float v0, const float v1, const float v2, const float v3)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform4fARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2, v3);
    }

    void Shader::setvar1i(const char* var, const int v0)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform1iARB(glGetUniformLocationARB(pShaderProgram, var), v0);
    }

    void Shader::setvar2i(const char* var, const int v0, const int v1)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform2iARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1);
    }

    void Shader::setvar3i(const char* var, const int v0, const int v1, const int v2)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform3iARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2);
    }

    void Shader::setvar4i(const char* var, const int v0, const int v1, const int v2, const int v3)
    {
        if (pLoaded && !lp_CONFIG->disable_GLSL)
            glUniform4iARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2, v3);
    }





// } // namespace GFX
} // namespace TA3D
