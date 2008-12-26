#ifndef __TA3D_GFX_FX_PARTICLE_H__
# define __TA3D_GFX_FX_PARTICLE_H__

# include "../stdafx.h"
# include "../misc/vector.h"



namespace TA3D
{
    class RenderQueue;


    class FXParticle
    {
    public:
        FXParticle(const Vector3D& P, const Vector3D& S, const float L);
        bool move(const float dt);
        void draw(RenderQueue &renderQueue);

    private:
        Vector3D Pos;
        Vector3D Speed;
        float life;
        float timer;

    }; // class FXParticle


} // namespace TA3D


#endif // __TA3D_GFX_FX_PARTICLE_H__
