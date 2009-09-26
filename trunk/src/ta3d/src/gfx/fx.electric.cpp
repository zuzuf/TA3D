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

#include "../TA3D_NameSpace.h"
#include "fx.electric.h"

namespace TA3D
{


    FXElectric::FXElectric(const Vector3D& P)
        : Pos(P), life(1.0f)
    {
        U.x = (Math::RandomTable() % 201) * 0.01f - 1.0f;
        U.y = (Math::RandomTable() % 201) * 0.01f - 1.0f;
        U.z = (Math::RandomTable() % 201) * 0.01f - 1.0f;

        V.x = 1.0f; V.y = 0.0f; V.z = 0.0f;
        V = V * U;
        V.unit();

        U = V * U;
        U.unit();
    }



    bool FXElectric::move(const float dt)
    {
        life -= dt;
        // When it shoud die, return true
        return (life <= 0.0f);
    }



    void FXElectric::draw()
    {
        float start = 2.0f * life * PI;
        float end   = start + 1.0f;
        static const float step = 0.1f;
		Vector3D p;

        glBegin(GL_LINE_STRIP);
        for (float i = start; i <= end; i += step)
        {
            glColor4ub(0x70 + (Math::RandomTable() & 0x1F), 0x70 + (Math::RandomTable() & 0x1F),
				0xFF - ((int)Math::RandomTable() & 0xF), 0xFF);

			p = Pos;
			p += (cosf(i) * 2.0f) * V;
			p += (sinf(i) * 2.0f) * U;
            p.x += (Math::RandomTable() % 61) * 0.01f - 0.3f;
            p.y += (Math::RandomTable() % 61) * 0.01f - 0.3f;
            p.z += (Math::RandomTable() % 61) * 0.01f - 0.3f;
            glVertex3fv((GLfloat*)& p);
        }
        glEnd();
    }



} // namespace TA3D

