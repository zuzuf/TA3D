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
        static HWLight *inGame;
    public:
        inline HWLight() { init(); }

        void init();

        /*!
        ** \brief
        */
        inline void Enable() const
        { glEnable (HWNb); glEnable (GL_COLOR_MATERIAL); }

        /*!
        ** \brief
        */
        inline void Disable() const { glDisable(HWNb); }

        /*!
        ** \brief
        ** \param c
        */
        void Set(Camera& c);

        /*!
        ** \brief Set a camera from the light position, centered on the view frustum of c, used for shadow mapping
        ** \param c
        */
        void SetView(const std::vector<Vector3D> &frustum);

    public:
        //! Position
        Vector3D Pos;
        //! Dir
        Vector3D Dir;
        GLfloat LightAmbient[4];
        GLfloat LightDiffuse[4];
        GLfloat LightSpecular[4];
        GLuint HWNb;				// Indice de lumière matérielle
        float Att;				// Attenuation
        bool Directionnal;		// Indique si il s'agit d'une lumière directionnelle

    }; // class HWLight



} // namespace TA3D

#endif // __TA3D_XX_MISC_MATERIAL_LIGHT_H__
