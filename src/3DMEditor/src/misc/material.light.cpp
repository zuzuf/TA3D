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

#include "material.light.h"
#include "math.h"

namespace TA3D
{
    HWLight* HWLight::inGame = NULL;

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
        Dir = Vec(1.0f, 1.0f, 1.0f);
        Dir.unit();
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

} // namespace TA3D
