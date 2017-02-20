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
#include "script.data.h"
#include "cob.h"
#include "lua.chunk.h"
#include "lua.data.h"

namespace TA3D
{


	ScriptData *ScriptData::loadScriptFile(const QString &filename)
	{
        const QString &tmp_cob = filename + ".cob";
        const QString &tmp_lua = filename + ".lua";
		if (VFS::Instance()->fileExists(tmp_lua) && VFS::Instance()->filePriority(tmp_lua) >= VFS::Instance()->filePriority(tmp_cob))
		{
			ScriptData *script = new LuaData;
			script->load(tmp_lua);
			return script;
		}

		if (VFS::Instance()->fileExists(tmp_cob))
		{
			ScriptData *script = new CobScript;
			script->load(tmp_cob);
			return script;
		}
		return NULL;
	}


}
