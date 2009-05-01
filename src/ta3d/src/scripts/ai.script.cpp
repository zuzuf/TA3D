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
#include "../TA3D_NameSpace.h"
#include "../misc/paths.h"
#include "ai.script.h"
#include "../ai/ai.h"
#include "../ingame/players.h"
#include "script.h"
#include "../UnitEngine.h"

using namespace TA3D::UTILS::HPI;

namespace TA3D
{
    void AiScript::monitor()
    {
        if (!isRunning())
            start();
    }

    AiScript::AiScript()
    {
        this->playerID = -1;
    }

    AiScript::~AiScript()
    {
        destroyThread();
    }

    void AiScript::setPlayerID(int id)
    {
        playerID = id;
        register_info();
    }

    int AiScript::getPlayerID()
    {
        return playerID;
    }

    void AiScript::setType(int type)
    {
    }

    int AiScript::getType()
    {
        return AI_TYPE_LUA;
    }

    void AiScript::changeName(const String& newName)		// Change le nom de l'IA (conduit à la création d'un nouveau fichier)
    {
        pMutex.lock();
        name = newName;
        pMutex.unlock();
    }

    void AiScript::save()
    {
        String filename;
        Paths::MakeDir( Paths::Resources + "ai" );
        filename << Paths::Resources << "ai" << Paths::Separator << name << TA3D_AI_FILE_EXTENSION;
        remove( filename.c_str() );     // We don't want to save anything here, the Lua script is responsible for everything now
    }


    void AiScript::loadAI(const String& filename, const int id)
    {
        TA3D_FILE* file = ta3d_fopen(filename);

        // Length of the name
        byte l;
        ta3d_fread(&l,1,file);

        // Reading the name
        char* n = new char[l+1];
        n[l]=0;
        ta3d_fread(n, l, file);
        name = n;
        delete[] n;

        ta3d_fclose(file);
        playerID = id;

        register_info();
    }

    int lua_currentPlayerID(lua_State *L)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, "playerID");
        int playerID = lua_tointeger(L, -1);
        lua_pop(L, 1);
        return playerID;
    }

    int ai_playerID(lua_State *L)                   // playerID()
    {
        lua_pushinteger(L, lua_currentPlayerID(L));
        return 1;
    }

    int ai_nb_players(lua_State *L)                 // nb_players()
    {
        lua_pushinteger(L, players.nb_player);
        return 1;
    }

    int ai_get_unit_number_for_player( lua_State *L )		// get_unit_number_for_player( player_id )
    {
        return program_get_unit_number_for_player(L);
    }

    int ai_get_unit_owner( lua_State *L )		// get_unit_owner( unit_id )
    {
        return program_get_unit_owner(L);
    }

    int ai_get_unit_number( lua_State *L )		// get_unit_number()
    {
        lua_pushinteger( L, units.nb_unit );
        return 1;
    }

    int ai_get_max_unit_number( lua_State *L )		// get_max_unit_number()
    {
        lua_pushinteger( L, units.max_unit );
        return 1;
    }

    int ai_add_build_mission( lua_State *L )		// add_build_mission( unit_id, pos_x, pos_z, unit_type )
    {
        int unit_id = lua_tointeger( L, 1 );
        float pos_x = (float) lua_tonumber( L, 2 );
        float pos_z = (float) lua_tonumber( L, 3 );
        int unit_type_id = lua_isstring( L, 4 ) ? unit_manager.get_unit_index( lua_tostring( L, 4 ) ) : lua_tointeger( L, 4 ) ;
        lua_pop( L, 4 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L)
            && unit_type_id >= 0 && unit_manager.unit_type[unit_type_id]->Builder)
        {
            Vector3D target;
            target.x = ((int)(pos_x) + the_map->map_w_d) >> 3;
            target.z = ((int)(pos_z) + the_map->map_h_d) >> 3;
            target.y = Math::Max(the_map->get_max_rect_h((int)target.x, (int)target.z, unit_manager.unit_type[unit_type_id]->FootprintX, unit_manager.unit_type[unit_type_id]->FootprintZ), the_map->sealvl);
            target.x = target.x*8.0f-the_map->map_w_d;
            target.z = target.z*8.0f-the_map->map_h_d;

            units.lock();
            if (units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_BUILD,&target,false,unit_type_id);
            units.unlock();
        }

        return 0;
    }

    int ai_add_move_mission( lua_State *L )		// add_move_mission( unit_id, pos_x, pos_z )
    {
        int unit_id = lua_tointeger( L, 1 );
        float pos_x = (float) lua_tonumber( L, 2 );
        float pos_z = (float) lua_tonumber( L, 3 );
        lua_pop( L, 3 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L))
        {
            Vector3D target;
            target.x = pos_x;
            target.y = the_map->get_unit_h( pos_x, pos_z );
            target.z = pos_z;

            units.lock();
            if (units.unit[ unit_id ].flags)
                units.unit[ unit_id ].add_mission(MISSION_MOVE,&target,false,0);
            units.unlock();
        }

        return 0;
    }

    int ai_add_attack_mission( lua_State *L )		// add_attack_mission( unit_id, target_id )
    {
        int unit_id = lua_tointeger( L, 1 );
        int target_id = lua_tointeger( L, 2 );
        lua_pop( L, 2 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L)
            && target_id >= 0 && target_id < units.max_unit)
        {
            Vector3D target(units.unit[ target_id ].Pos);

            units.lock();
            if (units.unit[ unit_id ].flags)
                units.unit[ unit_id ].add_mission(MISSION_ATTACK,&(target),false,0,&(units.unit[target_id]),NULL,MISSION_FLAG_COMMAND_FIRE );
            units.unlock();
        }

        return 0;
    }

    int ai_add_patrol_mission( lua_State *L )		// add_patrol_mission( unit_id, pos_x, pos_z )
    {
        int unit_id = lua_tointeger( L, 1 );
        float pos_x = (float) lua_tonumber( L, 2 );
        float pos_z = (float) lua_tonumber( L, 3 );
        lua_pop( L, 3 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L))
        {
            Vector3D target;
            target.x = pos_x;
            target.y = the_map->get_unit_h( pos_x, pos_z );
            target.z = pos_z;

            units.lock();
            if (units.unit[ unit_id ].flags)
                units.unit[ unit_id ].add_mission(MISSION_PATROL,&target,false,0);
            units.unlock();
        }

        return 0;
    }

    int ai_add_wait_mission( lua_State *L )		// add_wait_mission( unit_id, time )
    {
        int unit_id = lua_tointeger( L, 1 );
        float time = (float) lua_tonumber( L, 2 );
        lua_pop( L, 2 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L))
        {
            units.lock();
            if (units.unit[ unit_id ].flags)
                units.unit[ unit_id ].add_mission(MISSION_WAIT,NULL,false,(int)(time * 1000));
            units.unlock();
        }

        return 0;
    }

    int ai_add_guard_mission( lua_State *L )		// add_guard_mission( unit_id, target_id )
    {
        int unit_id = lua_tointeger( L, 1 );
        int target_id = lua_tointeger( L, 2 );
        lua_pop( L, 2 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L)
            && target_id >= 0 && target_id < units.max_unit)
        {
            units.lock();
            if (units.unit[ unit_id ].flags)
                units.unit[ unit_id ].add_mission(MISSION_GUARD,&units.unit[ target_id ].Pos,false,0,&(units.unit[ target_id ]),NULL);
            units.unlock();
        }

        return 0;
    }

    int ai_set_standing_orders( lua_State *L )		// set_standing_orders( unit_id, move_order, fire_order )
    {
        int unit_id = lua_tointeger( L, 1 );
        int move_order = lua_tointeger( L, 2 );
        int fire_order = lua_tointeger( L, 3 );
        lua_pop( L, 3 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L))
        {
            units.lock();
            if (units.unit[ unit_id ].flags)
            {
                units.unit[ unit_id ].port[ STANDINGMOVEORDERS ] = move_order;
                units.unit[ unit_id ].port[ STANDINGFIREORDERS ] = fire_order;
            }
            units.unlock();
        }

        return 0;
    }

    int ai_get_unit_health( lua_State *L )		// get_unit_health( unit_id )
    {
        int unit_id = lua_tointeger( L, 1 );
        lua_pop( L, 1 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[unit_id].owner_id == lua_currentPlayerID(L))
        {
            units.lock();
            if (units.unit[ unit_id ].flags)
                lua_pushnumber( L, units.unit[ unit_id ].hp * 100.0f / unit_manager.unit_type[ units.unit[ unit_id ].type_id ]->MaxDamage );
            else
                lua_pushnumber( L, 0.0 );
            units.unlock();
        }

        return 0;
    }

    int ai_map_w( lua_State *L )		// map_w()
    {
        lua_pushinteger( L, the_map->map_w );
        return 1;
    }

    int ai_map_h( lua_State *L )		// map_h()
    {
        lua_pushinteger( L, the_map->map_h );
        return 1;
    }

    int ai_player_side( lua_State *L )		// player_side( player_id )
    {
        int player_id = lua_tointeger( L, 1 );
        lua_pop( L, 1 );

        if (player_id >= 0 && player_id < NB_PLAYERS)
            lua_pushstring( L, players.side[ player_id ].c_str() );
        else
            lua_pushstring( L, "" );

        return 1;
    }

    int ai_allied( lua_State *L )		// allied( id0, id1 )
    {
        int player_id0 = lua_tointeger( L, 1 );
        int player_id1 = lua_tointeger( L, 2 );
        lua_pop( L, 2 );

        if (player_id0 >= 0 && player_id0 < NB_PLAYERS && player_id1 >= 0 && player_id1 < NB_PLAYERS)
            lua_pushboolean( L, players.team[ player_id0 ] & players.team[ player_id1 ] );
        else
            lua_pushboolean( L, false );

        return 1;
    }

    int ai_unit_position( lua_State *L )		// unit_position( unit_id )
    {
        int unit_id = lua_tointeger( L, 1 );
        lua_pop( L, 1 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags)
        {
            units.lock();
            lua_pushvector( L, units.unit[ unit_id ].Pos );
            units.unlock();
        }
        else
            lua_pushvector( L, Vector3D() );

        return 1;
    }

    int ai_self_destruct_unit( lua_State *L )		// self_destruct_unit( unit_id )
    {
        int unit_id = lua_tointeger( L, 1 );
        lua_pop( L, 1 );

        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags)
        {

            units.unit[ unit_id ].lock();
            units.unit[ unit_id ].toggle_self_destruct();
            units.unit[ unit_id ].unlock();
        }

        return 0;
    }

    int ai_attack( lua_State *L )					// attack( attacker_id, target_id )
    {
        int attacker_idx = lua_tointeger( L, 1 );
        int target_idx = lua_tointeger( L, 2 );
        lua_pop( L, 2 );

        if (attacker_idx >= 0 && attacker_idx < units.max_unit && units.unit[attacker_idx].owner_id == lua_currentPlayerID(L) && units.unit[ attacker_idx ].flags)		// make sure we have an attacker and a target
            if (target_idx >= 0 && target_idx < units.max_unit && units.unit[ target_idx ].flags)
            {
                units.lock();
                units.unit[ attacker_idx ].set_mission( MISSION_ATTACK,&(units.unit[ target_idx ].Pos),false,0,true,&(units.unit[ target_idx ]) );
                units.unlock();
            }
        return 0;
    }

    int ai_get_build_list( lua_State *L )           // get_build_list( type )
    {
        int type = lua_tointeger(L, -1);
        lua_pop(L, 1);

        if (type >= 0 && type < unit_manager.nb_unit)
        {
            UnitType *pType = unit_manager.unit_type[type];
            lua_newtable(L);            // Create the list

            for(int i = 0 ; i < pType->nb_unit ; i++)       // Fill the list
            {
                lua_pushinteger(L, pType->BuildList[i]);         // buildable unit index
                lua_rawseti(L, -2, i);
            }
        }
        else
            lua_pushnil(L);
        return 1;
    }

    int ai_get_type_data( lua_State *L )        // get_type_data( type )
    {
        int type = lua_tointeger(L, -1);
        lua_pop(L, 1);

        if (type >= 0 && type < unit_manager.nb_unit)
        {
            UnitType *pType = unit_manager.unit_type[type];
            lua_newtable(L);            // Create the list

            lua_pushboolean(L, pType->canattack);    // unit can attack
            lua_setfield(L, -2, "canattack");

            lua_pushboolean(L, pType->canmove);     // unit can move
            lua_setfield(L, -2, "canmove");

            lua_pushboolean(L, pType->canfly);      // unit can fly
            lua_setfield(L, -2, "canfly");

            lua_pushboolean(L, pType->canguard);    // unit can guard
            lua_setfield(L, -2, "canguard");

            lua_pushboolean(L, pType->canstop);     // unit can stop
            lua_setfield(L, -2, "canstop");

            lua_pushboolean(L, pType->candgun);     // unit can dgun
            lua_setfield(L, -2, "candgun");

            lua_pushboolean(L, pType->canhover);    // unit can hover
            lua_setfield(L, -2, "canhover");

            lua_pushboolean(L, pType->canload);     // unit can load
            lua_setfield(L, -2, "canload");

            lua_pushboolean(L, pType->canpatrol);   // unit can patrol
            lua_setfield(L, -2, "canpatrol");

            lua_pushboolean(L, pType->Builder);     // unit can build
            lua_setfield(L, -2, "canbuild");

            lua_pushboolean(L, pType->canresurrect);     // unit can resurrect
            lua_setfield(L, -2, "canresurrect");

            lua_pushboolean(L, pType->MaxDamage);   // max hit points
            lua_setfield(L, -2, "maxhp");

            lua_pushboolean(L, pType->MaxVelocity);     // speed
            lua_setfield(L, -2, "speed");

            lua_pushinteger(L, type);
            ai_get_build_list(L);
            lua_setfield(L, -2, "buildlist");

            lua_pushinteger(L, pType->BuildCostMetal);      // metal cost
            lua_setfield(L, -2, "metalcost");

            lua_pushinteger(L, pType->BuildCostEnergy);     // energy cost
            lua_setfield(L, -2, "energycost");

            lua_pushinteger(L, pType->EnergyMake);     // energy make
            lua_setfield(L, -2, "energymake");

            lua_pushinteger(L, pType->MetalMake);     // metal make
            lua_setfield(L, -2, "metalmake");

            lua_pushinteger(L, pType->EnergyUse);     // energy use
            lua_setfield(L, -2, "energyuse");

            lua_pushinteger(L, pType->EnergyStorage);     // energy storage
            lua_setfield(L, -2, "energystorage");

            lua_pushinteger(L, pType->MetalStorage);     // metal storage
            lua_setfield(L, -2, "metalstorage");

            lua_pushinteger(L, pType->MetalStorage);     // metal storage
            lua_setfield(L, -2, "metalstorage");

            lua_pushstring(L, pType->name.c_str());     // unit name
            lua_setfield(L, -2, "name");
        }
        else
            lua_pushnil(L);
        return 1;
    }

    int ai_get_unit_data( lua_State *L )        // get_unit_data( index )
    {
        int idx = lua_tointeger(L, -1);
        lua_pop(L, 1);

        if (idx >= 0 && idx < units.max_unit)
        {
            Unit *pUnit = &(units.unit[idx]);
            pUnit->lock();
            lua_newtable(L);            // Create a new entry

            lua_pushinteger(L, pUnit->idx);         // unit index
            lua_setfield(L, -2, "index");

            lua_pushinteger(L, pUnit->ID);          // unit Unique ID
            lua_setfield(L, -2, "UID");

            lua_pushinteger(L, pUnit->owner_id);    // the player this unit belongs to
            lua_setfield(L, -2, "owner");

            lua_pushnumber(L, pUnit->hp);           // unit hit points
            lua_setfield(L, -2, "hp");

            int type_id = pUnit->type_id;
            lua_pushinteger(L, pUnit->type_id);     // unit type_id
            lua_setfield(L, -2, "type");
            pUnit->unlock();
        }
        else
            lua_pushnil(L);
        return 1;
    }

    int ai_get_unit_list( lua_State *L )        // get_unit_list( player_id ), if player_id == -1 or unset, returns all units
    {
        int player_id = lua_isnoneornil(L, 1) ? -1 : lua_tointeger( L, 1 );
        if (!lua_isnone(L, 1))
            lua_pop( L, 1 );

        lua_newtable(L);
        int n = 0;

        units.lock();
        for(int i = 0 ; i < units.index_list_size ; i++)
        {
            int e = units.idx_list[i];
            units.unlock();

            Unit *pUnit = &(units.unit[e]);
            pUnit->lock();
            if (pUnit->owner_id == player_id || player_id == -1)
            {
                lua_pushinteger(L, e);
                ai_get_unit_data(L);
                lua_rawseti(L, -2, n++);    // Add the entry to the list
            }

            units.lock();
        }
        units.unlock();
        return 1;
    }

    int ai_kmeans( lua_State *L )        // kmeans( array_of_vectors, k ), returns k centroids
    {
        int k = lua_tointeger( L, 2 );
        int n = lua_objlen(L, 1);

        std::vector<Vector3D> points;       // Read vector data
        points.resize(n);
        for(int i = 0 ; i < n ; i++)
        {
            lua_rawgeti(L, 1, i);
            points[i] = lua_tovector(L, 3);
            lua_pop(L, 1);
        }

        lua_pop( L, 2 );

        if (n == 0)             // Returns nothing if there is no data
            return 0;

        std::vector<Vector3D> centroids;
        for(int i = 0 ; i < k ; i++)
            centroids.push_back(points[i % n]);

        for(int i = 0 ; i < k + 2 ; k++)           // k + 2 steps for k centroids
        {
            std::vector<Vector3D> newCentroids;
            std::vector<int> clusterSize;
            newCentroids.resize(k);
            clusterSize.resize(k);
            for(int j = 0 ; j < n ; j++)
            {
                int clusterID = -1;
                float best = 0.0f;
                for(int l = 0 ; l < k ; l++)
                {
                    float dist = (points[j] - centroids[l]).sq();
                    if (dist < best || clusterID == -1)
                    {
                        best = dist;
                        clusterID = l;
                    }
                    newCentroids[clusterID] += points[j];
                    clusterSize[clusterID]++;
                }
            }
            for(int j = 0 ; j < k ; j++)
                centroids[j] = clusterSize[j] ? 1.0f / clusterSize[j] * newCentroids[j] : newCentroids[j];
        }

        lua_newtable(L);
        for(int i = 0 ; i < centroids.size() ; i++)
        {
            lua_pushvector(L, centroids[i]);
            lua_rawseti(L, 1, i);
        }

        return 1;
    }

    int ai_nb_unit_types( lua_State *L )        // nb_unit_types()
    {
        lua_pushinteger(L, unit_manager.nb_unit);
        return 1;
    }

    void AiScript::register_functions()
    {
        lua_register(L, "playerID", ai_playerID);                                           // playerID()
        lua_register(L, "nb_players", ai_nb_players);                                       // nb_players()
        lua_register(L, "get_unit_number_for_player", ai_get_unit_number_for_player);       // get_unit_number_for_player( player_id )
        lua_register(L, "get_unit_owner", ai_get_unit_owner);                               // get_unit_owner( unit_id )
        lua_register(L, "get_unit_number", ai_get_unit_number);                             // get_unit_number()
        lua_register(L, "get_max_unit_number", ai_get_max_unit_number);                     // get_max_unit_number()
        lua_register(L, "add_build_mission", ai_add_build_mission);                         // add_build_mission( unit_id, pos_x, pos_z, unit_type )
        lua_register(L, "add_move_mission", ai_add_move_mission);                           // add_move_mission( unit_id, pos_x, pos_z )
        lua_register(L, "add_attack_mission", ai_add_attack_mission);                       // add_attack_mission( unit_id, target_id )
        lua_register(L, "add_patrol_mission", ai_add_patrol_mission);                       // add_patrol_mission( unit_id, pos_x, pos_z )
        lua_register(L, "add_wait_mission", ai_add_wait_mission);                           // add_wait_mission( unit_id, time )
        lua_register(L, "add_guard_mission", ai_add_guard_mission);                         // add_guard_mission( unit_id, target_id )
        lua_register(L, "set_standing_orders", ai_set_standing_orders);                     // set_standing_orders( unit_id, move_order, fire_order )
        lua_register(L, "get_unit_health", ai_get_unit_health);                             // get_unit_health( unit_id )
        lua_register(L, "map_w", ai_map_w);                                                 // map_w()
        lua_register(L, "map_h", ai_map_h);                                                 // map_h()
        lua_register(L, "player_side", ai_player_side);                                     // player_side( player_id )
        lua_register(L, "allied", ai_allied);                                               // allied( id0, id1 )
        lua_register(L, "unit_position", ai_unit_position);                                 // unit_position( unit_id )
        lua_register(L, "self_destruct_unit", ai_self_destruct_unit);                       // self_destruct_unit( unit_id )
        lua_register(L, "attack", ai_attack);                                               // attack( attacker_id, target_id )
        lua_register(L, "get_unit_list", ai_get_unit_list);                                 // get_unit_list( player_id )
        lua_register(L, "get_unit_data", ai_get_unit_data);                                 // get_unit_data( index )
        lua_register(L, "kmeans", ai_kmeans);                                               // kmeans( array_of_vector, k )
        lua_register(L, "get_build_list", ai_get_build_list);                               // get_build_list( type )
        lua_register(L, "get_type_data", ai_get_type_data);                                 // get_type_data( type )
        lua_register(L, "nb_unit_types", ai_nb_unit_types);                                 // nb_unit_types()
    }

    void AiScript::register_info()
    {
        if (L)
        {
            lua_pushinteger(L, playerID);
            lua_setfield(L, LUA_REGISTRYINDEX, "playerID");
        }
    }
}
