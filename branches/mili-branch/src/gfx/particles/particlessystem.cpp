
#include "particlessystem.h"
#include "../../logs/logs.h"


namespace TA3D
{


    ParticlesSystem::ParticlesSystem()
        :nb_particles(0), pos(NULL), V(NULL), common_pos(), common_V(),
        size(1.0f), dsize(1.0f), mass(1.0f), life(1.0f),
        use_wind(true), light_emitter(false), tex(0), cur_idx(0)
    {}

    ParticlesSystem::~ParticlesSystem()
    {
        destroy();
    }

    void ParticlesSystem::destroy()
    {
        if (pos)
            delete[] pos;
        if(V)
            delete[] V;
        pos = NULL;
        V = NULL;
    }



    void ParticlesSystem::create(const uint16 nb, GLuint gltex)
    {
        nb_particles = nb;
        pos = new VECTOR[nb];
        V = new VECTOR[nb];
        common_V.x = common_V.y = common_V.z = 0.0f;
        common_pos = common_pos;
        tex = gltex;
        cur_idx = 0;
    }


    void ParticlesSystem::move(const float dt, VECTOR *p_wind_dir, const float g, const float factor, const float factor2)
    {
        LOG_ASSERT(p_wind_dir);
        life -= dt;
        size += dt * dsize;
        common_V = (*p_wind_dir);				// To simplify calculations
        common_V.y -= mass * g;

        common_pos = dt * common_V + common_pos;

        col[0] += dt * dcol[0];
        col[1] += dt * dcol[1];
        col[2] += dt * dcol[2];
        col[3] += dt * dcol[3];

        float real_factor = mass > 0.0f ? factor : factor2;

        for( uint32 i = 0 ; i < nb_particles ; ++i)
        {
            V[ i ] = real_factor * V[ i ];
            pos[ i ] = dt * V[ i ] + pos[ i ];
        }
    }


    void ParticlesSystem::draw()
    {
        glPushMatrix();
        glColor4fv(col);
        glPointSize(size);
        glTranslatef(common_pos.x, common_pos.y, common_pos.z );
        glBindTexture(GL_TEXTURE_2D,tex);
        glVertexPointer(3, GL_FLOAT, 0, pos);
        glDrawArrays(GL_POINTS, 0, nb_particles);
        glPopMatrix();
    }


} // namespace TA3D


