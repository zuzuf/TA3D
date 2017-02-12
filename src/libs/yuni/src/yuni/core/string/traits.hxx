#ifndef __YUNI_CORE_STRING_STRING_TRAITS_TRAITS_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_TRAITS_HXX__

# include <stdarg.h>
# include <stdio.h>
# include "../static/assert.h"


namespace Yuni
{
namespace Private
{
namespace StringImpl
{


	template<typename C, typename U>
	struct CString
	{
		static const C* Extractor(const U& u);
	};



	template<typename C, int K>
	struct Impl
	{

		static inline C ToLower(const C a)
		{
			return (C) std::tolower(a);
		}


		static int Compare(const C* a, const C* b, const typename StringBase<C,K>::Size maxLen)
		{
			typename StringBase<C,K>::Size p(0);
			for (; p < maxLen && 0 != *a && 0 != *b; ++a, ++b, ++p)
			{
				if (*a != *b)
					return (*a < *b) ? -1 : 1;
			}
			return (*a == *b) ? 0 : ((*a < *b) ? -1 : 1);
		}

		static int CompareInsensitive(const C* a, const C* b, const typename StringBase<C,K>::Size maxLen)
		{
			C s = 0;
			C t = 0;
			for (typename StringBase<C,K>::Size p = 0; p < maxLen && 0 != *a && 0 != *b; ++a, ++b, ++p)
			{
				if ((s = ToLower(*a)) != (t = ToLower(*b)))
					return (int)(s - t);
			}
			return (s == t) ? 0 : ((int)(s - t));
		}


		static bool StrictlyEquals(const C* a, const C* b)
		{
			if (a && b)
			{
				while (1)
				{
					if (*a != *b)
						return false;
					if ('\0' == *a)
						return true;
					++a;
					++b;
				}
				return true;
			}
			return (b && '\0' == *b && a && '\0' == *a);
		}


		static inline bool StrictlyEquals(const C* a, const C* b, const typename StringBase<C,K>::Size maxLen)
		{
			return !memcmp(a, b, sizeof(C) * maxLen);
		}

		static bool Equals(const C* a, const C* b, const typename StringBase<C,K>::Size maxLen)
		{
			if (maxLen && a && b)
			{
				typename StringBase<C,K>::Size pos(0);
				while (pos < maxLen)
				{
					if (*a != *b)
						return false;
					if ('\0' == *a)
						return true;
					++a;
					++b;
				}
				return true;
			}
			return (b && '\0' == *b && a && '\0' == *a);
		}

	};


	template<int K1>
	struct Impl<char,K1>
	{
		static inline int Compare(const char* a, const char* b, const typename StringBase<char,K1>::Size maxLen)
		{
			return strncmp(a, b, maxLen);
		}

		static inline char ToLower(const char a)
		{
			return (char) ::tolower(a);
		}



		static int CompareInsensitive(const char* a, const char* b, const typename StringBase<char,K1>::Size maxLen)
		{
			char s = 0;
			char t = 0;
			for (typename StringBase<char,K1>::Size p = 0; p < maxLen && 0 != *a && 0 != *b; ++a, ++b, ++p)
			{
				if ((s = ToLower(*a)) != (t = ToLower(*b)))
					return (int)(s - t);
			}
			return (s == t) ? 0 : ((int)(s - t));
		}


		static bool StrictlyEquals(const char* a, const char* b)
		{
			if (a && b)
			{
				while (1)
				{
					if (*a != *b)
						return false;
					if ('\0' == *a)
						return true;
					++a;
					++b;
				}
				return true;
			}
			return (b && '\0' == *b && a && '\0' == *a);
		}

		static inline bool StrictlyEquals(const char* a, const char* b, const typename StringBase<char,K1>::Size maxLen)
		{
			return !memcmp(a, b, sizeof(char) * maxLen);
		}

		static bool Equals(const char* a, const char* b, const typename StringBase<char,K1>::Size maxLen)
		{
			if (maxLen && a && b)
			{
				typename StringBase<char,K1>::Size pos(0);
				while (pos < maxLen)
				{
					if (*a != *b)
						return false;
					if ('\0' == *a)
						return true;
					++a;
					++b;
				}
				return true;
			}
			return (b && '\0' == *b && a && '\0' == *a);
		}

	};



} // namespace StringImpl
} // namespace Private
} // namespace Yuni



# include "traits/vsprintf.hxx"
# include "traits/length.hxx"
# include "traits/countchar.hxx"
# include "traits/haschar.hxx"
# include "traits/find.hxx"
# include "traits/remove.hxx"
# include "traits/findlastof.hxx"
# include "traits/findfirstof.hxx"
# include "traits/cstring.hxx"


#endif // __YUNI_CORE_STRING_STRING_TRAITS_TRAITS_HXX__
