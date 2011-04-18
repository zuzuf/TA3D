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
#include "weight.h"

namespace TA3D
{
	AiWeight::AiWeight() : built_by()
    {
        nb = 0;
        w = 0.0f;
        o_w = 0.0f;
        type = 0;
        army = 0.0f;
        defense = 0.0f;
        metal_p = 0.0f;
        energy_p = 0.0f;
        metal_s = 0.0f;
        energy_s = 0.0f;
    }

	AiWeight::~AiWeight()
    {
        built_by.clear();
    }

	WeightCoef::WeightCoef()	{	idx = 0;	c = 0;	}
	WeightCoef::WeightCoef( uint32 a, uint32 b )	{	idx = a;	c = b;	}
}
