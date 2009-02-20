/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#ifndef __TA3D_XX__MATH_H__
# define __TA3D_XX__MATH_H__

# include <cmath>


namespace TA3D
{

/*!
** \brief Convenient routines for Mathematics calculations
*/
namespace Math
{


    template<typename T> inline T Max(const T a, const T b)
    { return (a > b) ? a : b; }

    template<typename T> inline T Min(const T a, const T b)
    { return (a > b) ? b : a; }


	inline bool IsPowerOfTwo(const int x)
    { return !(x & (x - 1)) && x; }

    template<typename T> inline T Deg2Rad(const T deg)
    { return (deg * 0.017453292f); }

    template<typename T> inline T Rad2Deg(const T rad)
    { return (rad * 57.29578122f); }


    //! \name Random table
    //@{

    /*!
    ** \brief Initialize the random table
    **
    ** The random table may be used to speed up some part of the code
    */
    void InitializeRandomTable();

    /*!
    ** \brief Get a random number from the random table
    */
    uint32 RandFromTable();

    /*!
    ** \brief Get log2(n)
    */
    uint32 Log2(uint32 n);

    //@}

} // namespace Math
} // namespace TA3D




//! Alias to the isnan() function
#define isNaN(x) std::isnan(x)


/*
** The below functions don't exists within windows math routines.
*/
#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC

	inline const real32 asinh(const real32 f)
	{
		return (real32)logf((real32)(f + sqrtf(f * f + 1)));
	}

	inline const real32 acosh(const real32 f)
	{
		return (real32)logf((real32)( f + sqrtf( f * f- 1)));
	}

	inline const real32 atanh(const real32 f)
	{
		return (real32)logf((real32)((real32)(1.0f / f + 1.0f) / (real32)(1.0f / f - 1.0f))) / 2.0f;
	}

#endif



// The rand() function on Windows platforms should be replaced by something
// that fits a 32bits integer (it would be slower of course)
# ifdef TA3D_PLATFORM_WINDOWS
#   define TA3D_RAND()	(rand() | (rand() << 16))
# else
#   define TA3D_RAND()	rand()
# endif




// Pi, our favorite number
# ifdef PI
#   undef PI
# endif
# define PI 3.141592653589793238462643383279502884197169399375105
# define DB_PI 6.28318530717958647693

// Square root of 2
# ifdef SQRT2
#   undef SQRT2
# endif
# define SQRT2 1.414213562373095048801688724209698078569671875376948

// Square root of 2 over 2
# ifdef SQRT2OVER2
#   undef SQRT2OVER2
# endif
# define SQRT2OVER2 0.707106781186547524400844362104849039284835937688474

// Square root of 3
# ifdef SQRT3
#   undef SQRT3
# endif
# define SQRT3 1.732050807568877293527446341505872366942805253810381

// Square root of 3 over 2
# ifdef SQRT3OVER2
#   undef SQRT3OVER2
# endif
# define SQRT3OVER2 0.866025403784438646763723170752936183471402626905190

// The natural number "e"
# ifdef NAT_E
#   undef NAT_E
# endif
# define NAT_E 2.718281828459045235360287471352662497757247093699959


# define DEG2RAD  (PI / 180.0f)
# define RAD2DEG  (180.0f / PI)

# define DEG2TA   (65536.0f / 360.0f)
# define TA2DEG   (360.0f / 65536.0f)

# define RAD2TA   (RAD2DEG * DEG2TA)
# define TA2RAD   (TA2DEG  * DEG2RAD)

# define I2PWR16  (1.0f / 65536.0f)




#endif // __TA3D_XX__MATH_H__
