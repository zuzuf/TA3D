
#include "../directory.h"
#include "commons.h"
#include "../../customstring.h"
#include "../file.h"

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

	bool WindowsMake(const char* path, unsigned int len)
	{
		if (len)
		{
			String norm;
			Yuni::Core::IO::Normalize(norm, path, len);

			wchar_t* buffer = new wchar_t[norm.size() + 10];
			buffer[0] = L'\\';
			buffer[1] = L'\\';
			buffer[2] = L'?';
			buffer[3] = L'\\';
			const int n = MultiByteToWideChar(CP_UTF8, 0, norm.c_str(), norm.size(), buffer + 4, norm.size());
			if (!n)
			{
				delete[] buffer;
				return false;
			}
			// The Win32 function MultiByteToWideChar() does not append the final zero
			// when a valid length is given (!= -1)
			buffer[n + 4] = L'\0';

			wchar_t* t = buffer + 4;

			while (*t != L'\0')
			{
				if ((*t == L'\\' || *t == L'/') && (*(t-1) != ':'))
				{
					*t = L'\0';
					if (!CreateDirectoryW(buffer, nullptr))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							delete[] buffer;
							return false;
						}
					}
					*t = L'\\';
				}
				++t;
			}

			if (!CreateDirectoryW(buffer, nullptr))
			{
				if (GetLastError() != ERROR_ALREADY_EXISTS)
				{
					delete[] buffer;
					return false;
				}
			}
			delete[] buffer;
		}
		return true;
	}

	# else

	bool UnixMake(const char* path, unsigned int len, unsigned int mode)
	{
		if (!len || !path || '\0' == *path)
			return true;

		char* buffer = new char[len + 1];
		memcpy(buffer, path, len);
		buffer[len] = '\0';
		char* pt = buffer;
		char tmp;

		while (1)
		{
			if ('\\' == *pt || '/' == *pt || '\0' == *pt)
			{
				tmp = *pt;
				*pt = '\0';
				if ('\0' != buffer[0] && '\0' != buffer[1] && '\0' != buffer[2])
				{
					if (mkdir(buffer, static_cast<mode_t>(mode)) < 0 && errno != EEXIST && errno != EISDIR && errno != ENOSYS)
					{
						delete[] buffer;
						return false;
					}
				}
				if ('\0' == tmp)
					break;
				*pt = tmp;
			}
			++pt;
		}
		delete[] buffer;
		return true;
	}

# endif




} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni

