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
    class LUA_THREAD : public Thread, public ObjectSync
    {
    protected:
        byte        *buffer;
        lua_State   *L;             // The Lua state
        int         n_args;         // Number of arguments given to lua_resume

        //! Variables to control thread execution
        int         last;           // Last timer check
        bool        running;

        float       sleep_time;     // Time to wait
        bool        sleeping;       // Is the thread paused ?
        bool        waiting;        // Is the thread waiting for some user action ?

        uint32      signal_mask;    // This thread will be killed as soon as it catchs this signal
        LUA_THREAD  *caller;        // NULL if main thread
        std::vector<LUA_THREAD*> childs;    // Child processes, empty for childs this is to keep track of running threads
        String      name;

    public:

        LUA_THREAD();
        inline ~LUA_THREAD()    {   destroy();  }

        void init();
        void destroy();

        //! stops definitely the thread
        void kill();
        void stop();
        void sleep(float time);
        void resume();

        void load(const String &filename);                    // Load a lua script
        void load(LUA_CHUNK *chunk);
        LUA_CHUNK *dump();

        int run(float dt);                   // Run the script
        int run();                           // Run the script, using default delay

        inline bool is_running() { return running || !childs.empty(); }
        inline bool is_waiting() { return waiting; }
        inline bool is_sleeping() { return sleeping; }

        //! functions used to call/run Lua functions
        void call(const String &functionName, int *parameters = NULL, int nb_params = 0);
        int execute(const String &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        LUA_THREAD *fork();
        LUA_THREAD *fork(const String &functionName, int *parameters = NULL, int nb_params = 0);
        LUA_THREAD *fork(lua_State *cL, int n);
    private:
        //! functions to manipulate the Lua processes
        void addThread(LUA_THREAD *pChild);
        void removeThread(LUA_THREAD *pChild);
        void setThreadID();
    public:
        void clean();
        void processSignal(uint32 signal);
        void setSignalMask(uint32 signal);
        uint32 getSignalMask();

    private:
        //! functions that register new Lua functions
        void register_basic_functions();
        virtual void register_functions()   {}
        virtual void register_info()   {}

    protected:
        virtual void proc(void* param);
    };

    /*!
    ** \brief returns a pointer to the current thread, or NULL on error
    */
    LUA_THREAD *lua_threadID( lua_State *L );
}

#endif
