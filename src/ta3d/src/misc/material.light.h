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

#ifndef __TA3D_XX_MISC_MATERIAL_LIGHT_H__
# define __TA3D_XX_MISC_MATERIAL_LIGHT_H__

# include <stdafx.h>
# include "vector.h"
# include "camera.h"
# include <gfx/gfx.h>


namespace TA3D
{


	class HWLight
	{
	public:
		static HWLight *inGame;

	public:
        HWLight();

		void init();

		/*!
		** \brief
		*/
        void Enable() const;

		/*!
		** \brief
		*/
        void Disable() const;

		/*!
		** \brief
		** \param c
		*/
		void Set(Camera& c);

		/*!
		** \brief Set a camera from the light position, centered on the view frustum of c, used for shadow mapping
		** \param c
		*/
		void SetView(const std::vector<Vector3D> &frustum);

	public:
		//! Position
		Vector3D Pos;
		//! Dir
		Vector3D Dir;
		GLfloat LightAmbient[4];
		GLfloat LightDiffuse[4];
		GLfloat LightSpecular[4];
		GLuint HWNb;				// Indice de lumière matérielle
		float Att;				// Attenuation
		bool Directionnal;		// Indique si il s'agit d'une lumière directionnelle

	}; // class HWLight



} // namespace TA3D

#endif // __TA3D_XX_MISC_MATERIAL_LIGHT_H__
