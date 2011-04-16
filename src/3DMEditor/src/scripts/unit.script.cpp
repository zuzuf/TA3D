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

#include "unit.script.h"
#include "lua.thread.h"
#include "../mesh.h"

namespace TA3D
{
    UnitScript *UnitScript::pInstance = NULL;
    UnitScript *UnitScript::instance()
    {
        if (!pInstance)
            pInstance = new UnitScript;
        return pInstance;
    }

    lua_State *UnitScript::pLuaVM = NULL;

	inline UnitScript *lua_scriptID( lua_State *L, int pos )
	{
        UnitScript *p = UnitScript::instance();
		return p;
	}

	int unit_thread_signal( lua_State *L )       // signal( unitID, sig )
	{
		UnitScript *lua_script = lua_scriptID(L, 1);
		if (lua_script)
			lua_script->processSignal( lua_tointeger(L, 2) );
		lua_settop(L, 0);
		return 0;
	}

	int unit_thread_start_script( lua_State *L )         // start_script( unitID, function, params )
	{
		int n = lua_gettop(L);

		if (lua_isfunction(L, 2))
		{
			UnitScript *lua_script = lua_scriptID(L, 1);
			if (lua_script)
			{
				UnitScript *newThread = lua_script->fork(L, n - 1);
				newThread->setSignalMask( lua_script->getSignalMask() );
			}
		}
		lua_settop(L, 0);

		return 0;
	}

	int UnitScript::getNextID()
	{
		if (caller)
			return static_cast<UnitScript*>(caller)->getNextID();
		return nextID++;
	}

	lua_State *UnitScript::luaVM()
	{
		if (!pLuaVM)
		{
			pLuaVM = lua_open();				// Create a lua state object
			if (pLuaVM)
			{
				lua_register( pLuaVM, "logmsg", thread_logmsg );
				lua_register( pLuaVM, "mouse_x", thread_mouse_x );
				lua_register( pLuaVM, "mouse_y", thread_mouse_y );
				lua_register( pLuaVM, "mouse_z", thread_mouse_z );
				lua_register( pLuaVM, "mouse_b", thread_mouse_b );
				lua_register( pLuaVM, "time", thread_time );
				lua_register( pLuaVM, "_signal", unit_thread_signal );               // Those functions will be used through a wrapper using the object like call convention
				lua_register( pLuaVM, "_start_script", unit_thread_start_script );

#warning TODO : add code to register fake LuaEnv functions
//				LuaEnv::register_global_functions( pLuaVM );
				register_functions( pLuaVM );
			}
			else
				LOG_CRITICAL(LOG_PREFIX_LUA << "creating Lua VM for unit scripts failed");
		}

		return pLuaVM;
	}

	UnitScript::UnitScript() : nextID(0), n_args(0), L(NULL), name()
	{
	}

	UnitScript::~UnitScript()
	{
		destroy();
	}

	int unit_is_turning( lua_State *L )        // is_turning(unitID, obj_id, axis_id)
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		lua_settop(L, 0);
        lua_pushboolean(L, script_is_turning(obj, axis));
		return 1;
	}

	int unit_is_moving( lua_State *L )        // is_moving(unitID, obj_id, axis_id)
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		lua_settop(L, 0);
        lua_pushboolean(L, script_is_moving(obj, axis));
		return 1;
	}

	int unit_move( lua_State *L )           // move(unitID, obj_id, axis_id, target_pos, speed)
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		float pos = (float) lua_tonumber(L, 4);
		float speed = (float) lua_tonumber(L, 5);
		lua_settop(L, 0);
        script_move_object(obj, axis, pos, speed);
		return 0;
	}

	int unit_explode( lua_State *L )        // explode(unitID, obj_id, explosion_type)
	{
		int obj = lua_tointeger(L, 2);
		int type = lua_tointeger(L, 3);
		lua_settop(L, 0);
        script_explode(obj, type);
		return 0;
	}

	int unit_turn( lua_State *L )        // turn(unitID, obj_id, axis, angle, speed)
	{
		int obj = lua_tointeger(L, 2);
		int type = lua_tointeger(L, 3);
		float angle = (float) lua_tonumber(L, 4);
		float speed = (float) lua_tonumber(L, 5);
		lua_settop(L, 0);
        script_turn_object(obj, type, angle, speed);
		return 0;
	}

	int unit_get_value_from_port( lua_State *L )        // get_value_from_port(unitID, port)
	{
		int port = lua_tointeger(L, 2);
		lua_settop(L, 0);

        lua_pushinteger(L, script_get_value_from_port(port));
		return 1;
	}

	int unit_show( lua_State *L )        // show(unitID, obj_id)
	{
		int obj = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_show_object(obj);
		return 0;
	}

	int unit_hide( lua_State *L )        // hide(unitID, obj_id)
	{
		int obj = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_hide_object(obj);
		return 0;
	}

	int unit_cache( lua_State *L )          // cache(unitID, obj_id)
	{
		int obj = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_cache(obj);
		return 0;
	}

	int unit_dont_cache( lua_State *L )        // dont_cache(unitID, obj_id)
	{
		int obj = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_dont_cache(obj);
		return 0;
	}

	int unit_dont_shade( lua_State *L )          // dont_shade(unitID, obj_id)
	{
		int obj = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_dont_shade(obj);
		return 0;
	}

	int unit_shade( lua_State *L )          // shade(unitID, obj_id)
	{
		int obj = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_shade(obj);
		return 0;
	}

	int unit_emit_sfx( lua_State *L )           // emit_sfx(unitID, smoke_type, from_piece)
	{
		int smoke_type = lua_tointeger(L, 2);
		int from_piece = lua_tointeger(L, 3);
		lua_settop(L, 0);
        script_emit_sfx(smoke_type, from_piece);
		return 0;
	}

	int unit_spin( lua_State *L )               // spin(unitID, obj, axis, speed, (accel))
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		float speed = (float) lua_tonumber(L, 4);
		float accel = lua_isnoneornil(L, 5) ? 0.0f : (float) lua_tonumber(L, 5);
		lua_settop(L, 0);
        script_spin_object(obj, axis, speed, accel);
		return 0;
	}

	int unit_stop_spin( lua_State *L )               // stop_spin(unitID, obj, axis, (speed))
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		float speed = lua_isnoneornil(L,4) ? 0.0f : (float) lua_tonumber(L, 4);
		lua_settop(L, 0);
        script_stop_spin(obj, axis, speed);
		return 0;
	}

	int unit_move_piece_now( lua_State *L )           // move_piece_now(unitID, obj, axis, pos)
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		float pos = (float) lua_tonumber(L, 4);
		lua_settop(L, 0);
        script_move_piece_now(obj, axis, pos);
		return 0;
	}

	int unit_turn_piece_now( lua_State *L )           // turn_piece_now(unitID, obj, axis, angle)
	{
		int obj = lua_tointeger(L, 2);
		int axis = lua_tointeger(L, 3);
		float angle = (float) lua_tonumber(L, 4);
		lua_settop(L, 0);
        script_turn_piece_now(obj, axis, angle);
		return 0;
	}

	int unit_get( lua_State *L )           // get(unitID, type, v1, v2)
	{
		int type = lua_tointeger(L, 2);
		int v1 = lua_isnoneornil(L,3) ? 0 : lua_tointeger(L, 3);
		int v2 = lua_isnoneornil(L,4) ? 0 : lua_tointeger(L, 4);
		lua_settop(L, 0);
        lua_pushinteger( L, script_get(type, v1, v2) );
		return 1;
	}

	int unit_set_value( lua_State *L )           // set_value(unitID, type, v)
	{
		int type = lua_tointeger(L, 2);
		int v = lua_isboolean(L,3) ? lua_toboolean(L, 3) : lua_tointeger(L, 3);
		lua_settop(L, 0);
        script_set_value(type, v);
		return 0;
	}

	int unit_attach_unit( lua_State *L )           // attach_unit(unitID, unit_id, piece_id)
	{
		int unit_id = lua_tointeger(L, 2);
		int piece_id = lua_tointeger(L, 3);
		lua_settop(L, 0);
        script_attach_unit(unit_id, piece_id);
		return 0;
	}

	int unit_drop_unit( lua_State *L )           // drop_unit(unitID, unit_id)
	{
		int unit_id = lua_tointeger(L, 2);
		lua_settop(L, 0);
        script_drop_unit(unit_id);
		return 0;
	}

	int unit_unit_position( lua_State *L )           // unit_position(unitID, unit_id)
	{
		int unit_id = lua_tointeger(L, 2);
		lua_settop(L, 0);
        lua_pushvector( L, Vector3D() );
		return 1;
	}

	int unit_set_script_value( lua_State *L )       // set_script_value(unitID, script_name, value)
	{
        QString scriptName = lua_isstring(L, 2) ? QString(lua_tostring(L, 2)) : QString();
		int value = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : lua_tointeger(L, 3);
		lua_settop(L, 0);
        LOG_DEBUG(LOG_PREFIX_SCRIPT << "setReturnValue(\"" << scriptName << "\", " << value << ")");
		return 0;
	}

	void UnitScript::register_functions(lua_State *L)
	{
		lua_gc( L, LUA_GCSTOP, 0 );		// Load libraries
		luaopen_base( L );
		luaopen_math( L );
		lua_gc( L, LUA_GCRESTART, 0 );

		lua_register(L, "is_turning", unit_is_turning );                    // is_turning(unitID, obj_id, axis_id)
		lua_register(L, "is_moving", unit_is_moving );                      // is_moving(unitID, obj_id, axis_id)
		lua_register(L, "move", unit_move );                                // move(unitID, obj_id, axis_id, target_pos, speed)
		lua_register(L, "explode", unit_explode );                          // explode(unitID, obj_id, explosion_type)
		lua_register(L, "turn", unit_turn );                                // turn(unitID, obj_id, axis, angle, speed)
		lua_register(L, "get_value_from_port", unit_get_value_from_port );  // get_value_from_port(unitID, port)
		lua_register(L, "spin", unit_spin );                                // spin(unitID, obj_id, axis, speed, (accel))
		lua_register(L, "stop_spin", unit_stop_spin );                      // stop_spin(unitID, obj_id, axis, (speed))
		lua_register(L, "show", unit_show );                                // show(unitID, obj_id)
		lua_register(L, "hide", unit_hide );                                // hide(unitID, obj_id)
		lua_register(L, "emit_sfx", unit_emit_sfx );                        // emit_sfx(unitID, smoke_type, from_piece)
		lua_register(L, "move_piece_now", unit_move_piece_now );            // move_piece_now(unitID, obj, axis, pos)
		lua_register(L, "turn_piece_now", unit_turn_piece_now );            // turn_piece_now(unitID, obj, axis, angle)
		lua_register(L, "get", unit_get );                                  // get(unitID, type, v1, v2)
		lua_register(L, "set_value", unit_set_value );                      // set_value(unitID, type, v)
		lua_register(L, "set", unit_set_value );                            // set(unitID, type, v)
		lua_register(L, "attach_unit", unit_attach_unit );                  // attach_unit(unitID, unit_id, piece_id)
		lua_register(L, "drop_unit", unit_drop_unit );                      // drop_unit(unitID, unit_id)
		lua_register(L, "unit_position", unit_unit_position );              // unit_position(unitID, unit_id) = vector(x,y,z)
		lua_register(L, "cache", unit_cache );                              // cache(unitID, obj_id)
		lua_register(L, "dont_cache", unit_dont_cache );                    // dont_cache(unitID, obj_id)
		lua_register(L, "shade", unit_shade );                              // shade(unitID, obj_id)
		lua_register(L, "dont_shade", unit_dont_shade );                    // dont_shade(unitID, obj_id)
		lua_register(L, "set_script_value", unit_set_script_value );        // set_script_value(unitID, script_name, value)
	}

	void UnitScript::lua_getUnitTable()
	{
		lua_getglobal(L, "__units");
        lua_pushinteger(L, 0);
		lua_gettable(L, -2);
		lua_remove(L, -2);
	}

	void UnitScript::register_info()
	{
		if (L == NULL)
		{
			L = luaVM();
			lua_getglobal(L, "cloneUnitScript");
            lua_pushstring(L, name.toStdString().c_str());
            lua_pushinteger(L, 0);
            if (lua_pcall(L, 2, 0, 0))
            {
                LOG_ERROR(LOG_PREFIX_LUA << "error calling cloneUnitScript");
                if (lua_gettop(L) > 0 && lua_isstring(L, -1))
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
            }
			lua_settop(L, 0);
		}

		lua_getUnitTable();
        lua_pushinteger(L, 0);
		lua_setfield(L, -2, "unitID");
		lua_pop(L, 1);
	}

	void UnitScript::setUnitID(uint32 ID)
	{
		register_info();
	}

	int UnitScript::getNbPieces()
	{
		int nb_piece = 0;
		lua_getUnitTable();
		lua_getfield(L, -1, "__piece_list");
		if (!lua_isnil(L, -1))
			nb_piece = lua_objlen(L, -1);
		lua_pop(L, 2);
		return nb_piece;
	}

    void UnitScript::load(const QString &code)
	{
        destroy();
        if (pLuaVM)
        {
            lua_close(pLuaVM);
            pLuaVM = NULL;
        }

        name = "test";
        L = UnitScript::luaVM();

        uint32 buffersize(0);
        QByteArray qBuffer;
        byte *buffer = readFile("scripts/ta3d.lh", &buffersize);
        if (!buffer)
        {
            LOG_ERROR(LOG_PREFIX_LUA << "could not load file 'scripts/ta3d.lh'");
            return;
        }
        QByteArray buf2((char*)buffer, buffersize);
        delete[] buffer;
        buffer = loadLuaCode(QString(buf2), buffersize);
        if (!buffer)
        {
            LOG_ERROR(LOG_PREFIX_LUA << "could not load code into a buffer");
            return;
        }
        qBuffer.append((char*)buffer, buffersize - 1);
        qBuffer.append('\n');
        delete[] buffer;

        buffer = loadLuaCode(code, buffersize);
        if (!buffer)
        {
            LOG_ERROR(LOG_PREFIX_LUA << "could not load code into a buffer");
            return;
        }
        qBuffer.append((char*)buffer, buffersize);
        delete[] buffer;

        running = false;
        if (luaL_loadbuffer(L, (const char*)qBuffer.data(), qBuffer.size() - 1, name.toStdString().c_str() ))
        {
            if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
            {
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));
                LOG_ERROR(qBuffer.data());
            }

            L = NULL;
            buffer = NULL;
			return;
        }
        else
        {
            buffer = NULL;

            last = QTime().msecsTo(QTime::currentTime());
        }

        if (lua_pcall(L, 0, 0, 0))
        {
            if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
            {
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                LOG_ERROR(qBuffer.data());
            }
            return;
        }

        lua_getglobal(L, "__name");
        name = lua_isstring(L, -1) ? QString(lua_tostring(L, -1)) : QString();
        lua_pop(L, 1);

        lua_getglobal(L, name.toStdString().c_str());
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            LOG_ERROR(LOG_PREFIX_LUA << "could not find the unit structure!");
            L = NULL;
            return;
        }
        lua_getfield(L, -1, "__piece_list");
        Mesh::instance()->resetAnimData();
        Mesh::instance()->resetScriptData();
       if (lua_istable(L, -1))
        {
            int n = lua_objlen(L, -1);
            for(int i = 1 ; i <= n ; ++i)
            {
                lua_rawgeti(L, -1, i);
                QString piece_name = lua_tostring(L, -1);
                Mesh *mesh = Mesh::instance()->getMesh(piece_name);
                if (mesh)
                    mesh->scriptID = i - 1;
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 2);
		L = NULL;

        setUnitID(0);
    }

	int UnitScript::run(float dt, bool alone)                  // Run the script
	{
		if (!L)
			return -1;

		if (caller == NULL && !alone)
		{
			for (int i = childs.size() - 1 ; i >= 0 ; --i)
				childs[i]->run(dt);
			clean();
		}

		if (!running)   return 0;
		if (waiting)    return -3;

		if (sleeping)
		{
			sleep_time -= dt;
			if (sleep_time <= 0.0f)
				sleeping = false;
			if (sleeping)
				return -2; 			// Keep sleeping
		}

		try
		{
			int result = lua_resume(L, n_args);
			n_args = 0;
			if (result != LUA_YIELD && result != 0)
			{
				if (lua_gettop(L) > 0 && !lua_isnoneornil(L, -1) && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
					LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
				running = false;
				return -0xFFFF;         // Crashed
			}
			else if (lua_gettop(L) > 0)
			{
				running = (result == LUA_YIELD);
				result = 0;
				while(lua_gettop(L) > 0 && !lua_isnone(L, -1) && !lua_isfunction(L, -1))
				{
					switch(result)
					{
						case 0:
							result = lua_tointeger(L, -1);
							break;
						case 1:             // sleep
							pause( (float)lua_tonumber(L, -1) );
							result = 0;
							break;
						case 2:             // wait
							stop();
							result = 0;
							break;
						case 3:             // set_signal_mask
							setSignalMask( lua_tointeger(L, -1) );
							result = 0;
							break;
						case 4:             // end_thread
							kill();
							result = 0;
							break;
					};
					lua_pop(L, 1);
				}
				return result;
			}
			running = (result == LUA_YIELD);
			return 0;
		}
		catch(...)
		{
			if (lua_gettop(L) > 0 && !lua_isnoneornil(L, -1) && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
				LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
			running = false;
			return -0xFFFF;         // Crashed
		}
		return 0;
	}

	//! functions used to call/run Lua functions
    void UnitScript::call(const QString &functionName, int *parameters, int nb_params)
	{
		if (running)    return;     // We cannot run several functions at the same time on the same stack

		name = functionName;

		lua_settop(L, 0);
		lua_getUnitTable();
        lua_getfield( L, -1, functionName.toStdString().c_str() );
		lua_remove(L, -2);
		if (lua_isnil( L, -1 ))     // Function not found
		{
			lua_pop(L, 1);
			LOG_DEBUG(LOG_PREFIX_LUA << "call: function not found `" << functionName << "`");
			return;
		}

		if (parameters == NULL)
			nb_params = 0;
		lua_getUnitTable();         // Take the unit table as parameter
		for(int i = 0 ; i < nb_params ; i++)
			lua_pushinteger(L, parameters[i]);
		n_args = nb_params + 1;
		running = true;
	}

    int UnitScript::execute(const QString &functionName, int *parameters, int nb_params)
	{
		lua_settop(L, 0);
		lua_getUnitTable();
        lua_getfield( L, -1, functionName.toStdString().c_str() );
		lua_remove(L, -2);
		if (lua_isnil( L, -1 ))     // Function not found
		{
			lua_pop(L, 1);
            LOG_ERROR(LOG_PREFIX_LUA << "execute: function not found `" << functionName << "`");
			return -2;
		}

		if (parameters == NULL)
			nb_params = 0;
		lua_getUnitTable();         // the this parameter
		for(int i = 0 ; i < nb_params ; i++)
			lua_pushinteger(L, parameters[i]);
		++nb_params;
		try
		{
			if (lua_pcall( L, nb_params, 1, 0))
			{
				if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
					LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
				running = false;
				return -1;
			}
		}
		catch(...)
		{
			if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
				LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
			running = false;
			return -1;
		}

		if (lua_gettop(L) > 0)
		{
			int result = lua_isboolean(L,-1) ? lua_toboolean(L,-1) : lua_tointeger( L, -1 );    // Read the result
			lua_pop( L, 1 );
			return result;
		}
		return 0;
	}

	//! functions used to create new threads sharing the same environment
	UnitScript *UnitScript::fork()
	{
		if (caller)
			caller->clean();
		else
			clean();

		UnitScript *newThread = static_cast<UnitScript*>(getFreeThread());
		if (newThread)
		{
			if (lua_status(newThread->L) == LUA_YIELD)     // Some work ir required
			{
				lua_getUnitTable();
				lua_getfield(L, -1, "__threads");
				newThread->L = lua_newthread(L);
				lua_rawseti(L, -2, newThread->nextID);  // We don't want to keep this thread value on top of the stack
				lua_pop(L, 2);
			}
			newThread->running = false;
			newThread->waiting = false;
			newThread->sleeping = false;
			newThread->sleep_time = 0.0f;
			newThread->signal_mask = 0;
			lua_settop(newThread->L, 0);
			addThread(newThread);

			return newThread;
		}

		newThread = new UnitScript();

		newThread->running = false;
		newThread->waiting = false;
		newThread->sleeping = false;
		newThread->sleep_time = 0.0f;
		newThread->caller = (caller != NULL) ? caller : this;
		newThread->name = name;

		lua_getUnitTable();
		lua_getfield(L, -1, "__threads");
		newThread->L = lua_newthread(L);
		newThread->nextID = getNextID();
		lua_rawseti(L, -2, newThread->nextID);  // We don't want to keep this thread value on top of the stack
		lua_pop(L, 2);

		addThread(newThread);

		return newThread;
	}

    UnitScript *UnitScript::fork(const QString &functionName, int *parameters, int nb_params)
	{
		UnitScript *newThread = fork();
		if (newThread)
			newThread->call(functionName, parameters, nb_params);

		return newThread;
	}

	UnitScript *UnitScript::fork(lua_State *cL, int n)
	{
		UnitScript *newThread = fork();

		if (lua_isfunction(cL, -n))
		{
			lua_xmove(cL, newThread->L, n);
			newThread->n_args = n - 1;
			newThread->running = true;
		}
		else
			newThread->running = false;

		return newThread;
	}

	void UnitScript::init()
	{
		caller = NULL;

		signal_mask = 0;
		running = false;

		L = NULL;

		sleep_time = 0.0f;
		sleeping = false;
		waiting = false;

		n_args = 0;

        last = QTime().msecsTo(QTime::currentTime());
	}

	void UnitScript::destroy()
	{
		deleteThreads();

		nextID = 0;

		if (L)
			lua_settop(L, 0);
		running = false;

		init();
	}

    void UnitScript::runCommand(const QString &cmd)
    {
        lua_State *L = luaVM();
        if (L == NULL)
        {
            LOG_ERROR(LOG_PREFIX_LUA << "could not create the Lua virtual machine OO!");
            return;
        }

        if (luaL_loadbuffer(L, (const char*)cmd.toStdString().c_str(), cmd.size(), "user command" ))
        {
            if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
            {
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));
                LOG_ERROR(cmd);
            }
            return;
        }
        else
        {
            try
            {
                if (lua_pcall(L, 0, 0, 0))
                {
                    if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
                        LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                    return;
                }
            }
            catch(...)
            {
                if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                return;
            }
        }
        return;
    }
}
