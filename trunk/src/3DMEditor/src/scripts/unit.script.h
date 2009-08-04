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

#ifndef __UnitScript_H__
#define __UnitScript_H__

# include <QtCore>
# include "unit.script.interface.h"
# include "../../../ta3d/src/lua/lua.hpp"

namespace TA3D
{
    /*!
    ** This class represents unit scripts, it's used to script unit behavior
    ** This is a Lua version of TA COB/BOS scripts
    */
    class UnitScript : public UnitScriptInterface
    {
        friend class LuaData;
        virtual const char *className() { return "UnitScript"; }
    public:

        UnitScript();
        virtual ~UnitScript();

        /*virtual*/ void load(ScriptData *data);
        /*virtual*/ int run(float dt, bool alone = false);                  // Run the script

        //! functions used to call/run Lua functions
        /*virtual*/ void call(const String &functionName, int *parameters = NULL, int nb_params = 0);
        /*virtual*/ int execute(const String &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        /*virtual*/ UnitScript *fork();
        /*virtual*/ UnitScript *fork(const String &functionName, int *parameters = NULL, int nb_params = 0);
        UnitScript *fork(lua_State *cL, int n);

        //! functions used to save/restore scripts state
        /*virtual*/ void save_thread_state(gzFile file);
        /*virtual*/ void restore_thread_state(gzFile file);

        int getNextID();

    private:
        lua_State *L;

        int nextID;

        int n_args;

        String name;

    private:
        static void register_functions(lua_State *L);
        void register_info();

        /*virtual*/ void setUnitID(uint32 ID);
        /*virtual*/ int getNbPieces();

        void lua_getUnitTable();

        void init();
        void destroy();
    private:
        static lua_State *pLuaVM;
        static lua_State *luaVM();
    };

}

#endif
