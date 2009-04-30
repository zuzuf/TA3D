#include <QFile>
#include <QDebug>
#include "program.h"

GLhandleARB loadFragmentProgramFromMemory(const QString& src)
{
    QByteArray bytes = src.toAscii();
    if (bytes.isEmpty())
        return 0;

    GLhandleARB	program = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    GLint filesizeGL = (GLint)bytes.size();
    GLcharARB *buf = (GLcharARB*) bytes.data();
    glShaderSourceARB(program, 1, (const GLcharARB **)&buf, &filesizeGL);
    glCompileShaderARB(program);

    int compiled = 0;
    glGetObjectParameterivARB(program, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (compiled)
    {
        // compilation successful!
        qDebug() << "Fragment shader compiled";
    }
    else
    {
        // compilation error! Check compiler log!
        qDebug() << "Fragment shader failed to compile";
        char log[10000];
        GLsizei len = 0;
        glGetInfoLogARB(program, 10000, &len, log);
        qDebug() << log;
    }
    return program;
}

GLhandleARB loadVertexProgramFromMemory(const QString &src)
{
    QByteArray bytes = src.toAscii();
    if (bytes.isEmpty())
        return 0;

    GLhandleARB	program = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

    int compiled = 0;

    GLint filesizeGL = (GLint)bytes.size();
    GLcharARB *buf = (GLcharARB*) bytes.data();
    glShaderSourceARB(program, 1, (const GLcharARB **)&buf, &filesizeGL);
    glCompileShaderARB(program);
    glGetObjectParameterivARB(program, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (compiled)
    {
        // compilation successful!
        qDebug() << "Vertex shader compiled";
    }
    else
    {
        // compilation error! Check compiler log!
        qDebug() << "Vertex sharder failed to compile";
        char log[10000];
        GLsizei len = 0;
        glGetInfoLogARB(program, 10000, &len, log);
        qDebug() << log;
    }
    return program;
}

GLhandleARB loadFragmentProgram(const QString& filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        return 0;
    QByteArray bytes = file.readAll();
    file.close();
    if (bytes.isEmpty())
        return 0;

    GLhandleARB	program = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    GLint filesizeGL = (GLint)bytes.size();
    GLcharARB *buf = (GLcharARB*) bytes.data();
    glShaderSourceARB(program, 1, (const GLcharARB **)&buf, &filesizeGL);
    glCompileShaderARB(program);

    int compiled = 0;
    glGetObjectParameterivARB(program, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (compiled)
    {
        // compilation successful!
        qDebug() << "Fragment shader:` " << filename << "` compiled";
    }
    else
    {
        // compilation error! Check compiler log!
        qDebug() << "Fragment shader: `" << filename << "` failed to compile";
        char log[10000];
        GLsizei len = 0;
        glGetInfoLogARB(program, 10000, &len, log);
        qDebug() << log;
    }
    return program;
}

GLhandleARB loadVertexProgram(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        return 0;
    QByteArray bytes = file.readAll();
    file.close();
    if (bytes.isEmpty())
        return 0;

    GLhandleARB	program = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

    int compiled = 0;

    GLint filesizeGL = (GLint)bytes.size();
    GLcharARB *buf = (GLcharARB*) bytes.data();
    glShaderSourceARB(program, 1, (const GLcharARB **)&buf, &filesizeGL);
    glCompileShaderARB(program);
    glGetObjectParameterivARB(program, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    if (compiled)
    {
        // compilation successful!
        qDebug() << "Vertex shader: `" << filename << "` compiled";
    }
    else
    {
        // compilation error! Check compiler log!
        qDebug() << "Vertex sharder: `" << filename << "` failed to compile";
        char log[10000];
        GLsizei len = 0;
        glGetInfoLogARB(program, 10000, &len, log);
        qDebug() << log;
    }
    return program;
}

void Program::destroy()
{
    if (pLoaded)
    {
        glDetachObjectARB(pProgram, pFragmentProgram);
        glDetachObjectARB(pProgram, pVertexProgram);
        glDeleteObjectARB(pProgram);
        glDeleteObjectARB(pFragmentProgram);
        glDeleteObjectARB(pVertexProgram);
    }
    pLoaded = false;
}

QString Program::load(const QString &programFilename)
{
    // Reset
    pProgram  = glCreateProgramObjectARB();
    pVertexProgram   = loadVertexProgram(programFilename + ".vert");
    pFragmentProgram = loadFragmentProgram(programFilename + ".frag");

    // OpenGL  - attach objects
    glAttachObjectARB(pProgram, pVertexProgram);
    glAttachObjectARB(pProgram, pFragmentProgram);
    glLinkProgramARB(pProgram);
    GLint link = 0;
    glGetObjectParameterivARB(pProgram, GL_OBJECT_LINK_STATUS_ARB, &link);
    QString sLog;
    if (link)
    {
        // LOG_DEBUG(LOG_PREFIX_GLSL << "Successfully loaded shader: `" << fragmentFilename << "`");
        pLoaded = true;
    }
    else
    {
        qDebug() << "Failed to load shader: `" << programFilename << "`";
        sLog.append("Failed to load shader: `").append(programFilename).append("`");
        char log[10000];
        GLsizei len = 0;
        glGetInfoLogARB(pProgram, 10000, &len, log);
        qDebug() << log;
        sLog.append(log);
        pLoaded = false;
    }
    return sLog;
}

QString Program::load_memory(const QString &vertexProgram, const QString &fragmentProgram)
{
    pProgram  = glCreateProgramObjectARB();
    pVertexProgram   = loadVertexProgramFromMemory(vertexProgram);
    pFragmentProgram = loadFragmentProgramFromMemory(fragmentProgram);

    // OpenGL  - attach objects
    glAttachObjectARB(pProgram, pVertexProgram);
    glAttachObjectARB(pProgram, pFragmentProgram);
    glLinkProgramARB(pProgram);
    GLint link = 0;
    glGetObjectParameterivARB(pProgram, GL_OBJECT_LINK_STATUS_ARB, &link);
    QString sLog;
    if (link)
    {
        pLoaded = true;
    }
    else
    {
        qDebug() << "Failed to load shader from memory";
        sLog.append("Failed to load shader from memory");
        char log[10000];
        GLsizei len = 0;
        glGetInfoLogARB(pProgram, 10000, &len, log);
        qDebug() << log;
        sLog.append(log);
        pLoaded = false;
    }
    return sLog;
}

void Program::on()
{
    if (pLoaded)
    {
        glUseProgramObjectARB(pProgram);
        pOn = true;
    }
}

void Program::off()
{
    if (pLoaded)
    {
        glUseProgramObjectARB(0);
        pOn = false;
    }
}

bool Program::isOn()
{
    return pOn;
}

void Program::setvar1f(const char* var, const float v0)
{
    if (pLoaded)
        glUniform1fARB(glGetUniformLocationARB(pProgram, var), v0);
}

void Program::setvar2f(const char* var, const float v0, const float v1)
{
    if (pLoaded)
        glUniform2fARB(glGetUniformLocationARB(pProgram, var), v0, v1);
}

void Program::setvar3f(const char* var, const float v0, const float v1, const float v2)
{
    if (pLoaded)
        glUniform3fARB(glGetUniformLocationARB(pProgram, var), v0, v1, v2);
}

void Program::setvar4f(const char* var, const float v0, const float v1, const float v2, const float v3)
{
    if (pLoaded)
        glUniform4fARB(glGetUniformLocationARB(pProgram, var), v0, v1, v2, v3);
}

void Program::setvar1i(const char* var, const int v0)
{
    if (pLoaded)
        glUniform1iARB(glGetUniformLocationARB(pProgram, var), v0);
}

void Program::setvar2i(const char* var, const int v0, const int v1)
{
    if (pLoaded)
        glUniform2iARB(glGetUniformLocationARB(pProgram, var), v0, v1);
}

void Program::setvar3i(const char* var, const int v0, const int v1, const int v2)
{
    if (pLoaded)
        glUniform3iARB(glGetUniformLocationARB(pProgram, var), v0, v1, v2);
}

void Program::setvar4i(const char* var, const int v0, const int v1, const int v2, const int v3)
{
    if (pLoaded)
        glUniform4iARB(glGetUniformLocationARB(pProgram, var), v0, v1, v2, v3);
}

void Program::setmat4f(const char* var, const GLfloat *mat)
{
    if (pLoaded)
        glUniformMatrix4fv(	glGetUniformLocationARB(pProgram, var), 1, GL_FALSE, mat);
}
