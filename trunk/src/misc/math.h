#ifndef __TA3D_XX__MATH_H__
# define __TA3D_XX__MATH_H__


# include <math.h>


namespace TA3D
{

/*!
** \brief Convenient routines for Mathematics calculations
*/
namespace Math
{


    template<class T> inline T Max(T a, T b)
    { return (a > b) ? a : b; }

    template<class T> inline T Min(T a, T b)
    { return (a > b) ? b : a; }


	inline bool IsPowerOfTwo(const int x)
    { return !(x & (x - 1)) && x; }


} // namespace Math
} // namespace TA3D




//! Alias to the isnan() function
#define isNaN(x) isnan(x)


/*
** The below functions don't exists within windows math routines.
*/
#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC

	static inline const real32 asinh(const real32 f)
	{
		return (real32)log((real32)(f + sqrt(f * f + 1)));
	}

	static inline const real32 acosh(const real32 f)
	{
		return (real32)log((real32)( f + sqrt( f * f- 1)));
	}

	static inline const real32 atanh(const real32 f)
	{
		return (real32)log((real32)((real32)(1.0f / f + 1.0f) / (real32)(1.0f / f - 1.0f))) / 2.0f;
	}

#endif



// The rand() function on Windows platforms should be replaced by something
// that fits a 32bits integer (it would be slower of course)
# ifdef TA3D_PLATFORM_WINDOWS
#   define TA3D_RAND()	(rand() | (rand() << 16))
# else
#   define TA3D_RAND()	rand()
# endif



#endif // __TA3D_XX__MATH_H__
