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

/*----------------------------------------------------------------------\
  |                                script.cpp                             |
  |      contient les classes nécessaires à la gestion des scripts de     |
  | controle du déroulement de la partie. Les scripts peuvent influencer  |
  | considérablement le déroulement de la partie en manipulant les unités |
  | les ressources mais aussi l'écran et déclenche les signaux de défaite |
  | et de victoire.                                                       |
  \----------------------------------------------------------------------*/

#include <stdafx.h>
#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <EngineClass.h>
#include <UnitEngine.h>
#include "script.h"
#include <misc/camera.h>
#include <languages/i18n.h>
#include <vector>
#include <misc/math.h>
#include <logs/logs.h>
#include <sounds/manager.h>
#include <ingame/players.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <ingame/battle.h>



namespace TA3D
{


	bool LuaProgram::passive = false;        // LuaProgram::passive mode, won't do anything like creating units, move units, etc... used to resync a multiplayer game

	LuaProgram	*LuaProgram::inGame = NULL;

	int program_print_for( lua_State *L )		// text_print_for( x, y, str, player_id )
	{
		const char *str = lua_tostring( L, 3 );		// Read the result
		if (str)
		{
			if (lua_tointeger( L, 4 ) == players.local_human_id || lua_tointeger( L, 4 ) == -1)
			{
				DrawObject draw_obj;
				draw_obj.type = DRAW_TYPE_TEXT;
				draw_obj.r[0] = 1.0f;
				draw_obj.g[0] = 1.0f;
				draw_obj.b[0] = 1.0f;
				draw_obj.x[0] = (float) lua_tonumber( L, 1 );
				draw_obj.y[0] = (float) lua_tonumber( L, 2 );
				draw_obj.text = I18N::Translate( str );
				LuaProgram::inGame->draw_list.add( draw_obj );
			}

			if (network_manager.isServer())
			{
				struct event print_event;
				print_event.type = EVENT_PRINT;
				print_event.opt1 = (lua_tointeger( L, 4 )) == -1 ? (uint16)0xFFFF : (uint16)lua_tointeger( L, 4 );
				print_event.x = (float) lua_tonumber( L, 1 );
				print_event.y = (float) lua_tonumber( L, 2 );
				memcpy( print_event.str, str, strlen( str ) + 1 );

				network_manager.sendEvent( &print_event );
			}
		}
		lua_pop( L, 4 );
		return 0;
	}

	int program_print( lua_State *L )		// text_print( x, y, str )
	{
		lua_pushinteger( L, -1 );
		program_print_for( L );
		return 0;
	}

	int program_line( lua_State *L )		// line( x1,y1,x2,y2,r,g,b )
	{
		DrawObject draw_obj;
		draw_obj.type = DRAW_TYPE_LINE;
		draw_obj.r[0] = (float) lua_tonumber( L, 5 );
		draw_obj.g[0] = (float) lua_tonumber( L, 6 );
		draw_obj.b[0] = (float) lua_tonumber( L, 7 );
		draw_obj.x[0] = (float) lua_tonumber( L, 1 );
		draw_obj.y[0] = (float) lua_tonumber( L, 2 );
		draw_obj.x[1] = (float) lua_tonumber( L, 3 );
		draw_obj.y[1] = (float) lua_tonumber( L, 4 );
		LuaProgram::inGame->draw_list.add( draw_obj );
		lua_pop( L, 7 );

		return 0;
	}

	int program_point( lua_State *L )		// point( x,y,r,g,b )
	{
		DrawObject draw_obj;
		draw_obj.type = DRAW_TYPE_POINT;
		draw_obj.r[0] = (float) lua_tonumber( L, 3 );
		draw_obj.g[0] = (float) lua_tonumber( L, 4 );
		draw_obj.b[0] = (float) lua_tonumber( L, 5 );
		draw_obj.x[0] = (float) lua_tonumber( L, 1 );
		draw_obj.y[0] = (float) lua_tonumber( L, 2 );
		LuaProgram::inGame->draw_list.add( draw_obj );
		lua_pop( L, 5 );

		return 0;
	}

	int program_triangle( lua_State *L )		// triangle( x1,y1,x2,y2,x3,y3,r,g,b )
	{
		DrawObject draw_obj;
		draw_obj.type = DRAW_TYPE_TRIANGLE;
		draw_obj.r[0] = (float) lua_tonumber( L, 7 );
		draw_obj.g[0] = (float) lua_tonumber( L, 8 );
		draw_obj.b[0] = (float) lua_tonumber( L, 9 );
		draw_obj.x[0] = (float) lua_tonumber( L, 1 );
		draw_obj.y[0] = (float) lua_tonumber( L, 2 );
		draw_obj.x[1] = (float) lua_tonumber( L, 3 );
		draw_obj.y[1] = (float) lua_tonumber( L, 4 );
		draw_obj.x[2] = (float) lua_tonumber( L, 5 );
		draw_obj.y[2] = (float) lua_tonumber( L, 6 );
		LuaProgram::inGame->draw_list.add( draw_obj );
		lua_pop( L, 9 );

		return 0;
	}

	int program_cls_for( lua_State *L )		// cls_for( player_id )
	{
		if (lua_tointeger( L, 1 ) == players.local_human_id || lua_tointeger( L, 1 ) == -1)
		{
			LuaProgram::inGame->lock();
			LuaProgram::inGame->draw_list.destroy();
			LuaProgram::inGame->unlock();
		}

		if (network_manager.isServer())
		{
			struct event cls_event;
			cls_event.type = EVENT_CLS;
			cls_event.opt1 = (lua_tointeger( L, 1 )) == -1 ? (uint16)0xFFFF : (uint16)lua_tointeger( L, 1 );

			network_manager.sendEvent( &cls_event );
		}

		lua_pop( L, 1 );

		return 0;
	}

	int program_cls( lua_State *L )		// cls()
	{
		lua_pushinteger( L, -1 );
		program_cls_for( L );
		return 0;
	}

	int program_box( lua_State *L )		// box( x1,y1,x2,y2,r,g,b )
	{
		DrawObject draw_obj;
		draw_obj.type = DRAW_TYPE_BOX;
		draw_obj.r[0] = (float) lua_tonumber( L, 5 );
		draw_obj.g[0] = (float) lua_tonumber( L, 6 );
		draw_obj.b[0] = (float) lua_tonumber( L, 7 );
		draw_obj.x[0] = (float) lua_tonumber( L, 1 );
		draw_obj.y[0] = (float) lua_tonumber( L, 2 );
		draw_obj.x[1] = (float) lua_tonumber( L, 3 );
		draw_obj.y[1] = (float) lua_tonumber( L, 4 );
		LuaProgram::inGame->draw_list.add( draw_obj );
		lua_pop( L, 7 );

		return 0;
	}

	int program_fillbox( lua_State *L )		// fillbox( x1,y1,x2,y2,r,g,b )
	{
		DrawObject draw_obj;
		draw_obj.type = DRAW_TYPE_FILLBOX;
		draw_obj.r[0] = (float) lua_tonumber( L, 5 );
		draw_obj.g[0] = (float) lua_tonumber( L, 6 );
		draw_obj.b[0] = (float) lua_tonumber( L, 7 );
		draw_obj.x[0] = (float) lua_tonumber( L, 1 );
		draw_obj.y[0] = (float) lua_tonumber( L, 2 );
		draw_obj.x[1] = (float) lua_tonumber( L, 3 );
		draw_obj.y[1] = (float) lua_tonumber( L, 4 );
		LuaProgram::inGame->draw_list.add( draw_obj );
		lua_pop( L, 7 );

		return 0;
	}

	int program_circle( lua_State *L )		// circle( x,y,ray,r,g,b )
	{
		DrawObject draw_obj;
		draw_obj.type = DRAW_TYPE_CIRCLE;
		draw_obj.r[0] = (float) lua_tonumber( L, 4 );
		draw_obj.g[0] = (float) lua_tonumber( L, 5 );
		draw_obj.b[0] = (float) lua_tonumber( L, 6 );
		draw_obj.x[0] = (float) lua_tonumber( L, 1 );
		draw_obj.y[0] = (float) lua_tonumber( L, 2 );
		draw_obj.r[1] = (float) lua_tonumber( L, 3 );
		LuaProgram::inGame->draw_list.add( draw_obj );
		lua_pop( L, 6 );

		return 0;
	}

	int program_draw_image_for( lua_State *L )		// draw_image_for( str image_name, x1, y1, x2, y2, player_id )
	{
		if (lua_tointeger( L, 6 ) == players.local_human_id || lua_tointeger( L, 6 ) == -1)
		{
			DrawObject draw_obj;
			draw_obj.type = DRAW_TYPE_BITMAP;
			draw_obj.x[0] = (float) lua_tonumber( L, 2 );
			draw_obj.y[0] = (float) lua_tonumber( L, 3 );
			draw_obj.x[1] = (float) lua_tonumber( L, 4 );
			draw_obj.y[1] = (float) lua_tonumber( L, 5 );
			draw_obj.text = I18N::Translate( lua_tostring( L, 1 ) );
			draw_obj.tex = 0;
			LuaProgram::inGame->draw_list.add( draw_obj );
		}

		if (network_manager.isServer() && !LuaProgram::passive)
		{
			struct event draw_event;
			draw_event.type = EVENT_DRAW;
			draw_event.opt1 = (lua_tointeger( L, 6 )) == -1 ? (uint16)0xFFFF : (uint16)lua_tointeger( L, 6 );
			draw_event.x = (float) lua_tonumber( L, 3 );
			draw_event.y = (float) lua_tonumber( L, 4 );
			draw_event.z = (float) lua_tonumber( L, 5 );
			draw_event.opt3 = (sint32)( ((float) lua_tonumber( L, 2 )) * 16384.0f );
			memcpy( draw_event.str, lua_tostring( L, 1 ), strlen( lua_tostring( L, 1 ) ) + 1 );

			network_manager.sendEvent( &draw_event );
		}

		lua_pop( L, 6 );

		return 0;
	}

	int program_draw_image(lua_State *L)		// draw_image( str image_name, x1, y1, x2, y2 )
	{
		lua_pushinteger( L, -1 );
		program_draw_image_for( L );
		return 0;
	}

	int program_get_image_size(lua_State *L)
	{
		SDL_Surface *img = gfx->load_image(lua_tostring(L, 1) );
		lua_pop(L, 1);
		if (img)
		{
			lua_pushinteger(L, img->w);
			lua_pushinteger(L, img->h);
			SDL_FreeSurface(img);
		}
		else
		{
			lua_pushinteger(L, 0);
			lua_pushinteger(L, 0);
		}
		return 2;
	}

	int program_get_screen_size(lua_State *L)
	{
		lua_pushinteger(L, SCREEN_W);
		lua_pushinteger(L, SCREEN_H);
		return 2;
	}

	int program_nb_players( lua_State *L )		// nb_players()
	{
		lua_pushinteger( L, NB_PLAYERS );
		return 1;
	}

	int program_get_unit_number_for_player( lua_State *L )		// get_unit_number_for_player( player_id )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );
		if (player_id >= 0 && player_id < NB_PLAYERS)
		{
			int n = 0;
			for (uint32 e = 0 ; e < units.index_list_size ; e++)
			{
				const uint32 i = units.idx_list[ e ];
				if (units.unit[i].flags != 0 && units.unit[ i ].owner_id == player_id )
					n++;
			}
			lua_pushinteger( L, n );
		}
		else
			lua_pushinteger( L, 0 );
		return 1;
	}

	int program_get_unit_owner( lua_State *L )		// get_unit_owner( unit_id )
	{
		const int unit_idx = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );
		if (unit_idx >= 0 && unit_idx < (int)units.max_unit)
		{
			if (units.unit[ unit_idx ].flags)
				lua_pushinteger( L, units.unit[ unit_idx ].owner_id );
			else
				lua_pushinteger( L, -1 );
		}
		else
			lua_pushinteger( L, -1 );
		return 1;
	}

	int program_get_unit_number( lua_State *L )		// get_unit_number()
	{
		lua_pushinteger( L, units.nb_unit );
		return 1;
	}

	int program_get_max_unit_number( lua_State *L )		// get_max_unit_number()
	{
		lua_pushinteger( L, units.max_unit );
		return 1;
	}

	int program_annihilated( lua_State *L )		// annihilated( player_id )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );
		if (player_id >= 0 && player_id < NB_PLAYERS )
			lua_pushboolean( L, players.annihilated[ player_id ] );
		else
			lua_pushboolean( L, false );
		return 1;
	}

	int program_has_unit( lua_State *L )		// has_unit( player_id, unit_type_id )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		if (player_id >= 0 && player_id < NB_PLAYERS)
		{
			const int unit_type = lua_isstring( L, 2 ) ? unit_manager.get_unit_index( lua_tostring( L, 2 ) ) : (int)lua_tointeger( L, 2 ) ;
			bool has = false;
			const uint32 last_possible_idx = Math::Min((int)units.max_unit, (player_id + 1) * MAX_UNIT_PER_PLAYER);
			for (uint32 i = player_id * MAX_UNIT_PER_PLAYER ; i < last_possible_idx ; ++i)
				if (units.unit[i].flags && units.unit[i].owner_id == player_id && units.unit[i].type_id == unit_type)
				{
					has = true;
					break;
				}
			lua_pop( L, 2 );
			lua_pushboolean( L, has );
		}
		else
		{
			lua_pop( L, 2 );
			lua_pushboolean( L, false );
		}
		return 1;
	}

	int program_nb_unit_of_type( lua_State *L )		// nb_unit_of_type( player_id, unit_type_id )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		if (player_id >= 0 && player_id < NB_PLAYERS)
		{
			const int unit_type = lua_isstring( L, 2 ) ? unit_manager.get_unit_index( lua_tostring( L, 2 ) ) : (int)lua_tointeger( L, 2 ) ;
			int nb = 0;
			const uint32 last_possible_idx = Math::Min((int)units.max_unit, (player_id + 1) * MAX_UNIT_PER_PLAYER);
			for (uint32 i = player_id * MAX_UNIT_PER_PLAYER ; i < last_possible_idx ; ++i)
				if (units.unit[i].flags && units.unit[i].owner_id == player_id && units.unit[i].type_id == unit_type)
					nb++;
			lua_pop( L, 2 );
			lua_pushinteger( L, nb );
		}
		else
		{
			lua_pop( L, 2 );
			lua_pushinteger( L, 0 );
		}
		return 1;
	}

	int program_is_unit_of_type( lua_State *L )		// is_unit_of_type( unit_id, unit_type_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		if (unit_id >= 0 && unit_id < (int)units.max_unit)
		{
			const int unit_type = lua_isstring( L, 2 ) ? unit_manager.get_unit_index( lua_tostring( L, 2 ) ) : (int)lua_tointeger( L, 2 ) ;
			lua_pop( L, 2 );
			lua_pushboolean( L, (units.unit[unit_id].flags & 1) && units.unit[unit_id].type_id == unit_type );
		}
		else
		{
			lua_pop( L, 2 );
			lua_pushboolean( L, false );
		}
		return 1;
	}

	int program_has_mobile_units( lua_State *L ) 		// has_mobile_units( player_id )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );
		if (player_id >= 0 && player_id < NB_PLAYERS )
		{
			bool result = false;
			for (uint32 e = 0 ; e < units.index_list_size && !result ; ++e)
			{
				const uint32 i = units.idx_list[ e ];
				if ((units.unit[ i ].flags & 1) && units.unit[ i ].owner_id == player_id )
				{
					const int type = units.unit[ i ].type_id;
					if (type >= 0 && type < unit_manager.nb_unit && unit_manager.unit_type[type]->canmove && unit_manager.unit_type[type]->BMcode)
						result = true;
				}
			}
			lua_pushboolean( L, result );
		}
		else
			lua_pushboolean( L, false );
		return 1;
	}

	int program_move_unit( lua_State *L )		// move_unit( unit_id, x, z )
	{
		int unit_id = (int)lua_tointeger( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags && !LuaProgram::passive)
		{
			units.lock();

			units.unit[ unit_id ].Pos.x = (float) lua_tonumber( L, 2 );
			units.unit[ unit_id ].Pos.z = (float) lua_tonumber( L, 3 );

			units.unit[ unit_id ].clear_from_map();
			units.unit[ unit_id ].lastEnergy = 999999999.99f;
			units.unit[ unit_id ].selfmove = false;

			int PX = ((int)(units.unit[ unit_id ].Pos.x + (float)the_map->map_w_d) >> 3);
			int PY = ((int)(units.unit[ unit_id ].Pos.z + (float)the_map->map_h_d) >> 3);
			if (!can_be_there( PX, PY, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ))
			{
				bool found = false;
				for (int r = 1 ; r < 120 && !found ; r++)		// Circular check
				{
					const int r2 = r * r;
					for (int y = 0 ; y <= r ; y++)
					{
						const int x = (int)(sqrtf( float(r2 - y * y) ) + 0.5f);
						if (can_be_there( PX+x, PY+y, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ))
						{
							PX += x;
							PY += y;
							found = true;
							break;
						}
						if (can_be_there( PX-x, PY+y, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ))
						{
							PX -= x;
							PY += y;
							found = true;
							break;
						}
						if (can_be_there( PX+x, PY-y, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ))
						{
							PX += x;
							PY -= y;
							found = true;
							break;
						}
						if (can_be_there( PX-x, PY-y, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ))
						{
							PX -= x;
							PY -= y;
							found = true;
							break;
						}
					}
				}
				if (found)
				{
					units.unit[ unit_id ].Pos.x = float((PX << 3) + 8 - the_map->map_w_d);
					units.unit[ unit_id ].Pos.z = float((PY << 3) + 8 - the_map->map_h_d);
					if (!units.unit[ unit_id ].mission.empty() && (units.unit[ unit_id ].mission->getFlags() & MISSION_FLAG_MOVE))
						units.unit[ unit_id ].mission->Flags() |= MISSION_FLAG_REFRESH_PATH;
				}
				else
				{
					int prev = 0;
					for (unsigned int i = 0 ; i < units.nb_unit ; ++i)
						if (units.idx_list[ i ] == unit_id)
						{
							prev = (int)i;
							break;
						}
					units.kill(unit_id, prev);
					unit_id = -1;
				}
			}
			if (unit_id >= 0)
			{
				units.unit[ unit_id ].cur_px = PX;
				units.unit[ unit_id ].cur_py = PY;

				Vector3D target_pos = units.unit[ unit_id ].Pos;
				target_pos.x = float(((int)(target_pos.x) + the_map->map_w_d) >> 3);
				target_pos.z = float(((int)(target_pos.z) + the_map->map_h_d) >> 3);
				target_pos.y = Math::Max(the_map->get_max_rect_h((int)target_pos.x,(int)target_pos.z,
																 unit_manager.unit_type[units.unit[unit_id].type_id]->FootprintX,
																 unit_manager.unit_type[units.unit[unit_id].type_id]->FootprintZ),
										 the_map->sealvl);
				units.unit[ unit_id ].Pos.y = target_pos.y;
				units.unit[ unit_id ].draw_on_map();
			}

			units.unlock();
		}
		lua_pop( L, 3 );

		return 0;
	}

	int program_create_unit( lua_State *L )		// create_unit( player_id, unit_type_id, x, z )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		int unit_type_id = !lua_isnumber( L, 2 ) ? unit_manager.get_unit_index( lua_tostring( L, 2 ) ) : (int)lua_tointeger( L, 2 ) ;
		if (lua_isnumber(L, 2) && unit_type_id == -1)
			unit_type_id = Math::RandomTable() % (int)unit_manager.nb_unit;
		const float x = (float) lua_tonumber( L, 3 );
		const float z = (float) lua_tonumber( L, 4 );

		lua_pop( L, 4 );

		if (unit_type_id >= 0 && unit_type_id < unit_manager.nb_unit && player_id >= 0 && player_id < NB_PLAYERS && !LuaProgram::passive)
		{
			units.lock();
			Vector3D pos;
			pos.x = x;
			pos.z = z;
			pos.y = Math::Max( the_map->get_max_rect_h((int)x,(int)z, unit_manager.unit_type[ unit_type_id ]->FootprintX, unit_manager.unit_type[unit_type_id]->FootprintZ ), the_map->sealvl);
			Unit *unit = (Unit*)create_unit( unit_type_id, player_id, pos, true, true);		// Force synchronization
			int idx = unit ? unit->idx : -1;
			if (unit)
			{
				unit->lock();
				unit->hp = (float)unit_manager.unit_type[unit_type_id]->MaxDamage;
				unit->build_percent_left = 0.0f;
				if (unit_manager.unit_type[ unit_type_id ]->ActivateWhenBuilt) // Start activated
				{
					if (unit->script)			// We have to do that in order to get the creation script done before activating the unit
						unit->script->run(1.0f / TICKS_PER_SEC);
					unit->port[ACTIVATION] = 0;
					unit->activate();
				}
				unit->unlock();
			}
			units.unlock();

			if (idx >= 0 && idx < (int)units.max_unit && units.unit[ idx ].flags)
				lua_pushinteger( L, idx );
			else
				lua_pushinteger( L, -1 );
		}
		else
			lua_pushinteger( L, -1 );

		return 1;
	}

	int program_change_unit_owner( lua_State *L )		// change_unit_owner( unit_id, player_id )
	{
		const int player_id = (int)lua_tointeger( L, 2 );
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && player_id >= 0 && player_id < NB_PLAYERS && units.unit[ unit_id ].flags && !LuaProgram::passive)
		{
			units.lock();
			units.unit[ unit_id ].owner_id = (byte)player_id;
			units.unlock();
		}

		return 0;
	}

	int program_set_unit_health( lua_State *L )		// set_unit_health( unit_id, health_percentage )
	{
		const float health = (float) lua_tonumber( L, 2 ) * 0.01f;
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
		{
			units.lock();
			if (units.unit[ unit_id ].flags)
				units.unit[ unit_id ].hp = health * (float)unit_manager.unit_type[ units.unit[ unit_id ].type_id ]->MaxDamage;
			units.unlock();
		}

		return 0;
	}

	int program_add_build_mission( lua_State *L )		// add_build_mission( unit_id, pos_x, pos_z, unit_type )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const float pos_x = (float) lua_tonumber( L, 2 );
		const float pos_z = (float) lua_tonumber( L, 3 );
		const int unit_type_id = lua_isstring( L, 4 ) ? unit_manager.get_unit_index( lua_tostring( L, 4 ) ) : (int)lua_tointeger( L, 4 ) ;
		lua_pop( L, 4 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && unit_type_id >= 0 && unit_manager.unit_type[unit_type_id]->Builder && !LuaProgram::passive)
		{
			Vector3D target;
			target.x = float(((int)(pos_x) + the_map->map_w_d) >> 3);
			target.z = float(((int)(pos_z) + the_map->map_h_d) >> 3);
			target.y = Math::Max(the_map->get_max_rect_h((int)target.x, (int)target.z, unit_manager.unit_type[unit_type_id]->FootprintX, unit_manager.unit_type[unit_type_id]->FootprintZ), the_map->sealvl);
			target.x = target.x * 8.0f - (float)the_map->map_w_d;
			target.z = target.z * 8.0f - (float)the_map->map_h_d;

			units.lock();
			if (units.unit[ unit_id ].flags )
				units.unit[ unit_id ].add_mission(MISSION_BUILD,&target,false,unit_type_id);
			units.unlock();
		}

		return 0;
	}

	int program_add_move_mission( lua_State *L )		// add_move_mission( unit_id, pos_x, pos_z )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const float pos_x = (float) lua_tonumber( L, 2 );
		const float pos_z = (float) lua_tonumber( L, 3 );
		lua_pop( L, 3 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
		{
			Vector3D target;
			target.x = pos_x;
			target.y = the_map->get_unit_h( pos_x, pos_z );
			target.z = pos_z;

			units.lock();
			if (units.unit[ unit_id ].flags )
				units.unit[ unit_id ].add_mission(MISSION_MOVE,&target,false,0);
			units.unlock();
		}

		return 0;
	}

	int program_add_attack_mission( lua_State *L )		// add_attack_mission( unit_id, target_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const int target_id = (int)lua_tointeger( L, 2 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && target_id >= 0 && target_id < (int)units.max_unit && !LuaProgram::passive)
		{
			Vector3D target(units.unit[ target_id ].Pos);

			units.lock();
			if (units.unit[ unit_id ].flags)
				units.unit[ unit_id ].add_mission(MISSION_ATTACK, &(target), false, 0, &(units.unit[target_id]), MISSION_FLAG_COMMAND_FIRE );
			units.unlock();
		}

		return 0;
	}

	int program_add_patrol_mission( lua_State *L )		// add_patrol_mission( unit_id, pos_x, pos_z )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const float pos_x = (float) lua_tonumber( L, 2 );
		const float pos_z = (float) lua_tonumber( L, 3 );
		lua_pop( L, 3 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
		{
			Vector3D target;
			target.x = pos_x;
			target.y = the_map->get_unit_h( pos_x, pos_z );
			target.z = pos_z;

			units.lock();
			if (units.unit[ unit_id ].flags )
				units.unit[ unit_id ].add_mission(MISSION_PATROL,&target,false,0);
			units.unlock();
		}

		return 0;
	}

	int program_add_wait_mission( lua_State *L )		// add_wait_mission( unit_id, time )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const float time = (float) lua_tonumber( L, 2 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
		{
			units.lock();
			if (units.unit[ unit_id ].flags )
				units.unit[ unit_id ].add_mission(MISSION_WAIT,NULL,false,(int)(time * 1000));
			units.unlock();
		}

		return 0;
	}

	int program_add_wait_attacked_mission( lua_State *L )		// add_wait_attacked_mission( unit_id, target_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const int target_id = (int)lua_tointeger( L, 2 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && target_id >= 0 && target_id < (int)units.max_unit && !LuaProgram::passive)
		{
			units.lock();
			if (units.unit[ unit_id ].flags)
				units.unit[ unit_id ].add_mission(MISSION_WAIT_ATTACKED,NULL,false,target_id);
			units.unlock();
		}

		return 0;
	}

	int program_add_guard_mission( lua_State *L )		// add_guard_mission( unit_id, target_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const int target_id = (int)lua_tointeger( L, 2 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && target_id >= 0 && target_id < (int)units.max_unit && !LuaProgram::passive)
		{
			units.lock();
			if (units.unit[ unit_id ].flags)
				units.unit[ unit_id ].add_mission(MISSION_GUARD,&units.unit[ target_id ].Pos,false,0,&(units.unit[ target_id ]));
			units.unlock();
		}

		return 0;
	}

	int program_set_standing_orders( lua_State *L )		// set_standing_orders( unit_id, move_order, fire_order )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const int move_order = (int)lua_tointeger( L, 2 );
		const int fire_order = (int)lua_tointeger( L, 3 );
		lua_pop( L, 3 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
		{
			units.lock();
			if (units.unit[ unit_id ].flags)
			{
				units.unit[ unit_id ].port[ STANDINGMOVEORDERS ] = sint16(move_order);
				units.unit[ unit_id ].port[ STANDINGFIREORDERS ] = sint16(fire_order);
			}
			units.unlock();
		}

		return 0;
	}

	int program_lock_orders( lua_State *L )		// lock_orders( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
			units.unit[ unit_id ].lock_command();

		return 0;
	}

	int program_unlock_orders( lua_State *L )		// unlock_orders( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && !LuaProgram::passive)
			units.unit[ unit_id ].unlock_command();

		return 0;
	}

	int program_get_unit_health( lua_State *L )		// get_unit_health( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit)
		{
			units.lock();
			if (units.unit[ unit_id ].flags )
				lua_pushnumber( L, units.unit[ unit_id ].hp * 100.0f / (float)unit_manager.unit_type[ units.unit[ unit_id ].type_id ]->MaxDamage );
			else
				lua_pushnumber( L, 0 );
			units.unlock();
		}

		return 0;
	}

	int program_local_player( lua_State *L )		// local_player()
	{
		lua_pushinteger( L, players.local_human_id );
		return 1;
	}

	int program_map_w( lua_State *L )		// map_w()
	{
		lua_pushinteger( L, the_map->map_w );
		return 1;
	}

	int program_map_h( lua_State *L )		// map_h()
	{
		lua_pushinteger( L, the_map->map_h );
		return 1;
	}

	int program_player_side( lua_State *L )		// player_side( player_id )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (player_id >= 0 && player_id < NB_PLAYERS)
			lua_pushstring( L, players.side[ player_id ].c_str() );
		else
			lua_pushstring( L, "" );

		return 1;
	}

	int program_allied( lua_State *L )		// allied( id0, id1 )
	{
		const int player_id0 = (int)lua_tointeger( L, 1 );
		const int player_id1 = (int)lua_tointeger( L, 2 );
		lua_pop( L, 2 );

		if (player_id0 >= 0 && player_id0 < NB_PLAYERS && player_id1 >= 0 && player_id1 < NB_PLAYERS )
			lua_pushboolean( L, players.team[ player_id0 ] & players.team[ player_id1 ] );
		else
			lua_pushboolean( L, false );

		return 1;
	}

	int program_unit_height( lua_State *L )		// unit_height( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			const UnitType *pType = units.unit[ unit_id ].type_id >= 0 ? unit_manager.unit_type[units.unit[ unit_id ].type_id] : NULL;
			if (pType && pType->model)
				lua_pushnumber( L, pType->model->top );
			else
				lua_pushnumber( L, 0 );
			units.unlock();
		}
		else
			lua_pushnumber( L, 0 );

		return 1;
	}

	int program_unit_piece_pos( lua_State *L )		// unit_piece_pos( unit_id, piece )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			UnitType *pType = units.unit[ unit_id ].type_id >= 0 ? unit_manager.unit_type[units.unit[ unit_id ].type_id] : NULL;
			if (pType && pType->model && pType->script)
			{
				const int piece_id = pType->script->identify(lua_tostring(L, -1));
				lua_pop( L, 2 );
				if (piece_id >= 0)
				{
					units.unit[ unit_id ].compute_model_coord();
					lua_pushvector( L, units.unit[ unit_id ].data.data[piece_id].pos );
				}
				else
					lua_pushvector( L, Vector3D() );
			}
			else
			{
				lua_pop( L, 2 );
				lua_pushvector( L, Vector3D() );
			}
			units.unlock();
		}
		else
		{
			lua_pop( L, 2 );
			lua_pushvector( L, Vector3D() );
		}

		return 1;
	}

	int program_unit_piece_dir( lua_State *L )		// unit_piece_dir( unit_id, piece )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			UnitType *pType = units.unit[ unit_id ].type_id >= 0 ? unit_manager.unit_type[units.unit[ unit_id ].type_id] : NULL;
			if (pType && pType->model && pType->script)
			{
				const int piece_id = pType->script->identify(lua_tostring(L, -1));
				lua_pop( L, 2 );
				if (piece_id >= 0)
				{
					units.unit[ unit_id ].compute_model_coord();
					lua_pushvector( L, Vector3D(0.0f, 0.0f, 1.0f) * units.unit[ unit_id ].data.data[piece_id].matrix );
				}
				else
					lua_pushvector( L, Vector3D() );
			}
			else
			{
				lua_pop( L, 2 );
				lua_pushvector( L, Vector3D() );
			}
			units.unlock();
		}
		else
		{
			lua_pop( L, 2 );
			lua_pushvector( L, Vector3D() );
		}

		return 1;
	}

	int program_unit_x( lua_State *L )		// unit_x( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			lua_pushnumber( L, units.unit[ unit_id ].Pos.x );
			units.unlock();
		}
		else
			lua_pushnumber( L, 0 );

		return 1;
	}

	int program_unit_y( lua_State *L )		// unit_y( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			lua_pushnumber( L, units.unit[ unit_id ].Pos.y );
			units.unlock();
		}
		else
			lua_pushnumber( L, 0 );

		return 1;
	}

	int program_unit_z( lua_State *L )		// unit_z( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			lua_pushnumber( L, units.unit[ unit_id ].Pos.z );
			units.unlock();
		}
		else
			lua_pushnumber( L, 0 );

		return 1;
	}

	int program_unit_pos( lua_State *L )		// unit_pos( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			lua_pushvector( L, units.unit[ unit_id ].Pos );
			units.unlock();
		}
		else
			lua_pushvector( L, Vector3D() );

		return 1;
	}

	int program_unit_angle( lua_State *L )		// unit_angle( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
		{
			units.lock();
			lua_pushvector( L, units.unit[ unit_id ].Angle );
			units.unlock();
		}
		else
			lua_pushvector( L, Vector3D() );

		return 1;
	}

	int program_kill_unit( lua_State *L )		// kill_unit( unit_id )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags && !LuaProgram::passive)
		{

			units.unit[ unit_id ].lock();
			if (!network_manager.isConnected() || units.unit[ unit_id ].local )
				units.unit[ unit_id ].hp = 0.0f;
			else
			{
				struct event event;
				event.type = EVENT_UNIT_DEATH;
				event.opt1 = uint16(unit_id);
				network_manager.sendEvent( &event );
			}
			units.unit[ unit_id ].unlock();
		}

		return 0;
	}

	int program_kick_unit( lua_State *L )		// kick_unit( unit_id, damage )
	{
		const int unit_id = (int)lua_tointeger( L, 1 );
		const float damage = (float) lua_tonumber( L, 2 );
		lua_pop( L, 2 );

		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags && !LuaProgram::passive)
		{
			units.lock();

			units.unit[ unit_id ].hp -= damage;

			units.unlock();
		}

		return 0;
	}

	int program_play_for(lua_State *L)		// play_for( filename, player_id )
	{
		if (lua_tointeger(L, 2) == players.local_human_id || lua_tointeger( L, 2 ) == -1)
			sound_manager->playSound((const char*)lua_tostring( L, 1 ));

		if (network_manager.isServer() && !LuaProgram::passive)
		{
			struct event play_event;
			play_event.type = EVENT_PLAY;
			play_event.opt1 = (lua_tointeger( L, 2 )) == -1 ? (uint16)0xFFFFU : (uint16)lua_tointeger(L, 2);
			memcpy( play_event.str, lua_tostring( L, 1 ), strlen( lua_tostring( L, 1 ) ) + 1);

			network_manager.sendEvent(&play_event);
		}
		lua_pop(L, 2);
		return 0;
	}


	int program_play(lua_State *L)		// play( filename )
	{
		lua_pushinteger(L, -1);
		program_play_for(L);
		return 0;
	}

	int program_set_cam_pos( lua_State *L )		// set_cam_pos( player_id,x,z )
	{
		if ((int)lua_tointeger( L, 1 ) == players.local_human_id )
		{
			Camera::inGame->rpos.x = (float) lua_tonumber( L, 2 );
			Camera::inGame->rpos.z = (float) lua_tonumber( L, 3 );
		}
		else
		{
			if (network_manager.isServer() && !LuaProgram::passive)
			{
				struct event cam_event;
				cam_event.type = EVENT_CAMERA_POS;
				cam_event.opt1 = (uint16)lua_tointeger( L, 1 );
				cam_event.x = (float) lua_tonumber( L, 2 );
				cam_event.z = (float) lua_tonumber( L, 3 );

				network_manager.sendEvent( &cam_event );
			}
		}
		lua_pop( L, 3 );
		return 0;
	}

	int program_set_cam( lua_State *L )		// set_cam( player_id, { pos = { x, y, z }, dir = { x, y, z }, mode } )
	{
		if ((int)lua_tointeger( L, 1 ) == players.local_human_id )
		{
			lua_pushstring(L, "pos");
			lua_rawget(L, -2);
			Camera::inGame->rpos = lua_tovector(L, -1);

			lua_pop(L, 1);
			lua_pushstring(L, "dir");
			lua_rawget(L, -2);
			Battle::Instance()->setCameraDirection( lua_tovector(L, -1) );

			lua_pop(L, 1);
			lua_pushstring(L, "mode");
			lua_rawget(L, -2);
			Battle::Instance()->setFreeCamera( lua_toboolean(L, -1) );

			lua_pop(L, 1);
		}
		else
		{
#warning TODO: implement program_set_cam in network protocol
//			if (network_manager.isServer() && !LuaProgram::passive)
//			{
//				struct event cam_event;
//				cam_event.type = EVENT_CAMERA_POS;
//				cam_event.opt1 = lua_tointeger( L, 1 );
//				cam_event.x = (float) lua_tonumber( L, 2 );
//				cam_event.z = (float) lua_tonumber( L, 3 );
//
//				network_manager.sendEvent( &cam_event );
//			}
		}
		lua_pop( L, 2 );
		return 0;
	}

	int program_clf(lua_State* /*L*/)
	{
		the_map->clear_FOW();
		units.lock();
		for (size_t i = 0 ; i < units.index_list_size ; ++i)
			units.unit[ units.idx_list[ i ] ].old_px = -10000;
		units.unlock();

		if (network_manager.isServer() && !LuaProgram::passive)
		{
			struct event clf_event;
			clf_event.type = EVENT_CLF;
			network_manager.sendEvent( &clf_event );
		}
		return 0;
	}

	int program_start_x( lua_State *L )			// start_x( player_id )
	{
		const unsigned int player_id = (unsigned int)lua_tointeger(L, 1);
		lua_pop(L, 1);

		if (player_id < players.count())
			lua_pushnumber(L, float(the_map->ota_data.startX[ player_id ] - the_map->map_w) * 0.5f);
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	int program_start_z( lua_State *L )			// start_z( player_id )
	{
		const unsigned int player_id = (unsigned int)lua_tointeger( L, 1 );
		lua_pop( L, 1 );

		if (player_id < players.count())
			lua_pushnumber(L, float(the_map->ota_data.startZ[ player_id ] - the_map->map_h) * 0.5f);
		else
			lua_pushnumber(L, 0);
		return 1;
	}

	int program_init_res(lua_State* /*L*/) // init_res()
	{
		for (unsigned int i = 0 ; i < players.count(); ++i)
		{
			players.metal[i] = (float)players.com_metal[i];
			players.energy[i] = (float)players.com_energy[i];
		}

		if (network_manager.isServer() && !LuaProgram::passive)
		{
			struct event init_res_event;
			init_res_event.type = EVENT_INIT_RES;
			network_manager.sendEvent( &init_res_event );
		}

		return 0;
	}

	int program_give_metal( lua_State *L )			// give_metal( player_id, amount )
	{
		const unsigned int player_id = (unsigned int)lua_tointeger(L, 1);
		const float amount = (float) lua_tonumber(L, 2);
		lua_pop( L, 2 );

		if (player_id < players.count() && !LuaProgram::passive)
		{
			units.lock();
			players.metal[ player_id ] += amount;
			players.c_metal[ player_id ] = players.metal[ player_id ];
			units.unlock();
		}
		return 0;
	}

	int program_give_energy( lua_State *L )			// give_energy( player_id, amount )
	{
		const unsigned int player_id = (unsigned int)lua_tointeger(L, 1);
		const float amount = (float) lua_tonumber(L, 2);
		lua_pop(L, 2);

		if (player_id < players.count() && !LuaProgram::passive)
		{
			units.lock();
			players.energy[ player_id ] += amount;
			players.c_energy[ player_id ] = players.energy[ player_id ];
			units.unlock();
		}
		return 0;
	}

	int program_commander( lua_State *L )				// commander( player_id )
	{
		const unsigned int player_id = (unsigned int)lua_tointeger(L, 1);
		lua_pop(L, 1);

		if (player_id < (unsigned int)NB_PLAYERS) // make sure we have a player
		{
			for (int i = 0; i < ta3dSideData.nb_side; ++i)
			{
				if (ToLower(ta3dSideData.side_name[i]) == ToLower(players.side[ player_id ]) )
				{
					lua_pushstring(L, ta3dSideData.side_com[i].c_str() );
					break;
				}
				else
					lua_pushstring(L, "");
			}
		}
		return 1;
	}

	int program_attack( lua_State *L )					// attack( attacker_id, target_id )
	{
		const int attacker_idx = (int)lua_tointeger( L, 1 );
		const int target_idx = (int)lua_tointeger( L, 2 );
		lua_pop( L, 2 );

		if (attacker_idx >= 0 && attacker_idx < (int)units.max_unit && units.unit[ attacker_idx ].flags && !LuaProgram::passive)		// make sure we have an attacker and a target
			if (target_idx >= 0 && target_idx < (int)units.max_unit && units.unit[ target_idx ].flags)
			{
				units.lock();
				units.unit[ attacker_idx ].set_mission( MISSION_ATTACK,&(units.unit[ target_idx ].Pos),false,0,true,&(units.unit[ target_idx ]) );
				units.unlock();
			}
		return 0;
	}

	int program_create_feature( lua_State *L )		// create_feature( feature_type, x, z )
	{
		const int feature_type_id = lua_isstring( L, 1 ) ? feature_manager.get_feature_index( lua_tostring( L, 1 ) ) : (int)lua_tointeger( L, 1 ) ;
		const float X = (float) lua_tonumber( L, 2 );
		const float Z = (float) lua_tonumber( L, 3 );

		lua_pop( L, 3 );

		Feature *feature = feature_manager.getFeaturePointer(feature_type_id);
		if (feature && !LuaProgram::passive)
		{
			const int x = (int)(X + (float)the_map->map_w_d - 8.0f) >> 3;
			const int y = (int)(Z + (float)the_map->map_h_d - 8.0f) >> 3;
			if (x > 0 && y > 0 && x < (the_map->bloc_w << 1) && y < (the_map->bloc_h << 1))
				if (the_map->map_data(x, y).stuff == -1)
				{
					Vector3D Pos;
					Pos.x = float((x << 3) - the_map->map_w_d) + 8.0f;
					Pos.z = float((y << 3) - the_map->map_h_d) + 8.0f;
					Pos.y = the_map->get_unit_h( Pos.x, Pos.z );
					the_map->map_data(x, y).stuff = features.add_feature( Pos, feature_type_id );
					if (feature && the_map->map_data(x, y).stuff != -1 && feature->blocking)
						the_map->rect(x - (feature->footprintx >> 1), y - (feature->footprintz >> 1), feature->footprintx, feature->footprintz, -2 - the_map->map_data(x, y).stuff);
				}
		}

		return 0;
	}

	int program_send_signal( lua_State *L )		// send_signal( player_id, signal )
	{
		const int player_id = (int)lua_tointeger( L, 1 );
		const int signal_id = (int)lua_tointeger( L, 2 );

		if (player_id == players.local_human_id || player_id == -1)
			g_ta3d_network->set_signal( signal_id );

		if (network_manager.isServer() && !LuaProgram::passive)
		{
			struct event signal_event;
			signal_event.type = EVENT_SCRIPT_SIGNAL;
			signal_event.opt1 = player_id == -1 ? (uint16)0xFFFF : (uint16)player_id;
			signal_event.opt2 = (uint16)signal_id;

			network_manager.sendEvent( &signal_event );
		}

		return 0;
	}

	int program_time( lua_State *L )		// time()
	{
		lua_pushnumber( L, (double)units.current_tick / (double)TICKS_PER_SEC );
		return 1;
	}

	void LuaProgram::register_functions()
	{
		lua_gc( L, LUA_GCSTOP, 0 );		// Load libraries
		luaL_openlibs( L );
		lua_gc( L, LUA_GCRESTART, 0 );

		lua_register( L, "text_print", program_print );
		lua_register( L, "text_print_for", program_print_for );
		lua_register( L, "line", program_line );
		lua_register( L, "cls", program_cls );
		lua_register( L, "cls_for", program_cls_for );
		lua_register( L, "point", program_point );
		lua_register( L, "triangle", program_triangle );
		lua_register( L, "box", program_box );
		lua_register( L, "fillbox", program_fillbox );
		lua_register( L, "circle", program_circle );
		lua_register( L, "time", program_time );
		lua_register( L, "draw_image", program_draw_image );
		lua_register( L, "draw_image_for", program_draw_image_for );
		lua_register( L, "get_image_size", program_get_image_size );
		lua_register( L, "get_screen_size", program_get_screen_size );
		lua_register( L, "nb_players", program_nb_players );
		lua_register( L, "get_unit_number_for_player", program_get_unit_number_for_player );
		lua_register( L, "get_unit_owner", program_get_unit_owner );
		lua_register( L, "get_unit_number", program_get_unit_number );
		lua_register( L, "get_max_unit_number", program_get_max_unit_number );
		lua_register( L, "annihilated", program_annihilated );
		lua_register( L, "has_unit", program_has_unit );
		lua_register( L, "create_unit", program_create_unit );
		lua_register( L, "change_unit_owner", program_change_unit_owner );
		lua_register( L, "local_player", program_local_player );
		lua_register( L, "map_w", program_map_w );
		lua_register( L, "map_h", program_map_h );
		lua_register( L, "player_side", program_player_side );
		lua_register( L, "unit_x", program_unit_x );
		lua_register( L, "unit_y", program_unit_y );
		lua_register( L, "unit_z", program_unit_z );
		lua_register( L, "unit_pos", program_unit_pos );
		lua_register( L, "unit_angle", program_unit_angle );
		lua_register( L, "unit_piece_pos", program_unit_piece_pos );
		lua_register( L, "unit_piece_dir", program_unit_piece_dir );
		lua_register( L, "unit_height", program_unit_height );
		lua_register( L, "move_unit", program_move_unit );
		lua_register( L, "kill_unit", program_kill_unit );
		lua_register( L, "kick_unit", program_kick_unit );
		lua_register( L, "play", program_play );
		lua_register( L, "play_for", program_play_for );
		lua_register( L, "set_cam_pos", program_set_cam_pos );
		lua_register( L, "set_cam", program_set_cam );
		lua_register( L, "clf", program_clf );
		lua_register( L, "start_x", program_start_x );
		lua_register( L, "start_z", program_start_z );
		lua_register( L, "init_res", program_init_res );
		lua_register( L, "give_metal", program_give_metal );
		lua_register( L, "give_energy", program_give_energy );
		lua_register( L, "commander", program_commander );
		lua_register( L, "attack", program_attack );
		lua_register( L, "set_unit_health", program_set_unit_health );
		lua_register( L, "get_unit_health", program_get_unit_health );
		lua_register( L, "is_unit_of_type", program_is_unit_of_type );
		lua_register( L, "add_build_mission", program_add_build_mission );
		lua_register( L, "add_move_mission", program_add_move_mission );
		lua_register( L, "add_attack_mission", program_add_attack_mission );
		lua_register( L, "add_patrol_mission", program_add_patrol_mission );
		lua_register( L, "add_wait_mission", program_add_wait_mission );
		lua_register( L, "add_wait_attacked_mission", program_add_wait_attacked_mission );
		lua_register( L, "add_guard_mission", program_add_guard_mission );
		lua_register( L, "set_standing_orders", program_set_standing_orders );
		lua_register( L, "unlock_orders", program_unlock_orders );
		lua_register( L, "lock_orders", program_lock_orders );
		lua_register( L, "nb_unit_of_type", program_nb_unit_of_type );
		lua_register( L, "create_feature", program_create_feature );
		lua_register( L, "has_mobile_units", program_has_mobile_units );
		lua_register( L, "send_signal", program_send_signal );
		lua_register( L, "allied", program_allied );
	}

	LuaProgram::LuaProgram()
	{
		LuaProgram::inGame = this;
		init();
	}

	int LuaProgram::run(float dt, bool /*alone*/)									// Run the script
	{
		if (!is_running())
			return -1;

		if (waiting && (amx!=mouse_x || amy!=mouse_y || amz!=mouse_z || amb!=mouse_b || keypressed()))
			waiting = false;

		int result = LuaThread::run(dt);		// Run the thread

		amx = mouse_x;
		amy = mouse_y;
		amz = mouse_z;
		amb = mouse_b;

		return result;
	}


	int LuaProgram::check()
	{
		pMutex.lock();
		draw_list.draw(gfx->big_font);			// Execute all display commands
		pMutex.unlock();

		int ret = signal;
		signal = 0;
		return ret;
	}

	void LuaProgram::proc(void* /*param*/)
	{
		uint32 last_tick = units.current_tick;
		signal = 0;
		while (isRunning() && is_running())
		{
			uint32 now = units.current_tick;
			float dt = ((float)(now - last_tick)) / TICKS_PER_SEC;
			last_tick = now;
			int ret = run(dt);
			if (ret != 0)
				signal = ret;
			do
				if (suspend(1))
					return;
			while (signal && isRunning() && is_running());
		}
	}

	void LuaProgram::init()
	{
		LuaThread::init();

		draw_list.init();

		amx = amy = amz = 0;
		amb = 0;

		signal = 0;
	}

	void LuaProgram::destroy()
	{
		LuaThread::destroy();

		draw_list.destroy();

		init();
	}

	// Create the script that will do what the mission description .ota file tells us to do
	void generate_script_from_mission( String Filename, TDFParser& ota_parser, int schema)
	{
		Yuni::Core::IO::File::Stream m_File(Filename, Yuni::Core::IO::OpenMode::write);

		if (!m_File.opened())
		{
			LOG_ERROR(LOG_PREFIX_SCRIPT << "Could not open file `" << Filename << "` (" << __FILE__ << ", " <<  __LINE__);
			return;
		}

		m_File << "#include \"signals.lh\"\n";
		m_File << "\nclf()\ninit_res()\n";
		m_File << "set_cam_pos( 0, start_x( 0 ), start_z( 0 ) )\n";

		int i = 0;
		String unit_name;

		while( !(unit_name = ota_parser.pullAsString( String("GlobalHeader.Schema ") << schema << ".units.unit" << i << ".Unitname")).empty())
		{
			String unit_key = String("GlobalHeader.Schema ") << schema << ".units.unit" << i;
			int player_id = ota_parser.pullAsInt( String(unit_key) << ".player" ) - 1;
			float x = ota_parser.pullAsFloat( String(unit_key) << ".XPos" ) * 0.5f;
			float z = ota_parser.pullAsFloat( String(unit_key) << ".ZPos" ) * 0.5f;

			m_File << String("\nunit_id = create_unit( ") << player_id << ", \"" << unit_name << "\", " << x << " - 0.5 * map_w(), " << z << " - 0.5 * map_h() )\n";

			const float health = ota_parser.pullAsFloat( String(unit_key) << ".HealthPercentage", -1.0f );
			if (health != -1.0f)
				m_File << String("set_unit_health( unit_id, ") << health << " )\n";

			const String Ident = ota_parser.pullAsString( String(unit_key) << ".Ident" );
			if (!Ident.empty() )
				m_File << Ident << " = unit_id\n";		// Links the unit_id to the given name

			m_File << unit_name << " = unit_id\n";		// Links the unit_id to the given unit_name so it can be used as an identifier

			String::Vector orders;
			ota_parser.pullAsString(String(unit_key) << ".InitialMission").explode(orders, ',');

			bool selectable = false;
			bool orders_given = false;

			for (String::Vector::const_iterator e = orders.begin(); e != orders.end(); ++e)	// Converts InitialMission to a mission list
			{
				String::Vector params;
				e->explode(params, ' ');
				if (params.empty())
					continue;

				params[0].toLower();

				if (params[0][0] == 'p' && params[0].size() > 1) // something like p3000 2000, convert it to p 3000 2000
				{
					params.resize( params.size() + 1 );
					for (int i = (int)params.size() - 1; i > 0; --i)
						if (i == 1)
						{
							params[ 1 ] = Substr(params[ 0 ], 1, params[ 0 ].size() - 1 );
							params[ 0 ] = params[ 0 ][ 0 ];
						}
						else
							params[ i ] = params[ i - 1 ];
				}

				if (params[ 0 ] == "m") // Move
				{
					if (params.size() >= 3)
					{
						const float pos_x = params[ 1 ].to<float>() * 0.5f;
						const float pos_z = params[ 2 ].to<float>() * 0.5f;
						m_File << String("add_move_mission( unit_id, ") << pos_x << " - 0.5 * map_w(), " << pos_z << " - 0.5 * map_h() )\n";
					}
					orders_given = true;
				}
				else if (params[ 0 ] == "a")		// Attack
				{
					if (params.size() >= 2)
						m_File << "add_attack_mission( unit_id, " << params[ 1 ] << " )\n";
					orders_given = true;
				}
				else if (params[ 0 ] == "b")		// Build
				{
					if (params.size() == 3)			// Factories
					{
						m_File << "for i = 1, " << params[ 2 ] << " do\n";
						m_File << "	add_build_mission( unit_id, unit_x( unit_id ), unit_z( unit_id ), " << params[ 1 ] << " )\n";
						m_File << "end\n";
					}
					else if (params.size() == 4)		// Mobile builders
					{
						float pos_x = params[ 2 ].to<float>() * 0.5f;
						float pos_z = params[ 3 ].to<float>() * 0.5f;
						m_File << "add_build_mission( unit_id, " << pos_x << " - 0.5 * map_w(), " << pos_z << " - 0.5 * map_h(), " << params[ 1 ] << " )\n";
					}
					orders_given = true;
				}
				else if (params[ 0 ] == "d")		// Destroy
					m_File << "kill_unit( unit_id )\n";
				else if (params[ 0 ] == "p")		// Patrol
				{
					unsigned int e = 0;
					while (params.size() >= 3 + e * 2)
					{
						float pos_x = params[ 2 * e + 1 ].to<float>() * 0.5f;
						float pos_z = params[ 2 * e + 2 ].to<float>() * 0.5f;
						m_File << "add_patrol_mission( unit_id, " << pos_x << " - 0.5 * map_w(), " << pos_z << " - 0.5 * map_h() )\n";
						++e;
					}
					orders_given = true;
				}
				else if (params[ 0 ] == "w")		// Wait
				{
					if (params.size() >= 2)
					{
						float time = params[ 1 ].to<float>();
						m_File << "add_wait_mission( unit_id, " << time << " )\n";
					}
					orders_given = true;
				}
				else if (params[ 0 ] == "wa")		// Wait attacked
				{
					if (params.size() >= 2)
						m_File << "add_wait_mission( unit_id, " << params[ 1 ] << " )\n";
					else
						m_File << "add_wait_mission( unit_id, unit_id )\n";
					orders_given = true;
				}
				else if (params[ 0 ] == "g")		// Guard
				{
					if (params.size() >= 2)
						m_File << "add_guard_mission( unit_id, " << params[ 1 ] << " )\n";
					orders_given = true;
				}
				else if (params[ 0 ] == "o")		// Set standing orders
				{
					if (params.size() >= 3)
						m_File << "set_standing_orders( unit_id, " << params[ 1 ] << ", " << params[ 2 ] << " )\n";
				}
				else if (params[ 0 ] == "s")		// Make it selectable
					selectable = true;
			}

			if (!selectable && orders_given)
				m_File << "lock_orders( unit_id )\n";

			++i;
		}

		i = 0;
		String feature_name;

		while( !(feature_name = ota_parser.pullAsString( String("GlobalHeader.Schema ") << schema << ".features.feature" << i << ".Featurename")).empty())
		{
			const String unit_key = String("GlobalHeader.Schema ") << schema << ".features.feature" << i;
			const float x = ota_parser.pullAsFloat( String(unit_key) << ".XPos" ) * 16.0f;
			const float z = ota_parser.pullAsFloat( String(unit_key) << ".ZPos" ) * 16.0f;

			m_File << "\ncreate_feature( \"" << feature_name << "\", " << x << " - 0.5 * map_w(), " << z << " - 0.5 * map_h() )\n";
			i++;
		}

		m_File << "\nlocal timer = time()\nlocal end_signal = 0\n";

		m_File << "local check = {}\nlocal first_launch = true\n";
		m_File << "for i = 0, get_max_unit_number() do\n";
		m_File << "	check[ i ] = false\n";
		m_File << "end\n\n";

		m_File << "local pos_x = {}\n";
		m_File << "local pos_z = {}\n";
		m_File << "local exist = {}\n";
		m_File << "for i = 0, get_max_unit_number() do\n";
		m_File << "	if get_unit_owner( i ) == -1 then\n";
		m_File << "		exist[ i ] = false\n";
		m_File << "	else\n";
		m_File << "		exist[ i ] = true\n";
		m_File << "		pos_x[ i ] = unit_x( i )\n";
		m_File << "		pos_z[ i ] = unit_z( i )\n";
		m_File << "	end\n";
		m_File << "end\n";

		if (!ota_parser.pullAsString("GlobalHeader.KillUnitType").empty())
		{
			String::Vector params;
			ota_parser.pullAsString("GlobalHeader.KillUnitType").explode(params, ',');
			if (params.size() >= 2)
			{
				m_File << "\nKillUnitType_nb = nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n" ;
				m_File << "local KilledUnitType = 0\n";
			}
		}

		if (!ota_parser.pullAsString( "GlobalHeader.UnitTypeKilled" ).empty())
		{
			String::Vector params;
			ota_parser.pullAsString("GlobalHeader.UnitTypeKilled").explode(params, ',');
			if (params.size() >= 2)
			{
				m_File << "\nUnitTypeKilled_nb = nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n" ;
				m_File << "local UnitTypeKilled_count = 0\n";
			}
		}

		m_File << "local AnyUnitPassesZ = false\n";
		m_File << "local UnitTypePassesZ = false\n";
		m_File << "local AnyUnitPassesX = false\n";
		m_File << "local UnitTypePassesX = false\n";
		m_File << "local BuildUnitType = false\n";
		m_File << "local CaptureUnitType = false\n";
		m_File << "local KillUnitType = false\n";
		m_File << "local KillAllOfType = false\n";
		m_File << "local KilledEnemyCommander = false\n";
		m_File << "local DestroyAllUnits = false\n";
		m_File << "local MoveUnitToRadius = false\n";
		m_File << "local KillAllMobileUnits = false\n";

		m_File << "\nlocal function main()\n";

		m_File << "	if end_signal ~= 0 and time() - timer >= 5 then\n";
		m_File << "		return end_signal\n";
		m_File << "	elseif end_signal ~= 0 then\n";
		m_File << "		return 0\n";
		m_File << "	end\n\n";

		// DEFEAT conditions

		if (ota_parser.pullAsInt( "GlobalHeader.DeathTimerRunsOut" ) > 0 ) {
			m_File << "	if time() >= " << ota_parser.pullAsString( "GlobalHeader.DeathTimerRunsOut" ) << " then\n";
			m_File << "     local w, h = get_image_size( \"gfx/defeat.png\" )\n";
			m_File << "     local sw, sh = get_screen_size()\n";
			m_File << "		w, h = w * 640 / sw, h * 480 / sh\n";
			m_File << "		draw_image( \"gfx/defeat.png\", 320 - w * 0.5, 240 - h * 0.5, 320 + w * 0.5, 240 + h * 0.5 )\n";
			m_File << "		timer = time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
			m_File << "	end\n\n";
		}

		if (!ota_parser.pullAsString("GlobalHeader.UnitTypeKilled").empty())
		{
			String::Vector params;
			ota_parser.pullAsString("GlobalHeader.UnitTypeKilled").explode(params, ',');
			if (params.size() >= 2)
			{
				m_File << "	local new_UnitTypeKilled_nb = nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n";
				m_File << "	if UnitTypeKilled_nb > new_UnitTypeKilled_nb then\n";
				m_File << "		UnitTypeKilled_count = UnitTypeKilled_count + UnitTypeKilled_nb - new_UnitTypeKilled_nb\n";
				m_File << "	end\n";
				m_File << "	if UnitTypeKilled_count >= " << params[ 1 ] << " then\n";
				m_File << "     local w, h = get_image_size( \"gfx/defeat.png\" )\n";
				m_File << "     local sw, sh = get_screen_size()\n";
				m_File << "		w, h = w * 640 / sw, h * 480 / sh\n";
				m_File << "		draw_image( \"gfx/defeat.png\", 320 - w * 0.5, 240 - h * 0.5, 320 + w * 0.5, 240 + h * 0.5 )\n";
				m_File << "		timer = time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
				m_File << "	end\n";
				m_File << "	UnitTypeKilled_nb = new_UnitTypeKilled_nb\n\n";
			}
		}

		if (ota_parser.pullAsInt( "GlobalHeader.AllUnitsKilled" ) == 1 )
		{
			m_File << "	if annihilated( 0 ) then\n";
			m_File << "     local w, h = get_image_size( \"gfx/defeat.png\" )\n";
			m_File << "     local sw, sh = get_screen_size()\n";
			m_File << "		w, h = w * 640 / sw, h * 480 / sh\n";
			m_File << "		draw_image( \"gfx/defeat.png\", 320 - w * 0.5, 240 - h * 0.5, 320 + w * 0.5, 240 + h * 0.5 )\n";
			m_File << "		timer = time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
			m_File << "	end\n\n";
		}

		if (!ota_parser.pullAsString( "GlobalHeader.AllUnitsKilledOfType" ).empty() ) {
			String type = ota_parser.pullAsString( "GlobalHeader.AllUnitsKilledOfType" );
			m_File << "	if not has_unit( 0, \"" << type << "\" ) and not has_unit( 1, \"" << type << "\" ) then\n";
			m_File << "     local w, h = get_image_size( \"gfx/defeat.png\" )\n";
			m_File << "     local sw, sh = get_screen_size()\n";
			m_File << "		w, h = w * 640 / sw, h * 480 / sh\n";
			m_File << "		draw_image( \"gfx/defeat.png\", 320 - w * 0.5, 240 - h * 0.5, 320 + w * 0.5, 240 + h * 0.5 )\n";
			m_File << "		timer = time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
			m_File << "	end\n\n";
		}

		if (ota_parser.pullAsInt( "GlobalHeader.CommanderKilled" ) == 1 ) {
			m_File << "	if not has_unit( 0, commander( 0 ) ) then\n";
			m_File << "     local w, h = get_image_size( \"gfx/defeat.png\" )\n";
			m_File << "     local sw, sh = get_screen_size()\n";
			m_File << "		w, h = w * 640 / sw, h * 480 / sh\n";
			m_File << "		draw_image( \"gfx/defeat.png\", 320 - w * 0.5, 240 - h * 0.5, 320 + w * 0.5, 240 + h * 0.5 )\n";
			m_File << "		timer = time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
			m_File << "	end\n\n";
		}

		// VICTORY conditions

		m_File << "	local victory_conditions = 0\n";
		int nb_victory_conditions = 0;

		m_File << "	if UnitTypePassesX then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if UnitTypePassesZ then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if AnyUnitPassesX then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if AnyUnitPassesZ then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if BuildUnitType then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if KillUnitType then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if CaptureUnitType then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if KillAllOfType then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if KilledEnemyCommander then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if DestroyAllUnits then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if MoveUnitToRadius then\n		victory_conditions = victory_conditions + 1\n	end\n";
		m_File << "	if KillAllMobileUnits then\n		victory_conditions = victory_conditions + 1\n	end\n";

		if (ota_parser.pullAsInt( "GlobalHeader.KillAllMobileUnits" ) == 1 )
		{
			m_File << "	if not KillAllMobileUnits and not has_mobile_units( 1 ) then\n";
			m_File << "		KillAllMobileUnits = true\n";
			m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "	end\n";
		}

		if (!ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesZ" ).empty()
			|| !ota_parser.pullAsString( "GlobalHeader.UnitTypePassesZ" ).empty()
			|| !ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesX" ).empty()
			|| !ota_parser.pullAsString( "GlobalHeader.UnitTypePassesX" ).empty() )
		{

			if (!ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesZ" ).empty() )
				m_File << "	ZPass0 = 0.5 * ( " << ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesZ" ) << " - map_h() )\n";
			if (!ota_parser.pullAsString( "GlobalHeader.UnitTypePassesZ" ).empty() )
			{
				String::Vector params;
				ota_parser.pullAsString("GlobalHeader.UnitTypePassesZ").explode(params, ',');
				if (params.size() == 2 )
					m_File << "	ZPass1 = 0.5 * ( " << params[ 1 ] << " - map_h() )\n";
			}
			if (!ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesX" ).empty() )
				m_File << "	XPass0 = 0.5 * ( " << ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesX" ) << " - map_w() )\n";
			if (!ota_parser.pullAsString( "GlobalHeader.UnitTypePassesX" ).empty() )
			{
				String::Vector params;
				ota_parser.pullAsString("GlobalHeader.UnitTypePassesX").explode(params, ',');
				if (params.size() == 2)
					m_File << "	XPass1 = 0.5 * ( " << params[1] << " - map_w() )\n";
			}

			m_File << "	for i = 0, get_max_unit_number() do\n";
			m_File << "		local z = unit_z( i )\n";
			m_File << "		local x = unit_x( i )\n";
			m_File << "		local unit_exist = ( get_unit_owner( i ) ~= -1 )\n";
			if (!ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesZ" ).empty() )
			{
				m_File << "		if exist[ i ] and unit_exist and (pos_z[ i ] - ZPass0) * (z - ZPass0) <= 0 and not AnyUnitPassesZ then\n";
				m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
				m_File << "			AnyUnitPassesZ = true\n";
				m_File << "		end\n";
			}
			if (!ota_parser.pullAsString( "GlobalHeader.UnitTypePassesZ" ).empty() )
			{
				String::Vector params;
				ota_parser.pullAsString("GlobalHeader.UnitTypePassesZ").explode(params, ',');
				if (params.size() == 2)
				{
					m_File << "		if exist[ i ] and unit_exist and is_unit_of_type( i, \"" << params[ 0 ] << "\" ) and (pos_z[ i ] - ZPass1) * (z - ZPass1) <= 0 and not UnitTypePassesZ then\n";
					m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
					m_File << "			UnitTypePassesZ = true\n";
					m_File << "		end\n";
				}
			}
			if (!ota_parser.pullAsString( "GlobalHeader.AnyUnitPassesX" ).empty() )
			{
				m_File << "		if exist[ i ] and unit_exist and (pos_x[ i ] - XPass0) * (x - XPass0) <= 0 and not AnyUnitPassesX then\n";
				m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
				m_File << "			AnyUnitPassesX = true\n";
				m_File << "		end\n";
			}
			if (!ota_parser.pullAsString( "GlobalHeader.UnitTypePassesX" ).empty() )
			{
				String::Vector params;
				ota_parser.pullAsString("GlobalHeader.UnitTypePassesX").explode(params, ',');
				if (params.size() == 2)
				{
					m_File << "		if exist[ i ] and unit_exist and is_unit_of_type( i, \"" << params[ 0 ] << "\" ) and (pos_x[ i ] - XPass1) * (x - XPass1) <= 0 and not UnitTypePassesX then\n";
					m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
					m_File << "			UnitTypePassesX = true\n";
					m_File << "		end\n";
				}
			}
			m_File << "		if unit_exist then\n";
			m_File << "			exist[ i ] = true\n";
			m_File << "			pos_x[ i ] = x\n";
			m_File << "			pos_z[ i ] = z\n";
			m_File << "		else\n";
			m_File << "			exist[ i ] = false\n";
			m_File << "		end\n";
			m_File << "	end\n";
		}

		if (!ota_parser.pullAsString( "GlobalHeader.BuildUnitType" ).empty() )
		{
			m_File << "	if has_unit( 0, \"" << ota_parser.pullAsString( "GlobalHeader.BuildUnitType" ) << "\" ) and not BuildUnitType then\n";
			m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "		BuildUnitType = true\n";
			m_File << "	end\n\n";
		}

		if (!ota_parser.pullAsString( "GlobalHeader.CaptureUnitType" ).empty() )
		{
			m_File << "	if has_unit( 0, \"" << ota_parser.pullAsString( "GlobalHeader.CaptureUnitType" ) << "\" ) and not CaptureUnitType then\n";
			m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "		CaptureUnitType = true\n";
			m_File << "	end\n\n";
		}

		if (!ota_parser.pullAsString( "GlobalHeader.KillUnitType" ).empty() )
		{
			String::Vector params;
			ota_parser.pullAsString("GlobalHeader.KillUnitType").explode(params, ',');
			if (params.size() >= 2 )
			{
				m_File << "	new_KillUnitType_nb = nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n";
				m_File << "	if KillUnitType_nb > new_KillUnitType_nb then\n";
				m_File << "		KilledUnitType = KilledUnitType + KillUnitType_nb - new_KillUnitType_nb\n";
				m_File << "	end\n";
				m_File << "	if KilledUnitType >= " << params[ 1 ] << " and not KillUnitType then\n";
				m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
				m_File << "		KillUnitType = true\n";
				m_File << "	end\n";
				m_File << "	KillUnitType_nb = new_KillUnitType_nb\n\n";
			}
		}

		if (!ota_parser.pullAsString( "GlobalHeader.KillAllOfType" ).empty() )
		{
			m_File << "	if not has_unit( 1, \"" << ota_parser.pullAsString( "GlobalHeader.KillAllOfType" ) << "\" ) and not KillAllOfType then\n";
			m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "		KillAllOfType = true\n";
			m_File << "	end\n\n";
		}

		if (ota_parser.pullAsInt( "GlobalHeader.KilledEnemyCommander" ) == 1 )
		{
			m_File << "	if not has_unit( 1, commander( 1 ) ) and not KilledEnemyCommander then\n";
			m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "		KilledEnemyCommander = true\n";
			m_File << "	end\n\n";
		}

		if (ota_parser.pullAsInt( "GlobalHeader.DestroyAllUnits" ) == 1 )
		{
			m_File << "	if annihilated( 1 ) and not DestroyAllUnits then\n";
			m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "		DestroyAllUnits = true\n";
			m_File << "	end\n\n";
		}

		if (!ota_parser.pullAsString( "GlobalHeader.MoveUnitToRadius" ).empty() )
		{
			String::Vector params;
			ota_parser.pullAsString("GlobalHeader.MoveUnitToRadius").explode(params, ',');
			m_File << "	for i = 0, get_max_unit_number() do\n";
			if (ToLower(params[0]) == "anytype")
				m_File << "		if get_unit_owner( i ) == 0 then\n";
			else
				m_File << "		if get_unit_owner( i ) == 0 and is_unit_of_type( i, \"" << params[0] << "\" ) then\n";
			m_File << "			local dx = unit_x( i ) + 0.5 * (map_w() - " << params[ 1 ] << " )\n";
			m_File << "			local dz = unit_z( i ) + 0.5 * (map_h() - " << params[ 2 ] << " )\n";
			m_File << "			local dist = dx * dx + dz * dz\n";
			const float dist = (float)params[ 3 ].to<sint32>() * 0.5f;
			m_File << "			if dist <= " << (dist * dist) << " then\n";
			m_File << "				if not first_launch and not check[ i ] and not MoveUnitToRadius then\n";
			m_File << "					victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
			m_File << "					MoveUnitToRadius = true\n";
			m_File << "				end\n";
			m_File << "				check[ i ] = true\n";
			m_File << "			end\n";
			m_File << "		end\n";
			m_File << "	end\n";
		}

		m_File << "	if victory_conditions == " << nb_victory_conditions << " then\n";
		m_File << "		play( \"VICTORY2\" )\n";
		m_File << "     local w, h = get_image_size( \"gfx/victory.png\" )\n";
		m_File << "     local sw, sh = get_screen_size()\n";
		m_File << "		w, h = w * 640 / sw, h * 480 / sh\n";
		m_File << "		draw_image( \"gfx/victory.png\", 320 - w * 0.5, 240 - h * 0.5, 320 + w * 0.5, 240 + h * 0.5 )\n";
		m_File << "		timer = time()\n";
		m_File << "		end_signal = SIGNAL_VICTORY\n";
		m_File << "	end\n";

		m_File << "	first_launch = false\n";

		m_File << "	return 0\n";
		m_File << "end\n";

		m_File << "\n";
		m_File << "while true do\n";
		m_File << "    game_signal( main() )\n";
		m_File << "    sleep(0.1)\n";             // 10 times a sec max is more than enough
		m_File << "end\n\n";

		m_File.flush();
		m_File.close();

		// Reset the VFS manager (because the VFS doesn't know what we have done)
		VFS::Instance()->reload();
	}


	void LuaProgram::signalExitThread()
	{
		while(!pDead)
			kill();
	}


} // namespace TA3D

