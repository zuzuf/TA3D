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
#ifndef __TA3D_GFX_PARTICLES_SYSTEM_H__
# define __TA3D_GFX_PARTICLES_SYSTEM_H__

# include <stdafx.h>
# include <misc/vector.h>
# include <QOpenGLFunctions>
# include <gfx/texture.h>

namespace TA3D
{


	/*! \class ParticlesSystem
	**
	** \brief The fast particle engine
	*/
	class ParticlesSystem
	{
	public:
		//! \name Constructor & destructor
		//{
		//! Default constructor
		ParticlesSystem();
		//! Destructor
		~ParticlesSystem();
		//}

		/*!
		** \brief
		*/
		void destroy();

		/*!
		** \brief
		*/
        void create(const uint32 nb, const GfxTexture::Ptr &gltex);

		/*!
		** \brief
		*/
		void move(const float dt, const float factor, const float factor2);

		/*!
		** \brief Draw
		*/
		void draw();

	public:
		uint32 nb_particles;
		Vector3D *pos;
		Vector3D *V;
		Vector3D common_pos;
		float size;
		float dsize;
		float mass;
		float life;
		float col[4];
		float dcol[4];
		bool use_wind;
		bool light_emitter;
        GfxTexture::Ptr tex;
		//! Index to fill the point array
		uint16 cur_idx;

	}; // class ParticlesSystem




} // namespace TA3D

#endif // __TA3D_GFX_PARTICLES_SYSTEM_H__
