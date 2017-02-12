#ifndef __YUNI_CORE_SYSTEM_USERNAME_HXX__
# define __YUNI_CORE_SYSTEM_USERNAME_HXX__

# include <stdlib.h>
# include "../traits/cstring.h"


namespace Yuni
{
namespace Private
{
namespace System
{

	# ifdef YUNI_OS_WINDOWS
	/*!
	** \brief Retrieves the calling user's name into a mere C-String buffer (Windows only)
	*/
	unsigned int WindowsUsername(char* cstring, unsigned int size);
	# endif



} // namespace System
} // namespace Private
} // namespace Yuni



namespace Yuni
{
namespace System
{


	template<class StringT>
	# ifndef YUNI_OS_WINDOWS
	inline
	# endif
	bool Username(StringT& out, bool emptyBefore)
	{
		// Assert, if a C* container can not be found at compile time
		YUNI_STATIC_ASSERT(Traits::CString<StringT>::valid, SystemUsername_InvalidTypeForString);

		# ifdef YUNI_OS_WINDOWS
		if (emptyBefore)
			out.clear();

		// The maximum size for a username is 256 on any platform
		// We will reserve enough space for that size
		out.reserve(out.size() + 256 + 1 /* zero-terminated */);

		// The target buffer
		char* target = const_cast<char*>(out.c_str()) + out.size();
		// Since it may be any string (like a static one), we may have less than 256 chars
		const unsigned int size = out.capacity() - out.size();
		if (!size)
			return false; // not enough rooms
		// Appending the username to our buffer and retrieving the size of
		// the username
		const unsigned int written = Yuni::Private::System::WindowsUsername(target, size);
		if (written)
		{
			// The username has been written, we have to properly resize the string
			// (absolutely required for zero-terminated strings)
			out.resize(out.size() + written);
			return true;
		}
		return false;

		# else

		return System::Environment::Read("LOGNAME", out, emptyBefore);

		# endif
	}





} // namespace System
} // namespace Yuni

#endif // __YUNI_CORE_SYSTEM_USERNAME_HXX__
