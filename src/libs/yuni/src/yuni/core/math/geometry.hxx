
#include <cassert>

namespace Yuni
{
namespace Geometry
{


	template<typename T>
	inline Point3D<T> Vector3D<T>::IntersectionLinePlane(const Point3D<T>& linePoint, const Vector3D<T>& lineDirection,
		const Point3D<T>& planePoint, const Vector3D<T>& planeNormal)
	{
		float dotProduct = DotProduct(planeNormal, lineDirection);
		assert(Math::Abs(dotProduct) < YUNI_EPSILON);
		// Vector connecting the two origin points from line to plane
		Vector3D<T> lineToPlane(planePoint.x - linePoint.x, planePoint.y - linePoint.y, planePoint.z - linePoint.z);
		float factor = DotProduct(lineToPlane, planeNormal) / dotProduct;
		// Scale the direction by the found value
		Vector3D<T> direction(lineDirection);
		direction *= factor;
		// Move the line point along the line to the intersection
		Point3D<T> result(linePoint);
		result.translate(direction.x, direction.y, direction.z);
		return result;
	}



} // namespace Geometry
} // namespace Yuni
