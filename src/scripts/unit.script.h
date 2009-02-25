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

#ifndef __UNIT_SCRIPT_H__
#define __UNIT_SCRIPT_H__

# include "lua.thread.h"

namespace TA3D
{
    /*!
    ** This class represents unit scripts, it's used to script unit behavior
    ** This is a Lua version of TA COB/BOS scripts
    */
    class UNIT_SCRIPT : public LUA_THREAD
    {
    private:
        uint32      unitID;
    public:

        UNIT_SCRIPT(int unitID);
        ~UNIT_SCRIPT();

    public:
        virtual void register_functions();
        virtual void register_info();
    };

}

#endif
