#ifndef __YUNI_CORE_MATH_COMPLEX_HXX__
# define __YUNI_CORE_MATH_COMPLEX_HXX__


namespace Yuni
{
namespace Private
{
namespace IStringImpl
{


	// std::complex
	template<class MemBufT, class U>
	struct Append<MemBufT, std::complex<U> >
	{
		static void Do(MemBufT& memoryBuffer, const std::complex<U>& rhs)
		{
			memoryBuffer << rhs.real() << ',' << rhs.imag();
		}
	};




} // namespace IStringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_MATH_COMPLEX_HXX__
