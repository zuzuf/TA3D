#ifndef __YUNI_CORE_STRING_STRING_TRAITS_COUNT_CHAR_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_COUNT_CHAR_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{



	template<class StrBase1, typename C1>
	struct CountChar<StrBase1, C1*>
	{
		static typename StrBase1::Size Value(const C1* str, const C1 c)
		{
			if (str)
			{
				typename StrBase1::Size n(0);
				for (const C1* i = str; '\0' != *i; ++i)
				{
					if (c == *i)
						++n;
				}
				return n;
			}
			return 0;
		}
	};


	template<class StrBase1, class C1, int N>
	struct CountChar<StrBase1, C1[N]>
	{
		static typename StrBase1::Size Value(const C1* str, const C1 c)
		{
			typename StrBase1::Size n(0);
			for (const C1* i = str; '\0' != *i; ++i)
			{
				if (c == *i)
					++n;
			}
			return n;
		}
	};



	template<class StrBase1, class C1,int Chnk1>
	struct CountChar<StrBase1, StringBase<C1,Chnk1> >
	{
		static typename StrBase1::Size Value(const StringBase<C1,Chnk1>& str, const C1 c)
		{
			if (str.pSize)
			{
				typename StrBase1::Size n(0);
				for (typename StrBase1::Size pos = 0; pos != str.pSize; ++pos)
				{
					if (c == str.pPtr[pos])
						++n;
				}
				return n;
			}
			return 0;
		}
	};


	template<class StrBase1, class C1,int Chnk1>
	struct CountChar<StrBase1, StringBase<C1,Chnk1>* >
	{
		static typename StrBase1::Size Value(const StringBase<C1,Chnk1>* str, const C1 c)
		{
			return (str && str->pSize)
				? CountChar<StrBase1, StringBase<C1, Chnk1> >::Value(*str, c)
				: 0;
		}
	};



	template<class StrBase1, class C1>
	struct CountChar<StrBase1, std::basic_string<C1> >
	{
		static typename StrBase1::Size Value(const std::basic_string<C1>& str, const C1 c)
		{
			if (!str.empty())
			{
				typename StrBase1::Size n(0);
				typename std::basic_string<C1>::const_iterator end = str.end();
				for (typename std::basic_string<C1>::const_iterator i = str.begin(); i != end; ++i)
				{
					if (c == *i)
						++n;
				}
				return n;
			}
			return 0;
		}
	};


	template<class StrBase1, class C1>
	struct CountChar<StrBase1, std::basic_string<C1>* >
	{
		static inline typename StrBase1::Size Value(const std::basic_string<C1>* str, const C1 c)
		{
			return (str && !str->empty())
				? CountChar<StrBase1, std::basic_string<C1> >::Value(*str, c)
				: 0;
		}
	};



	template<class StrBase1>
	struct CountChar<StrBase1, char>
	{
		static inline typename StrBase1::Size Value(const char str, const char c)
		{
			return (c == str) ? 1 : 0;
		}
	};


	template<class StrBase1>
	struct CountChar<StrBase1, wchar_t>
	{
		static inline typename StrBase1::Size Value(const wchar_t str, const wchar_t c)
		{
			return (c == str) ? 1 : 0;
		}
	};



} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_COUNT_CHAR_HXX__
