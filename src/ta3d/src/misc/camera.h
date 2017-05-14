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
#ifndef __TA3D_XX_MISC_CAMERA_H__
# define __TA3D_XX_MISC_CAMERA_H__

# include <vector>
# include "matrix.h"
# include "vector.h"

class QMatrix4x4;

namespace TA3D
{



	class Camera		// Classe pour la gestion de la caméra
	{
	public:
		static Camera* inGame;

	public:
		Camera();

		/*!
		** \brief Set the width ratio from the screen resolution
		** \param w Width of the screen
		** \param h Height of the screen
		*/
		void setWidthFactor(const int w, const int h);

		/*!
		** \brief
		*/
		void setShake(const float duration, float magnitude);

		/*!
		** \brief
		*/
		void updateShake(const float dt);

		/*!
		** \brief Set the matrix
		*/
		void setMatrix(const Matrix& v);

		/*!
		** \brief Get the camera matrix
		*/
		Matrix getMatrix() const;

		/*!
		** \brief Replace the OpenGL camera
		*/
		void setView(bool classic = false);

        /*!
        ** \brief Replace the OpenGL camera
        */
        void setView(QMatrix4x4 &projectionMatrix, QMatrix4x4 &modelViewMatrix);

		/*!
		** \brief Reset all data
		*/
		void reset();

		/*!
		** \brief Returns the 8 points defining the frustum volume
		*/
		void getFrustum(std::vector<Vector3D>& list);
	public:
		Vector3D up;					// Haut de la caméra
		Vector3D side;				// Coté de la caméra(optimisation pour les particules)

		//! Position
		Vector3D pos;
		Vector3D rpos;				// Position de la caméra
		//! Direction
		Vector3D dir;				// Direction de la caméra
		float zfar;				// Pour le volume visible
		float znear;
		float zfar2;				// Carré de la distance maximale
		bool mirror;				// Mirroir ??
		float mirrorPos;

		float shakeMagnitude;
		float shakeDuration;
		float shakeTotalDuration;
		Vector3D shakeVector;

		float zoomFactor;

		//! To support wide screen modes correctly
		float widthFactor;

	}; // class Camera



} // namespace TA3D


#endif // __TA3D_XX_MISC_CAMERA_H__
