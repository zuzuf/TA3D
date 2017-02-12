
#include "openmode.h"


namespace Yuni
{
namespace Core
{
namespace IO
{
namespace OpenMode
{


	const char* ToCString(const int mode)
	{
		if (mode == OpenMode::append)
			// Shortcut for write|append
			return "ab";
		if (mode == OpenMode::truncate)
			// Shortcut for write|append
			return "wb";

		if (mode & OpenMode::write)
		{
			if (0 == (mode & OpenMode::read))
			{
				if (mode & OpenMode::truncate)
					return "wb";
				if (mode & OpenMode::append)
					return "ab";
			}
			else
			{
				if (mode & OpenMode::truncate)
					return "w+b";
				if (mode & OpenMode::append)
					return "a+b";
				return "r+b";
			}
			return "wb";
		}
		return (mode & OpenMode::read) ? "rb" : "";
	}



	const wchar_t*  ToWCString(const int mode)
	{
		if (mode == OpenMode::append)
			// Shortcut for write|append
			return L"ab";
		if (mode == OpenMode::truncate)
			// Shortcut for write|append
			return L"wb";

		if (mode & OpenMode::write)
		{
			if (!(mode & OpenMode::read))
			{
				if (mode & OpenMode::truncate)
					return L"wb";
				if (mode & OpenMode::append)
					return L"ab";
			}
			else
			{
				if (mode & OpenMode::truncate)
					return L"w+b";
				if (mode & OpenMode::append)
					return L"a+b";
				return L"r+b";
			}
			return L"wb";
		}
		return (mode & OpenMode::read) ? L"rb" : L"";
	}





} // namespace OpenMode
} // namespace IO
} // namespace Core
} // namespace Yuni

