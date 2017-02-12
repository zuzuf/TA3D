#ifndef __YUNI_CORE_MATH_GEOMETRY_POINT2D_H__
# define __YUNI_CORE_MATH_GEOMETRY_POINT2D_H__

# include "../math.h"



namespace Yuni
{


	/*!
	** \brief Represents a 2D-point
	*/
	template<typename T = float>
	class Point2D
	{
	public:
		//! \name Constructors & Destructor
		//{
		//! Default constructor
		Point2D() : x(0), y(0) {}
		/*!
		** \brief Constructor
		** \param x1 The default x coordinate
		** \param y1 The default y coordinate
		*/
		template<typename U, typename V>
		Point2D(const U x1, const V y1): x((T)x1), y((T)y1) {}
		//! Constructor by copy
		template<typename U>
		Point2D(const Point2D<U>& p) : x((T)p.x), y((T)p.y) {}
		//}

		//! Reset the point to origin
		Point2D<T>& reset() {x = y = T(); return *this;}


		/*!
		** \brief Move the point to new coordinates
		**
		** \param x1 The new X coordinate
		** \param y1 The new Y coordinate
		*/
		template<typename U, typename V>
		void move(const U x1, const V y1) { x = (T)x1; y = (T)y1; }
		/*!
		** \brief Move the point to new coordinates
		** \param p The new coordinates
		*/
		template<typename U>
		void move(const Point2D<U>& p) { x = (T)p.x; y = (T)p.y; }


		/*!
		** \brief Translate the point with the same value for all coordinates
		**
		** \param k The value to add to all coordinates
		*/
		template<typename U>
		void translate(const U k) { x += (T)k; y += (T)k; }
		/*!
		** \brief Translate the point with relative coordinates
		**
		** \param x1 The value to add to the x coordinate
		** \param y1 The value to add to the y coordinate
		*/
		template<typename U, typename V>
		void translate(const U x1, const V y1) { x += (T)x1; y += (T)y1; }
		/*!
		** \brief Translate the point with relative coordinates from another Point
		** \param p The values to add to the coordinates
		*/
		template<typename U>
		void translate(const Point2D<U>& p) { x += (T)p.x; y += (T)p.y; }

		/*!
		** \brief Calculate the mean between two points
		**
		** The calling object is modified to store the value
		**
		** \param p Point to compute the mean with
		*/
		template<typename U> void mean(const Point2D<U>& p)
		{
			x = (x + p.x) / 2.0f;
			y = (y + p.y) / 2.0f;
		}
		/*!
		** \brief Calculate the mean between two points
		**
		** The calling object is modified to store the value
		**
		** \param p1 Point to compute the mean with
		** \param p2 Second point to compute the mean with
		** \return Always *this
		*/
		template<typename U, typename V>
		Point2D<T>& mean(const Point2D<U>& p1, const Point2D<V>& p2)
		{
			x = (p1.x + p2.x) / 2.0f;
			y = (p1.y + p2.y) / 2.0f;
			return *this;
		}
		/*!
		** \brief Calculate the mean between two points
		**
		** \param p1 First point
		** \param p2 Second point
		** \return A new instance of Point3D
		*/
		static Point2D<T>& Mean(const Point2D<T>& p1, const Point2D<T>& p2)
		{
			return Point2D<T>().mean(p1, p2);
		}


		/*!
		** \brief Get if the point is close to another point
		**
		** \param rhs The other point
		** \param delta Delta value
		*/
		template<typename U, typename V>
		bool closeTo(const Point2D<U>& rhs, const V delta) const
		{ return Math::Abs((U)x-rhs.x) < delta && Math::Abs((U)y-rhs.y) < delta; }

		/*!
		** \brief Get if the point is close to another point
		**
		** \param x1 The X coordinate of the other point
		** \param y1 The Y coordinate of the other point
		** \param delta Delta value
		*/
		template<typename U, typename V>
		bool closeTo(const U x1, const U y1, const V delta) const
		{ return Math::Abs((U)x-x1) < delta && Math::Abs((U)y-y1) < delta; }



		//! \name Operators
		//{

		/*!
		** \brief Reset all coordinates
		**
		** \param x1 The new value for the x coordinate
		** \param y1 The new value for the y coordinate
		** \see move()
		*/
		template<typename U> void operator () (const U x1, const U y1) { x = (T)x1; x = (T)y1; }
		/*!
		** \brief Copy all coordinates from another point
		** \param p The coordinates to copy
		** \see move()
		*/
		template<typename U> void operator () (const Point2D<U>& p) { x = (T)p.x; y = (T)p.y; }


		/*!
		** \brief Translate the point with the same value for all coordinates
		**
		** \param k The value to add to all coordinates
		** \return Always *this
		**
		** \see translate()
		*/
		template<typename U>
		Point2D<T>& operator += (const U k) { x += (T)k; y += (T)k; return (*this); }
		/*!
		** \brief Translate the point with relative coordinates
		**
		** \param p Point to use as reference x and y for the translation
		** \return Always *this
		**
		** \see translate()
		*/
		template<typename U>
		Point2D<T>& operator += (const Point2D<U>& p) { x += (T)p.x; y += (T)p.y; return (*this); }

		/*!
		** \brief Comparison operator (equal with)
		**
		** \param rhs The other point to compare with
		** \return True if the two points are equal
		*/
		template<typename U> bool operator == (const Point2D<U>& rhs) const
		{ return (T)rhs.x == x && (T)rhs.y == y; }

		/*!
		** \brief Comparison operator (non equal with)
		**
		** \param rhs The other point to compare with
		** \return True if the two points are not equal
		*/
		template<typename U> bool operator != (const Point2D<U>& rhs) const
		{ return !(*this == rhs); }

		/*!
		** \brief Assign new values for all coordinates from another point
		**
		** \param p The new coordinates
		** \return Always *this
		**
		** \see move()
		*/
		template<typename U>
		Point2D<T>& operator = (const Point2D<U>& p) { x = (T)p.x; y = (T)p.y; return (*this); }

		//} Operators


		/*!
		** \brief Print the point
		**
		** \param[in,out] out An output stream
		** \return The output stream `out`
		*/
		template<class StreamT> StreamT& print(StreamT& out) const
		{
			out << "(" << x << "," << y << ")";
			return out;
		}

	public:
		//! X coordinate
		T x;
		//! Y coordinate
		T y;

	}; // class Point2D




} // namespace Yuni



//! name Operator overload for stream printing
//@{
template<typename T>
inline std::ostream& operator << (std::ostream& out, const Yuni::Point2D<T>& p)
{ return p.print(out); }

template<typename T>
inline const Yuni::Point2D<T> operator + (const Yuni::Point2D<T>& lhs, const Yuni::Point2D<T>& rhs)
{ return Yuni::Point2D<T>(lhs) += rhs; }
//@}


#endif // __YUNI_CORE_MATH_GEOMETRY_POINT2D_H__
