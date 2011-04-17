#include "console.h"
#include <ingame/battle.h>
#include <ingame/players.h>
#include <UnitEngine.h>
#include "console.api.h"
#include <sounds/manager.h>
#include <input/mouse.h>

namespace TA3D
{

	int CAPI::print(lua_State *L)
	{
		String tmp;
		for(int i = 1 ; i <= lua_gettop(L) ; ++i)
		{
			if (!tmp.empty())
				tmp << ' ';
			switch(lua_type(L, i))
			{
			case LUA_TNIL:
				tmp << "<nil>";
				break;
			case LUA_TNUMBER:
				tmp << lua_tonumber(L, i);
				break;
			case LUA_TBOOLEAN:
				tmp << (lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TSTRING:
				tmp << lua_tostring(L, i);
				break;
			case LUA_TTABLE:
				tmp << "<table>";
				break;
			case LUA_TFUNCTION:
				tmp << "<function>";
				break;
			case LUA_TUSERDATA:
				tmp << "<userdata>";
				break;
			case LUA_TTHREAD:
				tmp << "<thread>";
				break;
			case LUA_TLIGHTUSERDATA:
				tmp << "<lightuserdata>";
				break;
			default:
				tmp << "<unknown>";
			};
		}
		Console::Instance()->addEntry(tmp);
		return 0;
	}

	int CAPI::setShowPing(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->setShowPing(lua_toboolean(L, -1));
		lua_pushboolean(L, Battle::Instance()->getShowPing());
		return 1;
	}

	int CAPI::setFps(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->showfps = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->showfps);
		return 1;
	}

	int CAPI::zshoot(lua_State *)
	{
		SDL_Surface *bmp = gfx->create_surface_ex(32,SCREEN_W,SCREEN_H);
		SDL_FillRect(bmp, NULL, 0);
		glReadPixels(0,0,SCREEN_W,SCREEN_H,GL_DEPTH_COMPONENT,GL_INT,bmp->pixels);
		//                        save_bitmap(String(TA3D::Paths::Screenshots) << "z.tga",bmp);
		SDL_FreeSurface(bmp);
		return 0;
	}

	int CAPI::setFarsight(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->far_sight = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->far_sight);
		return 1;
	}

	int CAPI::setTooltips(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->tooltips = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->tooltips);
		return 1;
	}

	int CAPI::setSoundVolume(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			lp_CONFIG->sound_volume = (int)lua_tointeger(L, -1);
			sound_manager->setVolume( lp_CONFIG->sound_volume );
		}
		lua_pushinteger(L, lp_CONFIG->sound_volume);
		return 1;
	}

	int CAPI::setMusicVolume(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			lp_CONFIG->music_volume = (int)lua_tointeger(L, -1);
			sound_manager->setMusicVolume( lp_CONFIG->music_volume );
		}
		lua_pushinteger(L, lp_CONFIG->music_volume);
		return 1;
	}

	int CAPI::setRightClickInterface(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->right_click_interface = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->right_click_interface);
		return 1;
	}

	int CAPI::setCameraOrtho(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->ortho_camera = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->ortho_camera);
		return 1;
	}

	int CAPI::setGrabInputs(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			lp_CONFIG->grab_inputs = lua_toboolean(L, -1);
			grab_mouse(lp_CONFIG->grab_inputs);
		}
		lua_pushboolean(L, lp_CONFIG->grab_inputs);
		return 1;
	}

	int CAPI::setVideoShoot(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->video_shoot = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->video_shoot);
		return 1;
	}

	int CAPI::screenshot(lua_State *)
	{
		Battle::Instance()->shoot = true;
		return 0;
	}

	int CAPI::makeposter(lua_State *L)
	{
		int w = SCREEN_W;
		int h = SCREEN_H;
		if (lua_gettop(L) >= 2)
		{
			w = (int)lua_tointeger(L, 1);
			h = (int)lua_tointeger(L, 2);
		}
		Battle::Instance()->makePoster(w, h);
		return 0;
	}

	int CAPI::reloadShaders(lua_State *)
	{
		Battle::Instance()->reloadShaders();
		return 0;
	}

	int CAPI::setShowMissionInfo(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->show_mission_info = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->show_mission_info);
		return 1;
	}

	int CAPI::setViewDebug(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->view_dbg = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->view_dbg);
		return 1;
	}

	int CAPI::setAIDebug(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->ia_debug = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->ia_debug);
		return 1;
	}

	int CAPI::setInternalName(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->internal_name = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->internal_name);
		return 1;
	}

	int CAPI::setInternalIdx(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->internal_idx = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->internal_idx);
		return 1;
	}

	int CAPI::exit(lua_State *)
	{
		Battle::Instance()->done = true;
		return 0;
	}

	int CAPI::setWireframe(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->wireframe = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->wireframe);
		return 1;
	}

	int CAPI::setPriority(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->priority_level = (sint16)lua_tointeger(L, -1);
		lua_pushinteger(L, lp_CONFIG->priority_level);
		return 1;
	}

	int CAPI::setWaterQuality(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->water_quality = (sint16)lua_tointeger(L, -1) % 6;
		lua_pushinteger(L, lp_CONFIG->water_quality);
		return 1;
	}

	int CAPI::setShadowQuality(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			lp_CONFIG->shadow_quality = (sint16)lua_tointeger(L, -1);
			gfx->delete_shadow_map();
		}
		lua_pushinteger(L, lp_CONFIG->shadow_quality);
		return 1;
	}

	int CAPI::setShadowMapSize(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			lp_CONFIG->shadowmap_size = (uint8)lua_tointeger(L, -1);
			gfx->delete_shadow_map();
		}
		lua_pushinteger(L, lp_CONFIG->shadowmap_size);
		return 1;
	}

	int CAPI::setDetailsTexture(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->detail_tex = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->detail_tex);
		return 1;
	}

	int CAPI::setParticles(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->particle = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->particle);
		return 1;
	}

	int CAPI::setExplosionParticles(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->explosion_particles = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->explosion_particles);
		return 1;
	}

	int CAPI::setWaves(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->waves = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->waves);
		return 1;
	}

	int CAPI::scriptDumpDebugInfo(lua_State *)
	{
		if (Battle::Instance()->cur_sel_index >= 0 && units.unit[Battle::Instance()->cur_sel_index].script)
			units.unit[Battle::Instance()->cur_sel_index].script->dumpDebugInfo();
		return 0;
	}

	int CAPI::setShowModel(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->show_model = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->show_model);
		return 1;
	}

	int CAPI::setRotateLight(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->rotate_light = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->rotate_light);
		return 1;
	}

	int CAPI::shake(lua_State *)
	{
		Battle::Instance()->cam.setShake(1.0f, 32.0f);
		return 0;
	}

	int CAPI::setFreecam(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			Battle::Instance()->freecam = lua_toboolean(L, -1);
			if (!Battle::Instance()->freecam)
				Battle::Instance()->r2 = 0.0f;
		}
		lua_pushboolean(L, Battle::Instance()->freecam);
		return 1;
	}

	int CAPI::setFpsLimit(lua_State *L)
	{
		if (lua_gettop(L) > 0)
		{
			Battle::Instance()->speed_limit = (float)lua_tonumber(L, -1);
			if (Yuni::Math::Zero(Battle::Instance()->speed_limit))
				Battle::Instance()->speed_limit = -1.0f;
			Battle::Instance()->delay = 1.0f / Battle::Instance()->speed_limit;
		}
		lua_pushnumber(L, Battle::Instance()->speed_limit);
		return 1;
	}

	int CAPI::spawn(lua_State *L)
	{
		int unit_type = -1;
		int player_id = players.local_human_id;
		int nb_to_spawn = 1;
		unit_type = lua_gettop(L) > 0 ? unit_manager.get_unit_index( lua_tostring(L, 1) ) : -1;
		if (lua_gettop(L) >= 2)
		{
			player_id = (int)lua_tointeger(L, 2);
			if (lua_gettop(L) >= 3)
				nb_to_spawn = (int)lua_tointeger(L, 3);
		}
		units.lock();
		for (int i = 0; i < nb_to_spawn; ++i)
		{
			int id = 0;
			if (unit_type < 0 || unit_type >= unit_manager.nb_unit)
				id = units.create( abs( TA3D_RAND()) % unit_manager.nb_unit,player_id);
			else
				id = units.create(unit_type,player_id);
			if (id == -1) // Ho ho no more space to store that unit, limit reached
				break;
			units.unit[id].lock();
			int e(0);

			do
			{
				units.unit[id].Pos.x = (float)((TA3D_RAND() % the_map->map_w) - the_map->map_w_d);
				units.unit[id].Pos.z = (float)((TA3D_RAND() % the_map->map_h) - the_map->map_h_d);
				++e;
			} while (e < 100 && !can_be_built(units.unit[id].Pos, units.unit[id].type_id, player_id));

			if (!can_be_built(units.unit[id].Pos, units.unit[id].type_id, player_id))
			{
				units.unit[id].flags = 4;
				units.kill(id, units.index_list_size - 1);
			}
			else
			{								// Compute unit's Y coordinate
				Vector3D target_pos(units.unit[id].Pos);
				target_pos.x = (float)(((int)(target_pos.x) + the_map->map_w_d) >> 3);
				target_pos.z = (float)(((int)(target_pos.z) + the_map->map_h_d) >> 3);
				target_pos.y = Math::Max(the_map->get_max_rect_h((int)target_pos.x,(int)target_pos.z,
															 unit_manager.unit_type[ units.unit[id].type_id ]->FootprintX,
															 unit_manager.unit_type[ units.unit[id].type_id ]->FootprintZ),
										 the_map->sealvl);

				units.unit[id].Pos.y  = target_pos.y;
				units.unit[id].cur_px = (sint16)target_pos.x;
				units.unit[id].cur_py = (sint16)target_pos.z;
				units.unit[id].Pos.x  = target_pos.x * 8.0f - static_cast<float>(the_map->map_w_d);
				units.unit[id].Pos.z  = target_pos.z * 8.0f - static_cast<float>(the_map->map_h_d);
				units.unit[id].draw_on_map();

				if (unit_manager.unit_type[units.unit[id].type_id]->ActivateWhenBuilt)// Start activated
				{
					units.unit[id].port[ ACTIVATION ] = 0;
					units.unit[id].activate();
				}
				if (unit_manager.unit_type[units.unit[id].type_id]->init_cloaked)				// Start cloaked
					units.unit[id].cloaking = true;
			}
			units.unit[ id ].unlock();
		}
		units.unlock();

		return 0;
	}

	int CAPI::setTimeFactor(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->timefactor = (float)lua_tonumber(L, -1);
		lua_pushnumber(L, lp_CONFIG->timefactor);
		return 1;
	}

	int CAPI::addhp(lua_State *L)
	{
		if (lua_gettop(L) == 0)
			return 0;
		if (Battle::Instance()->selected) // Sur les unités sélectionnées
		{
			const int value = (int)lua_tointeger(L, 1);
			units.lock();
			for (unsigned int e = 0; e < units.index_list_size; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
				{
					units.unit[i].hp += (float)value;
					if (units.unit[i].hp < 0)
						units.unit[i].hp = 0;
					else
					{
						if (units.unit[i].hp > unit_manager.unit_type[units.unit[i].type_id]->MaxDamage)
							units.unit[i].hp = (float)unit_manager.unit_type[units.unit[i].type_id]->MaxDamage;
					}
				}
			}
			units.unlock();
		}
		return 0;
	}

	int CAPI::deactivate(lua_State *)
	{
		if (Battle::Instance()->selected) // Sur les unités sélectionnées
		{
			units.lock();
			for (unsigned int e = 0; e < units.index_list_size; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
					units.unit[i].deactivate();
			}
			units.unlock();
		}
		return 0;
	}

	int CAPI::activate(lua_State *)
	{
		if (Battle::Instance()->selected) // Sur les unités sélectionnées
		{
			units.lock();
			for (unsigned int e = 0; e < units.index_list_size; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
					units.unit[i].activate();
			}
			units.unlock();
		}
		return 0;
	}

	int CAPI::resetScript(lua_State *)
	{
		if (Battle::Instance()->selected) // Sur les unités sélectionnées
		{
			units.lock();
			for (unsigned int e = 0; e < units.index_list_size; ++e)
			{
				const int i = units.idx_list[e];
				units.unlock();
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
					units.unit[i].resetScript();
				units.lock();
			}
			units.unlock();
		}
		return 0;
	}

	int CAPI::dumpUnitInfo(lua_State *)
	{
		if (Battle::Instance()->selected && Battle::Instance()->cur_sel != -1)	// Sur les unités sélectionnées
		{
			static const char *unit_info[] =
			{
				"ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS",
				"HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y","UNIT_XZ","UNIT_Y",
				"UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN",
				"HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
				"BUGGER_OFF","ARMORED"
			};
			units.lock();
			for (unsigned int e = 0; e < units.index_list_size; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
				{
					Console::Instance()->addEntry(String("flags=") << int(units.unit[i].flags));
					String tmp;
					for (int f = 1; f < 21; ++f)
						tmp << unit_info[f-1] << '=' << units.unit[i].port[f] << ", ";
					if (!tmp.empty())
						Console::Instance()->addEntry(tmp);
				}
			}
			units.unlock();
		}
		return 0;
	}

	int CAPI::kill(lua_State *)
	{
		if (Battle::Instance()->selected) // Sur les unités sélectionnées
		{
			units.lock();
			for (unsigned int e = 0; e < units.index_list_size; ++e)
			{
				const int i = units.idx_list[e];
				units.unlock();
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
				{
					units.kill(i, e);
					--e;
				}
				units.lock();
			}
			units.unlock();
			Battle::Instance()->selected = false;
			Battle::Instance()->cur_sel = -1;
		}
		return 0;
	}

	int CAPI::setShootall(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			units.shootallMode = lua_toboolean(L, -1);
		lua_pushboolean(L, units.shootallMode);
		return 1;
	}

	int CAPI::startScript(lua_State *L)					// Force l'éxecution d'un script
	{
		if (lua_gettop(L) == 0)
			return 0;
		if (Battle::Instance()->selected) // Sur les unités sélectionnées
		{
			units.lock();
			for (uint32 e = 0 ; e < units.index_list_size ; ++e)
			{
				const int i = units.idx_list[e];
				if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
					units.unit[i].launchScript(UnitScriptInterface::get_script_id(lua_tostring(L, 1)));
			}
			units.unlock();
		}
		return 0;
	}

	int CAPI::setPause(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->pause = lua_toboolean(L, -1);
		lua_pushboolean(L, lp_CONFIG->pause);
		return 1;
	}

	int CAPI::give(lua_State *L)							// Give resources to player_id
	{
		bool success = false;
		if (lua_gettop(L) == 3)
		{
			const unsigned int playerID = (unsigned int)lua_tointeger(L, 2);
			if (playerID < players.count())
			{
				const float amount = (float)lua_tointeger(L, 3);
				String type = lua_tostring(L, 1);
				if (type == "metal" || type == "both")
				{
					players.metal[playerID] = players.c_metal[playerID] = players.c_metal[playerID] + amount;					// cheat codes
					success = true;
				}
				if (type == "energy" || type == "both")
				{
					players.energy[playerID] = players.c_energy[playerID] = players.c_energy[playerID] + amount;					// cheat codes
					success = true;
				}
			}
		}
		if (!success)
			Console::Instance()->addEntry("Command error: The correct syntax is: give metal/energy/both player_id amount");
		return 0;
	}

	int CAPI::setMetalCheat(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->cheat_metal = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->cheat_metal);
		return 1;
	}

	int CAPI::setEnergyCheat(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			Battle::Instance()->cheat_energy = lua_toboolean(L, -1);
		lua_pushboolean(L, Battle::Instance()->cheat_energy);
		return 1;
	}

	int CAPI::setGUIalpha(lua_State *L)
	{
		if (lua_gettop(L) > 0)
			lp_CONFIG->menuTransparency = Math::Clamp(float(lua_tonumber(L, -1)), 0.0f, 1.0f);
		lua_pushnumber(L, lp_CONFIG->menuTransparency);
		return 1;
	}

	// ---------------    Debug commands    ---------------
	int CAPI::_debugSetContext(lua_State *L)			// Switch debug context
	{
		const String context = lua_gettop(L) > 0 ? lua_tostring(L, 1) : String();
		if (context == "mission")
			Battle::Instance()->debugInfo.process = &(Battle::Instance()->game_script);
		else if (context == "AI")
		{
			int pid = lua_gettop(L) > 1 ? (int)lua_tointeger(L, 2) : -1;
			if (pid >= 0 && pid < (int)players.count())
				Battle::Instance()->debugInfo.process = AiScript::Ptr::WeakPointer(players.ai_command[pid].getAiScript());
			else
				Battle::Instance()->debugInfo.process = NULL;
		}
		else
		{
			Console::Instance()->addEntry("error : available options are : \"mission\", \"AI\"");
			Battle::Instance()->debugInfo.process = NULL;
		}
		return 0;
	}

	int CAPI::_debugState(lua_State *)												// Print LuaThread state
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;

		if (Battle::Instance()->debugInfo.process->is_waiting())
			Console::Instance()->addEntry("LuaThread is paused");
		if (Battle::Instance()->debugInfo.process->is_sleeping())
			Console::Instance()->addEntry("LuaThread is sleeping");
		if (Battle::Instance()->debugInfo.process->is_running())
			Console::Instance()->addEntry("LuaThread is running");
		if (Battle::Instance()->debugInfo.process->is_crashed())
			Console::Instance()->addEntry("LuaThread is crashed");
		return 0;
	}

	int CAPI::_debugLoad(lua_State *L)				// Load a Lua script into the Lua thread
	{
		if (Battle::Instance()->debugInfo.process == NULL || lua_gettop(L) == 0)
			return 0;
		Battle::Instance()->debugInfo.process->load(lua_tostring(L, 1));
		Console::Instance()->addEntry("Lua script loaded");
		return 0;
	}

	int CAPI::_debugStop(lua_State *)				// Stop the LuaThread (it doesn't kill it)
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;
		Battle::Instance()->debugInfo.process->stop();
		Console::Instance()->addEntry("LuaThread is stopped");
		return 0;
	}

	int CAPI::_debugResume(lua_State *)				// Resume the LuaThread (it doesn't uncrash it)
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;
		Battle::Instance()->debugInfo.process->resume();
		Console::Instance()->addEntry("LuaThread resumed");
		return 0;
	}

	int CAPI::_debugKill(lua_State *)	// Kill the LuaThread
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;
		Battle::Instance()->debugInfo.process->kill();
		Console::Instance()->addEntry("LuaThread killed");
		return 0;
	}

	int CAPI::_debugCrash(lua_State *)				// Crash the LuaThread
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;
		Battle::Instance()->debugInfo.process->crash();
		Console::Instance()->addEntry("LuaThread crashed");
		return 0;
	}

	int CAPI::_debugContinue(lua_State *)			// Uncrash the LuaThread
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;
		Battle::Instance()->debugInfo.process->uncrash();
		Console::Instance()->addEntry("LuaThread uncrashed");
		return 0;
	}

	int CAPI::_debugRun(lua_State *L)
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;
		if (lua_gettop(L) == 0)
			return 0;

		const String code = lua_tostring(L, 1);
		LOG_INFO(LOG_PREFIX_LUA << "running : '" << code << "'");
		if (!Battle::Instance()->debugInfo.process->runCommand(code))
		{
			LOG_INFO(LOG_PREFIX_LUA << "running given code failed");
			Console::Instance()->addEntry("running given code failed");
		}
		return 0;
	}

	int CAPI::_debugMemory(lua_State *)						// Show the memory used by the select Lua VM
	{
		if (Battle::Instance()->debugInfo.process == NULL)
			return 0;

		Console::Instance()->addEntry(String("Lua GC reports ") << Battle::Instance()->debugInfo.process->getMem() << " bytes used");
		return 0;
	}

	// ---------------    OS specific commands    ---------------
	int CAPI::setFullscreen(lua_State* L)      // This works only on Linux/X11
	{
#ifdef TA3D_PLATFORM_LINUX
		if (lua_gettop(L) == 1 && lua_toboolean(L, 1) != lp_CONFIG->fullscreen)
		{
			if (SDL_WM_ToggleFullScreen(screen))
				lp_CONFIG->fullscreen ^= true;
		}
#endif
		lua_pushboolean(L, lp_CONFIG->fullscreen);
		return 1;
	}

	void Console::registerConsoleAPI()
	{
		lua_gc( L, LUA_GCSTOP, 0 );		// Load libraries
		luaL_openlibs( L );
		lua_gc( L, LUA_GCRESTART, 0 );

		// Reuse part of game script API
		lua_register( L, "time", program_time );
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
		lua_register( L, "move_unit", program_move_unit );
		lua_register( L, "kill_unit", program_kill_unit );
		lua_register( L, "kick_unit", program_kick_unit );
		lua_register( L, "play", program_play );
		lua_register( L, "play_for", program_play_for );
		lua_register( L, "set_cam_pos", program_set_cam_pos );
		lua_register( L, "clf", program_clf );
		lua_register( L, "start_x", program_start_x );
		lua_register( L, "start_z", program_start_z );
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

		lua_register( L, "logmsg", thread_logmsg );

		// Register CAPI functions
#define CAPI_REGISTER(x) lua_register( L, #x, CAPI::x)
		CAPI_REGISTER(print);
		CAPI_REGISTER(setFps);
		CAPI_REGISTER(zshoot);
		CAPI_REGISTER(setFarsight);
		CAPI_REGISTER(setTooltips);
		CAPI_REGISTER(setSoundVolume);
		CAPI_REGISTER(setMusicVolume);
		CAPI_REGISTER(setRightClickInterface);
		CAPI_REGISTER(setCameraOrtho);
		CAPI_REGISTER(setGrabInputs);
		CAPI_REGISTER(setVideoShoot);
		CAPI_REGISTER(screenshot);
		CAPI_REGISTER(makeposter);
		CAPI_REGISTER(reloadShaders);
		CAPI_REGISTER(setShowMissionInfo);
		CAPI_REGISTER(setViewDebug);
		CAPI_REGISTER(setAIDebug);
		CAPI_REGISTER(setInternalName);
		CAPI_REGISTER(setInternalIdx);
		CAPI_REGISTER(exit);
		CAPI_REGISTER(setWireframe);
		CAPI_REGISTER(setPriority);
		CAPI_REGISTER(setWaterQuality);
		CAPI_REGISTER(setShadowQuality);
		CAPI_REGISTER(setShadowMapSize);
		CAPI_REGISTER(setDetailsTexture);
		CAPI_REGISTER(setParticles);
		CAPI_REGISTER(setExplosionParticles);
		CAPI_REGISTER(setWaves);
		CAPI_REGISTER(scriptDumpDebugInfo);
		CAPI_REGISTER(setShowModel);
		CAPI_REGISTER(setRotateLight);
		CAPI_REGISTER(shake);
		CAPI_REGISTER(setFreecam);
		CAPI_REGISTER(setFpsLimit);
		CAPI_REGISTER(spawn);
		CAPI_REGISTER(setTimeFactor);
		CAPI_REGISTER(addhp);
		CAPI_REGISTER(deactivate);
		CAPI_REGISTER(activate);
		CAPI_REGISTER(resetScript);
		CAPI_REGISTER(dumpUnitInfo);
		CAPI_REGISTER(kill);
		CAPI_REGISTER(setShootall);
		CAPI_REGISTER(startScript);
		CAPI_REGISTER(setPause);
		CAPI_REGISTER(give);
		CAPI_REGISTER(setMetalCheat);
		CAPI_REGISTER(setEnergyCheat);
		CAPI_REGISTER(setGUIalpha);
		CAPI_REGISTER(setShowPing);

		// ---------------    Debug commands    ---------------
		CAPI_REGISTER(_debugSetContext);
		CAPI_REGISTER(_debugState);
		CAPI_REGISTER(_debugLoad);
		CAPI_REGISTER(_debugStop);
		CAPI_REGISTER(_debugResume);
		CAPI_REGISTER(_debugKill);
		CAPI_REGISTER(_debugCrash);
		CAPI_REGISTER(_debugContinue);
		CAPI_REGISTER(_debugRun);
		CAPI_REGISTER(_debugMemory);

		// ---------------    OS specific commands    ---------------
		CAPI_REGISTER(setFullscreen);
	}
}
