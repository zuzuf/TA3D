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
#include <TA3D_NameSpace.h>
#include "text.color.h"



namespace TA3D
{
namespace Gui
{


	TEXT_COLOR::TEXT_COLOR()
	{
		font_color = makeacol(0xFF,0xFF,0xFF,0xFF);
		shadow_color = makeacol(0,0,0,0xFF);
		shadow = false;
		shadow_dx = 1.0f;
		shadow_dy = 1.0f;
	}


	void TEXT_COLOR::load(TDFParser& parser, const QString &prefix, float scale)
	{
        font_color = parser.pullAsColor( prefix + "font_color", makeacol(0xFF,0xFF,0xFF,0xFF) );
        shadow_color = parser.pullAsColor( prefix + "shadow_color", makeacol(0,0,0,0xFF) );
        shadow = parser.pullAsBool( prefix + "shadow", false );
        shadow_dx = parser.pullAsFloat( prefix + "shadow_dx", 1.0f ) * scale;
        shadow_dy = parser.pullAsFloat( prefix + "shadow_dy", 1.0f ) * scale;
	}


	void TEXT_COLOR::print(Font *font, const float x, const float y, const QString &text) const
	{
        if (!text.isEmpty())
		{
			if (!font)
			{
				LOG_WARNING(LOG_PREFIX_GFX << "font == NULL !! cannot render text");
				return;
			}

			if (shadow)
                font->print(x + shadow_dx,
                            y + shadow_dy,
                            shadow_color, text);
            font->print(x, y, font_color, text);
		}
	}


	void TEXT_COLOR::print(Font *font, const float x, const float y, const uint32 col, const QString &text) const
	{
        if (!text.isEmpty())
		{
			if (!font)
			{
				LOG_WARNING(LOG_PREFIX_GFX << "font == NULL !! cannot render text");
				return;
			}

			if (shadow)
                font->print(x + shadow_dx, y + shadow_dy, shadow_color, text);

            font->print(x, y, col, text);
		}
	}




} // namespace Gui
} // namespace TA3D

