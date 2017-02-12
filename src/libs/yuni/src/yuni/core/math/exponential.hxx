#ifndef __YUNI_CORE_MATH_EXPONENTIAL_HXX__
# define __YUNI_CORE_MATH_EXPONENTIAL_HXX__



namespace Yuni
{
namespace Math
{

	template<typename T> inline T Exp(T x)
	{
		return (T)::exp((T)x);
	}

	template<> inline float Exp(float x)
	{
		return ::expf(x);
	}

	template<> inline double Exp(double x)
	{
		return ::exp(x);
	}

	template<> inline long double Exp<long double>(long double x)
	{
		return ::expl(x);
	}





} // namespace Math
} // namespace Yuni

#endif // __YUNI_CORE_MATH_EXPONENTIAL_HXX__
