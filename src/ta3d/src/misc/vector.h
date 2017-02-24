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


#ifndef __TA3D_XX_MISC_VECTOR_H__
# define __TA3D_XX_MISC_VECTOR_H__

# include <stdafx.h>
# include "math.h"


namespace TA3D
{


	/*!
	** \brief 2-dimensional vector
	*/
	class Vector2D
	{
	public:
		//! \name Constructors
		//@{
		//! Default constructor
        inline Vector2D() :x(0.f), y(0.f) {}
		//! Constructor by copy
        inline Vector2D(const Vector2D& c) : x(c.x), y(c.y) {}
		//! Constructor with initial values
        inline Vector2D(const float ax, const float ay) : x(ax), y(ay) {}
		//@}

        inline float sq() const { return (x*x + y*y); }         // carré scalaire

		/*!
		** \brief Reset all coordinates to 0
		*/
        inline void reset() { x = 0.0f; y = 0.0f; }

		/*!
		** \brief Vector norm
		** \return The value of the vector norm
		*/
        inline float norm() const { return sqrtf(x*x+y*y); }

		// Rend le vecteur unitaire si possible(de norme 1)
		void unit();

        inline bool isNull() const
		{ return (fabsf(x) < 0.000001f && fabsf(y) < 0.000001f); }

		//! \name Operators
		//@{

		/*!
		** \brief Operator += with another Vector2D
		*/
        inline Vector2D& operator += (const Vector2D& rhs)
		{ x += rhs.x; y += rhs.y; return (*this); }

        inline Vector2D& operator -= (const Vector2D& rhs)
		{ x -= rhs.x; y -= rhs.y; return (*this); }

        inline Vector2D& operator *= (const float v)
		{ x *= v; y *= v; return (*this); }

        inline bool operator == (const Vector2D& rhs) const
        { return (Math::Equals(x, rhs.x) && Math::Equals(y, rhs.y)); }

        inline bool operator != (const Vector2D& rhs) const
		{ return !(*this == rhs); }

        inline Vector2D& operator = (const Vector2D& rhs)
		{ x = rhs.x; y = rhs.y; return *this; }

		//@}

	public:
		//! The X-Coordinate
		float x;
		//! The Y-Coordinate
		float y;

	}; // class Vector2D





	/*!
	** \brief 3-dimensional vector
	*/
	class Vector3D
	{
	public:
		//! \name Constructors
		//@{
		//! Default constructor
        inline Vector3D() :x(0.0f), y(0.0f), z(0.0f) {}
		//! Constructor by copy
        inline Vector3D(const Vector3D& c) :x(c.x), y(c.y), z(c.z) {}
		//! Constructor with initial values
        inline Vector3D(const float ax, const float ay, const float az)
			:x(ax), y(ay), z(az) {}
		//@}

		/*!
		** \brief Reset all coordinates to 0
		*/
        inline void reset() { x = 0.0f; y = 0.0f; z = 0.0f; }

		// Fonction qui renvoie le carré scalaire du vecteur
        inline float sq() const { return (x*x + y*y + z*z); }

		// Fonction qui renvoie la norme du vecteur
        inline float norm() const { return sqrtf(x*x + y*y + z*z); }

		// Rend le vecteur unitaire si possible(de norme 1)
		inline void unit()
		{
			float n = norm(); // Inverse de la norme du vecteur
            if (!Math::Zero(n))
			{
				n = 1.0f / n;
				x *= n;
				y *= n;
				z *= n;
			}
		}


		/*!
		** \brief Get if the vector is a null vector
		** \return True if the vector is a null vector, false otherwise
		*/
        inline bool isNull() const
		{ return (fabsf(x) < 0.000001f && fabsf(y) < 0.000001f && fabsf(z) < 0.000001f); }

		//! \name Operators
		//@{

        inline Vector3D& operator += (const Vector3D& rhs)
		{ x += rhs.x; y += rhs.y; z += rhs.z; return (*this); }

        inline Vector3D& operator -= (const Vector3D& rhs)
		{ x -= rhs.x; y -= rhs.y; z -= rhs.z; return (*this); }

        inline Vector3D& operator *= (const float v)
		{ x *= v; y *= v; z *= v; return (*this); }

        inline Vector3D& operator *= (const Vector3D rhs)
		{
			const float nx =  y * rhs.z - z * rhs.y;
			const float ny = -x * rhs.z + z * rhs.x;
			const float nz =  x * rhs.y - y * rhs.x;
			x = nx;
			y = ny;
			z = nz;
			return (*this);
		}

        inline bool operator == (const Vector3D& rhs) const
		{ return (fabsf(x-rhs.x) < 0.0001f && fabsf(y-rhs.y) < 0.0001f && fabsf(z-rhs.z) < 0.0001f); }

        inline bool operator != (const Vector3D& rhs) const
		{ return !(*this == rhs); }

        inline Vector3D& operator = (const Vector3D& rhs)
		{ x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

		inline float operator[](int n) const
		{
			switch(n)
			{
			case 0:	return x;
			case 1:	return y;
			case 2:	return z;
			default:	return 0.0f;
			}
		}

		inline float &operator[](int n)
		{
			switch(n)
			{
			case 0:	return x;
			case 1:	return y;
			case 2:	return z;
			default:	return x;
			}
		}

		inline float min() const
		{
			return std::min(x, std::min(y, z));
		}

		inline float max() const
		{
			return std::max(x, std::max(y, z));
		}
		//@}

	public:
		//! The X-Coordinate
		float x;
		//! The Y-Coordinate
		float y;
		//! The Z-Coordinate
		float z;

	}; // class Vector3D


} // namespace TA3D


//! \name Operators for Vectors
//@{

inline const TA3D::Vector2D operator + (const TA3D::Vector2D& lhs, const TA3D::Vector2D& rhs)
{ return TA3D::Vector2D(lhs) += rhs; }

inline const TA3D::Vector2D operator - (const TA3D::Vector2D& lhs)
{ TA3D::Vector2D r(lhs); r.x = -r.x; r.y = -r.y; return r; }

inline const TA3D::Vector2D operator - (const TA3D::Vector2D& lhs, const TA3D::Vector2D& rhs)
{ return TA3D::Vector2D(lhs) -= rhs; }

inline const TA3D::Vector2D operator * (const float v, const TA3D::Vector2D& lhs)
{ return TA3D::Vector2D(lhs) *= v; }

inline float operator % (const TA3D::Vector2D& lhs, const TA3D::Vector2D& rhs)
{ return lhs.x*rhs.x + lhs.y*rhs.y; }


inline const TA3D::Vector3D operator + (const TA3D::Vector3D& lhs, const TA3D::Vector3D& rhs)
{ return TA3D::Vector3D(lhs) += rhs; }

inline const TA3D::Vector3D operator - (const TA3D::Vector3D& lhs)
{ TA3D::Vector3D r(lhs); r.x = -r.x; r.y = -r.y; r.z = -r.z; return r; }

inline const TA3D::Vector3D operator - (const TA3D::Vector3D& lhs, const TA3D::Vector3D& rhs)
{ return TA3D::Vector3D(lhs) -= rhs; }

inline const TA3D::Vector3D operator * (const float v, const TA3D::Vector3D& lhs)
{ return TA3D::Vector3D(lhs) *= v; }

inline const TA3D::Vector3D operator * (const TA3D::Vector3D& lhs, const TA3D::Vector3D& rhs)
{ return TA3D::Vector3D(lhs) *= rhs; }

inline float operator % (const TA3D::Vector3D& lhs, const TA3D::Vector3D& rhs)
{ return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z; }


//@}



/*!
** \brief Retourne l'angle en radians entre deux vecteurs
*/
inline float VAngle(const TA3D::Vector3D& A, const TA3D::Vector3D& B)
{
    float a = std::sqrt(A.sq() * B.sq());
    a = (TA3D::Math::Zero(a)) ? 0.0f : std::acos((A % B) / a);
	return isNaN(a) ? 0.0f : a;
}

namespace TA3D
{
	namespace Math
	{
		inline Vector3D Abs(const TA3D::Vector3D &a)
		{
			return Vector3D(std::fabs(a.x), std::fabs(a.y), std::fabs(a.z));
		}

		inline Vector3D Max(const TA3D::Vector3D &a, const TA3D::Vector3D &b)
		{
			const Vector3D mid = 0.5f * (a + b);
			return mid + Abs(a - mid);
		}

		inline Vector3D Min(const Vector3D &a, const Vector3D &b)
		{
			const Vector3D mid = 0.5f * (a + b);
			return mid - Abs(a - mid);
		}
	}
}

#endif // __TA3D_XX_MISC_VECTOR_H__
