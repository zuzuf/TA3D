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

#include "../stdafx.h"
#include "math.h"
#include <time.h>
#include "../logs/logs.h"


//! The size of the random table
# define TA3D_RANDOM_TABLE_SIZE  0x100000
//! The mask used to get the next position in the random
# define TA3D_RANDOM_TABLE_MASK  0xFFFFF



namespace TA3D
{
namespace Math
{

    namespace
    {
        //! \name Random table
        //@{

        //! Current position in the random table
        int gRandomTablePosition = 0;

        //! The Random table
        uint32 gRandomTableData[TA3D_RANDOM_TABLE_SIZE];

        //@}
    }


    void InitializeRandomTable()
    {
        LOG_DEBUG("Initializing the random table...");
        // Rebuil the table
        srand(time(NULL));
        for (int i = 0; i < TA3D_RANDOM_TABLE_SIZE; ++i)
		    gRandomTableData[i] = TA3D_RAND();
        // Reset the position in this table
        gRandomTablePosition = 0;
        LOG_DEBUG("Initializing the random table: Done.");
    }

    uint32 RandFromTable()
    {
        // The next position in the table
        gRandomTablePosition = (gRandomTablePosition + 1) & TA3D_RANDOM_TABLE_MASK;
        // The random value
        return gRandomTableData[gRandomTablePosition];
    }

    uint32 Log2(uint32 n)
    {
        uint32 p(0);
        n >>= 1;
        while(n)
        {
            p++;
            n >>= 1;
        }
        return p;
    }


} // namespace Math
} // namespace TA3D
