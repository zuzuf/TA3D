
#include "fx.particle.h"
#include "particles/particles.h"
#include "../EngineClass.h"
#include "fx.manager.h"
#include "../3do.h"
#include "../ingame/players.h"

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
//        Pos = Pos + dt * Speed;
        Pos.x += dt * Speed.x;
        Pos.y += dt * Speed.y;
        Pos.z += dt * Speed.z;

        float min_h = the_map->get_unit_h(Pos.x, Pos.z);
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
            int px=((int)(Pos.x+0.5f) + the_map->map_w_d)>>4;
            int py=((int)(Pos.z+0.5f) + the_map->map_h_d)>>4;
            if (px<0 || py<0 || px >= the_map->bloc_w || py >= the_map->bloc_h)	return;
            byte player_mask = 1 << players.local_human_id;
            if (the_map->view[py][px]!=1
               || !(the_map->sight_map->line[py][px]&player_mask))	return;
        }
        renderQueue.queue.push_back( Instance( Pos, 0xFFFFFFFF, 0.0f ) );
    }



} // namespace TA3D

