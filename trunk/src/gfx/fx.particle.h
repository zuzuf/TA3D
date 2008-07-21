#ifndef __TA3D_GFX_FX_PARTICLE_H__
# define __TA3D_GFX_FX_PARTICLE_H__

# include "../stdafx.h"
# include "../misc/vector.h"


namespace TA3D
{


    class FXParticle
    {
    public:
        FXParticle(const VECTOR3D& P, const VECTOR3D& S, const float L);
        bool move(const float dt);
        void draw();

    private:
        VECTOR3D Pos;
        VECTOR3D Speed;
        float life;
        float timer;

    }; // class FXParticle


} // namespace TA3D


#endif // __TA3D_GFX_FX_PARTICLE_H__
