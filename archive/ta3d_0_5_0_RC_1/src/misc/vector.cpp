
#include "vector.h"



void Vector2D::unit()
{
    if (!isNull()) // Si le vecteur n'est pas nul
    {
        float n = 1.0f / sqrtf(x*x + y*y);    // Inverse de la norme du vecteur
        x *= n;
        y *= n;
    }
}


void Vector3D::unit()
{
    if(!isNull()) // Si le vecteur n'est pas nul
    {
        float n = norm(); // Inverse de la norme du vecteur
        if(n != 0.0f)
        {
            n = 1.0f / n;
            x *= n;
            y *= n;
            z *= n;
        }
    }
}


