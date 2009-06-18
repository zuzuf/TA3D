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
#include "lua.data.h"
#include "unit.script.h"
#include "lua.thread.h"

using namespace std;

namespace TA3D
{
    LuaData::LuaData()
    {
        init();
    }

    LuaData::~LuaData()
    {
        destroy();
    }

    String LuaData::getName()
    {
        return name;
    }

    void LuaData::load(const String &filename)                    // Load a lua chunk
    {
        destroy();

        LuaThread *thread = new LuaThread;
        thread->load(filename);

        LuaChunk *chunk = thread->dump();
        chunk->load(UnitScript::luaVM());

        delete thread;
        delete chunk;

        lua_State *L = UnitScript::luaVM();

        lua_call(L, 0, 0);

        lua_getglobal(L, "__name");
        name = lua_isstring(L, -1) ? String(lua_tostring(L, -1)) : String();
        lua_pop(L, 1);

        lua_getglobal(L, name.c_str());
        lua_getfield(L, -1, "__piece_list");
        if (lua_istable(L, -1))
        {
            piece_name.resize(lua_objlen(L, -1));
            for(int i = 1 ; i <= piece_name.size() ; i++)
            {
                lua_rawgeti(L, -1, i);
                piece_name[i - 1] = lua_tostring(L, -1);
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 2);
    }

    void LuaData::init()
    {
        name.clear();
        piece_name.clear();
    }

    void LuaData::destroy()
    {
        init();
    }

    int LuaData::identify(const String &name)
    {
        String query = String::ToLower(name);
        for(int i = 0 ; i < piece_name.size() ; i++)
            if (piece_name[i] == query)
                return i;
        return -1;
    }
}
