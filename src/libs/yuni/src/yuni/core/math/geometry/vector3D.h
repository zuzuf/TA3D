#ifndef __YUNI_CORE_MATH_GEOMETRY_VECTOR3D_H__
# define __YUNI_CORE_MATH_GEOMETRY_VECTOR3D_H__

# include "../../../yuni.h"
# include "../math.h"
# include "../trigonometric.h"
# include "point3D.h"

namespace Yuni
{


	/*!
	** \brief Represents a 3D-vector, with generic homogeneous content
	*/
	template<typename T = float>
	class Vector3D
	{
	public:
		/*!
		** \brief Compute the mean between two arbitrary vectors
		**
		** \param p1 The first vector
		** \param p2 The second vector
		** \return A new instance of Vector3D
		*/
		static Vector3D Mean(const Vector3D& p1, const Vector3D& p2);

		/*!
		** \brief Compute the magnitude of the addition of two arbitrary vectors
		**
		** \code
		** Yuni::Vector3D<> a(1.,   2,4, 6.9);
		** Yuni::Vector3D<> b(4.1., 0.2, 3.1);
		**
		** // This way is faster
		** std::cout << "Magnitude : " << Yuni::Vector3D<>::Magnitude(a, b) << std::endl;
		** // than
		** std::cout << "Magnitude : " << (a + b).magnitude() << std::endl;
		** \endcode
		** \param p1 The first vector
		** \param p2 The second vector
		** \return The magnitude of the addition of the 2 vectors
		*/
		static T Magnitude(const Vector3D& p1, const Vector3D& p2);

		/*!
		** \brief Compute the dot product of two arbitrary vectors
		**
		** \param p1 The first vector
		** \param p2 The second vector
		** \return The magnitude of the addition of the 2 vectors
		*/
		static T DotProduct(const Vector3D& p1, const Vector3D& p2);

		/*!
		** \brief Compute the cross product of two arbitrary vectors
		**
		** \param p1 The first vector
		** \param p2 The second vector
		*/
		static Vector3D CrossProduct(const Vector3D& p1, const Vector3D& p2);

		/*!
		** \brief Compute the angle between two arbitrary vectors
		**
		** \param p1 The first vector
		** \param p2 The second vector
		*/
		static T Angle(const Vector3D& p1, const Vector3D& p2);

		/*!
		** \brief Compute the angle between two vectors, with a specific return type
		**
		** \param out Where to store the result
		** \param p1 The first vector
		** \param p2 The second vector
		*/
		template<class R>
		void Angle(R& out, const Vector3D<T>& p1, const Vector3D<T>& p2);


	public:
		//! \name Constructors
		//@{
		//! Default constructor
		Vector3D();

		/*!
		** \brief Constructor
		** \param x1 The default X coordinate
		** \param y1 The default Y coordinate
		** \param z1 The default Z coordinate
		*/
		template<typename U, typename V, typename W>
		Vector3D(const U x1, const V y1, const W z1 = W());

		/*!
		** \brief Constructor using two points
		**
		** \param origin Origin point of the vector
		** \param end End point of the vector
		*/
		template<typename U, typename V>
		Vector3D(const Point3D<U>& origin, const Point3D<V>& end);

		//! Constructor by copy
		Vector3D(const Vector3D& rhs);

		//! Constructor by copy
		template<typename U> Vector3D(const Vector3D<U>& v);
		//@}


		//! \name Reset the coordinates
		//@{
		//! Reset the vector to the null vector
		Vector3D<T>& reset();
		//@}


		//! \name Translation
		//@{
		/*!
		** \brief Add the same value for all coordinates to the vector
		** \param k The value to add to all coordinates
		*/
		template<typename U> void translate(const U k);
		/*!
		** \brief Translate the point with relative coordinates
		** \param x1 The value to add to the X coordinate
		** \param y1 The value to add to the Y coordinate
		** \param z1 The value to add to the Z coordinate
		*/
		template<typename U, typename V, typename W>
		void translate(const U x1, const V y1, const W z1);
		/*!
		** \brief Translate the point with relative coordinates from another Point
		** \param p The values to add to the coordinates
		*/
		template<typename U> void translate(const Vector3D<U>& p);
		//@}


		//! \name Components
		//@{
		/*!
		** \brief Get if the vector is null
		*/
		bool null() const;

		/*!
		** \brief Compute the magnitude of the vector
		*/
		T magnitude() const;

		/*!
		** \brief Compute the square magnitude of the vector
		*/
		T squareMagnitude() const;

		/*!
		** \brief Compute the dot product with another arbitrary vector
		*/
		T dotProduct(const Vector3D& rhs) const;

		/*!
		** \brief Normalize the vector (coefficient = 1.)
		*/
		Vector3D& normalize();
		/*!
		** \brief Normalize the vector with a different coefficient
		*/
		Vector3D& normalize(const T coeff);
		//@}


		//! \name Mean
		//@{
		/*!
		** \brief Calculate the mean between two points
		**
		** The calling object is modified to store the value
		**
		** \param p Point to compute the mean with
		*/
		template<typename U> Vector3D& mean(const Vector3D<U>& p);

		/*!
		** \brief Calculate the mean between two points
		**
		** The calling object is modified to store the value
		**
		** \param p1 Point to compute the mean with
		** \param p2 Second point to compute the mean with
		** \return Always *this
		*/
		template<typename U, typename V> Vector3D<T>& mean(const Vector3D<U>& p1, const Vector3D<V>& p2);
		//@}


		//! \name Operators
		//{
		/*!
		** \brief Reset all coordinates
		**
		** \param x1 The new value for the X coordinate
		** \param y1 The new value for the Y coordinate
		** \param z1 The new value for the Z coordinate
		** \see move()
		*/
		template<typename U, typename V, typename W>
		void operator () (const U x1, const V y1, const W z1);
		/*!
		** \brief Copy all coordinates from another vector
		**
		** \param v The coordinates to copy
		*/
		template<typename U> void operator () (const Vector3D<U>& v);
		/*!
		** \brief Reset a vector using two points
		**
		** \param origin Start point of the vector
		** \param end End point of the vector
		*/
		template<typename U, typename V>
		void operator () (const Point3D<U>& origin, const Point3D<V>& end);

		/*!
		** \brief Translate the point with the same value for all coordinates
		**
		** \param k The value to add to all coordinates
		** \return Always *this
		**
		** \see translate()
		*/
		Vector3D<T>& operator += (const T k);
		/*!
		** \brief Translate the point with relative coordinates
		**
		** \param p The values to add to coordinates
		** \return Always *this
		**
		** \see translate()
		*/
		template<typename U> Vector3D<T>& operator += (const Vector3D<U>& p);

		/*!
		** \brief Translate the point with the same value for all coordinates
		**
		** \param k The value to add to all coordinates
		** \return Always *this
		**
		** \see translate()
		*/
		Vector3D<T>& operator -= (const T k);
		/*!
		** \brief Translate the point with relative coordinates
		**
		** \param p The values to add to coordinates
		** \return Always *this
		**
		** \see translate()
		*/
		template<typename U> Vector3D<T>& operator -= (const Vector3D<U>& p);

		/*!
		** \brief Uniform scaling
		*/
		Vector3D<T>& operator *= (const T k);
		/*!
		** \brief Dot product
		*/
		template<typename U> Vector3D<T>& operator *= (const Vector3D<U>& p);

		/*!
		** \brief Uniform scaling
		*/
		Vector3D<T>& operator /= (const T k);
		/*!
		** \brief Dot product
		*/
		template<typename U> Vector3D<T>& operator /= (const Vector3D<U>& p);



		/*!
		** \brief Comparison operator (equal with)
		**
		** \param rhs The other point to compare with
		** \return True if the two points are equal
		*/
		template<typename U> bool operator == (const Vector3D<U>& rhs) const
		{ return Math::Equals((T)rhs.x, x) && Math::Equals((T)rhs.y, y) && Math::Equals((T)rhs.z, z); }

		/*!
		** \brief Comparison operator (non equal with)
		**
		** \param rhs The other point to compare with
		** \return True if the two points are not equal
		*/
		template<typename U> bool operator != (const Vector3D<U>& rhs) const
		{ return !(*this == rhs); }

		/*!
		** \brief Assign new values for all coordinates from another vector
		**
		** \param p The new coordinates
		** \return Always *this
		**
		** \see move()
		*/
		template<typename U>
		Vector3D<T>& operator = (const Vector3D<U>& p) { x = (T)p.x; y = (T)p.y; z = (T)p.z; return (*this); }

		//} Operators


		/*!
		** \brief Print the vector
		**
		** \param out An output stream
		** \return The output stream `out`
		*/
		std::ostream& print(std::ostream& out) const;


	public:
		//! X component
		T x;
		//! Y component
		T y;
		//! Z component
		T z;

	}; // class Vector3D




} // namespace Yuni


# include "vector3D.hxx"


//! \name Operator overload for stream printing
//@{

template<typename T>
inline std::ostream& operator << (std::ostream& out, const Yuni::Vector3D<T>& v)
{ return v.print(out); }

template<typename T>
inline const Yuni::Vector3D<T> operator + (const Yuni::Vector3D<T>& lhs, const Yuni::Vector3D<T>& rhs)
{ return Yuni::Vector3D<T>(lhs) += rhs; }

//@}


#endif // __YUNI_CORE_MATH_GEOMETRY_VECTOR3D_H__
