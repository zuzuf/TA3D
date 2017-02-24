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



namespace TA3D
{


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
			for (int j=0;j<4;j++)
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
			if (Math::Zero(Norme_Ligne(E)))
				i = P;
		}
		return B;
	}


	TA3D::Matrix RotateZYX(const float Rz, const float Ry, const float Rx)
	{
		const float cx = cosf(Rx);
		const float sx = sinf(Rx);
		const float cy = cosf(Ry);
		const float sy = sinf(Ry);
		const float cz = cosf(Rz);
		const float sz = sinf(Rz);

		const float sxcz = sx * cz;
		const float sxsz = sx * sz;
		const float czcx = cz * cx;
		const float szcx = sz * cx;

		TA3D::Matrix M;
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


	TA3D::Matrix RotateXYZ(const float Rx, const float Ry, const float Rz)
	{
		const float cx = cosf(Rx);
		const float sx = sinf(Rx);
		const float cy = cosf(Ry);
		const float sy = sinf(Ry);
		const float cz = cosf(Rz);
		const float sz = sinf(Rz);

		const float szcx = sz * cx;
		const float sxcz = sx * cz;
		const float czcx = cz * cx;
		const float sxsz = sx * sz;

		TA3D::Matrix M;
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



	TA3D::Matrix RotateXZY(const float Rx, const float Rz, const float Ry)
	{
		const float cx = cosf(Rx);
		const float sx = sinf(Rx);
		const float cy = cosf(Ry);
		const float sy = sinf(Ry);
		const float cz = cosf(Rz);
		const float sz = sinf(Rz);

		const float sxcy = sx * cy;
		const float sxsy = sx * sy;
		const float sycx = sy * cx;
		const float cxcy = cx * cy;

		TA3D::Matrix M;
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


	TA3D::Matrix RotateYZX(const float Ry, const float Rz, const float Rx)
	{
		const float cx = cosf(Rx);
		const float sx = sinf(Rx);
		const float cy = cosf(Ry);
		const float sy = sinf(Ry);
		const float cz = cosf(Rz);
		const float sz = sinf(Rz);

		const float sysx = sy * sx;
		const float sycx = sy * cx;
		const float cysx = cy * sx;
		const float cycx = cy * cx;

		TA3D::Matrix M;
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


	TA3D::Vector3D glNMult(const TA3D::Vector3D &A,const TA3D::Matrix &B)
	{
		TA3D::Vector3D C;
		C.x = A.x * B.E[0][0] + A.y * B.E[0][1] + A.z * B.E[0][2] + B.E[0][3];
		C.y = A.x * B.E[1][0] + A.y * B.E[1][1] + A.z * B.E[1][2] + B.E[1][3];
		C.z = A.x * B.E[2][0] + A.y * B.E[2][1] + A.z * B.E[2][2] + B.E[2][3];
		const float w = 1.0f / (A.x * B.E[3][0] + A.y * B.E[3][1] + A.z * B.E[3][2] + B.E[3][3]);
		C.x *= w;
		C.y *= w;
		C.z *= w;
		return C;
	}


	TA3D::Matrix Translate(const TA3D::Vector3D &A)
	{
		TA3D::Matrix B;
		B.E[0][0] = 1.0f;
		B.E[1][1] = 1.0f;
		B.E[2][2] = 1.0f;
		B.E[0][3] = A.x;
		B.E[1][3] = A.y;
		B.E[2][3] = A.z;
		B.E[3][3] = 1.0f;
		return B;
	}


	TA3D::Matrix Scale(const float Size)
	{
		TA3D::Matrix M;
		M.E[0][0] = Size;
		M.E[1][1] = Size;
		M.E[2][2] = Size;
		M.E[3][3] = 1.0f;
		return M;
	}
	
	
	TA3D::Matrix Perspective(const float w, const float h, const float zn, const float zf)
	{
		TA3D::Matrix M;
		M.E[0][0] = 2.0f * zn / w;
		M.E[1][1] = 2.0f * zn / h;
		M.E[2][2] = zf / (zf - zn);
		M.E[3][2] = 1.0f;
		M.E[2][3] = zn * zf / (zn - zf);

		return M;
	}



	TA3D::Matrix RotateX(const float Theta)
	{
		TA3D::Matrix M;
		M.E[0][0] = 1.0f;
		M.E[1][1] = cosf(Theta);
		M.E[2][1] = sinf(Theta);
		M.E[1][2] = -M.E[2][1];
		M.E[2][2] = M.E[1][1];
		M.E[3][3] = 1.0f;
		return M;
	}


	TA3D::Matrix RotateY(const float Theta)
	{
		TA3D::Matrix M;
		M.E[0][0] = cosf(Theta);
		M.E[2][0] = -sinf(Theta);
		M.E[1][1] = 1.0f;
		M.E[0][2] = -M.E[2][0];
		M.E[2][2] = M.E[0][0];
		M.E[3][3] = 1.0f;
		return M;
	}


	TA3D::Matrix RotateZ(const float Theta)
	{
		TA3D::Matrix M;
		M.E[0][0] = cosf(Theta);
		M.E[1][0] = sinf(Theta);
		M.E[0][1] = -M.E[1][0];
		M.E[1][1] = M.E[0][0];
		M.E[2][2] = 1.0f;
		M.E[3][3] = 1.0f;
		return M;
	}


} // namespace TA3D
