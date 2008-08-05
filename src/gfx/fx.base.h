#ifndef __TA3D_GFX_FX_BASE_H__
# define __TA3D_GFX_FX_BASE_H__

# include "../stdafx.h"
# include "../misc/vector.h"
# include "../gaf.h"
# include "../ta3dbase.h"
# include "../EngineClass.h"
# include "../misc/camera.h"



namespace TA3D
{


    class FX
    {
    public:
        FX();
        ~FX();

        void init();

        void destroy();

        /*!
        ** \brief
        **
        ** \param anim
        ** \param p
        ** \param s
        */
        void load(const int anim, const Vector3D& p, const float s);

        /*!
        ** \brief 
        **
        ** \param dt
        ** \param anims
        ** \return
        */
        bool move(const float dt, Gaf::Animation** anims);

        /*!
        ** \brief
        **
        ** \param cam
        ** \param map
        ** \param anims
        */
        void draw(Camera& cam, MAP *map, Gaf::Animation** anims);


    public:
        //! Effect duration
        float time;
        //! Get if the effect has been played
        bool playing;
        //! Position
        Vector3D Pos;
        float size;		// Taille (proportion de l'image d'origine)
        int anm;		// Animation

    private:
        /*!
        ** \brief
        */
        bool doCanDrawAnim(MAP* map) const;

        /*!
        ** \brief
        */
        void doDrawAnimFlash();

        /*!
        ** \brief
        */
        void doDrawAnimWave(const int animIndx);

        /*!
        ** \brief 
        */
        void doDrawAnimRipple();

        /*!
        ** \brief
        */
        void doDrawAnimDefault(Camera& cam, Gaf::Animation** anims);

    }; // class FX


} // namespace TA3D


#endif // __TA3D_GFX_FX_BASE_H__
