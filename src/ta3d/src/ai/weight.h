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

#ifndef __WEIGHT_H__
# define __WEIGHT_H__

# include <stdafx.h>
# include <vector>


namespace TA3D
{

#define AI_UNIT_TYPE_BUILDER	0x0
#define AI_UNIT_TYPE_FACTORY	0x1
#define AI_UNIT_TYPE_ARMY		0x2
#define AI_UNIT_TYPE_DEFENSE	0x3
#define AI_UNIT_TYPE_ENEMY		0x4
#define AI_UNIT_TYPE_METAL		0x5
#define AI_UNIT_TYPE_ENERGY		0x6

#define NB_AI_UNIT_TYPE			0x7

#define AI_FLAG_BUILDER		0x01
#define AI_FLAG_FACTORY		0x02
#define AI_FLAG_ARMY		0x04
#define AI_FLAG_DEFENSE		0x08
#define AI_FLAG_METAL_P		0x10			// Producers
#define AI_FLAG_ENERGY_P	0x20
#define AI_FLAG_METAL_S		0x40			// Storage
#define AI_FLAG_ENERGY_S	0x80

	class AiWeight
    {
    public:
        uint16			nb;						// Number of units of this type the AI has
        float			w;						// Weight given to this unit
        float			o_w;					// Remember w for possible changes
        byte			type;					// Builder, factory, army, defense, ...
		std::vector<uint16> built_by;			// Who can build it
        float			army;
        float			defense;
        float			metal_p;
        float			energy_p;
        float			metal_s;
        float			energy_s;

		AiWeight();
		~AiWeight();
    };

	class WeightCoef
    {
    public:
		uint32	idx;
        uint32	c;

		WeightCoef();
		WeightCoef( uint32 a, uint32 b );
    };

	inline bool operator<( const WeightCoef &a, const WeightCoef &b )		{	return a.c > b.c;	}

} // namespace TA3D

#endif // __WEIGHT_H__
