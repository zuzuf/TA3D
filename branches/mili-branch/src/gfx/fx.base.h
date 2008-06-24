#ifndef __TA3D_GFX_FX_BASE_H__
# define __TA3D_GFX_FX_BASE_H__

# include "../stdafx.h"
# include "../misc/vector.h"
# include "../gaf.h"
# include "../ta3dbase.h"
# include "../EngineClass.h"


namespace TA3D
{



    class FX
    {
    public:
        FX();
        ~FX();

        void init();

        void destroy();

        bool move(float dt,ANIM **anims);
        void draw(CAMERA *cam, MAP *map, ANIM **anims);

        void load(int anim,VECTOR P,float s);

    public:
        //! Effect duration
        float time;
        //! Get if the effect has been played
        bool playing;
        //! Position
        VECTOR Pos;
        float size;		// Taille (proportion de l'image d'origine)
        int anm;		// Animation

    }; // class FX


} // namespace TA3D


#endif // __TA3D_GFX_FX_BASE_H__
