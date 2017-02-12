#ifndef __YUNI_CORE_IO_DIRECTORY_COMMONS_H__
# define __YUNI_CORE_IO_DIRECTORY_COMMONS_H__

#include "../io.h"
#include "../directory.h"
#include "../../system/windows.hdr.h"
#ifndef YUNI_OS_WINDOWS
#	ifdef YUNI_HAS_STDLIB_H
#		include <stdlib.h>
#	endif
#	include <unistd.h>
#else
#	include <direct.h>
#endif
#include <sys/stat.h>
#include <fstream>
#include <errno.h>
#ifdef YUNI_OS_MSVC
# include <direct.h>
#else
# include <dirent.h>
#endif
#include <fcntl.h>


#ifndef S_ISDIR
#	define S_ISDIR(mode) ( (mode & S_IFMT) == S_IFDIR)
#endif


#endif // __YUNI_CORE_IO_DIRECTORY_COMMONS_H__
