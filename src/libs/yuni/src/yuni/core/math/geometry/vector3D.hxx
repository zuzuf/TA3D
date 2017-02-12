#ifndef __YUNI_CORE_MATH_GEOMETRY_VECTOR3D_HXX__
# define __YUNI_CORE_MATH_GEOMETRY_VECTOR3D_HXX__


namespace Yuni
{


	template<typename T>
	inline Vector3D<T>
	Vector3D<T>::Mean(const Vector3D<T>& p1, const Vector3D<T>& p2)
	{
		return Vector3D<T>().mean(p1, p2);
	}


	template<typename T>
	inline Vector3D<T>::Vector3D()
		:x(), y(), z()
	{}

	template<typename T>
	inline Vector3D<T>::Vector3D(const Vector3D<T>& rhs)
		:x(rhs.x), y(rhs.y), z(rhs.z)
	{}


	template<typename T>
	template<typename U>
	inline Vector3D<T>::Vector3D(const Vector3D<U>& rhs)
		:x(static_cast<T>(rhs.x)),
		y(static_cast<T>(rhs.y)),
		z(static_cast<T>(rhs.z))
	{}


	template<typename T>
	template<typename U, typename V, typename W>
	inline Vector3D<T>::Vector3D(const U x1, const V y1, const W z1)
		:x(static_cast<T>(x1)),
		y(static_cast<T>(y1)),
		z(static_cast<T>(z1))
	{}

	template<typename T>
	template<typename U, typename V>
	inline Vector3D<T>::Vector3D(const Point3D<U>& origin, const Point3D<V>& end)
		:x(static_cast<T>((end.x - origin.x))),
		y(static_cast<T>((end.y - origin.y))),
		z(static_cast<T>((end.z - origin.z)))
	{}

	template<typename T>
	inline Vector3D<T>& Vector3D<T>::reset()
	{
		x = T();
		y = T();
		z = T();
		return *this;
	}


	template<typename T>
	template<typename U>
	inline void Vector3D<T>::translate(const U v)
	{
		x += static_cast<T>(v);
		y += static_cast<T>(v);
		z += static_cast<T>(v);
	}


	template<typename T>
	template<typename U, typename V, typename W>
	inline void Vector3D<T>::translate(const U x1, const V y1, const W z1)
	{
		x += static_cast<T>(x1);
		y += static_cast<T>(y1);
		z += static_cast<T>(z1);
	}

	template<typename T>
	template<typename U>
	inline void Vector3D<T>::translate(const Vector3D<U>& v)
	{
		x += static_cast<T>(v.x);
		y += static_cast<T>(v.y);
		z += static_cast<T>(v.z);
	}


	template<typename T>
	template<typename U>
	inline Vector3D<T>& Vector3D<T>::mean(const Vector3D<U>& p)
	{
		x = static_cast<T>((x + static_cast<T>(p.x)) / 2.);
		y = static_cast<T>((y + static_cast<T>(p.y)) / 2.);
		z = static_cast<T>((z + static_cast<T>(p.z)) / 2.);
		return *this;
	}


	template<typename T>
	template<typename U, typename V>
	inline Vector3D<T>& Vector3D<T>::mean(const Vector3D<U>& p1, const Vector3D<V>& p2)
	{
		x = static_cast<T>(((p1.x + p2.x) / 2.0f));
		y = static_cast<T>(((p1.y + p2.y) / 2.0f));
		z = static_cast<T>(((p1.z + p2.z) / 2.0f));
		return *this;
	}



	template<typename T>
	template<typename U, typename V, typename W>
	inline void Vector3D<T>::operator () (const U x1, const V y1, const W z1)
	{
		x = static_cast<T>(x1);
		y = static_cast<T>(y1);
		z = static_cast<T>(z1);
	}


	template<typename T>
	template<typename U>
	inline void Vector3D<T>::operator () (const Vector3D<U>& v)
	{
		x = static_cast<T>(v.x);
		y = static_cast<T>(v.y);
		z = static_cast<T>(v.z);
	}


	template<typename T>
	template<typename U, typename V>
	inline void Vector3D<T>::operator () (const Point3D<U>& origin, const Point3D<V>& end)
	{
		x = static_cast<T>((end.x - origin.x));
		y = static_cast<T>((end.y - origin.y));
		z = static_cast<T>((end.z - origin.z));
	}


	template<typename T>
	inline std::ostream& Vector3D<T>::print(std::ostream& out) const
	{
		out << x << "," << y << "," << z;
		return out;
	}


	template<typename T>
	inline bool Vector3D<T>::null() const
	{
		return Math::Zero(x) && Math::Zero(y) && Math::Zero(z);
	}


	template<typename T>
	Vector3D<T>& Vector3D<T>::normalize()
	{
		T m = magnitude();
		if (!Math::Zero(m))
		{
			x /= m;
			y /= m;
			z /= m;
		}
		return *this;
	}

	template<typename T>
	Vector3D<T>& Vector3D<T>::normalize(const T coeff)
	{
		T m = magnitude();
		if (!Math::Zero(m))
		{
			m = coeff / m;
			x *= m;
			y *= m;
			z *= m;
		}
		return *this;
	}




	template<typename T>
	inline T Vector3D<T>::magnitude() const
	{
		return Math::SquareRoot(x*x + y*y + z*z);
	}

	template<typename T>
	inline T Vector3D<T>::squareMagnitude() const
	{
		return x*x + y*y + z*z;
	}

	template<typename T>
	inline T Vector3D<T>::Magnitude(const Vector3D& p1, const Vector3D& p2)
	{
		return Math::SquareRoot(p1.x*p2.x + p1.y*p2.y + p1.z*p2.z /* dot product */);
	}

	template<typename T>
	inline T Vector3D<T>::dotProduct(const Vector3D& p1) const
	{
		return p1.x*x + p1.y*y + p1.z*z;
	}

	template<typename T>
	inline T Vector3D<T>::DotProduct(const Vector3D& p1, const Vector3D& p2)
	{
		return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
	}

	template<typename T>
	inline Vector3D<T> Vector3D<T>::CrossProduct(const Vector3D<T>& p1, const Vector3D<T>& p2)
	{
		return Vector3D
			(p1.y * p2.z - p1.z * p2.y,
			p1.z * p2.x - p1.x * p2.z,
			p1.x * p2.y - p1.y * p2.x);
	}

	template<typename T>
	inline T Vector3D<T>::Angle(const Vector3D<T>& p1, const Vector3D<T>& p2)
	{
		// By definition: cos(alpha) = (p1.p2)/(|p1|.|p2|)
		return static_cast<T>(Math::ACos(DotProduct(p1, p2) / (p1.magnitude() * p2.magnitude())));
	}

	template<class T>
	template<class R>
	inline void Vector3D<T>::Angle(R& out, const Vector3D<T>& p1, const Vector3D<T>& p2)
	{
		// By definition: cos(alpha) = (p1.p2)/(|p1|.|p2|)
		out = static_cast<R>(Math::ACos(DotProduct(p1, p2) / (p1.magnitude() * p2.magnitude())));
	}




	template<typename T>
	inline Vector3D<T>& Vector3D<T>::operator += (const T k)
	{
		x += k;
		y += k;
		z += k;
		return (*this);
	}

	template<typename T>
	template<typename U>
	inline Vector3D<T>& Vector3D<T>::operator += (const Vector3D<U>& p)
	{
		x += static_cast<T>(p.x);
		y += static_cast<T>(p.y);
		z += static_cast<T>(p.z);
		return (*this);
	}

	template<typename T>
	inline Vector3D<T>& Vector3D<T>::operator -= (const T k)
	{
		x -= k;
		y -= k;
		z -= k;
		return (*this);
	}

	template<typename T>
	template<typename U>
	inline Vector3D<T>& Vector3D<T>::operator -= (const Vector3D<U>& p)
	{
		x -= static_cast<T>(p.x);
		y -= static_cast<T>(p.y);
		z -= static_cast<T>(p.z);
		return (*this);
	}


	template<typename T>
	inline Vector3D<T>& Vector3D<T>::operator *= (const T k)
	{
		x *= k;
		y *= k;
		z *= k;
		return (*this);
	}

	template<typename T>
	inline Vector3D<T>& Vector3D<T>::operator /= (const T k)
	{
		if (!Math::Zero(k))
		{
			x /= k;
			y /= k;
			z /= k;
		}
		return (*this);
	}


	template<typename T>
	template<typename U>
	inline Vector3D<T>& Vector3D<T>::operator *= (const Vector3D<U>& p)
	{
		x -= static_cast<T>(p.x);
		y -= static_cast<T>(p.y);
		z -= static_cast<T>(p.z);
		return (*this);
	}

	template<typename T>
	template<typename U>
	inline Vector3D<T>& Vector3D<T>::operator /= (const Vector3D<U>& p)
	{
		x -= static_cast<T>(p.x);
		y -= static_cast<T>(p.y);
		z -= static_cast<T>(p.z);
		return (*this);
	}





} // namespace Yuni

#endif // __YUNI_CORE_MATH_GEOMETRY_VECTOR3D_HXX__
