
#ifndef __YUNI_CORE_MATH_GEOMETRY_H__
# define __YUNI_CORE_MATH_GEOMETRY_H__

# include "point3D.h"
# include "vector3D.h"

namespace Yuni
{
namespace Geometry
{

	/*!
	** \brief Intersection point of a line and a plane
	**
	** \param linePoint Any point on the line
	** \param lineVector Direction vector of the line
	** \param planePoint Any point in the plane
	** \param planeNormal Normal vector of the plane
	*/
	static Point3D<T> LinePlaneIntersection(const Point3D<T>& linePoint,
		const Vector3D& lineDirection, const Point3D<T>& planePoint, const Vector3D& planeNormal);


} // namespace Geometry
} // namespace Yuni

# include "geometry.hxx"

#endif // __YUNI_CORE_MATH_GEOMETRY_H__
