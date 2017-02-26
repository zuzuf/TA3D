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

#include <stdafx.h>
#include "recttest.h"
#include <gfx/gfx.h>

namespace TA3D
{

	RectTest::RectTest (Camera& cam, const Rect<int>& pos) : cam(cam)
	{
		cam.setView();
		Matrix modelView;
		Matrix project;

		int	viewportCoords[4] = {0, 0, 0, 0};
        VARS::gfx->glGetIntegerv(GL_VIEWPORT, viewportCoords);
        VARS::gfx->glGetFloatv(GL_MODELVIEW_MATRIX,  (float*)modelView.E);
        VARS::gfx->glGetFloatv(GL_PROJECTION_MATRIX, (float*)project.E);

		modelView = Transpose(modelView);
		project = Transpose(project);

		VW =  static_cast<float>(viewportCoords[2] - viewportCoords[0]) * 0.5f;
		VH = -static_cast<float>(viewportCoords[3] - viewportCoords[1]) * 0.5f;

		T = modelView * project; // Matrice de transformation

		X1 = Math::Min(pos.x1, pos.x2);
		Y1 = Math::Min(pos.y1, pos.y2);
		X2 = Math::Max(pos.x1, pos.x2);
		Y2 = Math::Max(pos.y1, pos.y2);
	}


	bool RectTest::contains(const Vector3D &point) const
	{
		Vector3D Vec(point - cam.pos);
		float d = Vec.sq();

		if (d > 16384.0f && (Vec % cam.dir) <= 0.0f)
			return false;

        Vector3D UPos (glNMult(point, T));
		UPos.x = UPos.x * VW + VW;
		UPos.y = UPos.y * VH - VH;
		return X1 <= UPos.x && X2 >= UPos.x && Y1 <= UPos.y && Y2 >= UPos.y;
	}



} // namespace TA3D
