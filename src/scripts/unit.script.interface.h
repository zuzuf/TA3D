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

#ifndef __UNIT_SCRIPT_INTERFACE_H__
#define __UNIT_SCRIPT_INTERFACE_H__

# include "script.interface.h"
# include "../misc/hash_table.h"

namespace TA3D
{
    /*!
    ** This class is an interface for all unit scripts types
    */
    class UNIT_SCRIPT_INTERFACE : public SCRIPT_INTERFACE
    {
    protected:
        uint32                  unitID;
        UTILS::cHashTable<int>  return_value;
    public:

        virtual void setUnitID(uint32 ID) = 0;

        virtual int getNbPieces() = 0;

        int getReturnValue(const String &name);
        void setReturnValue(const String &name, int value);

    public:
        static UNIT_SCRIPT_INTERFACE *instanciate( SCRIPT_DATA *data );
    };
}

#endif
