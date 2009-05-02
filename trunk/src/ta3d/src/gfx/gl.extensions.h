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

#endif // __TA3D_GFX_GL_EXTENSIONS_H__