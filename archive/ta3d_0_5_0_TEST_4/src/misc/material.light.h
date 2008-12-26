#ifndef __TA3D_XX_MISC_MATERIAL_LIGHT_H__
# define __TA3D_XX_MISC_MATERIAL_LIGHT_H__

# include "../stdafx.h"
# include "vector.h"
# include "camera.h"



namespace TA3D
{


    class HWLight
    {
    public:
        HWLight() { init(); }

        void init();

        /*!
        ** \brief
        */
        void Enable() const
        { glEnable (HWNb); glEnable (GL_COLOR_MATERIAL); }

        /*!
        ** \brief 
        */
        void Disable() const { glDisable(HWNb); }

        /*!
        ** \brief 
        ** \param c
        */
        void Set(Camera& c);

    public:
        //! Position
        VECTOR Pos;
        //! Dir
        VECTOR Dir;	
        GLfloat LightAmbient[4];
        GLfloat LightDiffuse[4];
        GLfloat LightSpecular[4];
        GLuint HWNb;				// Indice de lumière matérielle
        float Att;				// Attenuation
        bool Directionnal;		// Indique si il s'agit d'une lumière directionnelle

    }; // class HWLight



} // namespace TA3D

#endif // __TA3D_XX_MISC_MATERIAL_LIGHT_H__
