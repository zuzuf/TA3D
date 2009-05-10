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
#ifndef __TA3D_TEXT_COLOR_H__
#define __TA3D_TEXT_COLOR_H__

#include "../../misc/tdf.h"
#include "../../misc/string.h"
#include "../font.h"


namespace TA3D
{

class TEXT_COLOR
{
public:
	TEXT_COLOR();

	void load(TA3D::TDFParser& parser, const String &prefix, float scale);

	void print(Font *font, float x, float y, const String &text);

	void print(Font *font, float x, float y, uint32 col, const String &text);

public:
	uint32      font_color;
	uint32      shadow_color;
	bool        shadow;
	float       shadow_dx;
	float       shadow_dy;
};

} // namespace TA3D


#endif
