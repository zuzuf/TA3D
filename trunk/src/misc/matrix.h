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

/*---------------------------------------------------------------------------\
  |                               matrix.h (pour allegro)                      |
  |     Contient la classe MATRIX_4x4 et les fonctions pour la gestion des     |
  | matrices, notament dans un moteur 3D.                                      |
  |     Ajoute également quelques fonctions à la classe MATRIX de allegro      |
  |                                                                            |
  \---------------------------------------------------------------------------*/


#ifndef __TA3D_MISC_MATRIX_H__
# define __TA3D_MISC_MATRIX_H__

# include "vector.h"
# include <string.h>




class MATRIX_4x4
{
public:
    //! Default constructor
    MATRIX_4x4() { clear(); }


    /*!
    ** \brief Clear the matrix
    */
    void clear() { memset(E,0,64); }

    //! \name Operators
    //@{

    /*!
    **
    */
    const MATRIX_4x4 operator = (const MATRIX_f& b);

    /*!
    **
    */
    const MATRIX_4x4 operator = (const MATRIX& b);

    /*!
    ** \brief Cast operator (MATRIX_f)
    */
    operator MATRIX_f() const;

    //@}

public:
    float E[4][4]; // Matrice 4x4
};




//-------  Opérations sur les matrices  -----------------------------------

// Addition
inline MATRIX_4x4 operator+(MATRIX_4x4 A,const MATRIX_4x4 &B)
{
    for(int i=0;i<16;i++)
        A.E[i>>2][i&3]+=B.E[i>>2][i&3];
    return A;
}

inline MATRIX operator+(MATRIX A,const MATRIX &B)
{
    for(int i=0;i<3;i++)
    {
        A.v[i][0]+=B.v[i][0];
        A.v[i][1]+=B.v[i][1];
        A.v[i][2]+=B.v[i][2];
        A.t[i]+=B.t[i];
    }
    return A;
}

inline MATRIX_f operator+(MATRIX_f A,const MATRIX_f &B)
{
    for(int i=0;i<3; ++i)
    {
        A.v[i][0] += B.v[i][0];
        A.v[i][1] += B.v[i][1];
        A.v[i][2] += B.v[i][2];
        A.t[i] += B.t[i];
    }
    return A;
}

// Soustraction
inline MATRIX_4x4 operator-(MATRIX_4x4 A,const MATRIX_4x4 &B)
{
    for(int i=0;i<16; ++i)
        A.E[i>>2][i&3] -= B.E[i>>2][i&3];
    return A;
}

inline MATRIX operator-(MATRIX A,const MATRIX &B)
{
    for(int i=0;i<3; ++i)
    {
        A.v[i][0] -= B.v[i][0];
        A.v[i][1] -= B.v[i][1];
        A.v[i][2] -= B.v[i][2];
        A.t[i]-=B.t[i];
    }
    return A;
}

inline MATRIX_f operator-(MATRIX_f A,const MATRIX_f &B)
{
    for(int i=0; i<3; ++i)
    {
        A.v[i][0]-=B.v[i][0];
        A.v[i][1]-=B.v[i][1];
        A.v[i][2]-=B.v[i][2];
        A.t[i]-=B.t[i];
    }
    return A;
}

// Multiplication
inline MATRIX_4x4 operator*(const MATRIX_4x4 &A,const MATRIX_4x4 &B)
{
    MATRIX_4x4 C;
    C.E[0][0] = A.E[0][0]*B.E[0][0]+A.E[1][0]*B.E[0][1]+A.E[2][0]*B.E[0][2]+A.E[3][0]*B.E[0][3];
    C.E[0][1] = A.E[0][1]*B.E[0][0]+A.E[1][1]*B.E[0][1]+A.E[2][1]*B.E[0][2]+A.E[3][1]*B.E[0][3];
    C.E[0][2] = A.E[0][2]*B.E[0][0]+A.E[1][2]*B.E[0][1]+A.E[2][2]*B.E[0][2]+A.E[3][2]*B.E[0][3];
    C.E[0][3] = A.E[0][3]*B.E[0][0]+A.E[1][3]*B.E[0][1]+A.E[2][3]*B.E[0][2]+A.E[3][3]*B.E[0][3];

    C.E[1][0] = A.E[0][0]*B.E[1][0]+A.E[1][0]*B.E[1][1]+A.E[2][0]*B.E[1][2]+A.E[3][0]*B.E[1][3];
    C.E[1][1] = A.E[0][1]*B.E[1][0]+A.E[1][1]*B.E[1][1]+A.E[2][1]*B.E[1][2]+A.E[3][1]*B.E[1][3];
    C.E[1][2] = A.E[0][2]*B.E[1][0]+A.E[1][2]*B.E[1][1]+A.E[2][2]*B.E[1][2]+A.E[3][2]*B.E[1][3];
    C.E[1][3] = A.E[0][3]*B.E[1][0]+A.E[1][3]*B.E[1][1]+A.E[2][3]*B.E[1][2]+A.E[3][3]*B.E[1][3];

    C.E[2][0] = A.E[0][0]*B.E[2][0]+A.E[1][0]*B.E[2][1]+A.E[2][0]*B.E[2][2]+A.E[3][0]*B.E[2][3];
    C.E[2][1] = A.E[0][1]*B.E[2][0]+A.E[1][1]*B.E[2][1]+A.E[2][1]*B.E[2][2]+A.E[3][1]*B.E[2][3];
    C.E[2][2] = A.E[0][2]*B.E[2][0]+A.E[1][2]*B.E[2][1]+A.E[2][2]*B.E[2][2]+A.E[3][2]*B.E[2][3];
    C.E[2][3] = A.E[0][3]*B.E[2][0]+A.E[1][3]*B.E[2][1]+A.E[2][3]*B.E[2][2]+A.E[3][3]*B.E[2][3];

    C.E[3][0] = A.E[0][0]*B.E[3][0]+A.E[1][0]*B.E[3][1]+A.E[2][0]*B.E[3][2]+A.E[3][0]*B.E[3][3];
    C.E[3][1] = A.E[0][1]*B.E[3][0]+A.E[1][1]*B.E[3][1]+A.E[2][1]*B.E[3][2]+A.E[3][1]*B.E[3][3];
    C.E[3][2] = A.E[0][2]*B.E[3][0]+A.E[1][2]*B.E[3][1]+A.E[2][2]*B.E[3][2]+A.E[3][2]*B.E[3][3];
    C.E[3][3] = A.E[0][3]*B.E[3][0]+A.E[1][3]*B.E[3][1]+A.E[2][3]*B.E[3][2]+A.E[3][3]*B.E[3][3];

    return C;
}


inline MATRIX operator*(const MATRIX &A,const MATRIX &B)
{
    MATRIX C;
    matrix_mul(&A,&B,&C);
    return C;
}

inline MATRIX_f operator*(const MATRIX_f &A,const MATRIX_f &B)
{
    MATRIX_f C;
    matrix_mul_f(&A,&B,&C);
    return C;
}

// Multiplication(transformation d'un vecteur)
inline Vector3D operator*(const Vector3D& A,const MATRIX_4x4 &B)
{
    Vector3D C;
    C.x=A.x*B.E[0][0]+A.y*B.E[0][1]+A.z*B.E[0][2];
    C.y=A.x*B.E[1][0]+A.y*B.E[1][1]+A.z*B.E[1][2];
    C.z=A.x*B.E[2][0]+A.y*B.E[2][1]+A.z*B.E[2][2];
    return C;
}

inline Vector3D operator*(const Vector3D& A,MATRIX B)
{
    Vector3D C;
    fixed x,y,z;
    apply_matrix(&B,ftofix(A.x),ftofix(A.y),ftofix(A.z),&x,&y,&z);
    C.x=fixtof(x);
    C.y=fixtof(y);
    C.z=fixtof(z);
    return C;
}

inline Vector3D operator*(const Vector3D& A,const MATRIX_f &B)
{
    Vector3D C;
    apply_matrix_f(&B,A.x,A.y,A.z,&C.x,&C.y,&C.z);
    return C;
}

// Multiplication d'une matrice par un réel
inline MATRIX_4x4 operator*(const float &A,MATRIX_4x4 B)
{
    for(int i=0;i<16; ++i)
        B.E[i>>2][i&3]*=A;
    return B;
}

inline MATRIX operator*(const fixed &A,MATRIX B)
{
    for(int i=0;i<3;i++)
    {
        B.v[i][0] = (B.v[i][0] >> 8) *A >> 8;
        B.v[i][1] = (B.v[i][1] >> 8) *A >> 8;
        B.v[i][2] = (B.v[i][2] >> 8) *A >> 8;
        B.t[i]=(B.t[i]>>8)*A>>8;
    }
    return B;
}

inline MATRIX_f operator*(const float &A,MATRIX_f B)
{
    for(int i=0;i<3; ++i)
    {
        B.v[i][0]*=A;
        B.v[i][1]*=A;
        B.v[i][2]*=A;
        B.t[i]*=A;
    }
    return B;
}

inline Vector3D glNMult(const Vector3D &A,const MATRIX_4x4 &B)
{
    Vector3D C;
    float w;
    C.x=A.x*B.E[0][0]+A.y*B.E[0][1]+A.z*B.E[0][2]+B.E[0][3];
    C.y=A.x*B.E[1][0]+A.y*B.E[1][1]+A.z*B.E[1][2]+B.E[1][3];
    C.z=A.x*B.E[2][0]+A.y*B.E[2][1]+A.z*B.E[2][2]+B.E[2][3];
    w=1.0f/(A.x*B.E[3][0]+A.y*B.E[3][1]+A.z*B.E[3][2]+B.E[3][3]);
    C.x*=w;
    C.y*=w;
    C.z*=w;
    return C;
}

// Crée une matrice de translation
inline MATRIX_4x4 Translate(const Vector3D &A)
{
    MATRIX_4x4 B;
    B.E[0][0]=1.0f;
    B.E[1][1]=1.0f;
    B.E[2][2]=1.0f;
    B.E[0][3]=A.x;
    B.E[1][3]=A.y;
    B.E[2][3]=A.z;
    B.E[3][3]=1.0f;
    return B;
}

// Crée une matrice de mise à l'échelle
inline MATRIX_4x4 Scale(const float &Size)
{
    MATRIX_4x4 M;
    M.E[0][0]=Size;
    M.E[1][1]=Size;
    M.E[2][2]=Size;
    M.E[3][3]=1.0f;
    return M;
}
// Crée une matrice de projection
inline MATRIX_4x4 Perspective(const float &FOV,const float &w,const float &h,const float &zn,const float &zf)
{
    MATRIX_4x4 M;
    M.E[0][0]=2*zn/w;
    M.E[1][1]=2*zn/h;
    M.E[2][2]=zf/(zf-zn);
    M.E[3][2]=1;
    M.E[2][3]=zn*zf/(zn-zf);

    return M;
}

// Crée une matrice de rotation autour de l'axe X
inline MATRIX_4x4 RotateX(const float &Theta)
{
    MATRIX_4x4 M;
    M.E[0][0]=1.0f;
    M.E[1][1]=cosf(Theta);
    M.E[2][1]=sinf(Theta);
    M.E[1][2]=-M.E[2][1];
    M.E[2][2]=M.E[1][1];
    M.E[3][3]=1.0f;
    return M;
}

// Crée une matrice de rotation autour de l'axe Y
inline MATRIX_4x4 RotateY(const float &Theta)
{
    MATRIX_4x4 M;
    M.E[0][0]=cosf(Theta);
    M.E[2][0]=-sinf(Theta);
    M.E[1][1]=1.0f;
    M.E[0][2]=-M.E[2][0];
    M.E[2][2]=M.E[0][0];
    M.E[3][3]=1.0f;
    return M;
}

// Crée une matrice de rotation autour de l'axe Z
inline MATRIX_4x4 RotateZ(const float &Theta)
{
    MATRIX_4x4 M;
    M.E[0][0]=cosf(Theta);
    M.E[1][0]=sinf(Theta);
    M.E[0][1]=-M.E[1][0];
    M.E[1][1]=M.E[0][0];
    M.E[2][2]=1.0f;
    M.E[3][3]=1.0f;
    return M;
}

// Returns RotateZ(Rz) * RotateY(Ry) * RotateX(Rx) but faster ;)
inline MATRIX_4x4 RotateZYX(const float &Rz, const float &Ry, const float &Rx)
{
    float cx = cosf(Rx);
    float sx = sinf(Rx);
    float cy = cosf(Ry);
    float sy = sinf(Ry);
    float cz = cosf(Rz);
    float sz = sinf(Rz);

    float sxcz = sx * cz;
    float sxsz = sx * sz;
    float czcx = cz * cx;
    float szcx = sz * cx;

    MATRIX_4x4 M;
    M.E[0][0] = cz * cy;
    M.E[1][0] = szcx + sxcz * sy;
    M.E[2][0] = sxsz - czcx * sy;
    M.E[0][1] = -sz * cy;
    M.E[1][1] = czcx - sy * sxsz;
    M.E[2][1] = sxcz + szcx * sy;
    M.E[0][2] = sy;
    M.E[1][2] = -sx * cy;
    M.E[2][2] = cx * cy;
    M.E[3][3] = 1.0f;
    return M;
}

// Returns RotateX(Rx) * RotateY(Ry) * RotateZ(Rz) but faster ;)
inline MATRIX_4x4 RotateXYZ(const float &Rx, const float &Ry, const float &Rz)
{
    float cx = cosf(Rx);
    float sx = sinf(Rx);
    float cy = cosf(Ry);
    float sy = sinf(Ry);
    float cz = cosf(Rz);
    float sz = sinf(Rz);

    float szcx = sz * cx;
    float sxcz = sx * cz;
    float czcx = cz * cx;
    float sxsz = sx * sz;

    MATRIX_4x4 M;
    M.E[0][0] = cz * cy;
    M.E[1][0] = cy * sz;
    M.E[2][0] = -sy;
    M.E[0][1] = sy * sxcz - szcx;
    M.E[1][1] = sy * sxsz + czcx;
    M.E[2][1] = sx * cy;
    M.E[0][2] = czcx * sy + sxsz;
    M.E[1][2] = szcx * sy - sxcz;
    M.E[2][2] = cx * cy;
    M.E[3][3] = 1.0f;
    return M;
}

// Returns RotateX(Rx) * RotateZ(Rz) * RotateY(Ry) but faster ;)
inline MATRIX_4x4 RotateXZY(const float &Rx, const float &Rz, const float &Ry)
{
    float cx = cosf(Rx);
    float sx = sinf(Rx);
    float cy = cosf(Ry);
    float sy = sinf(Ry);
    float cz = cosf(Rz);
    float sz = sinf(Rz);

    float sxcy = sx * cy;
    float sxsy = sx * sy;
    float sycx = sy * cx;
    float cxcy = cx * cy;

    MATRIX_4x4 M;
    M.E[0][0] = cz * cy;
    M.E[1][0] = sz;
    M.E[2][0] = -sy * cz;

    M.E[0][1] = -sz * cxcy + sxsy;
    M.E[1][1] = cz * cx;
    M.E[2][1] = sz * sycx + sxcy;

    M.E[0][2] = sz * sxcy + sycx;
    M.E[1][2] = -cz * sx;
    M.E[2][2] = -sz * sxsy + cxcy;

    M.E[3][3] = 1.0f;
    return M;
}

// Returns RotateY(Ry) * RotateZ(Rz) * RotateX(Rx) but faster ;)
inline MATRIX_4x4 RotateYZX(const float &Ry, const float &Rz, const float &Rx)
{
    float cx = cosf(Rx);
    float sx = sinf(Rx);
    float cy = cosf(Ry);
    float sy = sinf(Ry);
    float cz = cosf(Rz);
    float sz = sinf(Rz);

    float sysx = sy * sx;
    float sycx = sy * cx;
    float cysx = cy * sx;
    float cycx = cy * cx;

    MATRIX_4x4 M;
    M.E[0][0] = cy * cz;
    M.E[1][0] = cycx * sz + sysx;
    M.E[2][0] = cysx * sz - sycx;

    M.E[0][1] = -sz;
    M.E[1][1] = cz * cx;
    M.E[2][1] = cz * sx;

    M.E[0][2] = sy * cz;
    M.E[1][2] = sz * sycx - cysx;
    M.E[2][2] = sz * sysx + cycx;

    M.E[3][3] = 1.0f;
    return M;
}

// Transpose une matrice
MATRIX_4x4 Transpose(const MATRIX_4x4 &A);

// Renvoie la norme ligne
float Norme_Ligne(const MATRIX_4x4 &A);

// Renvoie la norme colonne
float Norme_Colonne(const MATRIX_4x4 &A);

// Inversion
MATRIX_4x4 Invert(const MATRIX_4x4 &A, const int P = 15);



#endif // __TA3D_MISC_MATRIX_H__
