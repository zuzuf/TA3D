#ifndef __YUNI_PREPROCESSOR_OS_DETECTION_H__
# define __YUNI_PREPROCESSOR_OS_DETECTION_H__

/* !!! "C compatibility" header !!! */


/*!
** \brief Operating System / Capabilities Auto-Detection
**
** General defines :
** YUNI_OS_NAME: Name of the operating system (without its version which can only be
**   determined at runtime)
** YUNI_COMPILER_NAME: Name of the compiler currently used
**
** Usefull defines :
**
** - Microsoft Windows
** YUNI_OS_WINDOWS: The Microsoft Windows Operating system (32/64Bits)
** YUNI_OS_WIN32: Microsoft Windows 32Bits
** YUNI_OS_WIN64: Microsoft Windows 64Bits
** YUNI_OS_WINCE: Microsoft Windows CE
** YUNI_OS_MSVC: The compiler is Microsoft Visual Studio
**
** - Unixes
** YUNI_OS_UNIX
**
** - MacOS
** YUNI_OS_MACOS or YUNI_OS_MAC or YUNI_OS_DARWIN
**
** - FreeBSD
** YUNI_OS_FREEBSD (The value is the version)
**
** - AIX
** YUNI_OS_AIX
**
** - HPUX
** YUNI_OS_HPUX
**
** - BEOS
** YUNI_OS_BEOS
**
** - DragonFLY
** YUNI_OS_DRAGONFLY
**
** - LynxOS
** YUNI_OS_LYNX
**
** - NetBSD
** YUNI_OS_NETBSD
**
** - OpenBSD
** YUNI_OS_OPENBSD
**
** - Solaris/SunOS
** YUNI_OS_SOLARIS
** YUNI_OS_SUNOS
**
** - Unknown
** YUNI_OS_UNKNOWN
** YUNI_OS_COMPILER
**
** Window systems
** - X Window System
** YUNI_WINDOWSYSTEM_X11
** - MFC (Microsoft Foundation Class) or any Microsoft Windows Window System
** YUNI_WINDOWSYSTEM_MSW
** - Mac OS X Cocoa
** YUNI_WINDOWSYSTEM_COCOA
**
** Misc:
**
** - Borland C++ : YUNI_OS_BORLAND
** - MinGW: YUNI_OS_MINGW
** - CLang: YUNI_OS_CLANG
** - Cygwin : YUNI_OS_CYGWIN
** - Gcc/G++ : YUNI_OS_GCC, YUNI_OS_GCC_VERSION : if YUNI_OS_GCC_VERSION > 30200  (> 3.2.0)
** - Intel Compiler: YUNI_OS_INTELCOMPILER
** - Sun Studio: YUNI_OS_SUNSTUDIO
**
** 32/64 Bits
** YUNI_OS_32 or YUNI_OS_64
**
** C++0X
** YUNI_CPP_0X
**
** All those informations can be found at http://predef.sourceforge.net/
*/


# define YUNI_OS_FLAG_WINDOWS  0
# define YUNI_OS_FLAG_UNIX     0
# define YUNI_OS_FLAG_LINUX    0
# define YUNI_OS_FLAG_MACOS    0


# if defined(__TOS_WIN__) || defined(__WIN32__) || defined(_WIN64) || defined(_WIN32)
#	include "windows.h"
# else
#	include "unixes.h"
# endif



/* Window System */
# ifdef YUNI_OS_WINDOWS
#	define YUNI_WINDOWSYSTEM_MSW
# else
#	ifdef YUNI_OS_MAC
#		define YUNI_WINDOWSYSTEM_COCOA
#	else
#		define YUNI_WINDOWSYSTEM_X11
#	endif
# endif


# ifdef __clang__
#	define YUNI_OS_CLANG
# else
/* GNU C and C++ compiler */
#	ifdef __GNUC__
#		define YUNI_OS_GCC
#		define YUNI_OS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#		ifndef YUNI_COMPILER_NAME
#			define YUNI_COMPILER_NAME "GCC (GNU Compiler Collection)"
#		endif
#	else
#		define YUNI_OS_GCC_VERSION 0
#	endif
# endif


/* Intel Compiler */
# ifdef __INTEL_COMPILER
#	define YUNI_OS_INTELCOMPILER
#	ifdef YUNI_COMPILER_NAME /* in some cases, the compiler may already have been detected as Visual Studio */
#		undef YUNI_COMPILER_NAME
#	endif
#	define YUNI_COMPILER_NAME "ICC (Intel C++ Compiler)"
# endif



/* 32/64 Bits modes */
# if !defined(YUNI_OS_32) && !defined(YUNI_OS_64)
#	if defined(__IA64__) || defined(_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_IA64) || defined(_WIN64)
#		 define YUNI_OS_64
#	else
#		 define YUNI_OS_32
#	endif
# endif






/* inline */
# ifdef YUNI_OS_GCC
#	define YUNI_ALWAYS_INLINE  __attribute__((always_inline))
# else
#	ifdef YUNI_OS_MSVC
#		define YUNI_ALWAYS_INLINE  __forceinline
# 	endif
# endif
# ifndef YUNI_ALWAYS_INLINE
#	define YUNI_ALWAYS_INLINE
# endif


/*!
** \macro YUNI_DEPRECATED
** \brief Deprecated
*/
# if defined(YUNI_OS_GCC)
#	if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#		define YUNI_DEPRECATED(text, func...)  func __attribute__((__deprecated__))
#	endif
# endif
# if defined(_MSC_VER) && (_MSC_VER >= 1300)
#	if (_MSC_FULL_VER >= 140050320)
#		define YUNI_DEPRECATED(text, func, ...) __declspec(deprecated(text)) func
#	else
#		define YUNI_DEPRECATED(text, func, ...) __declspec(deprecated) func
#	endif
# endif
# ifndef YUNI_DEPRECATED
#	define YUNI_DEPRECATED(text, func...) func
# endif

/* Noreturn */
# if defined(YUNI_OS_GCC)
#	if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#		define YUNI_NORETURN  __attribute__((noreturn))
#	endif
# endif
# ifndef YUNI_NORETURN
#	define YUNI_NORETURN
# endif




/* C++0x */
# if defined(__GXX_EXPERIMENTAL_CPP0X__) || defined(_RWSTD_EXT_CXX_0X)
#	define YUNI_CPP_0X
# endif



/* OS Detection */
# ifndef YUNI_OS_NAME
#	define YUNI_OS_NAME "Unknown"
#	define YUNI_OS_UNKNOWN
#	warning "OS Detection: Unable to guess the operating system"
# endif

/* Compiler Detection */
# ifndef YUNI_OS_NAME
#	ifndef YUNI_COMPILER_NAME
#		define YUNI_COMPILER_NAME "Unknown"
#	endif
#	define YUNI_COMPILER_UNKNOWN
#	warning "Compiler Detection: Unable to guess the compiler"
# endif



/*!
** \brief Thread-local variable
*/
/* Usage Example :
** \code
** YUNI_THREAD_LOCAL int myThreadLocalVariable = 0;
** \endcode
**
** \see Rules and Limitations for TLS on Windows http://msdn.microsoft.com/en-us/library/2s9wt68x.aspx
*/
# if defined(YUNI_OS_MSVC) || defined(__INTEL_COMPILER)
#	define YUNI_THREAD_LOCAL __declspec(thread)
#	define YUNI_HAS_THREAD_LOCAL_STORAGE 1
# else
#	define YUNI_THREAD_LOCAL __thread
#	define YUNI_HAS_THREAD_LOCAL_STORAGE 1
# endif



/*!
** \define YUNI_VA_COPY
** \brief Copy one variable argument list into another
**
** You should use va_end to release the new list
*/
# ifdef YUNI_HAS_VA_COPY
#	define YUNI_VA_COPY(dst, src)  va_copy((dst), (src))
# else
#	if (defined(__GNUC__) && defined(__PPC__) && (defined(_CALL_SYSV) || defined(__WIN32__))) || defined(__WATCOMC__)
#		define YUNI_VA_COPY(dst, src)	  (*(dst) = *(src))
#	elif defined(YUNI_VA_COPY_AS_ARRAY)
#		define YUNI_VA_COPY(dst, src)	  memmove((dst), (src), sizeof (va_list))
#	else /* va_list is a pointer */
#		define YUNI_VA_COPY(dst, src)	  ((dst) = (src))
#	endif
# endif


/* Force the define YUNI_DYNAMIC_LIBRARY (Visual Studio) */
# if !defined(YUNI_DYNAMIC_LIBRARY) && defined(_WINDLL)
#	define YUNI_DYNAMIC_LIBRARY
#	define YUNI_DYNAMIC_LIBRARY_EXPORT
# endif

# ifdef YUNI_DYNAMIC_LIBRARY
#	ifdef YUNI_OS_MSVC
#		ifdef YUNI_DYNAMIC_LIBRARY_EXPORT
#			define YUNI_DECL             __declspec(dllexport)
#			define YUNI_EXPIMP_TEMPLATE
#		else
#			define YUNI_DECL             __declspec(dllimport)
#			define YUNI_EXPIMP_TEMPLATE  extern
#		endif
#	endif
# endif
/* Fallback to empty */
# ifndef YUNI_DECL
#	define YUNI_DECL
# endif
# ifndef YUNI_EXPIMP_TEMPLATE
#	define YUNI_EXPIMP_TEMPLATE
# endif



/* In some cases, the macro 'unix' and 'linux' can be defined */
# ifdef unix
#	undef unix
# endif
# ifdef linux
#	undef linux
# endif


# ifndef YUNI_OS_GCC_VERSION
#	define YUNI_OS_GCC_VERSION 0
# endif



# ifdef __cplusplus /* Only with a C++ Compiler */

namespace Yuni
{
namespace System
{

	/* Operating systems */
	enum
	{
		/*! Flag to indicate if the current operating system is Microsoft Windows */
		windows = YUNI_OS_FLAG_WINDOWS,
		/*! Flag to indicate if the current operating system is Unix based */
		unix    = YUNI_OS_FLAG_UNIX,
		/*! Flag to indicate if the current operating system is Linux based */
		linux   = YUNI_OS_FLAG_LINUX,
		/*! Flag to indicate if the current operating system is Mac OS */
		macos   = YUNI_OS_FLAG_MACOS,
	};



} /* namespace System */
} /* namespace Yuni */

# endif



#endif /* __YUNI_PREPROCESSOR_OS_DETECTION_H__ */
