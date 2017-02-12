#ifndef __YUNI_CORE_STRING_STRING_TRAITS_CSTRING_EXTRACTOR_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_CSTRING_EXTRACTOR_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{


	template<typename C>
	struct CString<C, C>
	{
		static const C* Extractor(const C* u)
		{
			return u;
		}
	};

	template<typename C, int N>
	struct CString<C, C[N]>
	{
		static const C* Extractor(const C u[N])
		{
			return u;
		}
	};

	template<typename C, int N>
	struct CString<C, StringBase<C,N> >
	{
		static const C* Extractor(const StringBase<C,N>& u)
		{
			return u.c_str();
		}
	};


	template<typename C>
	struct CString<C, std::basic_string<C> >
	{
		static const C* Extractor(const std::string& u)
		{
			return u.c_str();
		}
	};



} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_CSTRING_EXTRACTOR_HXX__
