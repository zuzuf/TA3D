#ifndef __YUNI_CORE_MATH_MATH_HXX__
# define __YUNI_CORE_MATH_MATH_HXX__

# include <algorithm>
# include <cmath>
# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <float.h>



# ifdef YUNI_OS_MSVC
/* Those functions are not available on Windows... */
double rint(double nr);
#	ifdef YUNI_HAS_LONG_DOUBLE
long double rintl(long double x);
#	endif
float rintf(float x);
long int lrint(double x);
#	ifdef YUNI_HAS_LONG_DOUBLE
long int lrintl(long double x);
#	endif
long int lrintf(float x);
long long int llrint(double x);
#	ifdef YUNI_HAS_LONG_DOUBLE
long long int llrintl(long double x);
#	endif
long long int llrintf(float x);
# endif



namespace Yuni
{
namespace Math
{


	template<class U, class V>
	inline U Max(U a, V b)
	{
		return (a < b) ? b : a;
	}


	template<class T>
	inline const T& Max(const T& a, const T& b, const T& c)
	{
		return Max<T,T>(Max<T,T>(a, b), c);
	}


	template<class U, class V>
	inline U Min(U a, V b)
	{
		return (a < b) ? a : b;
	}



	template<class T>
	inline const T& Min(const T& a, const T& b, const T& c)
	{
		return Min<T>(Min<T>(a, b), c);
	}


	template<class T>
	inline void Swap(T& a, T&b)
	{
		std::swap(a, b);
	}


	template<class T>
	inline const T& MinMax(const T& expr, const T& min, const T& max)
	{
		return ((expr < min)
			? min
			: ((expr > max) ? max : expr));
	}


	template<class T, class Expr, class MinT, class MaxT>
	inline T MinMaxEx(const Expr& expr, const MinT& min, const MaxT& max)
	{
		return ((expr < min)
			? static_cast<T>(min)
			: ((expr > max)
				? static_cast<T>(max)
				: static_cast<T>(expr)));
	}



	//! Factorial(1) = 1
	template <> struct Factorial<1> { enum { value = 1 }; };


	//! PowerInt<X,0>
	template <int X> struct PowerInt<X,0> { enum { value = 1 }; };

	// partial specialization to end the iteration
	template<int N> struct SquareRootInt<N,N> { enum { value = N }; };


	template <class U> inline bool Equals(U a, U b)
	{
		return (a == b);
	}


	template<> inline bool Equals<float>(float a, float b)
	{
		return ::fabsf(a - b) < YUNI_EPSILON;
	}

	template<> inline bool Equals<double>(double a, double b)
	{
		return ::fabs(a - b) < YUNI_EPSILON;
	}

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline bool Equals<long double>(long double a, long double b)
	{
		return ::fabsl(a - b) < YUNI_EPSILON;
	}
	# endif


	template <class U> inline bool Zero(U a)
	{
		return (0 == a);
	}

	template<> inline bool Zero<float>(float a)
	{
		return ::fabsf(a) < YUNI_EPSILON;
	}

	template<> inline bool Zero<double>(double a)
	{
		return ::fabs(a) < YUNI_EPSILON;
	}

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline bool Zero<long double>(long double a)
	{
		return ::fabsl(a) < YUNI_EPSILON;
	}
	# endif


	template<class U>
	inline U SquareRoot(const U x)
	{
		return (x < YUNI_EPSILON) ? U() : (U)::sqrt((double)x);
	}

	template<> inline double SquareRoot(const double x)
	{
		return (x < YUNI_EPSILON) ? 0. : ::sqrt(x);
	}

	template<> inline float SquareRoot(const float x)
	{
		return (x < YUNI_EPSILON) ? 0.f : ::sqrtf(x);
	}

	
	template<class U>
	inline U SquareRootNoCheck(const U x)
	{
		return (U)::sqrt((double)x);
	}

	template<> inline double SquareRootNoCheck(const double x)
	{
		return ::sqrt(x);
	}

	template<> inline float SquareRootNoCheck(const float x)
	{
		return ::sqrtf(x);
	}





	inline bool PowerOfTwo(const int x)
	{
		return !(x & (x - 1)) && x;
	}


	template<class T> inline T DegreeToRadian(const T x)
	{
		return (x * 0.017453292);
	}


	template<class T> inline T RadianToDegree(const T x)
	{
		return (x * 57.29578122);
	}


	template<class T> inline bool NaN(const T& x)
	{
		// According to the IEEE standard, NaN values have the odd property that
		// comparisons involving them are always false
		return x != x;
	}

	template<class T> inline int Infinite(const volatile T& x)
	{
		return ((x >= DBL_MAX) ? 1 : ((x <= -DBL_MAX) ? -1 : 0));
	}

	template<class T> inline T Floor(T x)
	{
		return x;
	}

	template<> inline float Floor(float x)
	{
		return ::floorf(x);
	}

	template<> inline double Floor(double x)
	{
		return ::floor(x);
	}

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline long double Floor<long double>(long double x)
	{
		return ::floorl(x);
	}
	# endif


	template<class T> inline T Ceil(T x)
	{
		return x;
	}

	template<> inline float Ceil(const float x)
	{
		return ::ceilf(x);
	}

	template<> inline double Ceil(const double x)
	{
		return ::ceil(x);
	}

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline long double Ceil<long double>(const long double x)
	{
		return ::ceill(x);
	}
	# endif


	template<class T> inline T Fract(T x)
	{
		return x - Floor(x);
	}


	inline float Power(const float x, const float y)
	{
		return ::powf(x, y);
	}

	inline double Power(const double x, const double y)
	{
		return ::pow(x, y);
	}


	template<class T> inline T Round(T x, unsigned int)
	{
		return x;
	}


	template<> inline double Round<double>(double x, unsigned int place)
	{
		if (place)
		{
			double temp, mult;
			mult = Power(10., place);
			temp = Floor(x * mult + 0.5);
			temp = temp / mult;
			return temp;
		}
		else
		{
			# ifdef YUNI_OS_MSVC
			return (x < 0.) ? (::ceil(x - 0.5)) : (::floor(x + 0.5));
			# else
			return ::round(x);
			# endif
		}
	}

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline long double Round<long double>(long double x, unsigned int place)
	{
		if (place)
		{
			long double temp, mult;
			mult = Power(static_cast<long double>(10.), place);
			temp = Floor(x * mult + 0.5);
			temp = temp / mult;
			return temp;
		}
		else
		{
			# ifdef YUNI_OS_MSVC
			return (x < 0.) ? (::ceill(x - 0.5L)) : (::floorl(x + 0.5L));
			# else
			return ::roundl(x);
			# endif
		}
	}
	# endif

	template<> inline float Round<float>(float x, unsigned int place)
	{
		if (place)
		{
			float temp, mult;
			mult = Power(10.f, static_cast<float>(place));
			temp = Floor(x * mult + 0.5f);
			temp = temp / mult;
			return temp;
		}
		else
		{
			# ifdef YUNI_OS_MSVC
			return (x < 0.) ? (::ceilf(x - 0.5f)) : (::floorf(x + 0.5f));
			# else
			return ::roundf(x);
			# endif
		}
	}




	template<class T> inline T Trunc(T x, unsigned int)
	{
		return x;
	}

	template<> inline double Trunc<double>(double x, unsigned int place)
	{
		if (place)
		{
			double temp, mult;
			mult = Power(10.0, place);
			temp = Floor(x * mult);
			temp = temp / mult;
			return temp;
		}
		else
		{
			# ifndef YUNI_OS_MSVC
			return ::trunc(x);
			# else
			return ((x > 0.) ? ::floor(x) : ::ceil(x));
			# endif
		}
	}

	template<> inline float Trunc<float>(float x, unsigned int place)
	{
		if (place)
		{
			float temp, mult;
			mult = Power(10.f, static_cast<float>(place));
			temp = Floor(x * mult);
			temp = temp / mult;
			return temp;
		}
		else
		{
			# ifndef YUNI_OS_MSVC
			return ::truncf(x);
			# else
			return ((x > 0.) ? ::floorf(x) : ::ceilf(x));
			# endif
		}
	}


	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline long double Trunc<long double>(long double x, unsigned int place)
	{
		if (place)
		{
			long double temp, mult;
			mult = Power(10.0, place);
			temp = Floor(x * mult);
			temp = temp / mult;
			return temp;
		}
		else
		{
			# ifndef YUNI_OS_MSVC
			return ::trunc(x);
			# else
			return ((x > 0.) ? ::floor(x) : ::ceil(x));
			# endif
		}
	}
	# endif




	template<class T, class R>
	struct RoundToInt
	{
		typedef T Type;
		typedef R ResultType;
		static inline ResultType Value(Type x)
		{
			// Default Behavior
			return (ResultType)(Round<Type>(x));
		}
	};

	template<class T>
	struct RoundToInt<T,T>
	{
		typedef T Type;
		typedef T ResultType;
		static inline ResultType Value(Type x)
		{
			// Same type nothing to do
			return x;
		}
	};


	template<>
	struct RoundToInt<float, double>
	{
		typedef float Type;
		typedef double ResultType;
		static inline ResultType Value(Type x) { return (ResultType)x; }
	};

	template<>
	struct RoundToInt<double, float>
	{
		typedef double Type;
		typedef float ResultType;
		static inline ResultType Value(Type x) { return (ResultType)x; }
	};



	template<>
	struct RoundToInt<float, long int>
	{
		typedef float Type;
		typedef long int ResultType;
		static inline ResultType Value(Type x) { return ::lrintf(x); }
	};

	template<>
	struct RoundToInt<double, long int>
	{
		typedef double Type;
		typedef long int ResultType;
		static inline ResultType Value(Type x) { return ::lrint(x); }
	};


	# ifdef YUNI_HAS_LONG_DOUBLE
	template<>
	struct RoundToInt<long double, long int>
	{
		typedef long double Type;
		typedef long int ResultType;
		static inline ResultType Value(Type x) { return ::lrintl(x); }
	};
	# endif



	template<>
	struct RoundToInt<float, long long int>
	{
		typedef float Type;
		typedef long long int ResultType;
		static inline ResultType Value(Type x) { return ::llrintf(x); }
	};

	template<>
	struct RoundToInt<double, long long int>
	{
		typedef double Type;
		typedef long long int ResultType;
		static inline ResultType Value(Type x) { return ::llrint(x); }
	};

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<>
	struct RoundToInt<long double, long long int>
	{
		typedef long double Type;
		typedef long long int ResultType;
		static inline ResultType Value(Type x) { return ::llrintl(x); }
	};
	# endif

	template<class T> inline T Abs(const T x)
	{
		return ::abs(x);
	}

	template<> inline long Abs<long>(const long x)
	{
		return ::labs(x);
	}

	template<> inline long long Abs<long long>(const long long x)
	{
	# ifdef YUNI_OS_MSVC
		return ::_abs64(x);
	# else
		return ::llabs(x);
	# endif
	}

	template<> inline double Abs<double>(const double x)
	{
		return ::fabs(x);
	}

	template<> inline float Abs<float>(const float x)
	{
		return ::fabsf(x);
	}

	# ifdef YUNI_HAS_LONG_DOUBLE
	template<> inline long double Abs<long double>(const long double x)
	{
		return ::fabsl(x);
	}
	# endif




} // namespace Math
} // namespace Yuni


# ifdef YUNI_OS_MSVC
#	include "msvc.hxx"
# endif

#endif // __YUNI_CORE_MATH_MATH_HXX__
