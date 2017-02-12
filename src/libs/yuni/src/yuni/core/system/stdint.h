#ifndef __YUNI_SYSTEM_STANDARD_INTERFACE_INT_DATA_H__
# define __YUNI_SYSTEM_STANDARD_INTERFACE_INT_DATA_H__

/* !!! "C compatibility" header !!! */

/*!
** \file stdint.h
** \brief Standard types used by the Yuni Library
*/


# ifdef YUNI_OS_MSVC
#	include "windows/msinttypes/stdint.h"
#	include "windows/msinttypes/inttypes.h"
# else
#	include <stdint.h>
# endif

# ifdef YUNI_OS_MSVC
#	define YUNI_MSVC_SECURE_VSPRINTF
# endif




# ifdef __cplusplus /* Only with a C++ Compiler */

# ifndef YUNI_OS_CLANG
#	include <cstddef>
# endif

# ifdef YUNI_HAS_SYS_TYPES_H
#   include <sys/types.h>
# endif

namespace Yuni
{

	// \todo Fix support for int128 on Visual Studio 10
	# if defined(YUNI_HAS_INT128_T) && defined(__DISABLED_SUPPORT_FOR_INT128)
	/* 128-bit ints */
	typedef unsigned __int128  uint128;
	typedef __int128           sint128;
	# endif

	/* 64-bit ints, guaranteed to be 8 bytes in size */
	typedef uint64_t  uint64;
	typedef int64_t   sint64;
	/* 32-bit ints, guaranteed to be 4 bytes in size */
	typedef uint32_t  uint32;
	typedef int32_t   sint32;
	/* 16-bit ints, guaranteed to be 2 bytes in size */
	typedef uint16_t  uint16;
	typedef int16_t   sint16;
	/* 8-bit ints, guaranteed to be 1 byte in size */
	typedef unsigned char  uint8;
	typedef char           sint8;


# if defined(YUNI_OS_WINDOWS) && defined(YUNI_OS_MSVC)
	typedef sint64 ssize_t;
# endif




	/*!
	** \brief Constant int64 value to indicate that the size must be autodetected by the location
	** of the terminating null character
	*/
	static const uint64 AutoDetectNullChar = (uint64)(-1);



} /* namespace Yuni */

# else /* Actually we have a C Compiler */

# include "stddef.h"

# endif /* C++ Compiler */


#endif /* __YUNI_SYSTEM_STANDARD_INTERFACE_INT_DATA_H__ */
