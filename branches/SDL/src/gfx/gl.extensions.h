#ifndef __TA3D_GFX_GL_EXTENSIONS_H__
# define __TA3D_GFX_GL_EXTENSIONS_H__

#ifndef TA3D_PLATFORM_DARWIN

# define TA3D_GL_COMBINE_EXT         GL_COMBINE_EXT
# define TA3D_GL_COMBINE_RGB_EXT     GL_COMBINE_RGB_EXT
# define TA3D_GL_SOURCE0_RGB_EXT     GL_SOURCE0_RGB_EXT
# define TA3D_GL_SOURCE1_RGB_EXT     GL_SOURCE1_RGB_EXT
# define TA3D_GL_SOURCE2_RGB_EXT     GL_SOURCE2_RGB_EXT
# define TA3D_GL_OPERAND0_RGB_EXT    GL_OPERAND0_RGB_EXT
# define TA3D_GL_OPERAND1_RGB_EXT    GL_OPERAND1_RGB_EXT
# define TA3D_GL_OPERAND2_RGB_EXT    GL_OPERAND2_RGB_EXT
# define TA3D_GL_PREVIOUS_EXT        GL_PREVIOUS_EXT
# define TA3D_GL_CONSTANT_EXT        GL_CONSTANT_EXT

# else

# define TA3D_GL_COMBINE_EXT         GL_COMBINE_ARB
# define TA3D_GL_COMBINE_RGB_EXT     GL_COMBINE_RGB_ARB
# define TA3D_GL_SOURCE0_RGB_EXT     GL_SOURCE0_RGB_ARB
# define TA3D_GL_SOURCE1_RGB_EXT     GL_SOURCE1_RGB_ARB
# define TA3D_GL_SOURCE2_RGB_EXT     GL_SOURCE2_RGB_ARB
# define TA3D_GL_OPERAND0_RGB_EXT    GL_OPERAND0_RGB_ARB
# define TA3D_GL_OPERAND1_RGB_EXT    GL_OPERAND1_RGB_ARB
# define TA3D_GL_OPERAND2_RGB_EXT    GL_OPERAND2_RGB_ARB
# define TA3D_GL_PREVIOUS_EXT        GL_PREVIOUS_ARB
# define TA3D_GL_CONSTANT_EXT        GL_CONSTANT_ARB

# endif

#ifdef TA3D_PLATFORM_WINDOWS
    typedef unsigned long GLhandleARB;
    typedef char GLcharARB;
    typedef ptrdiff_t GLintptrARB;
    typedef ptrdiff_t GLsizeiptrARB;

    extern void (*glBindFramebufferEXT) (GLenum, GLuint);
    extern void (*glDeleteFramebuffersEXT) (GLsizei, const GLuint *);
    extern void (*glDeleteRenderbuffersEXT) (GLsizei, const GLuint *);
    extern void (*glBindRenderbufferEXT) (GLenum, GLuint);
    extern void (*glGenFramebuffersEXT) (GLsizei, GLuint *);
    extern void (*glGenRenderbuffersEXT) (GLsizei, GLuint *);
    extern void (*glFramebufferTexture2DEXT) (GLenum, GLenum, GLenum, GLuint, GLint);
    extern void (*glFramebufferRenderbufferEXT) (GLenum, GLenum, GLenum, GLuint);
    extern void (*glRenderbufferStorageEXT) (GLenum, GLenum, GLsizei, GLsizei);

    #define GL_RENDERBUFFER_EXT               0x8D41
    #define GL_DEPTH_ATTACHMENT_EXT           0x8D00
    #define GL_FRAMEBUFFER_EXT                0x8D40
    #define GL_COLOR_ATTACHMENT0_EXT          0x8CE0
    #define GL_DEPTH_COMPONENT24              0x81A6

    #define GL_ALPHA32F_ARB                   0x8816
    #define GL_ALPHA16F_ARB                   0x881C
    #define GL_RGBA32F_ARB                    0x8814
    #define GL_RGBA16F_ARB                    0x881A
    #define GL_RGB32F_ARB                     0x8815
    #define GL_RGB16F_ARB                     0x881B

    extern void (*glUseProgramObjectARB) (GLhandleARB);
    extern void (*glGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
    extern GLint (*glGetUniformLocationARB) (GLhandleARB, const GLcharARB *);
    extern void (*glGetObjectParameterivARB) (GLhandleARB, GLenum, GLint *);
    extern GLhandleARB (*glCreateProgramObjectARB) (void);
    extern void (*glAttachObjectARB) (GLhandleARB, GLhandleARB);
    extern void (*glLinkProgramARB) (GLhandleARB);
    extern void (*glDeleteObjectARB) (GLhandleARB);
    extern void (*glCompileShaderARB) (GLhandleARB);
    extern void (*glShaderSourceARB) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
    extern void (*glDetachObjectARB) (GLhandleARB, GLhandleARB);
    extern GLhandleARB (*glCreateShaderObjectARB) (GLenum);

    extern void (*glUniform1fARB) (GLint, GLfloat);
    extern void (*glUniform2fARB) (GLint, GLfloat, GLfloat);
    extern void (*glUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
    extern void (*glUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);

    extern void (*glUniform1iARB) (GLint, GLint);
    extern void (*glUniform2iARB) (GLint, GLint, GLint);
    extern void (*glUniform3iARB) (GLint, GLint, GLint, GLint);
    extern void (*glUniform4iARB) (GLint, GLint, GLint, GLint, GLint);

    #define GL_VERTEX_SHADER_ARB              0x8B31
    #define GL_FRAGMENT_SHADER_ARB            0x8B30
    #define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
    #define GL_OBJECT_LINK_STATUS_ARB         0x8B82

    extern void (*glPointParameterfv) (GLenum, const GLfloat *);
    extern void (*glPointParameterf) (GLenum, GLfloat);

    #define GL_COORD_REPLACE                  0x8862
    #define GL_POINT_SIZE_MIN                 0x8126
    #define GL_POINT_SIZE_MAX                 0x8127
    #define GL_POINT_SPRITE                   0x8861
    #define GL_POINT_DISTANCE_ATTENUATION     0x8129

    extern void (*glDeleteBuffersARB) (GLsizei, const GLuint *);
    extern void (*glGenBuffersARB) (GLsizei, GLuint *);
    extern void (*glBindBufferARB) (GLenum, GLuint);
    extern void (*glBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid *);
    extern void (*glBufferDataARB) (GLenum, GLsizeiptrARB, const GLvoid *, GLenum);

    #define GL_ARRAY_BUFFER_ARB               0x8892
    #define GL_ELEMENT_ARRAY_BUFFER_ARB       0x8893
    #define GL_STATIC_DRAW_ARB                0x88E4

    extern void (*glActiveStencilFaceEXT) (GLenum);
    #define GL_STENCIL_TEST_TWO_SIDE_EXT      0x8910
#endif

#endif // __TA3D_GFX_GL_EXTENSIONS_H__
