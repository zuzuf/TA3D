/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "matrix.h"





Matrix Transpose(const Matrix &A)
{
    Matrix B;
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
            B.E[i][j] = A.E[j][i];
    }
    return B;
}

float Norme_Ligne(const Matrix &A)
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

float Norme_Colonne(const Matrix &A)
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

inline Matrix Invert(const Matrix &A,const int P)
{
    Matrix I,E,B;
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


