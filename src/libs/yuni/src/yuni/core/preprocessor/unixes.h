#ifndef __YUNI_CORE_PREPROCESSOR_CAPABILITIES_UNIXES_H__
# define __YUNI_CORE_PREPROCESSOR_CAPABILITIES_UNIXES_H__

/*
** Information about Unix capabilities
*/

/* Unix compliant */
# define YUNI_OS_UNIX

# undef  YUNI_OS_FLAG_UNIX
# define YUNI_OS_FLAG_UNIX 1


# if defined(__HAIKU) || defined(__HAIKU__) || defined(_HAIKU)
/* Haiku */
#	define YUNI_OS_HAIKU
#	define YUNI_OS_NAME "Haiku"
# else
/* BeOS */
#	if defined(__BeOS) || defined(__BEOS__) || defined(_BEOS)
#		define YUNI_OS_BEOS
#		define YUNI_OS_NAME "BeOS"
#	endif
# endif



/* Linux */
# if defined(__linux) || defined(linux) || defined(__linux__)
#	define YUNI_OS_LINUX
#	define YUNI_OS_NAME "GNU/Linux"
#	undef  YUNI_OS_FLAG_LINUX
#	define YUNI_OS_FLAG_LINUX 1
# else

/* AIX */
# 	if defined(_AIX)
#		define YUNI_OS_AIX
#		define YUNI_OS_NAME "AIX"
# 	endif

/* DragonFly */
# 	if defined(__DragonFly__)
# 		define YUNI_OS_DRAGONFLY
#		define YUNI_OS_NAME "DragonFly"
# 	endif

/* HP-UX */
#	if defined(_hpux) || defined(__hpux) || defined(__hpux__)
#		define YUNI_OS_HPUX
#		define YUNI_OS_NAME "HP-UX"
#	endif

/* Mac OS */
#	if defined(__APPLE__) || defined(__MACH__)
#		define YUNI_OS_MAC
#		define YUNI_OS_MACOS
#		define YUNI_OS_DARWIN
#		define YUNI_OS_NAME "MacOS X"
#		undef  YUNI_OS_FLAG_MACOS
#		define YUNI_OS_FLAG_MACOS 1
#	else
/* FreeBSD */
#		if defined(__FreeBSD__)
#			define YUNI_OS_FREEBSD __FreeBSD__
#			define YUNI_OS_NAME "FreeBSD"
#		endif
#	endif

/* NetBSD */
#	if defined(__NetBSD__)
#		define YUNI_OS_NETBSD
#		define YUNI_OS_NAME "NetBSD"
#	endif

/* OpenBSD */
#	if defined(__OpenBSD__)
#		define YUNI_OS_OPENBSD
#		define YUNI_OS_NAME "OpenBSD"
#	endif
#	if defined(sun) || defined(__sun)

/* Solaris */
#		if defined(__SVR4) || defined(__svr4__)
#			 define YUNI_OS_SOLARIS
#			define YUNI_OS_NAME "Solaris"
#		else
/* SunOS */
#			define YUNI_OS_SUNOS
#			define YUNI_OS_NAME "SunOS"
#		endif
#		if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#			define YUNI_OS_SUNSTUDIO
#			define YUNI_COMPILER_NAME "SunCC"
#		endif
#	endif /* Sun */
/* Lynx */
# 	ifdef __Lynx__
#		define YUNI_OS_LYNX
#		define YUNI_OS_NAME "Lynx"
# 	endif
/* Cygwin */
#	ifdef __CYGWIN__
#	   define YUNI_OS_CYGWIN
#	   define YUNI_OS_NAME "Windows Cygwin"
#	endif


# endif /* Linux */



#endif /* __YUNI_CORE_PREPROCESSOR_CAPABILITIES_UNIXES_H__ */
