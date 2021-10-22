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

#include "particlessystem.h"
#include <logs/logs.h>
#include <gfx/gfx.h>

namespace TA3D
{
    using namespace VARS;

    ParticlesSystem::ParticlesSystem()
		:nb_particles(0), pos(NULL), V(NULL), common_pos(),
        size(1.0f), dsize(1.0f), mass(1.0f), life(1.0f),
        use_wind(true), light_emitter(false), cur_idx(0)
    {}

    ParticlesSystem::~ParticlesSystem()
    {
        destroy();
    }

    void ParticlesSystem::destroy()
    {
		DELETE_ARRAY(pos);
		DELETE_ARRAY(V);
    }



    void ParticlesSystem::create(const uint32 nb, const GfxTexture::Ptr &gltex)
    {
        nb_particles = nb;
        pos = new Vector3D[nb];
        V = new Vector3D[nb];
		common_pos.reset();
        tex = gltex;
        cur_idx = 0;
    }


	void ParticlesSystem::move(const float dt, const float factor, const float factor2)
    {
        if (pos == NULL || V == NULL)    return;     // Huh oO ? this is not expected to happen
        life -= dt;
        size += dt * dsize;

        col[0] += dt * dcol[0];
        col[1] += dt * dcol[1];
        col[2] += dt * dcol[2];
        col[3] += dt * dcol[3];

        const float real_factor = mass > 0.0f ? factor : factor2;

        for( uint32 i = 0 ; i < nb_particles ; ++i)
        {
            V[ i ] = real_factor * V[ i ];
            pos[ i ] = dt * V[ i ] + pos[ i ];
        }
    }


    void ParticlesSystem::draw(QMatrix4x4 modelViewMatrix)
    {
        if (pos == NULL)    return;     // Huh oO ? this is not expected to happen
        modelViewMatrix.translate(common_pos.x, common_pos.y, common_pos.z);

        gfx->particle_shader->setUniformValue("uColor", col[0], col[1], col[2], col[3]);
        gfx->particle_shader->setUniformValue("uPointSize", size);
        gfx->particle_shader->setUniformValue("uModelViewMatrix", modelViewMatrix);
        tex->bind();
        gfx->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, pos);
        CHECK_GL();
        gfx->glDrawArrays(GL_POINTS, 0, nb_particles);
        CHECK_GL();
    }


} // namespace TA3D


