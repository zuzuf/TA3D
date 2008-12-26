
#include "vector.h"



void VECTOR2D::unit()
{
    if (!isNull()) // Si le vecteur n'est pas nul
    {
        float n = 1.0f / sqrt(x*x + y*y);    // Inverse de la norme du vecteur
        x *= n;
        y *= n;
    }
}


void VECTOR3D::unit()
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


