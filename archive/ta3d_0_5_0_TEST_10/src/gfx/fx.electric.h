#ifndef __TA3D_GFX_FX_ELECTRIC_H__
# define __TA3D_GFX_FX_ELECTRIC_H__

# include "../stdafx.h"
# include "../misc/vector.h"



namespace TA3D
{
    class FXElectric
    {
    public:
        FXElectric(const Vector3D& P);
        bool move(const float dt);
        void draw();

    private:
        Vector3D Pos;
        float life;
        
        Vector3D U;
        Vector3D V;

    }; // class FXElectric


} // namespace TA3D


#endif // __TA3D_GFX_FX_PARTICLE_H__
