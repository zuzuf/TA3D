#ifndef __YUNI_CORE_MATH_GEOMETRY_BOUNDINGBOX_HXX__
# define __YUNI_CORE_MATH_GEOMETRY_BOUNDINGBOX_HXX__


namespace Yuni
{


	template<typename T>
	inline
	BoundingBox<T>::BoundingBox()
		:pMin(), pMax(), pCenter()
	{}


	template<typename T>
	inline
	BoundingBox<T>::BoundingBox(const Point3D<T>& min, const Point3D<T>& max)
		:pMin(min), pMax(max),
		pCenter((min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2)
	{}


	template<typename T>
	void BoundingBox<T>::addPoint(const Point3D<T>& point)
	{
		// Update the minimum
		if (point.x < pMin.x)
			pMin.x = point.x;
		if (point.y < pMin.y)
			pMin.y = point.y;
		if (point.z < pMin.z)
			pMin.z = point.z;
		// Update the maximum
		if (point.x > pMax.x)
			pMax.x = point.x;
		if (point.y > pMax.y)
			pMax.y = point.y;
		if (point.z > pMax.z)
			pMax.z = point.z;
	}


	template<typename T>
	inline void BoundingBox<T>::addTriangle(const Triangle& tri)
	{
		addPoint(tri.vertex1());
		addPoint(tri.vertex2());
		addPoint(tri.vertex3());
	}


	template<typename T>
	inline bool BoundingBox<T>::contains(const Point3D<T>& point) const
	{
		if (pMin.x > point.x || pMax.x < point.x)
			return false;
		if (pMin.y > point.y || pMax.y < point.y)
			return false;
		if (pMin.z > point.z || pMax.z < point.z)
			return false;
		return true;
	}

	template<typename T>
	template<typename U>
	inline void BoundingBox<T>::reset(Point3D<U> newCenter)
	{
		pCenter.move(newCenter);
		pMin.move(pCenter);
		pMax.move(pCenter);
	}


} // namespace Yuni

#endif // __YUNI_CORE_MATH_GEOMETRY_BOUNDINGBOX_HXX__
