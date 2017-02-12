#ifndef __YUNI_CORE_MATH_DISTANCE_H__
# define __YUNI_CORE_MATH_DISTANCE_H__

# include "math.h"


namespace Yuni
{
namespace Math
{


	/*!
	** \brief Compute the distance between two points (2D)
	*/
	template<typename T> inline T Distance2D(T x1, T y1, T x2, T y2);

	/*!
	** \brief Compute the distance between two points (3D)
	*/
	template<typename T> inline T Distance3D(T x1, T y1, T z1, T x2, T y2, T z2);




} // namespace Math
} // namespace Yuni

# include "distance.hxx"

#endif // __YUNI_CORE_MATH_DISTANCE_H__
