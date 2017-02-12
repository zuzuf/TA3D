#ifndef __YUNI_CORE_MATH_MSVC_HXX__
# define __YUNI_CORE_MATH_MSVC_HXX__


# ifdef YUNI_OS_MSVC

/* Those functions are not available on Windows... */

inline double rint(double nr)
{
	const double f = Yuni::Math::Floor(nr);
	const double c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? f : c);
}


# ifdef YUNI_HAS_LONG_DOUBLE
inline long double rintl(long double nr)
{
	const long double f = Yuni::Math::Floor(nr);
	const long double c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? f : c);
}
# endif


inline float rintf(float nr)
{
	const float f = Yuni::Math::Floor(nr);
	const float c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? f : c);
}


inline long int lrint(double nr)
{
	const double f = Yuni::Math::Floor(nr);
	const double c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? (long int)f : (long int)c);
}


# ifdef YUNI_HAS_LONG_DOUBLE
inline long int lrintl(long double nr)
{
	const long double f = Yuni::Math::Floor(nr);
	const long double c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? (long int)f : (long int)c);
}
# endif

inline long int lrintf(float nr)
{
	const float f = Yuni::Math::Floor(nr);
	const float c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? (long int)f : (long int)c);
}


inline long long int llrint(double nr)
{
	const double f = Yuni::Math::Floor(nr);
	const double c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? (long long int)f : (long long int)c);
}


# ifdef YUNI_HAS_LONG_DOUBLE
inline long long int llrintl(long double nr)
{
	const long double f = Yuni::Math::Floor(nr);
	const long double c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? (long long int)f : (long long int)c);
}
# endif


inline long long int llrintf(float nr)
{
	const float f = Yuni::Math::Floor(nr);
	const float c = Yuni::Math::Ceil(nr);
	return (((c -nr) >= (nr - f)) ? (long long int)f : (long long int)c);
}

# endif /* ifdef MSVC */


#endif // __YUNI_CORE_MATH_MSVC_HXX__
