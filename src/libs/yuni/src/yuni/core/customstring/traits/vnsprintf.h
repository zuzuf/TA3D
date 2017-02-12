#ifndef __YUNI_CORE_MEMORY_BUFFER_TRAITS_VSPRINTF_H__
# define __YUNI_CORE_MEMORY_BUFFER_TRAITS_VSPRINTF_H__

# include "../../traits/length.h"
# include "integer.h"



namespace Yuni
{
namespace Private
{
namespace CustomStringImpl
{


	/*!
	** \brief Generic implementation of vsnprintf
	**
	** \return Number of characters written, -1 if the string was truncated
	*/
	template<class C>
	int vnsprintf(C* buffer, size_t bufferSize, const C* format, va_list argptr);


	template<>
	inline int
	vnsprintf<char>(char* buffer, size_t bufferSize, const char* format, va_list argptr)
	{
		// Behavior: -1 when the string was truncated

		# if defined YUNI_OS_MSVC

		// From http://msdn.microsoft.com/en-us/library/d3xd30zz(VS.80).aspx
		//
		// If count is _TRUNCATE, then these functions write as much of the string as will
		// fit in buffer while leaving room for a terminating null. If the entire string
		// (with terminating null) fits in buffer, then these functions return the number
		// of characters written (not including the terminating null); otherwise, these
		// functions return -1 to indicate that truncation occurred.
		return ::_vsnprintf_s(buffer, bufferSize, _TRUNCATE, format, argptr);

		# else // YUNI_OS_MSVC

		// man 3 vnsprintf
		//
		// Upon  successful return, these functions return the number of characters printed
		// (not including the trailing ’\0’ used to end output to strings).
		// The functions snprintf() and vsnprintf() do not write more than size bytes
		// (including the trailing ’\0’).  If the output was truncated due to this limit then
		// the return value is the number of characters (not including the trailing ’\0’)
		// which would  have been  written  to  the  final  string if enough space had been
		// available. Thus, a return value of size or more means that the output was truncated.
		// (See also below under NOTES.)  If an output error is encountered, a negative
		// value is returned.
		int r = ::vsnprintf(buffer, bufferSize, format, argptr);
		return (((size_t) r >= bufferSize) ? -1 : r);

		# endif
	}




} // namespace CustomStringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_MEMORY_BUFFER_TRAITS_VSPRINTF_H__
