#include "shader.h"
#include "glfunc.h"
#include "../logs/logs.h"
#include "../misc/files.h"




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
            GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            int compiled = 0;

            glShaderSourceARB(shader, 1, (const GLcharARB **)&data, &size);
            glCompileShaderARB(shader); 
            glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

            if (compiled) 
            {
                // compilation successful!
                LOG_DEBUG(LOG_PREFIX_OPENGL << "Pixel shader: successfully compiled");
            }
            else 
            {
                // compilation error! Check compiler log! 
                LOG_ERROR(LOG_PREFIX_OPENGL << "Pixel shader: the compilation has failed");
                char log[10000];
                GLsizei len = 0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_OPENGL << log);
            }
            return shader;
        }


        GLhandleARB loadVertexShaderFromMemory(const char *data, const int size)
        {
            GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
            int compiled = 0;

            glShaderSourceARB(shader, 1, (const GLcharARB **)&data, &size);
            glCompileShaderARB(shader);
            glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

            if (compiled) 
            {
                // compilation successful!
                LOG_DEBUG("Vertex shader: successfully compiled");
            }
            else 
            {
                // compilation error! Check compiler log! 
                LOG_ERROR(LOG_PREFIX_OPENGL << "Vertex shader: the compilation has failed");
                char log[10000];
                GLsizei len=0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_OPENGL << log);
            }
            return shader;
        }


        GLhandleARB loadFragmentShader(const String& filename)
        {
            GLhandleARB	shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            uint64 filesize;
            char* buf = Paths::Files::LoadContentInMemory(filename, filesize, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
            if (!buf)
            {
                LOG_ERROR(LOG_PREFIX_OPENGL << "`" << filename << "` could not be opened");
                return shader;
            }

            GLint filesizeGL = (GLint)filesize;
            glShaderSourceARB(shader, 1, (const GLcharARB **)&buf, &filesizeGL);
            glCompileShaderARB(shader); 

            int compiled = 0;
            glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

            if (compiled) 
            {
                // compilation successful!
                LOG_DEBUG("Fragment shader:` " << filename << "` compiled");
            }
            else 
            {
                // compilation error! Check compiler log! 
                LOG_ERROR(LOG_PREFIX_OPENGL << "Fragment shader: `" << filename << "` failed to compile");
                char log[10000];
                GLsizei len = 0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_OPENGL << log);
            }
            delete[] buf;
            return shader;
        }



        GLhandleARB loadVertexShader(const String& filename)
        {
            GLhandleARB	shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

            uint64 filesize;
            char* buf = Paths::Files::LoadContentInMemory(filename, filesize, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
            if (!buf)
            {
                LOG_ERROR(LOG_PREFIX_OPENGL << "`" << filename << "` could not be opened");
                return shader;
            }

            int compiled = 0;

            GLint filesizeGL = (GLint)filesize;
            glShaderSourceARB(shader, 1, (const GLcharARB **)&buf, &filesizeGL);
            glCompileShaderARB(shader);
            glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

            if (compiled) 
            {
                // compilation successful!
                LOG_DEBUG("Vertex shader: `" << filename << "` compiled");
            }
            else 
            {
                // compilation error! Check compiler log! 
                LOG_ERROR(LOG_PREFIX_OPENGL << "Vertex sharder: `" << filename << "` failed to compile");
                char log[10000];
                GLsizei len = 0;
                glGetInfoLogARB(shader, 10000, &len, log);
                LOG_ERROR(LOG_PREFIX_OPENGL << log);
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
        if(!g_useProgram)
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
            LOG_DEBUG(LOG_PREFIX_OPENGL << "Object Link (ARB): Succes.");
            pLoaded = true;
        }
        else
        {
            LOG_ERROR(LOG_PREFIX_OPENGL << "Object Link (ARB): Failure");
            char log[10000];
            GLsizei len = 0;
            glGetInfoLogARB(pShaderProgram, 10000, &len, log);
            LOG_ERROR(LOG_PREFIX_OPENGL << log);
            pLoaded = false;
        }
    }


    void Shader::load(const String& fragmentFilename, const String& vertexFilename)
    {
        if (!g_useProgram)
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
            // LOG_DEBUG("Successfully loaded shader: `" << fragmentFilename << "`");
            pLoaded = true;
        }
        else
        {
            LOG_ERROR("Failed to load shader: `" << fragmentFilename << "`");
            char log[10000];
            GLsizei len = 0;
            glGetInfoLogARB(pShaderProgram, 10000, &len, log);
            LOG_ERROR(LOG_PREFIX_OPENGL << log);
            pLoaded = false;
        }
    }

    void Shader::on()
    {
        if (pLoaded)
            glUseProgramObjectARB(pShaderProgram);
    }

    void Shader::off()
    {
        if (pLoaded)
            glUseProgramObjectARB(0);
    }

    void Shader::setvar1f(const char* var, const float v0)
    {
        if (pLoaded)
            glUniform1fARB(glGetUniformLocationARB(pShaderProgram, var), v0);
    }

    void Shader::setvar2f(const char* var, const float v0, const float v1)
    {
        if (pLoaded)
            glUniform2fARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1);
    }

    void Shader::setvar3f(const char* var, const float v0, const float v1, const float v2)
    {
        if (pLoaded)
            glUniform3fARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2);
    }

    void Shader::setvar4f(const char* var, const float v0, const float v1, const float v2, const float v3)
    {
        if (pLoaded)
            glUniform4fARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2, v3);
    }

    void Shader::setvar1i(const char* var, const int v0)
    {
        if (pLoaded)
            glUniform1iARB(glGetUniformLocationARB(pShaderProgram, var), v0);
    }

    void Shader::setvar2i(const char* var, const int v0, const int v1)
    {
        if (pLoaded)
            glUniform2iARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1);
    }

    void Shader::setvar3i(const char* var, const int v0, const int v1, const int v2)
    {
        if (pLoaded)
            glUniform3iARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2);
    }

    void Shader::setvar4i(const char* var, const int v0, const int v1, const int v2, const int v3)
    {
        if (pLoaded)
            glUniform4iARB(glGetUniformLocationARB(pShaderProgram, var), v0, v1, v2, v3);
    }





    // } // namespace GFX
} // namespace TA3D
