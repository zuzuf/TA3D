#ifndef __YUNI_CORE_MATH_LOG_HXX__
# define __YUNI_CORE_MATH_LOG_HXX__

# include <cassert>


namespace Yuni
{
namespace Math
{



	template<typename T> inline T Log(T x)
	{
		return ::log(x);
	}

	template<> inline float Log<float>(float x)
	{
		return ::logf(x);
	}

	template<> inline long double Log<long double>(long double x)
	{
		return ::logl(x);
	}


	template<typename T> inline T Log2(T x)
	{
		# if defined(YUNI_OS_MSVC)
		return static_cast<T>(log(x) / YUNI_LOG_2);
		# else
		return ::log2(x);
		# endif
	}

	template<> inline float Log2<float>(float x)
	{
		# if defined(YUNI_OS_MSVC)
		return static_cast<float>(logf(x) / YUNI_LOG_2);
		# else
		return ::log2f(x);
		# endif

	}

	template<> inline long double Log2<long double>(long double x)
	{
		# if defined(YUNI_OS_MSVC)
		return static_cast<long double>(logl(x) / YUNI_LOG_2);
		# else
		return ::log2l(x);
		# endif
	}


	template<typename T> inline T LogOnePlusX(T x)
	{
		/* Assert */
		assert(x > -1. && "x must be greater than -1.0");

		return (Abs(x) >= 1e-4)
			// Direct calculation for larger values
			? Log(1. + x)
			// Taylor approx. log(1 + x) = x - x^2/2! + x^3/3!...
			: (-0.5 * x + 1.) * x;
	}






} // namespace Math
} // namespace Yuni


#endif // __YUNI_CORE_MATH_LOG_HXX__
