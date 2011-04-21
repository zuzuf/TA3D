/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005   Roland BROCHARD

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

/*----------------------------------------------------------\
  |                       UnitEngine.cpp                      |
  |    Contains the unit engine, which simulates units and    |
  | make them interact with each other.                       |
  |                                                           |
  \----------------------------------------------------------*/

#include "stdafx.h"
#include "misc/math.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "mesh/mesh.h"              // To handle 3D models
#include "tdf.h"					// For map features
#include "EngineClass.h"
#include "UnitEngine.h"
#include "network/TA3D_Network.h"
#include "gfx/fx.h"
#include "misc/camera.h"
#include "ingame/sidedata.h"
#include "languages/i18n.h"
#include "sounds/manager.h"
#include "ingame/players.h"
#include "scripts/cob.vm.h"
#include "scripts/unit.script.h"
#include "engine/unit.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "engine.h"




namespace TA3D
{
	int MAX_UNIT_PER_PLAYER = 2000; 		// 2000 Units per player by default


	INGAME_UNITS units;



	void INGAME_UNITS::set_wind_change()
	{
		pMutex.lock();
		wind_change = true;
		pMutex.unlock();
	}


	INGAME_UNITS::INGAME_UNITS()
	{
		init();
	}


	void INGAME_UNITS::destroy(bool delete_interface)
	{
		shootallMode = false;
		pMutex.lock();

		if (delete_interface)
		{
			for (int i = 0 ; i < 13 ; ++i)
				icons[i].destroy();

			DeleteInterface();			// Shut down the interface
		}

		DELETE_ARRAY(mini_pos);
		DELETE_ARRAY(mini_col);

		DELETE_ARRAY(idx_list);
		DELETE_ARRAY(free_idx);
		if (max_unit > 0 && unit)			// Destroy all units
			for(size_t i = 0; i < max_unit; ++i)
				unit[i].destroy(true);
		DELETE_ARRAY(unit);
		pMutex.unlock();
		init();
	}


	void INGAME_UNITS::give_order_move(int player_id, const Vector3D& target,bool set,byte flags)
	{
		pMutex.lock();

		for (uint32 e = 0U ; e < index_list_size ; ++e)
		{
			uint32 i = idx_list[e];
			if ((unit[i].flags & 1)
				&& unit[i].owner_id == player_id
				&& unit[i].sel
				&& Yuni::Math::Zero(unit[i].build_percent_left)
				&& unit_manager.unit_type[unit[i].type_id]->canmove)
			{
				if (set)
					unit[i].set_mission(MISSION_MOVE, &target, false, 0, true, NULL, flags);
				else
					unit[i].add_mission(MISSION_MOVE, &target, false, 0, NULL, flags);
				if (unit_manager.unit_type[unit[i].type_id]->BMcode && set)
					unit[i].playSound("ok1");
			}
		}
		pMutex.unlock();
	}


	void INGAME_UNITS::give_order_patrol(int player_id, const Vector3D& target, bool set)
	{
		pMutex.lock();
		for (uint16 e = 0; e < index_list_size; ++e)
		{
			uint16 i = idx_list[e];
			if ((unit[i].flags & 1)
				&& unit[i].owner_id == player_id
				&& unit[i].sel
				&& Yuni::Math::Zero(unit[i].build_percent_left)
				&& unit_manager.unit_type[unit[i].type_id]->canpatrol)
			{
				if (set)
					unit[i].set_mission(MISSION_PATROL, &target, false, 0, true, NULL);
				else
					unit[i].add_mission(MISSION_PATROL, &target, false, 0, NULL);
				if (unit_manager.unit_type[unit[i].type_id]->BMcode && set)
					unit[i].playSound("ok1");
			}
		}
		pMutex.unlock();
	}


	void INGAME_UNITS::give_order_guard(int player_id,int target,bool set)
	{
		pMutex.lock();
		for (uint16 e = 0; e < index_list_size; ++e)
		{
			uint16 i = idx_list[e];
			if ((unit[i].flags & 1)
				&& unit[i].owner_id == player_id
				&& unit[i].sel
				&& Yuni::Math::Zero(unit[i].build_percent_left)
				&& unit_manager.unit_type[unit[i].type_id]->canguard)
			{
				if (set)
                    unit[i].set_mission(MISSION_GUARD,&unit[target].Pos,false,0,true,&(unit[target]));
				else
                    unit[i].add_mission(MISSION_GUARD,&unit[target].Pos,false,0,&(unit[target]));
				if (unit_manager.unit_type[unit[i].type_id]->BMcode && set )
					unit[i].playSound( "ok1" );
			}
		}
		pMutex.unlock();
	}


	void INGAME_UNITS::give_order_unload(int player_id, const Vector3D& target, bool set)
	{
		pMutex.lock();
		for (uint32 e = 0; e < index_list_size; ++e)
		{
			const uint32 i = idx_list[e];
			if ((unit[i].flags & 1)
				&& unit[i].owner_id == player_id
				&& unit[i].sel
				&& Yuni::Math::Zero(unit[i].build_percent_left)
				&& unit_manager.unit_type[unit[i].type_id]->canload
				&& unit_manager.unit_type[unit[i].type_id]->BMcode && unit[i].nb_attached > 0 )
			{
				if (set)
				{
                    unit[i].set_mission(MISSION_UNLOAD, &target, false, 0, true, NULL);
					unit[i].playSound("ok1");
				}
				else
                    unit[i].add_mission(MISSION_UNLOAD, &target, false, 0, NULL);
			}
		}
		pMutex.unlock();
	}



	void INGAME_UNITS::give_order_load(int player_id, int target, bool set)
	{
		pMutex.lock();
		if (unit[target].flags == 0 || !unit_manager.unit_type[unit[target].type_id]->canmove)
		{
			pMutex.unlock();
			return;
		}
		switch(unit_manager.unit_type[unit[target].type_id]->TEDclass)
		{
			case CLASS_UNDEF:
			case CLASS_WATER:
			case CLASS_SHIP:
			case CLASS_PLANT:
			case CLASS_SPECIAL:
			case CLASS_FORT:
				pMutex.unlock();
				return;
				break;
		}
		for (uint32 e = 0; e < index_list_size; ++e)
		{
			const uint32 i = idx_list[e];
			if ((unit[i].flags & 1)
				&& unit[i].owner_id == player_id
				&& unit[i].sel
				&& Yuni::Math::Zero(unit[i].build_percent_left)
				&& unit_manager.unit_type[unit[i].type_id]->canload
				&& unit_manager.unit_type[unit[i].type_id]->BMcode)
			{
				if (set)
				{
                    unit[i].set_mission(MISSION_LOAD, &unit[target].Pos, false, 0, true, &(unit[target]));
					unit[i].playSound("ok1");
				}
				else
                    unit[i].add_mission(MISSION_LOAD, &unit[target].Pos, false, 0, &(unit[target]));
			}
		}
		pMutex.unlock();
	}



	void INGAME_UNITS::give_order_build(int player_id, int unit_type_id, const Vector3D& target, bool set)
	{
		if (unit_type_id < 0)
			return;

		Vector3D t(target);
		t.x = float(((int)(t.x) + map->map_w_d) >> 3);
		t.z = float(((int)(t.z) + map->map_h_d) >> 3);
		t.y = map->get_max_rect_h((int)t.x, (int)t.z, unit_manager.unit_type[unit_type_id]->FootprintX,
								  unit_manager.unit_type[unit_type_id]->FootprintZ);
		if (unit_manager.unit_type[unit_type_id]->floatting())
			t.y = Math::Max(t.y, map->sealvl + ((float)unit_manager.unit_type[unit_type_id]->AltFromSeaLevel - (float)unit_manager.unit_type[unit_type_id]->WaterLine) * H_DIV);
		t.x = t.x * 8.0f - (float)map->map_w_d;
		t.z = t.z * 8.0f - (float)map->map_h_d;

		pMutex.lock();
		for (uint32 e = 0; e < index_list_size; ++e)
		{
			const uint32 i = idx_list[e];
			if ((unit[i].flags & 1) && unit[i].owner_id == player_id && unit[i].sel && Yuni::Math::Zero(unit[i].build_percent_left) && unit_manager.unit_type[unit[i].type_id]->Builder)
			{
				if (set)
					unit[i].set_mission(MISSION_BUILD, &t, false, unit_type_id);
				else
					unit[i].add_mission(MISSION_BUILD, &t, false, unit_type_id);
			}
		}
		pMutex.unlock();
	}


	void INGAME_UNITS::init(bool register_interface)
	{
		shootallMode = false;

		pMutex.lock();

		visible_unit.clear();

		next_unit_ID = 1;
		mini_pos = NULL;
		mini_col = NULL;

		last_on = -1;
		current_tick = 0;
		last_tick[0]=0;
		last_tick[1]=0;
		last_tick[2]=0;
		last_tick[3]=0;
		last_tick[4]=0;
		apparent_timefactor = 1.0f;
		thread_running = false;
		thread_ask_to_stop = false;

		if (register_interface)
		{
			InitInterface();		// Initialization of the interface

			icons[ ICON_UNKNOWN ].load( "gfx/tactical_icons/unknown.tga" );
			icons[ ICON_BUILDER ].load( "gfx/tactical_icons/builder.tga" );
			icons[ ICON_GROUND_ASSAULT ].load( "gfx/tactical_icons/tank.tga" );
			icons[ ICON_LANDUNIT ].load( "gfx/tactical_icons/landunit.tga" );
			icons[ ICON_DEFENSE ].load( "gfx/tactical_icons/defense.tga" );
			icons[ ICON_ENERGY ].load( "gfx/tactical_icons/energy.tga" );
			icons[ ICON_METAL ].load( "gfx/tactical_icons/metal.tga" );
			icons[ ICON_WATERUNIT ].load( "gfx/tactical_icons/waterunit.tga" );
			icons[ ICON_COMMANDER ].load( "gfx/tactical_icons/commander.tga" );
			icons[ ICON_SUBUNIT ].load( "gfx/tactical_icons/subunit.tga" );
			icons[ ICON_AIRUNIT ].load( "gfx/tactical_icons/airunit.tga" );
			icons[ ICON_FACTORY ].load( "gfx/tactical_icons/factory.tga" );
			icons[ ICON_KAMIKAZE ].load( "gfx/tactical_icons/kamikaze.tga" );
		}

		sound_min_ticks = 500;
		index_list_size = 0;
		for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
			free_index_size[i] = 0;
		idx_list = free_idx = NULL;
		page = 0;
		nb_unit = 0;
		unit = NULL;
		max_unit = 0;
		nb_attacked = 0.0f;
		nb_built = 0.0f;
		exp_dt_1 = 0.0f;
		exp_dt_2 = 0.0f;
		exp_dt_4 = 0.0f;
		g_dt = 0.0f;

		pMutex.unlock();
	}


	bool INGAME_UNITS::selectUnits(const RectTest &reigon)
	{
		bool selected = false;

		for (std::vector<uint32>::const_iterator e = visible_unit.begin(); e != visible_unit.end(); ++e)
		{
			unit[*e].lock();

			// Select only units completely built and visible
			if (unit[*e].owner_id == players.local_human_id
				&& (unit[*e].flags & 1)
				&& Yuni::Math::Zero(unit[*e].build_percent_left)
				&& unit[*e].visible)
			{
				if (TA3D_SHIFT_PRESSED && unit[*e].sel)
				{
					selected = true;
				}
				else
				{
					if (!TA3D_SHIFT_PRESSED)
						unit[*e].sel = false;

					if (reigon.contains(unit[*e].Pos))
					{
						unit[*e].sel = true;
						selected = true;
					}
				}
			}
			unit[*e].unlock();
		}
		return selected;
	}


	int INGAME_UNITS::pick(Camera& cam)
	{
		int index = -1;

		if (nb_unit <= 0)
			return -1;

		// Things didn't change :-) seen from the mouse cursor since the screen wasn't refreshed
		if (last_on != -1)
			return last_on;

		Vector3D Dir;
		Vector3D CamPos;
		if (lp_CONFIG->ortho_camera)
		{
			Dir = cam.dir;
			CamPos = cam.pos + cam.zoomFactor * (float(mouse_x - gfx->SCREEN_W_HALF) * cam.side - float(mouse_y - gfx->SCREEN_H_HALF) * cam.up);
		}
		else
		{
			CamPos = cam.pos;
			Dir = cam.dir + cam.widthFactor * 2.0f * float(mouse_x - gfx->SCREEN_W_HALF) * gfx->SCREEN_W_INV
				* cam.side - 1.5f * float(mouse_y - gfx->SCREEN_H_HALF)
				* gfx->SCREEN_H_INV * cam.up;
			Dir.unit();		// Direction pointée par le curseur
		}

		std::vector<uint16> detectable;

		for (std::vector<uint32>::const_iterator e = visible_unit.begin(); e != visible_unit.end(); ++e)
		{
			if (!(unit[*e].flags & 1) || !unit[*e].visible )
				continue;
			unit[ *e ].lock();
			if (!(unit[*e].flags & 1) || !unit[*e].visible )
			{
				unit[*e].unlock();
				continue;		// Si l'unité n'existe pas on la zappe
			}
			unit[*e].flags &= 0xFD;	// Enlève l'indicateur de possibilité d'intersection
			Vector3D center (unit[*e].model->center + unit[*e].Pos - CamPos);
			float size = unit[*e].model->size * unit_manager.unit_type[unit[*e].type_id]->Scale * unit_manager.unit_type[unit[*e].type_id]->Scale;
			center = Dir * center;
			float dist = center.sq();
			if (dist < size)
			{
				detectable.push_back(uint16(*e));
				unit[*e].flags |= 0x2;		// Unité détectable
			}
			unit[*e].unlock();
		}

		if (detectable.empty()) // If no unit is near the cursor, then skip the precise method
		{
			last_on = index;
			return index;
		}

		float best_dist = 1000000.0f;

		for (std::vector<uint16>::iterator e = detectable.begin(); e != detectable.end(); ++e)
		{
			if (!(unit[*e].flags & 1) || !unit[*e].visible)
				continue;
			unit[*e].lock();
			if (!(unit[*e].flags & 1) || !unit[*e].visible)
			{
				unit[*e].unlock();
				continue;		// Si l'unité n'existe pas on la zappe
			}
			if ((unit[*e].flags & 0x2) == 0x2) // Si l'unité existe et est sélectionnable
			{
				unit[*e].flags &= 0xFD;
				Vector3D D;
				if (unit[*e].hit(CamPos, Dir, &D, 1000000.0f)) // Vecteur "viseur unité" partant de la caméra vers l'unité
				{
					float dist = (D - CamPos).sq();
					if (dist < best_dist || index == -1 )
					{
						best_dist = dist;
						index = *e;
					}
				}
			}
			unit[*e].unlock();
		}

		last_on = index;
		return index;
	}

	int INGAME_UNITS::pick_minimap()
	{
		int index = -1;

		if (nb_unit <= 0)
			return index;

		// Things didn't change :-) seen from the mouse cursor since the screen wasn't refreshed
		if (last_on != -1 )
			return last_on;

		const float conv_x = ((float)map->mini_w) / (float)map->map_w * 128.0f / 252.0f;
		const float conv_z = ((float)map->mini_h) / (float)map->map_h * 128.0f / 252.0f;

		const byte player_mask = byte(1 << players.local_human_id);

		pMutex.lock();
		for(uint32 e = 0 ; e < index_list_size ; ++e)
		{
			const uint32 i = idx_list[e];
			pMutex.unlock();

			unit[ i ].lock();
			if (!(unit[i].flags & 1) )
			{
				unit[ i ].unlock();
				pMutex.lock();
				continue;		// Si l'unité n'existe pas on la zappe
			}

			if (!unit[i].visible) // Additional checks that have to be done
			{
				int px = unit[i].cur_px >> 1;
				int py = unit[i].cur_py >> 1;
				if (px < 0 || py < 0 || px >= map->bloc_w || py >= map->bloc_h)
				{
					unit[ i ].unlock();
					pMutex.lock();
					continue;	// Out of the map
				}
				if (!(map->view_map(px, py) & player_mask) && !(map->sight_map(px,py) & player_mask)
					&& !unit[i].is_on_radar( player_mask ))
				{
					unit[ i ].unlock();
					pMutex.lock();
					continue;	// Not visible
				}
			}

			const int x = (int)(unit[i].Pos.x * conv_x + 64.5f);
			const int y = (int)(unit[i].Pos.z * conv_z + 64.5f);

			if (x == mouse_x && y == mouse_y )
			{
				last_on = i;
				unit[ i ].unlock();
				return i;
			}

			if (abs(mouse_x - x) <= 1 && abs(mouse_y - y) <= 1 )
				index = i;

			unit[i].unlock();
			pMutex.lock();
		}
		pMutex.unlock();

		last_on = index;
		return index;
	}

	void *create_unit( int type_id, int owner, Vector3D pos, bool sync, bool script )
	{
		const int id = units.create(type_id,owner);
		if (id >= 0)
		{
			// In order to avoid race conditions we have to update it now !
			players.annihilated[owner] = false;
			units.unit[id].lock();

			if (network_manager.isConnected())
			{
				units.unit[id].local = g_ta3d_network->isLocal( owner );
				if (sync) // Send event packet if needed
				{
					struct event event;
					event.type = EVENT_UNIT_CREATION;
					event.opt1 = uint16(id);
					event.opt2 = uint16(script ? (owner | 0x1000) : owner);
					event.x = pos.x;
					event.z = pos.z;
					memcpy( event.str, unit_manager.unit_type[type_id]->Unitname.c_str(), unit_manager.unit_type[type_id]->Unitname.size() + 1 );
					network_manager.sendEvent( &event );
				}
			}

			units.unit[id].Pos = pos;
			units.unit[id].build_percent_left = 100.0f;
			units.unit[id].cur_px = ((int)(units.unit[id].Pos.x) + the_map->map_w_d + 4) >> 3;
			units.unit[id].cur_py = ((int)(units.unit[id].Pos.z) + the_map->map_h_d + 4) >> 3;
			units.unit[id].birthTime = float(units.current_tick) / float(TICKS_PER_SEC);
			units.unit[id].unlock();

			units.unit[id].draw_on_map();

			return &(units.unit[id]);
		}
		return NULL;
	}


	bool can_be_there_ai(const int px, const int py, const int unit_type_id,
						 const int, const int unit_id, const bool leave_space )
	{
		if (unit_type_id < 0 || unit_type_id >= unit_manager.nb_unit)
			return false;

		const int w = unit_manager.unit_type[unit_type_id]->FootprintX;
		const int h = unit_manager.unit_type[unit_type_id]->FootprintZ;
		const int x = px - (w>>1);
		const int y = py - (h>>1);
		const int side = Yuni::Math::Zero(unit_manager.unit_type[unit_type_id]->ExtractsMetal) ? 12 : leave_space ? 12 : 0;
		if (x < 0 || y < std::max(0, (((int)the_map->get_zdec(x, 0) + 7) >> 3))
			|| x + w >= the_map->bloc_w_db - 1 || y + h >= the_map->bloc_h_db - 5 - std::max(0, (((int)the_map->get_zdec(x, the_map->bloc_h_db - 2) + 7) >> 3)))
			return false;	// check if it is inside the map

		if (!the_map->check_rect( px - ((w + side) >> 1), py - ((h + side) >> 1), w + side, h + side, unit_id))
			return false;		// There is already something
		const float dh = fabsf(the_map->check_rect_dh(x,y,w,h));
		const float max_depth = the_map->check_max_depth(x,y,w,h);
		const float min_depth = the_map->check_min_depth(x,y,w,h);

		if (max_depth > 0.0f && the_map->ota_data.whitefog)
			return false;

		if (dh > (float)unit_manager.unit_type[unit_type_id]->MaxSlope * H_DIV
			&& !( unit_manager.unit_type[unit_type_id]->canhover && min_depth <= the_map->sealvl ) )
			return false;	// Check the slope, check if hovering too

		// Check if unit can be there
		if (min_depth < (float)unit_manager.unit_type[unit_type_id]->MinWaterDepth * H_DIV
			|| (!unit_manager.unit_type[unit_type_id]->canhover && max_depth > (float)unit_manager.unit_type[unit_type_id]->MaxWaterDepth * H_DIV))
			return false;

		if (!the_map->check_vents(x, y, w, h, unit_manager.unit_type[unit_type_id]->yardmap))
			return false;

		if (the_map->check_lava((x+1)>>1,(y+1)>>1,(w+1)>>1,(h+1)>>1))
			return false;

		return true;
	}

	bool can_be_there( const int px, const int py, const int unit_type_id,
					   const int, const int unit_id )
	{
		if (unit_type_id < 0 || unit_type_id >= unit_manager.nb_unit)
			return false;

		const int w = unit_manager.unit_type[unit_type_id]->FootprintX;
		const int h = unit_manager.unit_type[unit_type_id]->FootprintZ;
		const int x = px - (w>>1);
		const int y = py - (h>>1);
		if (x < 0 || y < std::max(0, (((int)the_map->get_zdec(x,0) + 7) >> 3)) || x + w >= the_map->bloc_w_db - 1 || y + h >= the_map->bloc_h_db - 1)
			return false;	// check if it is inside the map

		if (!the_map->check_rect(x,y,w,h,unit_id))
			return false;		// There is already something

		const float dh = fabsf(the_map->check_rect_dh(x,y,w,h));
		const float max_depth = the_map->check_max_depth(x,y,w,h);
		const float min_depth = the_map->check_min_depth(x,y,w,h);

		if (max_depth > 0.0f && the_map->ota_data.whitefog)
			return false;

		if (dh > (float)unit_manager.unit_type[unit_type_id]->MaxSlope * H_DIV
			&& !( unit_manager.unit_type[unit_type_id]->canhover && min_depth <= the_map->sealvl ) )
			return false;	// Check the slope, check if hovering too

		// Check if unit can be there
		if (min_depth < (float)unit_manager.unit_type[unit_type_id]->MinWaterDepth * H_DIV
			|| (!unit_manager.unit_type[unit_type_id]->canhover && max_depth > (float)unit_manager.unit_type[unit_type_id]->MaxWaterDepth * H_DIV))
			return false;

		if (!the_map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id]->yardmap))
			return false;

		if (the_map->check_lava((x + 1) >> 1, (y + 1) >> 1, (w + 1) >> 1, (h + 1) >> 1))
			return false;

		return true;
	}

	bool can_be_built(const Vector3D& Pos,const int unit_type_id, const int player_id )
	{
		if (unit_type_id < 0 || unit_type_id >= unit_manager.nb_unit)
			return false;

		const int w = unit_manager.unit_type[unit_type_id]->FootprintX;
		const int h = unit_manager.unit_type[unit_type_id]->FootprintZ;
		const int x = (((int)(Pos.x) + the_map->map_w_d + 4) >> 3) - (w >> 1);
		const int y = (((int)(Pos.z) + the_map->map_h_d + 4) >> 3) - (h >> 1);
		if (x < 0 || y < (((int)the_map->get_zdec(x, 0) + 7) >> 3) || x + w >= the_map->bloc_w_db - 1 || y + h >= the_map->bloc_h_db - 1)
			return false;	// check if it is inside the map

		if (!the_map->check_rect(x, y, w, h, -1))
			return false;		// There already something
		const float dh = fabsf(the_map->check_rect_dh(x, y, w, h));
		const float max_depth = the_map->check_max_depth(x, y, w, h);
		const float min_depth = the_map->check_min_depth(x, y, w, h);

		if (max_depth > 0.0f && the_map->ota_data.whitefog)
			return false;

		if (!the_map->check_rect_discovered( x, y, w, h, 1 << player_id ) )
			return false;

		if (dh > (float)unit_manager.unit_type[unit_type_id]->MaxSlope * H_DIV)
			return false;	// Check the slope

		// Check if unit can be there
		if (min_depth < (float)unit_manager.unit_type[unit_type_id]->MinWaterDepth * H_DIV
			|| max_depth > (float)unit_manager.unit_type[unit_type_id]->MaxWaterDepth * H_DIV)
			return false;
		//	if (depth>0 && (unit_manager.unit_type[unit_type_id]->Category&NOTSUB))	return false;

		if (!the_map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id]->yardmap))
			return false;

		if (the_map->check_lava((x+1)>>1,(y+1)>>1,(w+1)>>1,(h+1)>>1))
			return false;

		return true;
	}


	void INGAME_UNITS::complete_menu(int index,bool hide_info,bool hide_bpic)
	{
		bool pointed_only = false;
		if (last_on >= 0 && ( last_on >= (int)max_unit || unit[ last_on ].flags == 0 )) 	last_on = -1;
		if (index >= 0 && (uint32)index < max_unit)
			unit[index].lock();
		if (index < 0 || index >= (int)max_unit || unit[index].flags == 0 || unit[index].type_id < 0)
		{
			if (last_on >= 0 )
				pointed_only = true;
			else
			{
				if (index >= 0 && (uint32)index < max_unit)
					unit[index].unlock();
				return;		// On n'affiche que des données sur les unités EXISTANTES
			}
		}

		Unit *target = pointed_only ? NULL : (!unit[index].mission.empty() ? unit[index].mission->getTarget().getUnit() : NULL);
		if (target && target->flags == 0)
			target = NULL;

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		if (!pointed_only && !hide_bpic)
		{
			int stock = 0;
			for(uint32 i = 0 ; i < unit_manager.unit_type[unit[index].type_id]->weapon.size() ; ++i)
				if (unit_manager.unit_type[unit[index].type_id]->weapon[i] && unit_manager.unit_type[unit[index].type_id]->weapon[i]->stockpile)
				{
					stock = unit[index].weapon[i].stock;
					break;
				}

			if ((unit_manager.unit_type[unit[index].type_id]->Builder && !unit_manager.unit_type[unit[index].type_id]->BMcode)
				|| unit[index].planned_weapons>0.0f || stock>0) // Affiche la liste de construction
			{
				int page = unit_manager.unit_type[unit[index].type_id]->page;

				glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
				for( int i = 0 ; i < unit_manager.unit_type[unit[index].type_id]->nb_unit ; ++i) // Affiche les différentes images d'unités constructibles
				{
					if (unit_manager.unit_type[unit[index].type_id]->Pic_p[i] != page )
						continue;
					int px = unit_manager.unit_type[unit[index].type_id]->Pic_x[ i ];
					int py = unit_manager.unit_type[unit[index].type_id]->Pic_y[ i ];
					int pw = unit_manager.unit_type[unit[index].type_id]->Pic_w[ i ];
					int ph = unit_manager.unit_type[unit[index].type_id]->Pic_h[ i ];

					int nb(0);
					for(MissionStack::iterator m = unit[index].mission.begin() ; m != unit[index].mission.end() ; ++m)
					{
						if ((m->lastMission() == MISSION_BUILD
							 || m->lastMission() == MISSION_BUILD_2)
							&& m->lastStep().getData() == unit_manager.unit_type[unit[index].type_id]->BuildList[i])
							++nb;
					}
					if (nb > 0)
					{
						String buf;
						buf << nb;
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						gfx->print(gfx->TA_font,
								   float(px + 1) + (float)pw * 0.5f - 0.5f * gfx->TA_font->length(buf),
								   float(py + 1) + (float)ph * 0.5f - 0.5f * gfx->TA_font->height(),
								   0.0f, Black, buf);
						gfx->print(gfx->TA_font,
								   (float)px + (float)pw * 0.5f - 0.5f * gfx->TA_font->length(buf),
								   (float)py + (float)ph * 0.5f - 0.5f * gfx->TA_font->height(),
								   0.0f, 0xFFFFFFFFU, buf);
					}
					else
					{
						if (unit_manager.unit_type[unit[index].type_id]->BuildList[i] == -1) // Il s'agit d'une arme / It's a weapon
						{
							String buf;
							if (float(int(unit[index].planned_weapons)) == unit[index].planned_weapons)
								buf << (int)unit[index].planned_weapons << '(' << stock << ')';
							else
								buf << (int)unit[index].planned_weapons + 1 << '(' << stock << ')';
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
							gfx->print(gfx->TA_font,
									   float(px + 1) + (float)pw * 0.5f - 0.5f * gfx->TA_font->length(buf),
									   float(py + 1) + (float)ph * 0.5f - 0.5f * gfx->TA_font->height(),
									   0.0f, Black, buf);
							gfx->print(gfx->TA_font,
									   (float)px + (float)pw * 0.5f - 0.5f * gfx->TA_font->length(buf),
									   (float)py + (float)ph * 0.5f - 0.5f * gfx->TA_font->height(),
									   0.0f, 0xFFFFFFFFU, buf);
						}
					}
				}
			}
		}

		if (index >= 0 && (uint32)index < max_unit)
			unit[index].unlock();

		if (last_on >= 0 )
		{
			index = last_on;
			unit[index].lock();
			if (unit[index].owner_id == players.local_human_id)
			{
				target = !unit[index].mission.empty() ? unit[index].mission->getTarget().getUnit() : NULL;
				if (target && target->flags == 0)
					target = NULL;
			}
			else
				target = NULL;
			unit[index].unlock();
		}

		if (!hide_info)
		{
			unit[index].lock();

			if (unit[index].type_id >= 0 && (unit[index].flags & 1) )
			{
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				gfx->print_center(gfx->normal_font,
								  (float)ta3dSideData.side_int_data[ players.side_view ].UnitName.x1,
								  (float)ta3dSideData.side_int_data[ players.side_view ].UnitName.y1,
								  0.0f,0xFFFFFFFF,unit_manager.unit_type[unit[index].type_id]->name);
				if (target && !unit[index].mission.empty()
					&& (unit[index].mission->getFlags() & MISSION_FLAG_TARGET_WEAPON) != MISSION_FLAG_TARGET_WEAPON)
				{
					unit[index].unlock();
					target->lock();
					if ((target->flags & 1) && target->type_id >= 0 )
					{
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						gfx->print_center(gfx->normal_font,
										  (float)ta3dSideData.side_int_data[ players.side_view ].UnitName2.x1,
										  (float)ta3dSideData.side_int_data[ players.side_view ].UnitName2.y1,
										  0.0f,0xFFFFFFFF,unit_manager.unit_type[target->type_id]->name);
					}
					target->unlock();
					unit[index].lock();
				}
				else if (unit[index].planned_weapons>0.0f && unit[index].owner_id == players.local_human_id )
				{
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					gfx->print_center(gfx->normal_font,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitName2.x1,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitName2.y1,
									  0.0f,0xFFFFFFFF,I18N::Translate("weapon"));
				}

				glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

				if (unit[index].owner_id == players.local_human_id  )
				{
					gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].metal_color );
					gfx->print_center(gfx->small_font,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitMetalMake.x1,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitMetalMake.y1,
									  0.0f, String().format("+%.2f",unit[index].cur_metal_prod));
					gfx->print_center(gfx->small_font,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitMetalUse.x1,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitMetalUse.y1,
									  0.0f,String().format("-%.2f",unit[index].cur_metal_cons));

					gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].energy_color );
					gfx->print_center(gfx->small_font,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitEnergyMake.x1,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitEnergyMake.y1,
									  0.0f,String().format("+%.2f",unit[index].cur_energy_prod));
					gfx->print_center(gfx->small_font,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitEnergyUse.x1,
									  (float)ta3dSideData.side_int_data[ players.side_view ].UnitEnergyUse.y1,
									  0.0f,String().format("-%.2f",unit[index].cur_energy_cons));
				}

				glColor4ub(0xFF,0xFF,0xFF,0xFF);

				glDisable(GL_TEXTURE_2D);

				glDisable(GL_BLEND);

				glBegin(GL_QUADS);
				glColor4ub(0xFF,0,0,0xFF);

				if (unit[index].owner_id == players.local_human_id || !unit_manager.unit_type[unit[index].type_id]->HideDamage )
				{
					glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y1 );
					glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y1 );
					glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y2 );
					glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y2 );
				}

				if (unit[index].owner_id == players.local_human_id )
				{
					if (target && (unit[index].mission->getFlags() & MISSION_FLAG_TARGET_WEAPON) != MISSION_FLAG_TARGET_WEAPON)
					{
						unit[index].unlock();
						target->lock();
						if ((target->flags & 1) && target->type_id >= 0 && !unit_manager.unit_type[target->type_id]->HideDamage)	// Si l'unité a une cible
						{
							glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
							glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
							glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
							glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
						}
						target->unlock();
						unit[index].lock();
					}
					else if (unit[index].planned_weapons>0.0f )
					{
						glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
						glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
						glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
						glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
					}
				}

				glColor3ub(0,0xFF,0);

				if (unit[index].hp>0 && ( unit[index].owner_id == players.local_human_id || !unit_manager.unit_type[unit[index].type_id]->HideDamage ) )
				{
					const UnitType* const pType = unit_manager.unit_type[unit[index].type_id];
					const InterfaceData &side_data = ta3dSideData.side_int_data[ players.side_view ];
					glVertex2f( (float)side_data.DamageBar.x1,
								(float)side_data.DamageBar.y1 );
					glVertex2f( (float)side_data.DamageBar.x1 + unit[index].hp / float(pType->MaxDamage) * float(side_data.DamageBar.x2 - side_data.DamageBar.x1),
								(float)side_data.DamageBar.y1 );
					glVertex2f( (float)side_data.DamageBar.x1 + unit[index].hp / float(pType->MaxDamage) * float(side_data.DamageBar.x2 - side_data.DamageBar.x1),
								(float)side_data.DamageBar.y2 );
					glVertex2f( (float)side_data.DamageBar.x1,
								(float)side_data.DamageBar.y2 );
				}

				if (unit[index].owner_id == players.local_human_id)
				{
					if (target && (unit[index].mission->getFlags() & MISSION_FLAG_TARGET_WEAPON) != MISSION_FLAG_TARGET_WEAPON )
					{
						unit[index].unlock();
						target->lock();
						if ((target->flags & 1) && target->type_id >= 0 && !unit_manager.unit_type[target->type_id]->HideDamage && target->hp > 0)
						{
							const UnitType* const pType = unit_manager.unit_type[target->type_id];
							const InterfaceData &side_data = ta3dSideData.side_int_data[ players.side_view ];
							glVertex2f( (float)side_data.DamageBar2.x1,
										(float)side_data.DamageBar2.y1 );
							glVertex2f( (float)side_data.DamageBar2.x1 + target->hp / float(pType->MaxDamage) * float(side_data.DamageBar2.x2 - side_data.DamageBar2.x1),
										(float)side_data.DamageBar2.y1 );
							glVertex2f( (float)side_data.DamageBar2.x1 + target->hp / float(pType->MaxDamage) * float(side_data.DamageBar2.x2 - side_data.DamageBar2.x1),
										(float)side_data.DamageBar2.y2 );
							glVertex2f( (float)side_data.DamageBar2.x1,
										(float)side_data.DamageBar2.y2 );
						}
						target->unlock();
						unit[index].lock();
					}
					else if (unit[index].planned_weapons > 0.0f) 	// construit une arme / build a weapon
					{
						float p = 1.0f - (unit[index].planned_weapons - float(int(unit[index].planned_weapons)));
						if (p == 1.0f)
							p = 0.0f;
						const InterfaceData &side_data = ta3dSideData.side_int_data[ players.side_view ];
						glVertex2f( (float)side_data.DamageBar2.x1,
									(float)side_data.DamageBar2.y1 );
						glVertex2f( (float)side_data.DamageBar2.x1 + p * float(side_data.DamageBar2.x2 - side_data.DamageBar2.x1),
									(float)side_data.DamageBar2.y1 );
						glVertex2f( (float)side_data.DamageBar2.x1 + p * float(side_data.DamageBar2.x2 - side_data.DamageBar2.x1),
									(float)side_data.DamageBar2.y2 );
						glVertex2f( (float)side_data.DamageBar2.x1,
									(float)side_data.DamageBar2.y2 );
					}
				}

				glEnd();
			}

			unit[index].unlock();
		}
		else
		{
			glDisable( GL_BLEND );
			glDisable( GL_TEXTURE_2D );
		}
		glColor4ub(0xFF,0xFF,0xFF,0xFF);
	}

	void INGAME_UNITS::move(float dt,int key_frame,bool wind_change)
	{
		if (nb_unit <= 0 || unit == NULL)
		{
			suspend(1);
			return;// No units to move
		}

		// Build a KDTree to allow fast look ups when looking for targets
		std::vector<UnitTKit::T> detectableUnits[10];
		std::vector<UnitTKit::T> allUnits[10];
		std::vector<UnitTKit::T> repairPads[10];
		pMutex.lock();
		for (uint32 e = 0U ; e < index_list_size ; ++e) // Compte les stocks de ressources et les productions
		{
			const uint32 i = idx_list[e];
			const Unit* const pUnit = &(unit[i]);
			if (!(pUnit->flags & 1))
				continue;
			const unsigned int owner = pUnit->owner_id;
			const int type = pUnit->type_id;
			const UnitType* const pUnitType = (type >= 0) ? unit_manager.unit_type[type] : NULL;
			if (type >= 0 && owner < 10)
			{
				allUnits[owner].push_back(std::make_pair(pUnit, pUnit->Pos));
				if (pUnitType->IsAirBase)
					repairPads[owner].push_back(std::make_pair(pUnit, pUnit->Pos));
			}
			if (type < 0 || (!shootallMode && !pUnitType->ShootMe))
				continue;
			if (owner < 10)
				detectableUnits[owner].push_back(std::make_pair(pUnit, pUnit->Pos));
		}
		static MemoryPool<KDTree<UnitTKit::T, UnitTKit> > pool(32768U);
		pool.reset();
		pMutex.unlock();
		for(int i = 0 ; i < NB_PLAYERS ; ++i)
		{
			kdTree[i] = KDTree<UnitTKit::T, UnitTKit>::create(&pool, detectableUnits[i].begin(), detectableUnits[i].end());
			kdTreeFriends[i] = KDTree<UnitTKit::T, UnitTKit>::create(&pool, allUnits[i].begin(), allUnits[i].end());
			kdTreeRepairPads[i] = KDTree<UnitTKit::T, UnitTKit>::create(&pool, repairPads[i].begin(), repairPads[i].end());
		}

		players.clear();		// Réinitialise le compteur de ressources

		uint32 i;
		pMutex.lock();
		for (unsigned int e = 0; e < index_list_size; ++e) // Compte les stocks de ressources et les productions
		{
			i = idx_list[e];
			pMutex.unlock();

			unit[i].lock();

			if (unit[i].flags == 0 || unit[i].type_id == -1)        // It has nothing to do there
			{
				unit[i].unlock();
				pMutex.lock();
				continue;
			}

			if (unit[i].just_created
				&& !Yuni::Math::Zero(unit_manager.unit_type[unit[i].type_id]->ExtractsMetal)) // Compute amount of metal extracted by sec
			{
				int metal_base = 0;
				const int px = unit[i].cur_px;
				const int py = unit[i].cur_py;
				const int start_x = px - (unit_manager.unit_type[unit[i].type_id]->FootprintX >> 1 );
				const int start_y = py - (unit_manager.unit_type[unit[i].type_id]->FootprintZ >> 1 );
				const int end_y = start_y + unit_manager.unit_type[unit[i].type_id]->FootprintZ;
				const int end_x = start_x + unit_manager.unit_type[unit[i].type_id]->FootprintX;
				for (int ry = start_y; ry <= end_y; ++ry)
				{
					if (ry >= 0 && ry < map->bloc_h_db)
					{
						for( int rx = start_x ; rx <= end_x ; rx++ )
						{
							if (rx >= 0 && rx < map->bloc_w_db)
							{
								if (map->map_data(rx, ry).stuff >= 0)
								{
									const Feature* const feature = feature_manager.getFeaturePointer(features.feature[map->map_data(rx, ry).stuff].type);
									metal_base = feature->metal * unit_manager.unit_type[unit[i].type_id]->FootprintZ * unit_manager.unit_type[unit[i].type_id]->FootprintX;
									ry = end_y;
									break;
								}
								else
									metal_base += map->ota_data.SurfaceMetal;
							}
						}
					}
				}
				unit[i].metal_extracted = float(metal_base) * unit_manager.unit_type[unit[i].type_id]->ExtractsMetal;

				int param[] = { metal_base << 2 };
				unit[i].launchScript( SCRIPT_SetSpeed, 1, param);
				unit[i].just_created = false;
			}

			if (Yuni::Math::Zero(unit[i].build_percent_left))
			{
				unit[i].metal_prod = 0.0f;
				unit[i].metal_cons = 0.0f;
				unit[i].energy_prod = 0.0f;
				unit[i].energy_cons = 0.0f;
				players.c_metal_s[unit[i].owner_id] += unit_manager.unit_type[unit[i].type_id]->MetalStorage;
				players.c_energy_s[unit[i].owner_id] += unit_manager.unit_type[unit[i].type_id]->EnergyStorage;
				players.c_commander[unit[i].owner_id] |= (unit_manager.unit_type[unit[i].type_id]->TEDclass == CLASS_COMMANDER);
				unit[i].energy_prod += (float)unit_manager.unit_type[unit[i].type_id]->EnergyMake;
				if ((unit[i].port[ACTIVATION] || !unit_manager.unit_type[unit[i].type_id]->onoffable)
					&& unit_manager.unit_type[unit[i].type_id]->EnergyUse<=players.energy[unit[i].owner_id])
				{
					unit[i].metal_prod += unit_manager.unit_type[unit[i].type_id]->MakesMetal + unit_manager.unit_type[unit[i].type_id]->MetalMake;
					if (!Yuni::Math::Zero(unit_manager.unit_type[unit[i].type_id]->ExtractsMetal))	// Extracteur de métal
						unit[i].metal_prod += unit[i].metal_extracted;
					if (unit_manager.unit_type[unit[i].type_id]->WindGenerator) // Wind Generator
					{
						unit[i].energy_prod += map->wind * (float)unit_manager.unit_type[unit[i].type_id]->WindGenerator * 0.0002f;
						if (wind_change)
						{
							int param[] = { (int)(map->wind * 50.0f) };
							unit[i].launchScript( SCRIPT_SetSpeed, 1, param);
							param[0] = (int)((map->wind_dir - unit[i].Angle.y) * DEG2TA);
							unit[i].launchScript(SCRIPT_SetDirection, 1, param);
							unit[i].launchScript(SCRIPT_go);
						}
					}
					if (unit_manager.unit_type[unit[i].type_id]->TidalGenerator)	// Tidal Generator
						unit[i].energy_prod += float(map->ota_data.tidalstrength);
					if (unit_manager.unit_type[unit[i].type_id]->EnergyUse < 0)
						unit[i].energy_prod -= float(unit_manager.unit_type[unit[i].type_id]->EnergyUse);
					else
						unit[i].energy_cons = (float)unit_manager.unit_type[unit[i].type_id]->EnergyUse;
					TA3D::players.requested_energy[unit[i].owner_id] += unit[i].energy_cons;
					TA3D::players.requested_metal[unit[i].owner_id] += unit[i].metal_cons;
				}
			}
			unit[i].unlock();
			pMutex.lock();
		}
		pMutex.unlock();

		exp_dt_1 = expf(-dt);
		exp_dt_2 = expf(-2.0f * dt);
		exp_dt_4 = expf(-4.0f * dt);
		g_dt = dt * map->ota_data.gravity;
		pMutex.lock();
		for (unsigned int e = 0; e < index_list_size; ++e)
		{
			i = idx_list[e];
			pMutex.unlock();
			unit[ i ].lock();

			if (unit[ i ].flags == 0) // ho ho what is it doing there ??
			{
				unit[ i ].unlock();
				kill(i,e);
				--e;			// Can't skip a unit
				pMutex.lock();
				continue;
			}

			if (unit[i].owner_id==players.local_human_id)
			{
				if (unit[i].attacked || (!unit[i].mission.empty() && unit[i].mission->mission() == MISSION_ATTACK))
					nb_attacked += 100;
				if (unit[i].built)
					nb_built++;
			}
			players.c_nb_unit[unit[i].owner_id]++;			// Compte les unités de chaque joueur
			unit[i].unlock();
			if (unit[i].move(dt,key_frame) == -1) // Vérifie si l'unité a été détruite
			{
				if (unit[i].local) // Don't kill remote units, since we're told when to kill them
				{
					kill(i,e);
					e--;			// Can't skip a unit
				}
			}
			pMutex.lock();
		}
		pMutex.unlock();

		pMutex.lock();
		for (unsigned int e = 0 ; e < index_list_size ; ++e)
		{
			i = idx_list[e];
			pMutex.unlock();

			unit[ i ].lock();
			players.c_metal_t[unit[i].owner_id] += unit[i].metal_prod;
			players.c_metal_u[unit[i].owner_id] += unit[i].metal_cons;
			players.c_energy_t[unit[i].owner_id] += unit[i].energy_prod;
			players.c_energy_u[unit[i].owner_id] += unit[i].energy_cons;

			unit[i].cur_energy_cons = unit[i].energy_cons;
			unit[i].cur_energy_prod = unit[i].energy_prod;
			unit[i].cur_metal_cons = unit[i].metal_cons;
			unit[i].cur_metal_prod = unit[i].metal_prod;
			unit[ i ].unlock();

			pMutex.lock();
		}
		pMutex.unlock();

		float exp_r = expf(-dt*0.1f);
		nb_attacked*=exp_r;
		nb_built*=exp_r;

		pMutex.lock();

		for (i = 0; i < players.count(); ++i)
		{
			players.c_annihilated[i] = !players.c_nb_unit[i]; // Has this player units ?
			if (players.c_commander[i])
			{
				players.c_metal_s[i]+=players.com_metal[i];
				players.c_energy_s[i]+=players.com_energy[i];
			}
		}
		for (i = 0; i < players.count(); ++i)
		{
			players.c_metal[i] += dt * (players.c_metal_t[i] - players.c_metal_u[i]);
			players.c_energy[i] += dt * (players.c_energy_t[i] - players.c_energy_u[i]);
			players.metal_total[i] += dt * players.metal_t[i];
			players.energy_total[i] += dt * players.energy_t[i];
			if (players.c_metal[i] < 0.0f)
				players.c_metal[i] = 0.0f;
			else
				if (players.c_metal[i] > players.c_metal_s[i])
					players.c_metal[i] = float(players.c_metal_s[i]);
			if (players.c_energy[i] < 0.0f)
				players.c_energy[i] = 0.0f;
			else
				if (players.c_energy[i] > players.c_energy_s[i])
					players.c_energy[i] = float(players.c_energy_s[i]);
		}

		players.refresh();
		pMutex.unlock();

		// Now that it is obsolete let's delete this kdTree
		for(int i = 0 ; i < NB_PLAYERS ; ++i)
		{
			pool.release(kdTree[i]);
			pool.release(kdTreeFriends[i]);
			pool.release(kdTreeRepairPads[i]);
			kdTree[i] = NULL;
			kdTreeFriends[i] = NULL;
			kdTreeRepairPads[i] = NULL;
		}
	}

	int INGAME_UNITS::create(int type_id,int owner)
	{
		if (type_id < 0 || type_id >= unit_manager.nb_unit)	return -1;
		if (owner < 0 || owner >= NB_PLAYERS)	return -1;
		if (nb_unit >= uint32(MAX_UNIT_PER_PLAYER * NB_PLAYERS))	return -1;
		if (free_index_size[owner] <= 0 && max_unit > 0 )	return -1;

		pMutex.lock();

		nb_unit++;
		if (nb_unit > max_unit && max_unit == 0)
		{
			DELETE_ARRAY(mini_col);
			DELETE_ARRAY(mini_pos);

			max_unit = MAX_UNIT_PER_PLAYER*NB_PLAYERS;

			mini_col = new uint32[ max_unit ];
			mini_pos = new float[ max_unit * 2 ];

			Unit *n_unit = new Unit[max_unit];
			uint16	*n_idx = new uint16[max_unit];
			uint16	*n_new_idx = new uint16[max_unit];
			if (index_list_size > 0)
				memcpy(n_idx,idx_list,index_list_size << 1);
			if (free_idx)
				memcpy(n_new_idx,free_idx,max_unit << 1);
			DELETE_ARRAY(idx_list);
			DELETE_ARRAY(free_idx);
			idx_list = n_idx;
			free_idx = n_new_idx;
			for(uint32 i = 0 ; i < max_unit ; ++i)
				free_idx[i] = (uint16)i;
			for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT ; ++i)
				free_index_size[i] = (uint16)MAX_UNIT_PER_PLAYER;
			for (size_t i = 0 ; i < max_unit ; ++i)
			{
				n_unit[i].init(-1, -1, i >= nb_unit - 1);
				n_unit[i].flags = 0;
				n_unit[i].idx = uint16(i);
			}
			if (unit)
			{
				memcpy(n_unit, unit, sizeof(Unit) * (nb_unit - 1));
				DELETE_ARRAY(unit);
			}
			unit = n_unit;
		}
		if (!unit)
			LOG_CRITICAL("Memory alloc failed");
		if (free_index_size[owner] <= 0)
		{
			pMutex.unlock();
			LOG_WARNING("Unit limit reached !");
			return -1;
		}
		const int unit_index = free_idx[owner * MAX_UNIT_PER_PLAYER + free_index_size[owner] - 1];
		free_index_size[owner]--;
		unit[unit_index].init(type_id, owner);
		unit[unit_index].ID = next_unit_ID++;		// So now we know who is this unit :)

		// Angle de 10° maximum
		unit[unit_index].Angle.y = float(((sint32)(Math::RandomTable() % 20001)) - 10000) * 0.0001f * (float)unit_manager.unit_type[type_id]->BuildAngle * TA2DEG;

		idx_list[index_list_size++] = uint16(unit_index);

		players.nb_unit[owner]++;
		pMutex.unlock();

		return unit_index;
	}

	void INGAME_UNITS::draw_mini(float map_w,float map_h,int mini_w,int mini_h)				// Repère les unités sur la mini-carte
	{
		if (nb_unit<=0 || unit==NULL)
		{
			last_on = -1;
			return;		// Pas d'unités à dessiner
		}

		const float rw = 128.0f * (float)mini_w / 252.0f / (float)map_w;
		const float rh = 128.0f * (float)mini_h / 252.0f / (float)map_h;

		glDisable(GL_TEXTURE_2D);
		glPointSize(3.0f);

		const byte mask = byte(1 << players.local_human_id);
		int b_w = (int) map_w >> 3;
		int b_h = (int) map_h >> 3;
		int nb = 0;

		uint32 player_col_32[TA3D_PLAYERS_HARD_LIMIT];
		uint32 player_col_32_h[TA3D_PLAYERS_HARD_LIMIT];
		for (uint32 i = 0 ; i < players.count() ; ++i)
		{
			player_col_32[i] =  makeacol( (int)(player_color[ player_color_map[ i ] * 3 ] * 255.0f),
										  (int)(player_color[ player_color_map[ i ] * 3 + 1 ] * 255.0f),
										  (int)(player_color[ player_color_map[ i ] * 3 + 2 ] * 255.0f),
										  i );
			player_col_32_h[i] =  makeacol( (int)(player_color[ player_color_map[ i ] * 3 ] * 127.5f),
											(int)(player_color[ player_color_map[ i ] * 3 + 1 ] * 127.5f),
											(int)(player_color[ player_color_map[ i ] * 3 + 2 ] * 127.5f),
											i );
		}

		pMutex.lock();
		const byte player_mask = byte(1 << players.local_human_id);
		for (unsigned int e = 0; e < index_list_size; ++e)
		{
			const size_t i = idx_list[e];
			pMutex.unlock();

			units.unit[ i ].lock();

			if (unit[i].flags & 1)
			{
				const int px = unit[i].cur_px;
				const int py = unit[i].cur_py;
				if (px < 0 || py < 0 || px >= b_w || py >= b_h)
				{
					units.unit[ i ].unlock();
					pMutex.lock();
					continue;
				}
				if ((!(map->view_map(px >> 1, py >> 1) & mask)
					|| !(map->sight_map(px >> 1, py >> 1) & mask)
					|| (unit[i].cloaked && unit[i].owner_id != players.local_human_id))
					&& !unit[i].is_on_radar(player_mask))
				{
					units.unit[ i ].unlock();
					pMutex.lock();
					continue;	// Unité non visible / Unit is not visible
				}
				//			unit[i].flags|=0x10;
				mini_pos[ nb << 1 ] = unit[i].Pos.x;
				mini_pos[ (nb << 1) + 1 ] = unit[i].Pos.z;
				mini_col[ nb++ ] = player_col_32_h[ unit[i].owner_id ];
			}
			units.unit[ i ].unlock();
			pMutex.lock();
		}
		pMutex.unlock();
		glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
		glEnableClientState(GL_COLOR_ARRAY);		// Colors(for fog of war)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);		// vertex coordinates

		glColorPointer(4, GL_UNSIGNED_BYTE, 0, mini_col);
		glVertexPointer( 2, GL_FLOAT, 0, mini_pos);
		glPushMatrix();
		glTranslatef( 63.0f, 64.0f, 0.0f );
		glScalef( rw, rh, 0.0f );
		glDrawArrays(GL_POINTS, 0, nb);		// draw the points

		glPopMatrix();
		glPushMatrix();
		glTranslatef( 65.0f, 64.0f, 0.0f );
		glScalef( rw, rh, 0.0f );
		glDrawArrays(GL_POINTS, 0, nb);		// draw the points

		glPopMatrix();
		glPushMatrix();
		glTranslatef( 64.0f, 65.0f, 0.0f );
		glScalef( rw, rh, 0.0f );
		glDrawArrays(GL_POINTS, 0, nb);		// draw the points

		glPopMatrix();
		glPushMatrix();
		glTranslatef( 64.0f, 63.0f, 0.0f );
		glScalef( rw, rh, 0.0f );
		glDrawArrays(GL_POINTS, 0, nb);		// draw the points

		glPopMatrix();
		glPushMatrix();
		glTranslatef( 64.0f, 64.0f, 0.0f );
		glScalef( rw, rh, 0.0f );
		for( int i = 0 ; i < nb ; i++ )
			mini_col[ i ] = player_col_32[ geta( mini_col[ i ] ) ];
		glDrawArrays(GL_POINTS, 0, nb);		// draw the points
		glPopMatrix();

		int cur_id = -1;
		glBegin( GL_POINTS );
		pMutex.lock();
		for(uint16 e=0;e<index_list_size; ++e)
		{
			uint16 i = idx_list[e];
			pMutex.unlock();

			units.unit[ i ].lock();
			if (units.unit[ i ].cur_px < 0 || units.unit[ i ].cur_py < 0 || units.unit[ i ].cur_px >= b_w || units.unit[ i ].cur_py >= b_h)
			{
				units.unit[ i ].unlock();
				pMutex.lock();
				continue;
			}

			if ((unit[i].flags & 1)
				&& ( (unit[i].owner_id == players.local_human_id && unit[i].sel)
					 || (i == last_on && (players.team[unit[i].owner_id] & players.team[players.local_human_id]) )))
			{
				cur_id = unit[i].owner_id;
				float pos_x=unit[i].Pos.x*rw+64.0f;
				float pos_y=unit[i].Pos.z*rh+64.0f;
				bool anti_missile = unit[i].weapon.size() > 0 && unit_manager.unit_type[unit[i].type_id]->antiweapons && unit_manager.unit_type[unit[i].type_id]->weapon[0];
				if (unit[i].radar_range > 0 || unit[i].radar_jam_range > 0 || unit[i].sonar_jam_range || unit[i].sonar_range > 0 || anti_missile)
				{
					glEnd();
					glPointSize(1.0f);
				}
				if (unit[i].radar_range > 0)
					gfx->circle_zoned( pos_x, pos_y, float(unit[i].radar_range << 4) * rw, 0.0f, 0.0f, 127.0f, 127.0f, makeacol( 0, 255, 0, 255 ) );
				if (unit[i].sonar_range > 0)
					gfx->circle_zoned( pos_x, pos_y, float(unit[i].sonar_range << 4) * rw, 0.0f, 0.0f, 127.0f, 127.0f, makeacol( 0, 255, 0, 255 ) );
				if (unit[i].radar_jam_range > 0)
					gfx->circle_zoned( pos_x, pos_y, float(unit[i].radar_jam_range << 4) * rw, 0.0f, 0.0f, 127.0f, 127.0f, makeacol( 192, 192, 0, 255 ) );
				if (unit[i].sonar_jam_range > 0)
					gfx->circle_zoned( pos_x, pos_y, float(unit[i].sonar_jam_range << 4) * rw, 0.0f, 0.0f, 127.0f, 127.0f, makeacol( 192, 192, 0, 255 ) );
				if (anti_missile)
					gfx->dot_circle_zoned( float(msec_timer) * 0.001f, pos_x, pos_y, (float)unit_manager.unit_type[ unit[i].type_id ]->weapon[0]->coverage * rw, 0.0f, 0.0f, 127.0f, 127.0f, 0xFFFFFFFF );
				if (unit[i].radar_range > 0 || unit[i].radar_jam_range > 0 || unit[i].sonar_jam_range || unit[i].sonar_range > 0 || anti_missile)
				{
					glPointSize(3.0f);
					glBegin( GL_POINTS );
				}
				glColor3ub(0xFF,0xFF,0xFF);
				glVertex2f(pos_x-1.0f,pos_y);
				glVertex2f(pos_x+1.0f,pos_y);
				glVertex2f(pos_x,pos_y-1.0f);
				glVertex2f(pos_x,pos_y+1.0f);

				glColor3f(player_color[3*player_color_map[cur_id]],player_color[3*player_color_map[cur_id]+1],player_color[3*player_color_map[cur_id]+2]);
				glVertex2f(pos_x,pos_y);
			}
			units.unit[ i ].unlock();
			pMutex.lock();
		}
		pMutex.unlock();
		glEnd();
		glPointSize(1.0f);
		glEnable(GL_TEXTURE_2D);

		last_on = -1;
	}

	void INGAME_UNITS::kill(int index,int prev,bool sync)			// Détruit une unité
	{
		if (index < 0 || index >= (int)max_unit || prev < 0 || prev >= (int)index_list_size)	// On ne peut pas détruire une unité qui n'existe pas
			return;

		unit[index].lock();

		if (unit[index].local && network_manager.isConnected() && sync ) // Send EVENT_UNIT_DEATH
		{
			struct event event;
			event.type = EVENT_UNIT_DEATH;
			event.opt1 = uint16(index);
			network_manager.sendEvent( &event );
		}

		if (unit[index].flags & 1)
		{
			if (!unit[ index ].mission.empty()
				&& !unit_manager.unit_type[ unit[ index ].type_id ]->BMcode
				&& (unit[ index ].mission->mission() == MISSION_BUILD_2
					|| unit[ index ].mission->mission() == MISSION_BUILD)		// It was building something that we must destroy too
				&& unit[ index ].mission->getTarget().getUnit())
			{
				Unit *p = unit[ index ].mission->getTarget().getUnit();
				p->lock();
				p->hp = 0.0f;
				p->built = false;
				p->unlock();
			}
			players.nb_unit[ unit[index].owner_id ]--;
			players.losses[ unit[index].owner_id ]++;		// Statistics
		}

		unit[index].unlock();
		unit[index].clear_from_map();
		unit[index].lock();

		if (unit[index].type_id >= 0 && unit_manager.unit_type[unit[index].type_id]->canload && unit[index].nb_attached>0)
		{
			for(int i = 0 ; i < unit[index].nb_attached ; ++i)
			{
				unit[unit[index].attached_list[i]].lock();
				unit[unit[index].attached_list[i]].hp = 0.0f;
				unit[unit[index].attached_list[i]].unlock();
			}
		}
		unit[index].unlock();
		unit[index].destroy();		// Détruit l'unité

		pMutex.lock();

		const uint32 owner = index / MAX_UNIT_PER_PLAYER;
		free_idx[ MAX_UNIT_PER_PLAYER * owner + free_index_size[ owner ]++ ] = uint16(index);
		idx_list[ prev ] = idx_list[ --index_list_size ];
		--nb_unit; // Unité détruite

		pMutex.unlock();
	}

	void INGAME_UNITS::renderTick()
	{
		if (nb_unit <= 0 || !unit)
			return;		// Pas d'unités à dessiner

		const Matrix mCam = Camera::inGame->getMatrix();
		visible_unit.clear();
		const int bloc_w = the_map->bloc_w_db;
		const int bloc_h = the_map->bloc_h_db;
		const uint32 player_mask = 1 << players.local_human_id;
		for(uint32 i = 0 ; i < max_unit ; ++i)
		{
			unit[i].visible = false;
			if (!(unit[i].flags & 1))
				continue;
			unit[i].visible = false;
			const int type_id = unit[i].type_id;
			if (type_id < 0)
				continue;
			const UnitType* const pUnitType = unit_manager.unit_type[type_id];
			if (!pUnitType->model)
				continue;

			const Vector3D pos = unit[i].Pos + pUnitType->model->center;
			const Vector3D pPos = glNMult(pos, mCam);
			if (std::fabs(pPos.x) > 1.0f || std::fabs(pPos.y) > 1.0f || std::fabs(pPos.z) > 1.0f)
			{
				const Vector3D rel = pos - Camera::inGame->pos;
				if (rel.sq() > pUnitType->model->size2)
				{
					Vector3D D = rel - (rel % Camera::inGame->dir) * Camera::inGame->dir;
					D.unit();
					const Vector3D pos2 = pos - 1.42f * pUnitType->model->size2 * D;
					const Vector3D pPos2 = glNMult(pos2, mCam);
					if (std::fabs(pPos2.x) > 1.0f || std::fabs(pPos2.y) > 1.0f || std::fabs(pPos2.z) > 1.0f)
						continue;
				}
			}
			unit[i].renderTick();
			const int px = unit[i].render.px;
			const int py = unit[i].render.py;
			const int px2 = px >> 1;
			const int py2 = py >> 1;
			if (px < 0 || py < 0 || px >= bloc_w || py >= bloc_h)
				continue;
			gfx->lock();
			if (!(the_map->view_map(px2, py2) & player_mask))
			{
				if (!((the_map->radar_map(px2, py2) & player_mask) || (the_map->sonar_map(px2, py2) & player_mask)))       // We need to check that in case there is a radar or a sonar around
				{
					gfx->unlock();
					continue;
				}
			}
			gfx->unlock();
			unit[i].visible = true;
			visible_unit.push_back(i);
		}
	}


	void INGAME_UNITS::draw(bool underwater, bool limit, bool cullface, bool height_line)					// Dessine les unités visibles
	{
		if (nb_unit <= 0 || !unit)
			return;		// Pas d'unités à dessiner

		glEnable(GL_LIGHTING);
		if (cullface)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		glDisable(GL_BLEND);
		glColor4ub(0xFF,0xFF,0xFF,0xFF);
		const float sea_lvl = limit ? the_map->sealvl - 5.0f : the_map->sealvl;
		const float virtual_t = ((float)current_tick) / TICKS_PER_SEC;
		const bool low_def = Camera::inGame->rpos.y > gfx->low_def_limit;
		if (low_def)
			glDisable(GL_DEPTH_TEST);

		for (std::vector<uint32>::const_iterator e = visible_unit.begin(); e != visible_unit.end(); ++e)
		{
			const uint32 i = *e;

			Unit* const pUnit = &(unit[i]);
			const Model* const model = pUnit->model;

			if (model
				&& (low_def || (pUnit->render.Pos.y + model->bottom <= the_map->sealvl && underwater)
					|| (pUnit->render.Pos.y + model->top >= sea_lvl && !underwater))) // Si il y a une unité / If there is a unit
				pUnit->draw(virtual_t, height_line);
		}

		glDisable(GL_ALPHA_TEST);

		if (low_def)
			glEnable(GL_DEPTH_TEST);

		if (!cullface)
			glEnable(GL_CULL_FACE);
	}



	void INGAME_UNITS::draw_shadow(float t, const Vector3D& Dir, float alpha)	// Dessine les ombres des unités visibles
	{
		if (nb_unit <= 0 || unit == NULL) // Pas d'unités à dessiner
			return;

		if (g_useStencilTwoSide) // Si l'extension GL_EXT_stencil_two_side est disponible
		{
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
			glDisable(GL_CULL_FACE);
			glClear(GL_STENCIL_BUFFER_BIT);
			glDepthMask(GL_FALSE);
			glColorMask(0, 0, 0,  0);
			glStencilFunc(GL_ALWAYS, 128, 0xffffffff);

			glActiveStencilFaceEXT(GL_FRONT);
			glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
			glActiveStencilFaceEXT(GL_BACK);
			glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);

			for (std::vector<uint32>::const_iterator e = visible_unit.begin(); e != visible_unit.end(); ++e)
			{
				const uint32 i = *e;

				gfx->lock();

				unit[i].draw_shadow(Dir);

				gfx->unlock();
			}
		}
		else // Si elle ne l'est pas
		{
			glDepthMask(GL_FALSE);
			glColorMask(0,0,0,0);

			glClear(GL_STENCIL_BUFFER_BIT);
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_ALWAYS,128, 0xffffffff);
			glEnable(GL_CULL_FACE);

			for (std::vector<uint32>::const_iterator e = visible_unit.begin(); e != visible_unit.end(); ++e)
			{
				const uint32 i = *e;

				gfx->lock();
				unit[i].drawShadowBasic(Dir);
				gfx->unlock();
			}
		}

		gfx->lock();
        features.draw_shadow(t, Dir);

		glColorMask(0xFF,0xFF,0xFF,0xFF);
		glColor4f(0.0f,0.0f,0.0f,alpha);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);	glVertex3f(0, 0, 0);
		glTexCoord2f(1.0f, 1.0f);	glVertex3f(float(SCREEN_W), 0, 0);
		glTexCoord2f(1.0f, 0.0f);	glVertex3f(float(SCREEN_W), float(SCREEN_H), 0);
		glTexCoord2f(0.0f, 0.0f);	glVertex3f(0, float(SCREEN_H), 0);
		glEnd();
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glColor4ub(0xFF,0xFF,0xFF,0xFF);

		gfx->unlock();
	}

	bool INGAME_UNITS::remove_order(int player_id, const Vector3D& target)
	{
		bool removed_something = false;
		pMutex.lock();
		for (uint16 e = 0; e < index_list_size; ++e)
		{
			uint16 i = idx_list[e];
			pMutex.unlock();
			unit[i].lock();
			if ((unit[i].flags & 1)
				&& !unit[i].command_locked && unit[i].owner_id == player_id
				&& unit[i].sel
				&& Yuni::Math::Zero(unit[i].build_percent_left))
			{
				MissionStack &mission = unit_manager.unit_type[unit[i].type_id]->BMcode ? unit[i].mission : unit[i].def_mission;
				MissionStack::iterator cur = mission.begin();
				if (cur != mission.end() && unit_manager.unit_type[unit[i].type_id]->BMcode)
					++cur;		// Don't read the first one ( which is being executed )

				while (cur != mission.end()) // Reads the mission list
				{
					if (cur->lastMission() == MISSION_BUILD)
						++cur;
					else
					{
						if ((cur->lastStep().getTarget().getPos() - target).sq() < 256.0f) // Remove it
						{
							// Path is a std::list so it'll be cleared automatically
							cur = mission.erase(cur);
							removed_something = true;
						}
                        else
							++cur;
					}
				}
			}
			unit[i].unlock();
			pMutex.lock();
		}
		pMutex.unlock();

		return removed_something;
	}



	uint32 INGAME_UNITS::InterfaceMsg(const uint32 MsgID, const String &msg)
	{
		if (MsgID == TA3D_IM_GUI_MSG )	// for GUI messages, test if it's a message for us
		{
			if (msg.empty())
				return INTERFACE_RESULT_HANDLED;		// Oups badly written things
			String message(ToLower(msg));				// Get the string associated with the signal
			if (!message.empty())
			{
				if (message == "pause game")
				{
					if (network_manager.isConnected())
						network_manager.sendAll("PAUSE");
					lp_CONFIG->pause = true;
					return INTERFACE_RESULT_HANDLED;
				}
				if (message == "resume game")
				{
					if (network_manager.isConnected())
						network_manager.sendAll("RESUME");
					lp_CONFIG->pause = false;
					return INTERFACE_RESULT_HANDLED;
				}
				if (message == "toggle pause")
				{
					lp_CONFIG->pause ^= true;
					if (network_manager.isConnected())
						network_manager.sendAll(lp_CONFIG->pause ? "PAUSE" : "RESUME");
					return INTERFACE_RESULT_HANDLED;
				}
			}
		}
		return INTERFACE_RESULT_CONTINUE;						// Temporary, for now it does nothing
	}


	void INGAME_UNITS::proc(void*)
	{
		thread_running = true;
		float dt = 1.0f / TICKS_PER_SEC;
		uint32 unit_timer = msec_timer;
		uint32 tick_timer;
		float counter = 0.0f;
		int tick = 1000 / TICKS_PER_SEC;
		tick_timer = msec_timer;
		uint32 net_timer = msec_timer;
		float step = 1.0f;
		if (lp_CONFIG->timefactor > 0.0f )	step = 1.0f / lp_CONFIG->timefactor;
		current_tick = 0;
		for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
			client_tick[i] = client_speed[i] = 0;
		apparent_timefactor = lp_CONFIG->timefactor;

		while( !thread_ask_to_stop )
		{
			counter += step;

			move( dt, current_tick, wind_change );					// Animate units

			pMutex.lock();

			if (!(current_tick & 0xF))
			{
				gfx->lock();

				if (map->fog_of_war & FOW_GREY)
					map->sight_map.clear(0);
				map->radar_map.clear(0);		// Clear radar map
				map->sonar_map.clear(0);		// Clear sonar map

				for (size_t i = 0 ; i < index_list_size ; ++i)			// update fog of war, radar and sonar data
					unit[ idx_list[ i ] ].draw_on_FOW();

				for (size_t i = 0 ; i < index_list_size ; ++i)			// update radar and sonar jamming data
					unit[ idx_list[ i ] ].draw_on_FOW( true );
				gfx->unlock();
			}

			wind_change = false;
			pMutex.unlock();

			uint32 min_tick = 1000 * current_tick + 30000;
			if (network_manager.isConnected())
			{
				if (network_manager.isServer())
				{
					for (unsigned int i = 0 ; i < players.count(); ++i)
						if (g_ta3d_network->isRemoteHuman( i ))
							min_tick = Math::Min(min_tick, client_tick[i]);
				}
				else
					min_tick = Math::Min(min_tick, client_tick[0]);
			}
			min_tick /= 1000;

			if (network_manager.isConnected() && min_tick > current_tick )
			{
				const int delay = (min_tick - current_tick) * 250 / TICKS_PER_SEC;
				tick += delay;
			}

			while (msec_timer - tick_timer + 1 < (uint32)tick)
				suspend(1);

			while (msec_timer - tick_timer >= (uint32)tick + 200U) // Prevent the game from running too fast for too long, we don't have to speed up to compute what we hadn't time to
			{
				counter += 1.0f;
				tick = (int)( ( (counter + step ) * 1000 ) / TICKS_PER_SEC );		// For perfect sync with tick clock
			}

			tick = (int)( ( (counter + step ) * 1000 ) / TICKS_PER_SEC );		// For perfect sync with tick clock
			if (lp_CONFIG->timefactor > 0.0f )	step = 1.0f / lp_CONFIG->timefactor;
			else	step = 1.0f;

			while (lp_CONFIG->pause && !thread_ask_to_stop)
			{
				lp_CONFIG->paused = true;
				suspend(10); // in pause mode wait for pause to be false again
			}
			lp_CONFIG->paused = false;

			if (network_manager.isConnected())
			{
				net_timer = msec_timer - net_timer;
				if (network_manager.isServer())
					for (unsigned int i = 0 ; i < players.count(); ++i)
					{
						if (g_ta3d_network->isRemoteHuman(i))
							client_tick[i] += client_speed[i] * net_timer / (1000 * TICKS_PER_SEC);
					}
				else
					client_tick[0] += client_speed[0] * net_timer / (1000 * TICKS_PER_SEC);

				net_timer = msec_timer;

				network_manager.sendTick(current_tick + 1, (uint16)(1000.0f * apparent_timefactor));		// + 1 to prevent it from running too slow
				network_manager.sendPing();
				if (current_tick > min_tick + TICKS_PER_SEC )
				{
					while (current_tick > min_tick && !thread_ask_to_stop)
					{
						while (lp_CONFIG->pause && !thread_ask_to_stop)            // We need this to prevent client dead lock when saving game
						{
							lp_CONFIG->paused = true;
							suspend(10); // in pause mode wait for pause to be false again
						}
						lp_CONFIG->paused = false;

						players.ta3d_network->check();
						network_manager.sendTick(current_tick + 1, (uint16)(1000.0f * apparent_timefactor));		// + 1 to prevent it from running too slow
						network_manager.sendPing();
						suspend(1);

						min_tick = current_tick * 1000;
						if (network_manager.isServer())
						{
							for(unsigned int i = 0; i < players.count(); ++i)
							{
								if (g_ta3d_network->isRemoteHuman(i))
									min_tick = Math::Min(min_tick, client_tick[i]);
							}
						}
						else
						{
							min_tick = Math::Min(min_tick, client_tick[0]);
						}
						min_tick /= 1000;
					}
				}
				else
				{
					if (current_tick > min_tick)
						tick += (current_tick - min_tick) * 250 / TICKS_PER_SEC;
				}
			}

			Engine::sync();
			current_tick++;		// To have a common time value

			last_tick[ 0 ] = last_tick[ 1 ];
			last_tick[ 1 ] = last_tick[ 2 ];
			last_tick[ 2 ] = last_tick[ 3 ];
			last_tick[ 3 ] = last_tick[ 4 ];
			last_tick[ 4 ] = msec_timer;

			if (last_tick[ 0 ] != 0 && last_tick[4] != last_tick[0])
				apparent_timefactor = 4000.0f / float((last_tick[ 4 ] - last_tick[ 0 ]) * TICKS_PER_SEC);
		}

		thread_running = false;
		thread_ask_to_stop = false;
		LOG_INFO("Unit engine: " << (float)(current_tick * 1000) / float(msec_timer - unit_timer) << " ticks/sec");
	}

	void INGAME_UNITS::signalExitThread()
	{
		if (thread_running)
			thread_ask_to_stop = true;
	}

	void INGAME_UNITS::drawHealthBars()
	{
		if (nb_unit <= 0 || !unit)
			return;		// Pas d'unités à dessiner

		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_FOG);
		glDepthMask(GL_FALSE);

		hbars.clear();
		hbars_bkg.clear();
		hbars_color.clear();

		for (std::vector<uint32>::const_iterator e = visible_unit.begin(); e != visible_unit.end(); ++e)
			unit[*e].drawHealthBar();

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &(hbars_bkg.front()));
		glColor4ub(0,0,0,0xFF);
		glDrawArrays(GL_QUADS, 0, (GLsizei)hbars_bkg.size());

		glVertexPointer(3, GL_FLOAT, 0, &(hbars.front()));
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(hbars_color.front()));
		glDrawArrays(GL_QUADS, 0, (GLsizei)hbars.size());

		glDisableClientState(GL_COLOR_ARRAY);

		glDepthMask(GL_TRUE);
		glEnable(GL_FOG);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

} // namespace TA3D

