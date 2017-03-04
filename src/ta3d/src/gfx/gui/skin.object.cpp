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
#include "skin.object.h"
#include <misc/paths.h>
#include <TA3D_NameSpace.h>



namespace TA3D
{
namespace Gui
{



	SKIN_OBJECT::SKIN_OBJECT()
        : x1(0), y1(0), x2(0), y2(0),
		t_x1(0.0f), t_y1(0.0f), t_x2(0.0f), t_y2(0.0f),
		w(0), h(0),
		sw(0.0f), sh(0.0f)
	{}


	SKIN_OBJECT::~SKIN_OBJECT()
	{
	}

	void SKIN_OBJECT::init()
	{
        tex = nullptr;
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
		t_x1 = 0.0f;
		t_y1 = 0.0f;
		t_x2 = 0.0f;
		t_y2 = 0.0f;
		w = 0;
		h = 0;
		sw = 0.0f;
		sh = 0.0f;
	}

	void SKIN_OBJECT::destroy()
	{
		init();
	}




	void SKIN_OBJECT::load(TDFParser& parser, const QString& prefix, float borderSize)
	{
        const QString filename = parser.pullAsString(prefix + "image");
		if (VFS::Instance()->fileExists(filename))
		{
			tex = gfx->load_texture(filename, FILTER_LINEAR, &w, &h);

            x1 = (float)parser.pullAsInt(prefix + "x1");
            y1 = (float)parser.pullAsInt(prefix + "y1");
            x2 = (float)parser.pullAsInt(prefix + "x2");
            y2 = (float)parser.pullAsInt(prefix + "y2");

			t_x1 = w ? ((float)x1) / (float)w : 0.0f;
			t_x2 = w ? ((float)x2) / (float)w : 0.0f;
			t_y1 = h ? ((float)y1) / (float)h : 0.0f;
			t_y2 = h ? ((float)y2) / (float)h : 0.0f;

			x2 -= (float)w;
			y2 -= (float)h;

            borderSize *= parser.pullAsFloat(prefix + "scale", 1.0f);		// Allow scaling the widgets

			x1 *= borderSize;
			y1 *= borderSize;
			x2 *= borderSize;
			y2 *= borderSize;
			sw = (float)w * borderSize;
			sh = (float)h * borderSize;
		}
	}



	void SKIN_OBJECT::draw(const float X1, const float Y1, const float X2, const float Y2, const bool bkg) const
	{
		const float l = X2 + x2 - X1 - x1;
		const float f = l < 0.0f ? (X2 - X1) / (x1 - x2) : 1.0f;
		const float fu = (1.0f - f * (1.0f - t_x2)) / t_x2;

        gfx->drawtexture(tex, X1, Y1, X1 + f * x1, Y1 + y1, 0.0f, 0.0f, f * t_x1, t_y1, makecol(0xFF,0xFF,0xFF));
        gfx->drawtexture(tex, X1 + f * x1, Y1, X2 + f * x2, Y1 + y1, f * t_x1, 0.0f, fu * t_x2, t_y1, makecol(0xFF,0xFF,0xFF));
        gfx->drawtexture(tex, X2 + f * x2, Y1, X2, Y1 + y1, fu * t_x2, 0.0f, 1.0f, t_y1, makecol(0xFF,0xFF,0xFF));

        gfx->drawtexture(tex, X1, Y1 + y1, X1 + f * x1, Y2 + y2, 0.0f, t_y1, f * t_x1, t_y2, makecol(0xFF,0xFF,0xFF));
        gfx->drawtexture(tex, X2 + f * x2, Y1 + y1, X2, Y2 + y2, fu * t_x2, t_y1, 1.0f, t_y2, makecol(0xFF,0xFF,0xFF));

        gfx->drawtexture(tex, X1, Y2 + y2, X1 + f * x1, Y2, 0.0f, t_y2, f * t_x1, 1.0f, makecol(0xFF,0xFF,0xFF));
        gfx->drawtexture(tex, X1 + f * x1, Y2 + y2, X2 + f * x2, Y2, f * t_x1, t_y2, fu * t_x2, 1.0f, makecol(0xFF,0xFF,0xFF));
        gfx->drawtexture(tex, X2 + f * x2, Y2 + y2, X2, Y2, fu * t_x2, t_y2, 1.0f, 1.0f, makecol(0xFF,0xFF,0xFF));

		if (bkg && l > 0.0f)
            gfx->drawtexture(tex, X1 + x1, Y1 + y1, X2 + x2, Y2 + y2, t_x1, t_y1, t_x2, t_y2, makecol(0xFF,0xFF,0xFF));
	}




} // namespace Gui
} // namespace TA3D
