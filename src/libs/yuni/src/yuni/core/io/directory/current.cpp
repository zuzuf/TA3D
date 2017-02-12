
#include "../directory.h"
#ifndef YUNI_OS_WINDOWS
# include <unistd.h>
#else
# include "../../system/windows.hdr.h"
# include <direct.h>
# include <stdlib.h>
# include <stdio.h>
#endif


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


	char* CurrentDirectory()
	{
		# ifdef YUNI_OS_WINDOWS

		char* ret = NULL;
		const wchar_t* c = _wgetcwd(NULL, 0 /* Arbitrary value */);

		const int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, c, -1, NULL, 0,  NULL, NULL);
		if (sizeRequired > 0)
		{
			ret = (char*)::malloc(sizeRequired * sizeof(char) + 1);
			if (ret)
			{
				if (WideCharToMultiByte(CP_UTF8, 0, c, -1, ret, sizeRequired,  NULL, NULL) > 0)
				{
					ret[sizeRequired] = '\0';
				}
				else
				{
                    ::free(ret);
					ret = NULL;
				}
			}
		}
		return ret;

		# else

		return ::getcwd(NULL, 0 /* arbitrary value */);

		# endif
	}



	bool ChangeCurrentDirectory(const char* src, unsigned int srclen)
	{
		# ifdef YUNI_OS_WINDOWS
		wchar_t* fsource = new wchar_t[srclen + 4];
		int n = MultiByteToWideChar(CP_UTF8, 0, src, srclen, fsource, srclen);
		if (n <= 0)
			return false;
		fsource[n]     = L'\0'; // This string must be double-null terminated

		const bool r = (0 == _wchdir(fsource));
		delete[] fsource;

		return r;

		# else
		(void) srclen;
		return (0 == ::chdir(src));
		# endif
	}


	bool ChangeCurrentDirectoryNotZeroTerminated(const char* path, unsigned int length)
	{
		char* p = new char[length * sizeof(char) + 1];
		(void)::memcpy(p, path, length);
		p[length] = '\0';
		const bool r = ChangeCurrentDirectory(p, length);
		delete[] p;
		return r;
	}




} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni


namespace Yuni
{
namespace Core
{
namespace IO
{
namespace Directory
{
namespace Current
{

	String Get()
	{
		char* c = Yuni::Private::Core::IO::Directory::CurrentDirectory();
		String current = c;
		::free(c);
		return current;
	}


} // namespace Current
} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Yuni



