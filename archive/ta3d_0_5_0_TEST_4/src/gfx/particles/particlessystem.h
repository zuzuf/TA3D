#ifndef __TA3D_GFX_PARTICLES_SYSTEM_H__
# define __TA3D_GFX_PARTICLES_SYSTEM_H__

# include "../../stdafx.h"
# include "../../misc/vector.h"


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
        void create(const uint16 nb, GLuint gltex);

        /*!
        ** \brief
        */
        void move(const float dt, VECTOR *p_wind_dir, const float g, const float factor, const float factor2);

        /*!
        ** \brief Draw
        */
        void draw();

    public:
        uint32 nb_particles;
        VECTOR *pos;
        VECTOR *V;
        VECTOR common_pos;
        VECTOR common_V;
        float size;
        float dsize;
        float mass;
        float life;
        float col[4];
        float dcol[4];
        bool use_wind;
        bool light_emitter;
        GLuint tex;
        //! Index to fill the point array
        uint16 cur_idx;

    }; // class ParticlesSystem


} // namespace TA3D

#endif // __TA3D_GFX_PARTICLES_SYSTEM_H__
