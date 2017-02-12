#ifndef __YUNI_CORE_CUSTOMSTRING_TRAITS_FILL_H__
# define __YUNI_CORE_CUSTOMSTRING_TRAITS_FILL_H__

# include "../../traits/length.h"


namespace Yuni
{
namespace Extension
{
namespace CustomString
{


	template<class CustomStringT, class StringT>
	class Fill
	{
	public:
		static void Perform(char* data, typename CustomStringT::Size size, const StringT& pattern)
		{
			const unsigned int patternSize = Traits::Length<StringT,unsigned int>::Value(pattern);
			const char* const cstr = Traits::CString<StringT>::Perform(pattern);

			if (patternSize == 0)
				return;
			// If equals to 1, it is merely a single char
			if (patternSize == 1)
			{
				for (typename CustomStringT::Size i = 0; i < size; ++i)
					data[i] = *cstr;
				return;
			}
			// We have to copy N times the pattern
			typename CustomStringT::Size p = 0;
			while (p + patternSize <= size)
			{
				(void)::memcpy(data + p, cstr, patternSize * sizeof(char));
				p += patternSize;
			}
			for (; p < size; ++p)
				data[p] = ' ';
		}
	};


	template<class CustomStringT>
	class Fill<CustomStringT, char>
	{
	public:
		static void Perform(char* data, typename CustomStringT::Size size, const char rhs)
		{
			for (typename CustomStringT::Size i = 0; i != size; ++i)
				data[i] = rhs;
		}
	};




} // namespace CustomString
} // namespace Extension
} // namespace Yuni

#endif // __YUNI_CORE_CUSTOMSTRING_TRAITS_FILL_H__
