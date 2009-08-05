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

#ifndef __LuaThread_H__
#define __LuaThread_H__

# include <QtCore>
# include "../misc/vector.h"
# include "../../../ta3d/src/lua/lua.hpp"
# include "script.interface.h"
# include "../logs.h"

namespace TA3D
{
    /*!
    ** \brief functions for vector interface with Lua
    */
    void lua_pushvector( lua_State *L, const Vector3D &vec );
    Vector3D lua_tovector( lua_State *L, int idx );

    /*!
    ** \brief loads a Lua script, preprocessing to allow including other files and use defines
    */
    byte *loadLuaFile(const QString &filename, uint32 &filesize);

    /*!
    ** This class represents a basic Lua thread without specialization
    ** To use it, create a new class that inherits LuaThread
    */
    class LuaThread : public ScriptInterface
    {
        virtual const char *className() { return "LuaThread"; }
        friend class UnitScript;
    protected:
        byte        *buffer;
        lua_State   *L;             // The Lua state
        int         n_args;         // Number of arguments given to lua_resume

        QString     name;

        bool        crashed;

        int         nextID;

    public:

        LuaThread();
        virtual ~LuaThread();

        int getMem();

        inline int getNextID()
        {
            if (caller)
                return static_cast<LuaThread*>(caller)->getNextID();
            return nextID++;
        }

        void init();
        void destroy();

        void load(const QString &filename);                    // Load a lua script

        virtual int run(float dt, bool alone = false);      // Run the script
        int run();                          // Run the script with default delay

        //! functions used to call/run Lua functions
        void call(const QString &functionName, int *parameters = NULL, int nb_params = 0);
        int execute(const QString &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        virtual LuaThread *fork();
        virtual LuaThread *fork(const QString &functionName, int *parameters = NULL, int nb_params = 0);
        virtual LuaThread *fork(lua_State *cL, int n);

        //! functions used for debugging
        inline bool is_crashed()    {   return crashed;  }
        inline void crash()         {   crashed = true;  }
        inline void uncrash()       {   crashed = false; }
        bool runCommand(const QString &cmd);
    private:
        //! functions to manipulate the Lua processes
        void setThreadID();

    private:
        //! functions that register new Lua functions
        void register_basic_functions();
        virtual void register_functions()   {}
        virtual void register_info()        {}
    };

    int thread_logmsg( lua_State *L );
    int thread_mouse_x( lua_State *L );
    int thread_mouse_y( lua_State *L );
    int thread_mouse_z( lua_State *L );
    int thread_mouse_b( lua_State *L );
    int thread_time( lua_State *L );
    int thread_signal( lua_State *L );
    int thread_start_script( lua_State *L );

    /*!
    ** \brief returns a pointer to the current thread, or NULL on error
    */
    LuaThread *lua_threadID( lua_State *L );
}

#endif
