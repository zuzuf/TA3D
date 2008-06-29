#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(TA3D, 0.5.0, zuzuf86@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/ta3d.cpp])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
# Check for OpenGL:
AC_CHECK_LIB([GL], [main])
# Check for GLUT:
AC_CHECK_LIB([GLU], [main])
# Check for zlib:
AC_CHECK_LIB([z], [main])
# Check for fmod presence ( choose 32 or 64 bits version depending on the platform ):
AC_CHECK_LIB([fmodex], [main],, AC_CHECK_LIB([fmodex64], [main],,AC_MSG_ERROR([FMODex isn't installed on your system!!]) ) )
# Check for allegro >= 4.2.0:
AM_PATH_ALLEGRO( [4.2.2], , AC_MSG_ERROR(TA3D requires Allegro >= 4.2.2 to be installed))
# Check for allegrogl:
AC_CHECK_HEADER( [alleggl.h], , AC_MSG_ERROR(TA3D requires AllegroGL >= 0.4.0 to be installed))

# Check for the cross-compiler option
AC_ARG_ENABLE(mingw32,
[  --enable-mingw32  use this when crosscompiling from linux to windows in order to fix undetected libraries],
[
make -C "src/lua/" clean
make -C "src/lua/" mingw CC=$CC RANLIB=$host-ranlib

make -C "src/hawknl/src/" -f makefile.mingw clean
make -C "src/hawknl/src/" -f makefile.mingw CC=$CC AR=$host-ar

cp src/lua/lua51.dll ./
cp src/hawknl/src/NL.dll ./

LIBS="-L./lua/ -llua -lagl -lalleg -luser32 -lgdi32 -lopengl32 -lglu32 -lz.dll -lfmodex -L./hawknl/src/ -lNL"
CXXFLAGS="$CXXFLAGS -I/usr/i586-mingw32msvc/include"],
[
make -C "src/lua/" clean
make -C "src/lua/" linux

make -C "src/hawknl/src/" -f makefile.linux clean
make -C "src/hawknl/src/" -f makefile.linux
LIBS="$LIBS -lagl -L./lua/ -llua -L./hawknl/src/ -lNL $allegro_LIBS"
CXXFLAGS="$CXXFLAGS -rdynamic $allegro_CFLAGS"])

CXXFLAGS="$CXXFLAGS -ffast-math -mmmx"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([select sqrt strcasecmp strdup strstr])

AC_CONFIG_FILES([
	Makefile
	src/Makefile])
AC_OUTPUT
