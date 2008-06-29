/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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

/*
**  File: TA3D_Platform.h
** Notes: **** PLEASE SEE README.txt FOR LICENCE AGREEMENT ****
**   Cire: *Defines the type of platform that we can compile an executable for.
**         *Since this is included within stdafx.h, and all cpp files MUST include
**           stdafx.h as its FIRST include there should be no need to directly
**           include this within .h files.
**         *This file also specifies variable types and sizes, which might be
**           size related.  It prevents any clashes or at least it should.  All other
**           files should use these 'variable typedefs' at all times when possible.
*/

#ifndef __TA3D_PLATFORM_H__
# define __TA3D_PLATFORM_H__


# ifdef TA3D_PLATFORM_WINDOWS
#   ifndef _MSC_VER
#       define TA3D_PLATFORM_GCC
#       define TA3D_PLATFORM_MINGW
#   else
#       define TA3D_PLATFORM_MSVC
#   endif
# endif


// The rand() function on Windows platforms should be replaced by something
// that fits a 32bits integer (it would be slower of course)
# ifdef TA3D_PLATFORM_WINDOWS
#   define TA3D_RAND()	(rand() | (rand() << 16))
# else
#   define TA3D_RAND()	rand()
# endif



# if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
#   if defined (_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64 // For Visual 6.x and later
    // 64-bit ints, guaranteed to be 4 bytes in size
	typedef unsigned __int64  uint64;
	typedef signed __int64    sint64;
#   else
#       error __int64 type not supported
#   endif
	// 32-bit ints, guaranteed to be 4 bytes in size
	typedef unsigned __int32  uint32;
	typedef signed __int32    sint32;
	// 16-bit ints, guaranteed to be 2 bytes in size
	typedef unsigned __int16  uint16;
	typedef signed __int16    sint16;
	// 8-bit ints, guaranteed to be 1 byte in size
	typedef unsigned __int8   uint8;
	typedef signed __int8     sint8;
# else
    // 64-bit ints, guaranteed to be 8 bytes in size
	typedef uint64_t  uint64;
	typedef int64_t   sint64;
	// 32-bit ints, guaranteed to be 4 bytes in size
	typedef uint32_t  uint32;
	typedef int32_t   sint32;
	// 16-bit ints, guaranteed to be 2 bytes in size
	typedef uint16_t uint16;
	typedef int16_t  sint16;
	// 8-bit ints, guaranteed to be 1 byte in size
	typedef uint8_t uint8;
	typedef int8_t  sint8;
# endif



#endif // __TA3D_PLATFORM_H__
