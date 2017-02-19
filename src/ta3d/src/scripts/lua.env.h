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

#ifndef __LuaEnv_H__
#define __LuaEnv_H__

#ifndef __LUA_INCLUDES__
#define __LUA_INCLUDES__
#ifdef LUA_NOJIT
# include "../lua/lua.hpp"
#else
# include "../luajit/src/lua.hpp"
#endif
#endif
# include <threads/thread.h>
# include <zuzuf/smartptr.h>

namespace TA3D
{
    /*!
    ** This class manages the Lua global environment and ensures Lua threads
    ** can access it safely
    */
    class LuaEnv : public ObjectSync
    {
	public:
        typedef zuzuf::smartptr<LuaEnv>	Ptr;
    protected:
        lua_State   *L;             // The global Lua state
    public:

        LuaEnv();
        ~LuaEnv();

        void set_global_string( const char *name, const char *value );
        void set_global_number( const char *name, const double value );
        void set_global_boolean( const char *name, const bool value );
        const char *get_global_string( const char *name );
        double get_global_number( const char *name );
        bool get_global_boolean( const char *name );
        bool is_global_string( const char *name );
        bool is_global_number( const char *name );
        bool is_global_boolean( const char *name );

    public:
		static LuaEnv::Ptr global;
		static LuaEnv::Ptr instance();
        static void destroy();

        static void register_global_functions( lua_State *L );

        static int global_set_global_string( lua_State *L );
        static int global_set_global_number( lua_State *L );
        static int global_set_global_boolean( lua_State *L );
        static int global_get_global_string( lua_State *L );
        static int global_get_global_number( lua_State *L );
        static int global_get_global_boolean( lua_State *L );
        static int global_is_global_string( lua_State *L );
        static int global_is_global_number( lua_State *L );
        static int global_is_global_boolean( lua_State *L );
    };

}

#endif
