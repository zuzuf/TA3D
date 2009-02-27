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
#include "unit.script.h"
#include "../UnitEngine.h"

namespace TA3D
{
    UNIT_SCRIPT::UNIT_SCRIPT()
    {
    }

    UNIT_SCRIPT::~UNIT_SCRIPT()
    {
        destroyThread();
    }

    UNIT *lua_currentUnit(lua_State *L)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, "unitID");
        UNIT *p = &(units.unit[lua_tointeger( L, -1 )]);
        lua_pop(L, 1);
        return p;
    }

    int unit_is_turning( lua_State *L )        // is_turning(obj_id, axis_id)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, 1);
        int axis = lua_tointeger(L, 2);
        lua_pushboolean(L, pUnit->script_is_turning(obj, axis));
        return 1;
    }

    int unit_is_moving( lua_State *L )        // is_moving(obj_id, axis_id)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, 1);
        int axis = lua_tointeger(L, 2);
        lua_pushboolean(L, pUnit->script_is_moving(obj, axis));
        return 1;
    }

    int unit_move_object( lua_State *L )        // move_object(obj_id, axis_id, target_pos, speed)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -4);
        int axis = lua_tointeger(L, -3);
        float pos = (float) lua_tonumber(L, -2);
        float speed = (float) lua_tonumber(L, -1);
        lua_pop(L, 4);
        pUnit->script_move_object(obj, axis, pos, speed);
        return 0;
    }

    int unit_explode( lua_State *L )        // explode(obj_id, explosion_type)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -2);
        int type = lua_tointeger(L, -1);
        lua_pop(L, 2);
        pUnit->script_explode(obj, type);
        return 0;
    }

    int unit_turn_object( lua_State *L )        // turn_object(obj_id, axis, angle, speed)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -4);
        int type = lua_tointeger(L, -3);
        float angle = (float) lua_tonumber(L, -2);
        float speed = (float) lua_tonumber(L, -1);
        lua_pop(L, 4);
        pUnit->script_turn_object(obj, type, angle, speed);
        return 0;
    }

    int unit_get_value_from_port( lua_State *L )        // get_value_from_port(port)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int port = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_pushinteger(L, pUnit->script_get_value_from_port(port));
        return 1;
    }

    int unit_spin_object( lua_State *L )        // spin_object(obj_id, axis, target_speed, acceleration)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -4);
        int type = lua_tointeger(L, -3);
        float target_speed = (float) lua_tonumber(L, -2);
        float accel = (float) lua_tonumber(L, -1);
        lua_pop(L, 4);
        pUnit->script_spin_object(obj, type, target_speed, accel);
        return 0;
    }

    int unit_show_object( lua_State *L )        // show_object(obj_id)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -1);
        lua_pop(L, 1);
        pUnit->script_show_object(obj);
        return 0;
    }

    int unit_hide_object( lua_State *L )        // hide_object(obj_id)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -1);
        lua_pop(L, 1);
        pUnit->script_hide_object(obj);
        return 0;
    }

    int unit_emit_sfx( lua_State *L )           // emit_sfx(smoke_type, from_piece)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int from_piece = lua_tointeger(L, -2);
        int smoke_type = lua_tointeger(L, -1);
        lua_pop(L, 2);
        pUnit->script_emit_sfx(smoke_type, from_piece);
        return 0;
    }

    int unit_stop_spin( lua_State *L )           // stop_spin(obj, axis, speed)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -3);
        int axis = lua_tointeger(L, -2);
        float speed = (float) lua_tonumber(L, -1);
        lua_pop(L, 3);
        pUnit->script_stop_spin(obj, axis, speed);
        return 0;
    }

    int unit_move_piece_now( lua_State *L )           // move_piece_now(obj, axis, pos)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -3);
        int axis = lua_tointeger(L, -2);
        float pos = (float) lua_tonumber(L, -1);
        lua_pop(L, 3);
        pUnit->script_move_piece_now(obj, axis, pos);
        return 0;
    }

    int unit_turn_piece_now( lua_State *L )           // turn_piece_now(obj, axis, angle)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int obj = lua_tointeger(L, -3);
        int axis = lua_tointeger(L, -2);
        float angle = (float) lua_tonumber(L, -1);
        lua_pop(L, 3);
        pUnit->script_turn_piece_now(obj, axis, angle);
        return 0;
    }

    int unit_get( lua_State *L )           // get(type, v1, v2)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int type = lua_tointeger(L, 1);
        int v1 = lua_tointeger(L, 2);
        int v2 = lua_tointeger(L, 3);
        lua_pop(L, 3);
        lua_pushinteger( L, pUnit->script_get(type, v1, v2) );
        return 1;
    }

    int unit_set_value( lua_State *L )           // set_value(type, v)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int type = lua_tointeger(L, 1);
        int v = lua_tointeger(L, 2);
        lua_pop(L, 2);
        pUnit->script_set_value(type, v);
        return 0;
    }

    int unit_attach_unit( lua_State *L )           // attach_unit(unit_id, piece_id)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int unit_id = lua_tointeger(L, 1);
        int piece_id = lua_tointeger(L, 2);
        lua_pop(L, 2);
        pUnit->script_attach_unit(unit_id, piece_id);
        return 0;
    }

    int unit_drop_unit( lua_State *L )           // drop_unit(unit_id)
    {
        UNIT *pUnit = lua_currentUnit(L);
        int unit_id = lua_tointeger(L, 1);
        lua_pop(L, 1);
        pUnit->script_drop_unit(unit_id);
        return 0;
    }

    int unit_unit_position( lua_State *L )           // unit_position(unit_id)
    {
        int unit_id = lua_tointeger(L, 1);
        lua_pop(L, 1);
        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].flags)
            lua_pushvector( L, units.unit[unit_id].Pos );
        else
            lua_pushvector( L, Vector3D() );
        return 1;
    }

    int unit_unit_ID( lua_State *L )           // unit_ID()
    {
        UNIT *pUnit = lua_currentUnit(L);
        lua_pushinteger( L, pUnit->idx );
        return 1;
    }

    void UNIT_SCRIPT::register_functions()
    {
        lua_register(L, "is_turning", unit_is_turning );                    // is_turning(obj_id, axis_id)
        lua_register(L, "is_moving", unit_is_moving );                      // is_moving(obj_id, axis_id)
        lua_register(L, "move_object", unit_move_object );                  // move_object(obj_id, axis_id, target_pos, speed)
        lua_register(L, "explode", unit_explode );                          // explode(obj_id, explosion_type)
        lua_register(L, "turn_object", unit_turn_object );                  // turn_object(obj_id, axis, angle, speed)
        lua_register(L, "get_value_from_port", unit_get_value_from_port );  // get_value_from_port(port)
        lua_register(L, "spin_object", unit_spin_object );                  // spin_object(obj_id, axis, target_speed, acceleration)
        lua_register(L, "show_object", unit_show_object );                  // show_object(obj_id)
        lua_register(L, "hide_object", unit_hide_object );                  // hide_object(obj_id)
        lua_register(L, "emit_sfx", unit_emit_sfx );                        // emit_sfx(smoke_type, from_piece)
        lua_register(L, "stop_spin", unit_stop_spin );                      // stop_spin(obj, axis, speed)
        lua_register(L, "move_piece_now", unit_move_piece_now );            // move_piece_now(obj, axis, pos)
        lua_register(L, "turn_piece_now", unit_turn_piece_now );            // turn_piece_now(obj, axis, angle)
        lua_register(L, "get", unit_get );                                  // get(type, v1, v2)
        lua_register(L, "set_value", unit_set_value );                      // set_value(type, v)
        lua_register(L, "attach_unit", unit_attach_unit );                  // attach_unit(unit_id, piece_id)
        lua_register(L, "drop_unit", unit_drop_unit );                      // drop_unit(unit_id)
        lua_register(L, "unit_position", unit_unit_position );              // unit_position(unit_id) = vector(x,y,z)
        lua_register(L, "unit_ID", unit_unit_ID );                          // unit_ID()
    }

    void UNIT_SCRIPT::register_info()
    {
        lua_pushinteger(L, unitID);
        lua_setfield(L, LUA_REGISTRYINDEX, "unitID");
    }

    void UNIT_SCRIPT::setUnitID(uint32 ID)
    {
        unitID = ID;
        lua_pushinteger(L, unitID);
        lua_setfield(L, LUA_REGISTRYINDEX, "unitID");
    }

    int UNIT_SCRIPT::getNbPieces()
    {
#warning TODO: fix model piece loader
        return 0;
    }

    void UNIT_SCRIPT::load(SCRIPT_DATA *data)
    {
        LUA_THREAD::load(data);
    }

    int UNIT_SCRIPT::run(float dt, bool alone)                  // Run the script
    {
        return LUA_THREAD::run(dt, alone);
    }

    //! functions used to call/run Lua functions
    void UNIT_SCRIPT::call(const String &functionName, int *parameters, int nb_params)
    {
        LUA_THREAD::call(functionName, parameters, nb_params);
    }

    int UNIT_SCRIPT::execute(const String &functionName, int *parameters, int nb_params)
    {
        LUA_THREAD::execute(functionName, parameters, nb_params);
    }

    //! functions used to create new threads sharing the same environment
    LUA_THREAD *UNIT_SCRIPT::fork()
    {
        return LUA_THREAD::fork();
    }

    LUA_THREAD *UNIT_SCRIPT::fork(const String &functionName, int *parameters, int nb_params)
    {
        return LUA_THREAD::fork(functionName, parameters, nb_params);
    }

    //! functions used to save/restore scripts state
    void UNIT_SCRIPT::save_state(gzFile file)
    {
        LUA_THREAD::save_state(file);
    }

    void UNIT_SCRIPT::restore_state(gzFile file)
    {
        LUA_THREAD::restore_state(file);
    }
}
