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
#include "../TA3D_NameSpace.h"
#include "script.data.h"
#include "cob.h"
#include "lua.chunk.h"

namespace TA3D
{
    SCRIPT_DATA *SCRIPT_DATA::loadScriptFile(const String &filename)
    {
        String tmp = filename + ".lua";
        if (TA3D::VARS::HPIManager->Exists(tmp))
        {
            SCRIPT_DATA *script = new LUA_CHUNK;
            script->load(tmp);
            return script;
        }

        tmp = filename + ".cob";
        if (TA3D::VARS::HPIManager->Exists(tmp))
        {
            SCRIPT_DATA *script = new COB_SCRIPT;
            script->load(tmp);
            return script;
        }
        return NULL;
    }
}
