
#include "../directory.h"
#include <errno.h>
#ifdef YUNI_HAS_STDLIB_H
# include <stdlib.h>
#endif
#ifndef YUNI_OS_MSVC
# include <dirent.h>
# include <unistd.h>
#endif
#ifdef YUNI_OS_WINDOWS
# include "../../system/windows.hdr.h"
# include <shellapi.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include "../../string.h"
#include "../../customstring/wstring.h"
#include <stdio.h>



namespace Yuni
{
namespace Private
{
namespace Core
{
namespace IO
{
namespace Directory
{

# ifdef YUNI_OS_WINDOWS

	bool Remove(const char* path)
	{
		WString<true> fsource(path);
		if (fsource.empty())
			return false;
		int cr;

		SHFILEOPSTRUCTW shf;
		shf.hwnd = NULL;

		shf.wFunc = FO_DELETE;
		shf.pFrom = fsource.c_str();
		shf.pTo = fsource.c_str();
		shf.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;

		cr = SHFileOperationW(&shf);
		return (!cr);
	}

# else


	namespace // Anonymous namespace
	{

		bool RmDirRecursiveInternal(const char path[])
		{
			DIR* dp;
			struct dirent* ep;
			CustomString<2096> buffer;
			struct stat st;

			if (NULL != (dp = ::opendir(path)))
			{
				while (NULL != (ep = ::readdir(dp)))
				{
					buffer.clear() << path << Yuni::Core::IO::Separator << (const char*)ep->d_name;
					if (0 == ::stat(buffer.c_str(), &st))
					{
						if (S_ISDIR(st.st_mode))
						{
							if (strcmp(".", (ep->d_name)) != 0 && strcmp("..", (ep->d_name)) != 0)
							{
								RmDirRecursiveInternal(buffer.c_str());
								::rmdir(buffer.c_str());
							}
						}
						else
							::unlink(buffer.c_str());
					}
				}
				(void)::closedir(dp);
			}
			return (0 == rmdir(path));
		}

	} // anonymous namespace




	bool Remove(const char path[])
	{
		if (NULL == path || '\0' == *path)
			return true;
		return RmDirRecursiveInternal(path);
	}


# endif



} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni

