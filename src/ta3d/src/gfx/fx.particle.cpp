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

#include "fx.particle.h"
#include "particles/particles.h"
#include <EngineClass.h>
#include "fx.manager.h"
#include <mesh/mesh.h>
#include <mesh/instancing.h>
#include <ingame/players.h>

namespace TA3D
{


    FXParticle::FXParticle(const Vector3D& P, const Vector3D& S, const float L)
        :Pos(P), Speed(S), life(L), timer(0.0f)
    {}



    bool FXParticle::move(const float dt)
    {
        life -= dt;
        timer += dt;

        Speed.y -= dt * the_map->ota_data.gravity;		// React to gravity
		Pos += dt * Speed;

		const float min_h = the_map->get_unit_h(Pos.x, Pos.z);
        if (Pos.y < min_h) // Bouncing on the map :)
        {
            Pos.y = 2.0f * min_h - Pos.y;
            float dx = the_map->get_unit_h(Pos.x + 16.0f, Pos.z) - min_h;
            float dz = the_map->get_unit_h(Pos.x, Pos.z + 16.0f) - min_h;
            Vector3D Normal(-dx, 16.0f, -dz);
            Normal.unit();

            float cross = Speed % Normal;
            if (cross < 0.0f)
                Speed -= (1.5f * cross) * Normal;
        }

        while (timer >= 0.2f) // Emit smoke
        {
            timer -= 0.2f;
            particle_engine.make_dark_smoke( Pos, 0, 1, 0.0f, -1.0f, -1.0f, 1.0f );
        }
        // When it shoud die, return true
        return (life <= 0.0f);
    }



    void FXParticle::draw(RenderQueue &renderQueue)
    {
        if (the_map)            // Visibility test
        {
			const int px = ((int)(Pos.x+0.5f) + the_map->map_w_d)>>4;
			const int py = ((int)(Pos.z+0.5f) + the_map->map_h_d)>>4;
			if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)	return;
			const byte player_mask = byte(1 << players.local_human_id);
			if (the_map->view(px, py) != 1
			   || !(the_map->sight_map(px, py) & player_mask))	return;
        }
        renderQueue.queue.push_back( Instance( Pos, 0xFFFFFFFF, 0.0f ) );
    }



} // namespace TA3D

