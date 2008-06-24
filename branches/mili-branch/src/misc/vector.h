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


#ifndef CLASS_VECTOR
# define CLASS_VECTOR

# include <math.h>
# include <allegro.h>



/*!
** \brief 2-dimensional vector
*/
class VECTOR2D
{
public:
    //! \name Constructors
    //{
    //! Default constructor
    VECTOR2D() :x(0.0f), y(0.0f) {}
    //! Constructor by copy
    VECTOR2D(const VECTOR2D& c) : x(c.x), y(c.y) {}
    //! Constructor with initial values
    VECTOR2D(const float ax, const float ay) : x(ax), y(ay) {}
    //}

    float sq() const { return (x*x + y*y); }         // carré scalaire

    /*!
    ** \brief Vector norm
    ** \return The value of the vector norm
    */
    float norm() const { return sqrt(x*x+y*y); }

    // Rend le vecteur unitaire si possible(de norme 1)
    void unit();

    bool isNull() const { return (x == 0.0f && y == 0.0f); }

    //! \name Operators
    //{

    /*!
    ** \brief Operator += with another VECTOR2D
    */
    VECTOR2D& operator += (const VECTOR2D& rhs)
    { x += rhs.x; y += rhs.y; return (*this); }

    VECTOR2D& operator -= (const VECTOR2D& rhs)
    { x -= rhs.x; y -= rhs.y; return (*this); }

    VECTOR2D& operator *= (const float v)
    { x *= v; y *= v; return (*this); }

    bool operator == (const VECTOR2D& rhs) const
    { return (x == rhs.x && y == rhs.y); }

    bool operator != (const VECTOR2D& rhs) const
    { return !(*this == rhs); } 

    VECTOR2D& operator = (const VECTOR2D& rhs)
    { x = rhs.x; y = rhs.y; return *this; }

    //}

public:
    float x;
    float y;

}; // class VECTOR2D





/*!
** \brief 3-dimensional vector
*/
class VECTOR3D
{
public:
    //! \name Constructors
    //{
    //! Default constructor
    VECTOR3D() :x(0.0f), y(0.0f), z(0.0f) {}
    //! Constructor by copy
    VECTOR3D(const VECTOR3D& c) :x(c.x), y(c.y), z(c.z) {}
    //! Constructor with initial values
    VECTOR3D(const float ax, const float ay, const float az)
        :x(ax), y(ay), z(az) {}
    //}

    // Fonction qui renvoie le carré scalaire du vecteur
    float sq() const { return (x*x + y*y + z*z); }

    // Fonction qui renvoie la norme du vecteur
    float norm() const { return sqrt(x*x + y*y + z*z); }

    // Rend le vecteur unitaire si possible(de norme 1)
    void unit();

    
    /*!
    ** \brief Get if the vector is a null vector
    ** \return True if the vector is a null vector, false otherwise
    */
    bool isNull() const { return (x == 0.0f && y == 0.0f && z == 0.0f); }

    //! \name Operators
    //{

    VECTOR3D& operator += (const VECTOR3D& rhs)
    { x += rhs.x; y += rhs.y; z += rhs.z; return (*this); }

    VECTOR3D& operator -= (const VECTOR3D& rhs)
    { x -= rhs.x; y -= rhs.y; z -= rhs.z; return (*this); }

    VECTOR3D& operator *= (const float v)
    { x *= v; y *= v; z *= v; return (*this); }

    VECTOR3D& operator *= (const VECTOR3D rhs)
    { cross_product_f(x, y, z,  rhs.x, rhs.y, rhs.z,  &x, &y, &z); return (*this); }

    bool operator == (const VECTOR3D& rhs) const
    { return (fabs(x-rhs.x) < 0.0001f && fabs(y-rhs.y) < 0.0001f && fabs(z-rhs.z) < 0.0001f); }

    bool operator != (const VECTOR3D& rhs) const
    { return !(*this == rhs); } 

    VECTOR3D& operator = (const VECTOR3D& rhs)
    { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

    //}

public:
    float x;
    float y;
    float z;

}; // class VECTOR3D


typedef VECTOR3D VECTOR;



//! \name Operators for Vectors
//{

inline const VECTOR2D operator + (const VECTOR2D& lhs, const VECTOR2D& rhs)
{ return VECTOR2D(lhs) += rhs; }

inline const VECTOR2D operator - (const VECTOR2D& lhs)
{ VECTOR2D r(lhs); r.x = -r.x; r.y = -r.y; return r; }

inline const VECTOR2D operator - (const VECTOR2D& lhs, const VECTOR2D& rhs)
{ return VECTOR2D(lhs) -= rhs; }

inline const VECTOR2D operator * (const float& v, const VECTOR2D& lhs)
{ return VECTOR2D(lhs) *= v; }

inline const float operator % (const VECTOR2D& lhs, const VECTOR2D& rhs)
{ return lhs.x*rhs.x + lhs.y*rhs.y; }


inline const VECTOR3D operator + (const VECTOR3D& lhs, const VECTOR3D& rhs)
{ return VECTOR3D(lhs) += rhs; }

inline const VECTOR3D operator - (const VECTOR3D& lhs)
{ VECTOR3D r(lhs); r.x = -r.x; r.y = -r.y; r.z = -r.z; return r; }

inline const VECTOR3D operator - (const VECTOR3D& lhs, const VECTOR3D& rhs)
{ return VECTOR3D(lhs) -= rhs; }

inline const VECTOR3D operator * (const float& v, const VECTOR3D& lhs)
{ return VECTOR3D(lhs) *= v; }

inline const VECTOR3D operator * (const VECTOR3D& lhs, const VECTOR3D& rhs)
{ return VECTOR3D(lhs) *= rhs; }

inline const float operator % (const VECTOR3D& lhs, const VECTOR3D& rhs)
{ return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z; }


//}


/*------------------------------------------------------------------------
  |              Retourne l'angle en radians entre deux vecteurs
  \-------------------------------------*/

inline double VAngle(const VECTOR& A, const VECTOR& B)
{
    float a = sqrt(A.sq() * B.sq());
    return (a == 0.0f) ? 0.0f : acos((A % B) / a );
}

#endif
