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
#include "lua.env.h"

namespace TA3D
{
    LuaEnv::LuaEnv()
    {
        L = lua_open();
    }

    LuaEnv::~LuaEnv()
    {
        lua_close( L );
    }

    void LuaEnv::set_global_string( const char *name, const char *value )
    {
        pMutex.lock();
            lua_pushstring( L, value );
            lua_setglobal( L, name );
        pMutex.unlock();
    }

    void LuaEnv::set_global_number( const char *name, const double value )
    {
        pMutex.lock();
            lua_pushnumber( L, value );
            lua_setglobal( L, name );
        pMutex.unlock();
    }

    void LuaEnv::set_global_boolean( const char *name, const bool value )
    {
        pMutex.lock();
            lua_pushboolean( L, value );
            lua_setglobal( L, name );
        pMutex.unlock();
    }

    const char *LuaEnv::get_global_string( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            const char *result = lua_tostring( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    double LuaEnv::get_global_number( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            double result = lua_tonumber( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LuaEnv::get_global_boolean( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_toboolean( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LuaEnv::is_global_string( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_isstring( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LuaEnv::is_global_number( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_isnumber( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LuaEnv::is_global_boolean( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_isboolean( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

	LuaEnv::Ptr LuaEnv::global = (LuaEnv*)NULL;

	LuaEnv::Ptr LuaEnv::instance()
    {
		if (!global)
            global = new LuaEnv();
        return global;
    }

    void LuaEnv::destroy()
    {
		global = (LuaEnv*)NULL;
    }

    void LuaEnv::register_global_functions( lua_State *L )
    {
        lua_register( L, "set_global_string", LuaEnv::global_set_global_string);
        lua_register( L, "set_global_number", LuaEnv::global_set_global_number);
        lua_register( L, "set_global_boolean", LuaEnv::global_set_global_boolean);

        lua_register( L, "get_global_string", LuaEnv::global_get_global_string);
        lua_register( L, "get_global_number", LuaEnv::global_get_global_number);
        lua_register( L, "get_global_boolean", LuaEnv::global_get_global_boolean);

        lua_register( L, "is_global_string", LuaEnv::global_is_global_string);
        lua_register( L, "is_global_number", LuaEnv::global_is_global_number);
        lua_register( L, "is_global_boolean", LuaEnv::global_is_global_boolean);
    }

    int LuaEnv::global_set_global_string( lua_State *L )
    {
        instance()->set_global_string(lua_tostring(L, -2), lua_tostring(L, -1));
        lua_pop(L, 2);
        return 0;
    }

    int LuaEnv::global_set_global_number( lua_State *L )
    {
        instance()->set_global_number(lua_tostring(L, -2), lua_tonumber(L, -1));
        lua_pop(L, 2);
        return 0;
    }

    int LuaEnv::global_set_global_boolean( lua_State *L )
    {
        instance()->set_global_boolean(lua_tostring(L, -2), lua_toboolean(L, -1));
        lua_pop(L, 2);
        return 0;
    }

    int LuaEnv::global_get_global_string( lua_State *L )
    {
        const char *str = instance()->get_global_string(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushstring(L, str);
        return 1;
    }

    int LuaEnv::global_get_global_number( lua_State *L )
    {
        double nb = instance()->get_global_number(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushnumber(L, nb);
        return 1;
    }

    int LuaEnv::global_get_global_boolean( lua_State *L )
    {
        bool b = instance()->get_global_boolean(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }

    int LuaEnv::global_is_global_string( lua_State *L )
    {
        bool b = instance()->is_global_string(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }

    int LuaEnv::global_is_global_number( lua_State *L )
    {
        bool b = instance()->is_global_number(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }

    int LuaEnv::global_is_global_boolean( lua_State *L )
    {
        bool b = instance()->is_global_boolean(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }
}
