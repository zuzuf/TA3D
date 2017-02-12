#ifndef __YUNI_CORE_STRING_STRING_TRAITS_HAS_CHAR_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_HAS_CHAR_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{



	template<class StrBase1, typename C1>
	struct HasChar<StrBase1, C1*>
	{
		static bool Value(const C1* str, const C1 c)
		{
			if (str)
			{
				for (const C1* i = str; '\0' != *i; ++i)
				{
					if (c == *i)
						return true;
				}
			}
			return false;
		}
	};


	template<class StrBase1, class C1, int N>
	struct HasChar<StrBase1, C1[N]>
	{
		static bool Value(const C1* str, const C1 c)
		{
			for (const C1* i = str; '\0' != *i; ++i)
			{
				if (c == *i)
					return true;
			}
			return false;
		}
	};


	template<class StrBase1, class C1,int Chnk1>
	struct HasChar<StrBase1, StringBase<C1,Chnk1> >
	{
		static inline bool Value(const StringBase<C1,Chnk1>& str, const C1 c)
		{
			return str.hasChar(c);
		}
	};


	template<class StrBase1, class C1,int Chnk1>
	struct HasChar<StrBase1, StringBase<C1,Chnk1>* >
	{
		static inline bool Value(const StringBase<C1,Chnk1>* str, const C1 c)
		{
			return str && str->hasChar(c);
		}
	};



	template<class StrBase1, class C1>
	struct HasChar<StrBase1, std::basic_string<C1> >
	{
		static inline bool Value(const std::basic_string<C1>& str, const C1 c)
		{
			return std::basic_string<C1>::npos != str.find(c);
		}
	};


	template<class StrBase1, class C1>
	struct HasChar<StrBase1, std::basic_string<C1>* >
	{
		static inline bool Value(const std::basic_string<C1>* str, const C1 c)
		{
			return str && std::basic_string<C1>::npos != str->find(c);
		}
	};




	template<class StrBase1>
	struct HasChar<StrBase1, char>
	{
		static inline bool Value(const char str, const char c)
		{
			return (c == str);
		}
	};


	template<class StrBase1>
	struct HasChar<StrBase1, wchar_t>
	{
		static inline bool Value(const wchar_t str, const wchar_t c)
		{
			return (c == str);
		}
	};



} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_HAS_CHAR_HXX__
