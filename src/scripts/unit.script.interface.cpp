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
#include "unit.script.interface.h"
#include "cob.vm.h"
#include "unit.script.h"

namespace TA3D
{
    UNIT_SCRIPT_INTERFACE *UNIT_SCRIPT_INTERFACE::instanciate( SCRIPT_DATA *data )
    {
        UNIT_SCRIPT_INTERFACE *usi = NULL;

        if ( dynamic_cast<COB_SCRIPT*>(data) )          // Try COB_SCRIPT (OTA COB/BOS)
            usi = new COB_VM();
        else if ( dynamic_cast<UNIT_SCRIPT*>(data) )    // Try UNIT_SCRIPT (Lua)
            usi = new UNIT_SCRIPT();

        usi->load( data );
        return usi;
    }
}
