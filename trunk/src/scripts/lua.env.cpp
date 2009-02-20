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

    void LUA_ENV::set_global_string( const String &name, const String &value )
    {
        pMutex.lock();
            lua_pushstring( L, value.c_str() );
            lua_getglobal( L, name.c_str() );
        pMutex.unlock();
    }

    void LUA_ENV::set_global_number( const String &name, const double value )
    {
        pMutex.lock();
            lua_pushnumber( L, value );
            lua_getglobal( L, name.c_str() );
        pMutex.unlock();
    }

    void LUA_ENV::set_global_boolean( const String &name, const bool value )
    {
        pMutex.lock();
            lua_pushboolean( L, value );
            lua_getglobal( L, name.c_str() );
        pMutex.unlock();
    }

    String LUA_ENV::get_global_string( const String &name )
    {
        pMutex.lock();
            lua_getglobal( L, name.c_str() );
            String result = lua_tostring( L, -1 ) ? String( lua_tostring( L, -1 ) ) : "";
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    double LUA_ENV::get_global_number( const String &name )
    {
        pMutex.lock();
            lua_getglobal( L, name.c_str() );
            double result = lua_tonumber( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::get_global_boolean( const String &name )
    {
        pMutex.lock();
            lua_getglobal( L, name.c_str() );
            bool result = lua_toboolean( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::is_global_string( const String &name )
    {
        pMutex.lock();
            lua_getglobal( L, name.c_str() );
            bool result = lua_isstring( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::is_global_number( const String &name )
    {
        pMutex.lock();
            lua_getglobal( L, name.c_str() );
            bool result = lua_isnumber( L, -1 );
            lua_pop( L, 1 );
        pMutex.unlock();
        return result;
    }

    bool LUA_ENV::is_global_boolean( const String &name )
    {
        pMutex.lock();
            lua_getglobal( L, name.c_str() );
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
}
