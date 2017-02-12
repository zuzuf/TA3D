
#include "../../yuni.h"

#ifdef YUNI_OS_WINDOWS
# include "username.h"
# include "windows.hdr.h"


namespace Yuni
{
namespace Private
{
namespace System
{

	unsigned int WindowsUsername(char* cstring, unsigned int size)
	{
		enum
		{
			// The maximum length, (see UCLEN)
			defaultSize = 256,
		};
		DWORD unwsize = defaultSize;

		wchar_t unw[defaultSize];
		if (GetUserNameW(unw, &unwsize))
		{
			if (unwsize > 0)
			{
				// The variable `unwsize` contains the final zero
				--unwsize;
				// Getting the size of the buffer into UTF8
				int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, unw, unwsize, NULL, 0,  NULL, NULL);
				if (sizeRequired > 0)
				{
					if (static_cast<unsigned int>(sizeRequired) > size)
						sizeRequired = size;
					WideCharToMultiByte(CP_UTF8, 0, unw, unwsize, cstring, sizeRequired,  NULL, NULL);
					return static_cast<unsigned int>(sizeRequired);
				}
			}
		}
		return 0;
	}




} // namespace System
} // namespace Private
} // namespace Yuni

#endif
