#ifndef __TA3D_MISC_RECT_H__
# define __TA3D_MISC_RECT_H__

namespace TA3D
{

    /*! \class Rect
    **
    ** \brief Structure holding rectangle values as either 4 coordinates
    */
    template<typename T>
    class Rect
    {
    public:
        //! \name Constructors
        //@{
        //! Default constructor
        Rect() : x1(0), y1(0), x2(0), y2(0) {}
        //! Copy constructor
        Rect(const Rect<T>& c) : x1(c.x1), y1(c.y1), x2(c.x2), y2(c.y2) {}
        //@}

        /*!
        ** \brief Reset all Coordinates in the same time
        */
        void reset() {x1 = y1 = x2 = y2 = 0;}
        /*!
        ** \brief Reset all Coordinates in the same time
        */
        void reset(const Rect<T>& c) { x1 = c.x1; y1 = c.y1; x2 = c.x2; y2 = c.y2; }

    public:
        //! Top-Left X Coordinate
        T x1;
        //! Top-Left Y Coordinate
        T y1;
        //! Bottom-Right X Coordinate
        T x2;
        //! Bottom-Right Y Coordinate
        T y2;

    }; // class Rect




} // namespace TA3D


#endif // __TA3D_MISC_RECT_H__
