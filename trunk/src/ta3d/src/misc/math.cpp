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
#include <cstdlib>
#include <ctime>
#include "math.h"



namespace TA3D
{
namespace Math
{

	// Our pre-cached random numbers table
	PreCachedRandomNumbers  RandomTable;



    uint32 Log2(uint32 n)
    {
        uint32 p(0);
        n >>= 1;
        while (n)
        {
            ++p;
            n >>= 1;
        }
        return p;
    }


	PreCachedRandomNumbers::PreCachedRandomNumbers()
	{
		reset();
	}

	void PreCachedRandomNumbers::reset()
	{
		srand((unsigned int)time(NULL));
		for(size_t i = 0U ; i < TA3D_MATH_RANDOM_TABLE_SIZE ; ++i)
			pCache[i] = TA3D_RAND();
	}


} // namespace Math
} // namespace TA3D
