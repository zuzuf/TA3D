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
#include "lua.env.h"

namespace TA3D
{
    LUA_ENV::LUA_ENV()
    {
        L = lua_open();
    }

    LUA_ENV::~LUA_ENV()
    {
        lua_close( L );
    }

    void LUA_ENV::set_global_string( const char *name, const char *value )
    {
        pMutex.lock();
            lua_pushstring( L, value );
            lua_getglobal( L, name );
        pMutex.unlock();
    }

    void LUA_ENV::set_global_number( const char *name, const double value )
    {
        pMutex.lock();
            lua_pushnumber( L, value );
            lua_getglobal( L, name );
        pMutex.unlock();
    }

    void LUA_ENV::set_global_boolean( const char *name, const bool value )
    {
        pMutex.lock();
            lua_pushboolean( L, value );
            lua_getglobal( L, name );
        pMutex.unlock();
    }

    const char *LUA_ENV::get_global_string( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            const char *result = lua_tostring( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    double LUA_ENV::get_global_number( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            double result = lua_tonumber( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::get_global_boolean( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_toboolean( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::is_global_string( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_isstring( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::is_global_number( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_isnumber( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::is_global_boolean( const char *name )
    {
        pMutex.lock();
            lua_getglobal( L, name );
            bool result = lua_isboolean( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    LUA_ENV *LUA_ENV::global = NULL;

    LUA_ENV *LUA_ENV::instance()
    {
        if (global == NULL)
            global = new LUA_ENV();
        return global;
    }

    void LUA_ENV::destroy()
    {
        if (global)
            delete global;
        global = NULL;
    }

    void LUA_ENV::register_global_functions( lua_State *L )
    {
        lua_register( L, "set_global_string", LUA_ENV::global_set_global_string);
        lua_register( L, "set_global_number", LUA_ENV::global_set_global_number);
        lua_register( L, "set_global_boolean", LUA_ENV::global_set_global_boolean);

        lua_register( L, "get_global_string", LUA_ENV::global_get_global_string);
        lua_register( L, "get_global_number", LUA_ENV::global_get_global_number);
        lua_register( L, "get_global_boolean", LUA_ENV::global_get_global_boolean);

        lua_register( L, "is_global_string", LUA_ENV::global_is_global_string);
        lua_register( L, "is_global_number", LUA_ENV::global_is_global_number);
        lua_register( L, "is_global_boolean", LUA_ENV::global_is_global_boolean);
    }

    int LUA_ENV::global_set_global_string( lua_State *L )
    {
        instance()->set_global_string(lua_tostring(L, -2), lua_tostring(L, -1));
        lua_pop(L, 2);
        return 0;
    }

    int LUA_ENV::global_set_global_number( lua_State *L )
    {
        instance()->set_global_number(lua_tostring(L, -2), lua_tonumber(L, -1));
        lua_pop(L, 2);
        return 0;
    }

    int LUA_ENV::global_set_global_boolean( lua_State *L )
    {
        instance()->set_global_boolean(lua_tostring(L, -2), lua_toboolean(L, -1));
        lua_pop(L, 2);
        return 0;
    }

    int LUA_ENV::global_get_global_string( lua_State *L )
    {
        const char *str = instance()->get_global_string(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushstring(L, str);
        return 1;
    }

    int LUA_ENV::global_get_global_number( lua_State *L )
    {
        double nb = instance()->get_global_number(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushnumber(L, nb);
        return 1;
    }

    int LUA_ENV::global_get_global_boolean( lua_State *L )
    {
        bool b = instance()->get_global_boolean(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }

    int LUA_ENV::global_is_global_string( lua_State *L )
    {
        bool b = instance()->is_global_string(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }

    int LUA_ENV::global_is_global_number( lua_State *L )
    {
        bool b = instance()->is_global_number(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }

    int LUA_ENV::global_is_global_boolean( lua_State *L )
    {
        bool b = instance()->is_global_boolean(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pushboolean(L, b);
        return 1;
    }
}
