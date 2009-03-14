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

#ifndef __LUA_THREAD_H__
#define __LUA_THREAD_H__

# include "../misc/vector.h"
# include "../lua/lua.hpp"
# include "../threads/thread.h"
# include "lua.chunk.h"
# include "script.interface.h"

namespace TA3D
{
    /*!
    ** \brief functions for vector interface with Lua
    */
    void lua_pushvector( lua_State *L, const Vector3D &vec );
    Vector3D lua_tovector( lua_State *L, int idx );

    /*!
    ** \brief functions for color interface with Lua
    */
    void lua_pushcolor( lua_State *L, const uint32 color );
    uint32 lua_tocolor( lua_State *L, int idx );

    /*!
    ** \brief loads a Lua script, preprocessing to allow including other files and use defines
    */
    byte *loadLuaFile(const String &filename, uint32 &filesize);

    /*!
    ** This class represents a basic Lua thread without specialization
    ** To use it, create a new class that inherits LUA_THREAD
    */
    class LUA_THREAD : public Thread, public SCRIPT_INTERFACE
    {
        friend class LUA_CHUNK;
        friend class UNIT_SCRIPT;
    protected:
        byte        *buffer;
        lua_State   *L;             // The Lua state
        int         n_args;         // Number of arguments given to lua_resume

        String      name;

    public:

        LUA_THREAD();
        virtual ~LUA_THREAD();

        void init();
        void destroy();

        void load(const String &filename);                    // Load a lua script
        virtual void load(SCRIPT_DATA *data);
        LUA_CHUNK *dump();

        virtual int run(float dt, bool alone = false);      // Run the script
        int run();                          // Run the script with default delay

        //! functions used to call/run Lua functions
        void call(const String &functionName, int *parameters = NULL, int nb_params = 0);
        int execute(const String &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        virtual LUA_THREAD *fork();
        virtual LUA_THREAD *fork(const String &functionName, int *parameters = NULL, int nb_params = 0);
        virtual LUA_THREAD *fork(lua_State *cL, int n);

        //! functions used to save/restore scripts state
        virtual void save_thread_state(gzFile file);
        virtual void restore_thread_state(gzFile file);
    private:
        //! functions to manipulate the Lua processes
        void setThreadID();

    private:
        //! functions that register new Lua functions
        void register_basic_functions();
        virtual void register_functions()   {}
        virtual void register_info()        {}

    protected:
        virtual void proc(void* param);
    };

    /*!
    ** \brief returns a pointer to the current thread, or NULL on error
    */
    LUA_THREAD *lua_threadID( lua_State *L );
}

#endif
