
#include "matrix.h"





MATRIX_4x4 Transpose(const MATRIX_4x4 &A)
{
    MATRIX_4x4 B;
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
            B.E[i][j] = A.E[j][i];
    }
    return B;
}

float Norme_Ligne(const MATRIX_4x4 &A)
{
    float n,n2;
    n=0.0f;
    for (int i=0;i<4; ++i)
    {
        n2=0.0f;
        for(int j=0;j<4;j++)
            n2+=fabsf(A.E[i][j]);
        if(n2>n) n=n2;
    }
    return n;
}

float Norme_Colonne(const MATRIX_4x4 &A)
{
    float n,n2;
    n=0.0f;
    for (int i = 0; i < 4; ++i)
    {
        n2=0.0f;
        for (int j=0;j<4; ++j)
            n2 += fabsf(A.E[j][i]);
        if(n2>n)
            n=n2;
    }
    return n;
}

inline MATRIX_4x4 Invert(const MATRIX_4x4 &A,const int P)
{
    MATRIX_4x4 I,E,B;
    I = Scale(1.0f);
    B = 1.0f / (Norme_Ligne(A) * Norme_Colonne(A)) * Transpose(A);
    int i;
    for(i=0;i<P; ++i)
    {
        E = I - B*A;
        B = (I+E) * B;
        if(Norme_Ligne(E) == 0)
            i=P;
    }
    return B;
}


