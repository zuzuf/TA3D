#ifndef __TA3D_MISC_POINT_H__
# define __TA3D_MISC_POINT_H__

namespace TA3D
{

	/*! \class Point
	**
	** \brief Structure holding values for 2D-coordinates
	*/
	template<typename T>
	class Point
	{
	public:
		//! \name Constructors
		//@{
		//! Default constructor
		Point() : x(0), y(0) {}
		//! Copy constructor
		Point(const Point<T>& c) : x(c.x), y(c.y) {}
		//@}

		/*!
		** \brief Reset all Coordinates in the same time
		*/
		void reset() {x = y = 0;}
		/*!
		** \brief Reset all Coordinates in the same time
		*/
		void reset(const Point<T>& c) { x = c.x; y = x.y; }

	public:
		//! X Coordinate
		T x;
		//! Y Coordinate
		T y;

	}; // class Point




} // namespace TA3D


#endif // __TA3D_MISC_POINT_H__
