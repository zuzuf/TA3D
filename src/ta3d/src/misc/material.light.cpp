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

#include <logs/logs.h>
#include "material.light.h"
#include "math.h"
#include "vector.h"
#include <TA3D_NameSpace.h>
#include <QMatrix4x4>


namespace TA3D
{


	HWLight* HWLight::inGame = NULL;

    HWLight::HWLight()
    {
        init();
    }

    void HWLight::Enable() const
    {
        gfx->glEnable(HWNb);
        gfx->glEnable(GL_COLOR_MATERIAL);
    }

    void HWLight::Disable() const
    {
        glDisable(HWNb);
    }

	void HWLight::init()
	{
		inGame = this;

		LightAmbient[0] = 0.125f;
		LightAmbient[1] = 0.125f;
		LightAmbient[2] = 0.125f;
		LightAmbient[3] = 1.0f;

		LightDiffuse[0] = 1.0f;
		LightDiffuse[1] = 1.0f;
		LightDiffuse[2] = 1.0f;
		LightDiffuse[3] = 1.0f;

		LightSpecular[0] = 1.0f;
		LightSpecular[1] = 1.0f;
		LightSpecular[2] = 1.0f;
		LightSpecular[3] = 1.0f;

		Pos.x = 0.0f;
		Pos.y = 0.0f;
		Pos.z = 0.0f;
		Dir = Pos;
		Dir.z = -1.0f;
		HWNb = GL_LIGHT0;
		Att = 0.05f;
		Directionnal = true;
	}



	void HWLight::Set(Camera& c)
	{
		GLfloat LightPosition[4];
		if (Directionnal)
		{
			LightPosition[0] = Dir.x;
			LightPosition[1] = Dir.y;
			LightPosition[2] = Dir.z;
			LightPosition[3] = 0.0f;
		}
		else
		{
			c.setView();
			LightPosition[0] = Pos.x;
			LightPosition[1] = Pos.y;
			LightPosition[2] = Pos.z;
			LightPosition[3] = 1.0f;
		}

		glLightfv(HWNb, GL_AMBIENT, LightAmbient);   // Setup The Ambient Light
		glLightfv(HWNb, GL_DIFFUSE, LightDiffuse);   // Setup The Diffuse Light
		glLightfv(HWNb, GL_SPECULAR, LightSpecular); // Setup The Diffuse Light
		glLightfv(HWNb, GL_POSITION,LightPosition);  // Position The Light
		glLightf(HWNb,  GL_LINEAR_ATTENUATION, Att); // Attenuation
	}


	void HWLight::SetView(const std::vector<Vector3D> &frustum)
	{
		if (Directionnal)
		{
			Vector3D Side(0.0f, 1.0f, 0.0f);
			Vector3D Up(Side * Dir);
			Up.unit();
			Side = Dir * Up;
			Side.unit();

			float mx = 0.0f, Mx = 0.0f;
			float my = 0.0f, My = 0.0f;
			float mz = 0.0f, Mz = 0.0f;
			for (unsigned int i = 0; i < frustum.size(); i++)
			{
				float X = frustum[i] % Side;
				float Z = frustum[i] % Dir;
				float Y = frustum[i] % Up;
				if (i == 0)
				{
					mx = Mx = X;
					my = My = Y;
					mz = Mz = Z;
				}
				else
				{
					mx = Math::Min(mx, X);
					Mx = Math::Max(Mx, X);
					my = Math::Min(my, Y);
					My = Math::Max(My, Y);
					mz = Math::Min(mz, Z);
					Mz = Math::Max(Mz, Z);
				}
			}
			Vector3D c_pos(Mz * Dir + 0.5f * (mx + Mx) * Side + 0.5f * (my + My) * Up);

			const float zfar = Mz - mz;
			const float znear = -128.0f;
			const float widthFactor = (Mx - mx) / (My - my);
			const float f = 0.5f * (My - my);

			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			glOrtho(-widthFactor * f, widthFactor * f, -f, f, znear, zfar);

			Vector3D c_at(c_pos - Dir);
            QMatrix4x4 M;
            M.lookAt(QVector3D(c_pos.x, c_pos.y, c_pos.z),
                     QVector3D(c_at.x, c_at.y, c_at.z),
                     QVector3D(Up.x, Up.y, Up.z));
            glMultMatrixf(M.data());

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}
		else
		{
			float zfar = 1000.0f;
			float znear = 0.01f;
			float widthFactor = 1.0f;
			//float f = 10.0f;

			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			glFrustum(-widthFactor * znear, widthFactor * znear, -0.75f * znear, 0.75f * znear, znear, zfar);

			Vector3D Up(Pos * Dir);
			Up.unit();
            QMatrix4x4 M;
            M.lookAt(QVector3D(Pos.x, Pos.y, Pos.z),
                     QVector3D(Dir.x, Dir.y, Dir.z),
                     QVector3D(Up.x, Up.y, Up.z));
            glMultMatrixf(M.data());

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}
	}


} // namespace TA3D
