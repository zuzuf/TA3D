#include "shader.h"
#include "glfunc.h"
#include "../logs/logs.h"

#define TA3D_OPENGL_PREFIX "[OpenGL] "

namespace TA3D
{
// namespace GFX
// {



    void Shader::destroy()
    {
        if (succes)
        {
            glDetachObjectARB(program,fragment);
            glDetachObjectARB(program,vertex);
            glDeleteObjectARB(program);
            glDeleteObjectARB(fragment);
            glDeleteObjectARB(vertex);
        }
        succes = false;
    }



    void Shader::load_memory(const char *fragment_data,int frag_len,const char *vertex_data,int vert_len)
    {
        if(!g_useProgram)
            return;

        program = glCreateProgramObjectARB();
        vertex = load_vertex_shader_memory(vertex_data,vert_len);
        fragment = load_fragment_shader_memory(fragment_data,frag_len);
        glAttachObjectARB(program,vertex);
        glAttachObjectARB(program,fragment);
        glLinkProgramARB(program);
        GLint link = 0;
        glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &link);
        if (link)
        {
            LOG_DEBUG(TA3D_OPENGL_PREFIX << "Object Link (ARB): Succes.");
            succes = true;
        }
        else
        {
            LOG_WARNING(TA3D_OPENGL_PREFIX << "Object Link (ARB): Failure");
            char log[10000];
            GLsizei len=0;
            glGetInfoLogARB(program, 10000, &len, log);
            LOG_DEBUG(TA3D_OPENGL_PREFIX << log);
            succes = false;
        }
    }


    void Shader::load(const char *fragment_file,const char *vertex_file)
    {
        if (!g_useProgram)
            return;

        program=glCreateProgramObjectARB();
        vertex=load_vertex_shader(vertex_file);
        fragment=load_fragment_shader(fragment_file);
        glAttachObjectARB(program,vertex);
        glAttachObjectARB(program,fragment);
        glLinkProgramARB(program);
        GLint link=0;
        glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &link);
        if(link)
        {
            // LOG_DEBUG("Successfully loaded shader: `" << fragment_file << "`");
            succes = true;
        }
        else
        {
            // LOG_WARNING("Failed to load shader: `" << fragment_file << "`");
            char log[10000];
            GLsizei len = 0;
            glGetInfoLogARB(program, 10000, &len, log);
            LOG_DEBUG(TA3D_OPENGL_PREFIX << log);
            succes = false;
        }
    }

    void Shader::on()
    {
        if (succes)
            glUseProgramObjectARB(program);
    }

    void Shader::off()
    {
        if (succes)
            glUseProgramObjectARB(0);
    }

    void Shader::setvar1f(const char *var_name, float v0)
    {
        if (succes)
            glUniform1fARB(glGetUniformLocationARB(program, var_name), v0);
    }

    void Shader::setvar2f(const char *var_name,float v0,float v1)
    {
        if (succes)
            glUniform2fARB(glGetUniformLocationARB(program, var_name), v0, v1);
    }

    void Shader::setvar3f(const char *var_name,float v0,float v1,float v2)
    {
        if (succes)
            glUniform3fARB(glGetUniformLocationARB(program, var_name), v0, v1, v2);
    }

    void Shader::setvar4f(const char *var_name,float v0,float v1,float v2,float v3)
    {
        if (succes)
            glUniform4fARB(glGetUniformLocationARB(program, var_name), v0, v1, v2, v3);
    }

    void Shader::setvar1i(const char *var_name,int v0)
    {
        if (succes)
            glUniform1iARB(glGetUniformLocationARB(program, var_name), v0);
    }

    void Shader::setvar2i(const char *var_name,int v0,int v1)
    {
        if (succes)
            glUniform2iARB(glGetUniformLocationARB(program, var_name), v0, v1);
    }

    void Shader::setvar3i(const char *var_name,int v0,int v1,int v2)
    {
        if (succes)
            glUniform3iARB(glGetUniformLocationARB(program, var_name), v0, v1, v2);
    }

    void Shader::setvar4i(const char *var_name,int v0,int v1,int v2,int v3)
    {
        if (succes)
            glUniform4iARB(glGetUniformLocationARB(program, var_name), v0, v1, v2, v3);
    }


// } // namespace GFX
} // namespace TA3D
