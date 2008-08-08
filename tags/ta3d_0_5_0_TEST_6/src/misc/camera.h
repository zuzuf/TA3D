#ifndef __TA3D_XX_MISC_CAMERA_H__
# define __TA3D_XX_MISC_CAMERA_H__

# include "../stdafx.h"
# include "matrix.h"


namespace TA3D
{



    class Camera		// Classe pour la gestion de la caméra
    {
    public:
        static Camera* inGame;

    public:
        Camera();

        /*!
        ** \brief Set the width ratio from the screen resolution
        ** \param w Width of the screen
        ** \param h Height of the screen
        */
        void setWidthFactor(const int w, const int h);

        /*!
        ** \brief
        */
        void setShake(const float duration, float magnitude);

        /*!
        ** \brief
        */
        void updateShake(const float dt);

        /*!
        **
        */
        void setMatrix(const MATRIX_4x4& v);

        /*!
        ** \brief Replace the OpenGL default camera
        */
        void setView();


    public:
        Vector3D up;					// Haut de la caméra
        Vector3D side;				// Coté de la caméra(optimisation pour les particules)

        //! Position
        Vector3D pos;
        Vector3D rpos;				// Position de la caméra
        //! Direction
        Vector3D dir;				// Direction de la caméra
        float zfar;				// Pour le volume visible
        float znear;
        float zfar2;				// Carré de la distance maximale
        bool mirror;				// Mirroir ??
        float mirrorPos;

        float shakeMagnitude;
        float shakeDuration;
        float shakeTotalDuration;
        Vector3D shakeVector;

        //! To support wide screen modes correctly
        float widthFactor;		

    }; // class Camera



} // namespace TA3D


#endif // __TA3D_XX_MISC_CAMERA_H__
