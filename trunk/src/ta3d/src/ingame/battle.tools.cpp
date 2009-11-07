
#include "battle.h"
#include "../UnitEngine.h"
#include "../languages/table.h"
#include "players.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../sounds/manager.h"



namespace TA3D
{


	Vector3D Battle::cursorOnMap(const Camera& cam, MAP& map, bool on_mini_map)
	{
		if (on_mini_map) // If the cursor is on the mini_map;
		{
			float x = (mouse_x - 64) * 252.0f / 128.0f * map.map_w / map.mini_w;
			float z = (mouse_y - 64) * 252.0f / 128.0f * map.map_h / map.mini_h;
			float y = map.get_unit_h(x, z);
			return Vector3D(x, y, z);
		}
		if (lp_CONFIG->ortho_camera)        // Orthographic camera
		{
			Vector3D cur_pos = cam.pos + cam.zoomFactor * ( (mouse_x - gfx->SCREEN_W_HALF) * cam.side
															- (mouse_y - gfx->SCREEN_H_HALF) * cam.up );
			return map.hit(cur_pos, cam.dir, true, 2000000000.0f, true);
		}
		// Normal perspective code
		Vector3D cur_dir = cam.dir + cam.widthFactor * 2.0f * (mouse_x - gfx->SCREEN_W_HALF) * gfx->SCREEN_W_INV * cam.side
			- 1.5f * (mouse_y - gfx->SCREEN_H_HALF) * gfx->SCREEN_H_INV * cam.up;
		cur_dir.unit();		// Direction pointée par le curseur
		return map.hit(cam.pos, cur_dir, true, 2000000000.0f, true);
	}


	void Battle::updateFOG()
	{
		if (freecam && cam.rpos.y < map->sealvl)
		{
			FogD = 0.03f;
			FogFar = cam.zfar;
			FogNear = 0.0f;
			FogMode = GL_EXP;

			FogColor[0] = 0.0f;
			FogColor[1] = 0.0f;
			FogColor[2] = 0.3f;
			FogColor[3] = 1.0f;
		}
		else
		{
			FogD = 0.3f;
			FogFar = lp_CONFIG->far_sight ? sqrtf( map->map_w * map->map_w + map->map_h * map->map_h ) : cam.zfar;
			FogNear = FogFar * 0.5f;
			FogMode = GL_LINEAR;

			memcpy(FogColor, pSkyData->FogColor, sizeof(float) * 4);
		}

		glClearColor(FogColor[0], FogColor[1], FogColor[2], FogColor[3]);

		glFogi (GL_FOG_MODE, FogMode);
		glFogfv (GL_FOG_COLOR, FogColor);
		glFogf (GL_FOG_DENSITY, FogD);
		glHint (GL_FOG_HINT, GL_NICEST);
		glFogf (GL_FOG_START, FogNear);
		glFogf (GL_FOG_END, FogFar);
	}


	void Battle::updateZFAR()
	{
		cam.zfar = (lp_CONFIG->far_sight)
			// We want to see everything
			? sqrtf( map->map_w * map->map_w + map->map_h * map->map_h + cam.rpos.y * cam.rpos.y)
			: 600.0f + Math::Max((cam_h - 150.0f) * 2.0f, 0.0f);
		// Set View
		cam.setView();
	}



	void Battle::showGameStatus()
	{
		Gui::WND::Ptr statuswnd = pArea.get_wnd("gamestatus");
		if (statuswnd)
			statuswnd->y = (int)(SCREEN_H - (statuswnd->height + 32) * show_gamestatus);

		uint32 game_time = units.current_tick / TICKS_PER_SEC;

		String tmp;

        if (pArea.get_state("gamestatus"))                         // Don't update things if we don't display them
        {
            // Time
            tmp << TranslationTable::gameTime << " : " << (game_time / 3600) << ':'
                << ((game_time / 60) % 60) << ':' << (game_time % 60);
            pArea.caption("gamestatus.time_label", tmp);

            // Units
            tmp.clear();
            tmp << TranslationTable::units << " : " << players.nb_unit[players.local_human_id] << '/' << MAX_UNIT_PER_PLAYER;
            pArea.caption("gamestatus.units_label", tmp);

            // Speed
            tmp.clear();
            tmp << TranslationTable::speed << " : " << (int)round(lp_CONFIG->timefactor) << '('
                << (int)round(units.apparent_timefactor) << ')';
            pArea.caption("gamestatus.speed_label", tmp);
        }

		statuswnd = pArea.get_wnd("playerstats");
		if (statuswnd)
			statuswnd->x = (int)(SCREEN_W - (statuswnd->width + 10) * show_gamestatus);

        if (pArea.get_state("playerstats"))                         // Don't update things if we don't display them
            for (unsigned int i = 0; i < players.count(); ++i)
            {
                tmp.clear();
                tmp << "playerstats.p" << i << "d_box";
                Gui::GUIOBJ::Ptr obj = pArea.get_object(tmp);
                if (obj)
                {
                    obj->Data = gfx->makeintcol(
                        player_color[3 * player_color_map[i]],
                        player_color[3 * player_color_map[i] + 1],
                        player_color[3 * player_color_map[i] + 2], 0.5f);
                }
                // Kills
                tmp.clear();
                tmp += players.kills[i];
                pArea.caption(String::Format("playerstats.p%d_kills", i), tmp);
                // Losses
                tmp.clear();
                tmp += players.losses[i];
                pArea.caption(String::Format("playerstats.p%d_losses", i), tmp);
            }
	}



	void Battle::keyArrowsInFreeCam()
	{
		if (key[KEY_UP])
		{
			cam.rpos += 100.0f * dt * cam_h / 151.0f * cam.dir;
			track_mode = -1;
		}
		if (key[KEY_DOWN])
		{
			cam.rpos +=  - 100.0f * dt * cam_h / 151.0f * cam.dir;
			track_mode = -1;
		}
		if (key[KEY_RIGHT])
		{
			cam.rpos += 100.0f * dt * cam_h / 151.0f * cam.side;
			track_mode = -1;
		}
		if (key[KEY_LEFT])
		{
			cam.rpos += - 100.0f * dt * cam_h / 151.0f * cam.side;
			track_mode = -1;
		}
	}


	void Battle::keyArrowsNotInFreeCam()
	{
		if (key[KEY_UP])
		{
			cam.rpos.z -= SCROLL_SPEED * dt * cam_h / 151.0f;
			cam_has_target = false;
			track_mode = -1;
		}
		if (key[KEY_DOWN])
		{
			cam.rpos.z += SCROLL_SPEED * dt * cam_h / 151.0f;
			cam_has_target = false;
			track_mode = -1;
		}
		if (key[KEY_RIGHT])
		{
			cam.rpos.x += SCROLL_SPEED * dt * cam_h / 151.0f;
			cam_has_target = false;
			track_mode = -1;
		}
		if (key[KEY_LEFT])
		{
			cam.rpos.x -= SCROLL_SPEED * dt * cam_h / 151.0f;
			cam_has_target = false;
			track_mode = -1;
		}
	}


	void Battle::handleGameStatusEvents()
	{
		// Enable the game status if the `space` is pressed
		if (!pCacheShowGameStatus && key[KEY_SPACE])
		{
			pCacheShowGameStatus = true;
			pArea.msg("gamestatus.show");	// Show it
			pArea.msg("playerstats.show");	// Show it
		}

		if (pCacheShowGameStatus)
		{
			if (key[KEY_SPACE]) // Show gamestatus window
			{
				if (show_gamestatus < 1.f)
				{
					show_gamestatus += 10.f * dt;
					if (show_gamestatus > 1.f)
						show_gamestatus = 1.f;
				}
			}
			else
			{									// Hide gamestatus window
				show_gamestatus -= 10.f * dt;

				if (show_gamestatus < 0.f)
				{
					show_gamestatus = 0.f;
					pCacheShowGameStatus = false;
					pArea.msg("gamestatus.hide");	// Hide it
					pArea.msg("playerstats.hide");	// Hide it
					return;
				}
			}

			showGameStatus();
		}
	}


   void Battle::parseCommands(const String &cmd)
   {
       if (!cmd.empty()) // Analyse les commandes tapées dans la console
       {
           String::Vector params;
           cmd.explode(params, ' ');
           if (params.size() > 0)
           {
               if (params[0] == "fps_on") lp_CONFIG->showfps=true;				// Affiche le nombre d'images/seconde
               else if (params[0] == "fps_off") lp_CONFIG->showfps=false;				// Cache le nombre d'images/seconde
               else if (params[0] == "zshoot") // Prend une capture d'écran(tampon de profondeur seulement)
               {
                   SDL_Surface *bmp = gfx->create_surface_ex(32,SCREEN_W,SCREEN_H);
                   SDL_FillRect(bmp, NULL, 0);
                   glReadPixels(0,0,SCREEN_W,SCREEN_H,GL_DEPTH_COMPONENT,GL_INT,bmp->pixels);
                   //                        save_bitmap( TA3D::Paths::Screenshots + "z.tga",bmp);
                   SDL_FreeSurface(bmp);
               }
               else if (params[0] == "toggle" && params.size() == 2 && params[1] == "farsight")    lp_CONFIG->far_sight ^= true;
               else if (params[0] == "set" && params.size() == 4 && params[2] == "volume")
               {
                   if (params[1] == "sound")
                   {
                       lp_CONFIG->sound_volume = params[3].to<uint32>();
                       sound_manager->setVolume(lp_CONFIG->sound_volume);
                   }
                   else if (params[1] == "music")
                   {
                       lp_CONFIG->music_volume = params[3].to<uint32>();
                       sound_manager->setMusicVolume(lp_CONFIG->music_volume);
                   }
               }
               else if ((params[0] == "enable" || params[0] == "disable") && params.size() > 1)
               {
                   if (params[1] == "right_click_interface")
                       lp_CONFIG->right_click_interface = params[0] == "enable";
                   else if (params[1] == "left_click_interface")
                       lp_CONFIG->right_click_interface = params[0] == "disable";
                   else if (params[1] == "camera_perspective")
                       lp_CONFIG->ortho_camera = params[0] == "disable";
                   else if (params[1] == "camera_orthographic")
                       lp_CONFIG->ortho_camera = params[0] == "enable";
                   else if (params[1] == "grab_inputs")
                   {
                       lp_CONFIG->grab_inputs = params[0] == "enable";
                       grab_mouse(lp_CONFIG->grab_inputs);
                   }
               }
               else if (params.size() == 2 && params[0] == "video" && params[1] == "shoot") video_shoot ^= true;		// Capture video
               else if (params[0] == "shoot") shoot = true;					// Prend une capture d'écran
               else if (params.size() == 2 && params[0] == "reload" && params[1] == "shaders" && g_useProgram)
               {
                   water_shader.destroy();
                   water_shader.load("shaders/water.frag","shaders/water.vert");
                   water_shader_reflec.destroy();
                   water_shader_reflec.load("shaders/water_reflec.frag","shaders/water.vert");
                   water_pass1.destroy();
                   water_pass1.load("shaders/water_pass1.frag","shaders/water_pass1.vert");
                   water_pass1_low.destroy();
                   water_pass1_low.load("shaders/water_pass1_low.frag","shaders/water_pass1.vert");
                   water_pass2.destroy();
                   water_pass2.load("shaders/water_pass2.frag","shaders/water_pass2.vert");
                   map->detail_shader.destroy();
                   map->detail_shader.load( "shaders/details.frag", "shaders/details.vert");
                   map->shadow2_shader.destroy();
                   map->shadow2_shader.load("shaders/map_shadow.frag", "shaders/map_shadow.vert");
                   water_simulator_shader.destroy();
                   water_simulator_shader.load("shaders/water_simulator.frag","shaders/water_simulator.vert");
                   water_simulator_shader2.destroy();
                   water_simulator_shader2.load("shaders/water_simulator2.frag","shaders/water_simulator.vert");
                   water_simulator_shader3.destroy();
                   water_simulator_shader3.load("shaders/water_simulator3.frag","shaders/water_simulator.vert");
                   water_simulator_shader4.destroy();
                   water_simulator_shader4.load("shaders/water_simulator4.frag","shaders/water_simulator4.vert");
                   water_simulator_reflec.destroy();
                   water_simulator_reflec.load("shaders/water_sim_reflec.frag","shaders/water.vert");
                   gfx->model_shader.destroy();
                   gfx->model_shader.load("shaders/3do_shadow.frag", "shaders/3do_shadow.vert");
               }
               else if (params.size() == 3 && params[0] == "show" && params[1] == "mission" && params[2] == "info")	show_mission_info ^= true;
               else if (params.size() == 2 && params[0] == "view" && params[1] == "debug")	view_dbg^=true;
               else if (params.size() == 2 && params[0] == "ia" && params[1] == "debug")	ia_debug^=true;
               else if (params.size() == 2 && params[0] == "internal" && params[1] == "name") internal_name^=true;		// Affiche/Cache le nom interne de l'unité
               else if (params.size() == 2 && params[0] == "internal" && params[1] == "idx") internal_idx^=true;		// Show/Hide unit indexes in unit array
               else if (params[0] == "exit") done=true;					// Quitte le programme
               else if (params[0] == "wireframe") 	lp_CONFIG->wireframe^=true;
               else if (params[0] == "priority" && params.size() == 2) lp_CONFIG->priority_level = params[1].to<sint32>();
               else if ( params.size() == 3 && params[0] == "water" && params[1] == "quality") lp_CONFIG->water_quality = params[2].to<sint32>() % 6;
               else if (params[0] == "shadow" && params.size() == 3)
               {
                   if (params[1] == "quality")
				   {
                       lp_CONFIG->shadow_quality = params[2].to<sint32>();
					   gfx->delete_shadow_map();
				   }
			   }
			   else if (params[0] == "shadowmapsize" && params.size() == 2)
			   {
				   lp_CONFIG->shadowmap_size = params[1].to<sint32>();
				   gfx->delete_shadow_map();
			   }
			   else if (params[0] == "details")	lp_CONFIG->detail_tex ^= true;
               else if (params[0] == "particle")	lp_CONFIG->particle^=true;
               else if (params.size() == 2 && params[0] == "explosion" && params[1] == "particles")	lp_CONFIG->explosion_particles^=true;
               else if (params[0] == "waves") lp_CONFIG->waves^=true;
               else if (params.size() == 2 && params[0] == "debug" && params[1] == "script")
               {
                   if (cur_sel_index >= 0 && units.unit[cur_sel_index].script)
                       units.unit[cur_sel_index].script->dumpDebugInfo();
               }
               else if (params.size() == 2 && params[0] == "show" && params[1] == "model") show_model^=true;
               else if (params.size() == 2 && params[0] == "rotate" && params[1] == "light") rotate_light^=true;
               else if (params[0] == "shake")
                   cam.setShake(1.0f, 32.0f);
               else if (params[0] == "freecam")
               {
                   freecam ^= true;
                   if (!freecam)
                       r2=0.0f;
               }
               else if (params[0] == "fps_limit" && params.size() == 2)
               {
                   speed_limit = params[1].to<sint32>();
                   if (Yuni::Math::Zero(speed_limit))
                       speed_limit = -1.0f;
                   delay = 1.0f / speed_limit;
               }
               else if (params[0] == "spawn" && params.size() >= 2)
               {
                   int unit_type = -1;
                   int player_id = players.local_human_id;
                   int nb_to_spawn = 1;
                   unit_type = unit_manager.get_unit_index( params[1] );
                   if (params.size() >= 3)
                   {
                       player_id = params[2].to<sint32>();
                       if (params.size() >= 4)
                           nb_to_spawn = params[3].to<sint32>();
                   }
                   ThreadSynchroniser->lock();		// Make sure we won't destroy something we mustn't
                   units.lock();
                   for (int i = 0; i < nb_to_spawn; ++i)
                   {
                       int id=0;
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
                           units.unit[id].Pos.x = (TA3D_RAND() % map->map_w) - map->map_w_d;
                           units.unit[id].Pos.z = (TA3D_RAND() % map->map_h) - map->map_h_d;
                           ++e;
                       } while (e < 100 && !can_be_built(units.unit[id].Pos, map.get(), units.unit[id].type_id, player_id));

                       if (!can_be_built(units.unit[id].Pos, map.get(), units.unit[id].type_id, player_id))
                       {
                           units.unit[id].flags = 4;
                           units.kill(id, map.get(), units.index_list_size - 1);
                       }
                       else
                       {								// Compute unit's Y coordinate
                           Vector3D target_pos(units.unit[id].Pos);
                           target_pos.x = ((int)(target_pos.x) + map->map_w_d) >> 3;
                           target_pos.z = ((int)(target_pos.z) + map->map_h_d) >> 3;
                           target_pos.y = Math::Max(map->get_max_rect_h((int)target_pos.x,(int)target_pos.z,
                                                                        unit_manager.unit_type[ units.unit[id].type_id ]->FootprintX,
                                                                        unit_manager.unit_type[ units.unit[id].type_id ]->FootprintZ),
                                                    map->sealvl);

                           units.unit[id].Pos.y  = target_pos.y;
                           units.unit[id].cur_px = (int)target_pos.x;
                           units.unit[id].cur_py = (int)target_pos.z;
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
                   ThreadSynchroniser->unlock();
               }
               else if (params[0] == "timefactor" && params.size() == 2)
                   lp_CONFIG->timefactor = params[1].to<float>();
               else if (params[0] == "addhp" && params.size() == 2)
               {
                   if (selected) // Sur les unités sélectionnées
                   {
                       int value = params[1].to<sint32>();
                       units.lock();
                       for (unsigned int e = 0; e < units.index_list_size; ++e)
                       {
                           int i = units.idx_list[e];
                           if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                           {
                               units.unit[i].hp+=value;
                               if (units.unit[i].hp < 0)
                                   units.unit[i].hp = 0;
                               else
                               {
                                   if (units.unit[i].hp > unit_manager.unit_type[units.unit[i].type_id]->MaxDamage)
                                       units.unit[i].hp = unit_manager.unit_type[units.unit[i].type_id]->MaxDamage;
                               }
                           }
                       }
                       units.unlock();
                   }
               }
               else if (params[0] == "deactivate")
               {
                   if (selected) // Sur les unités sélectionnées
                   {
                       units.lock();
                       for (unsigned int e = 0; e < units.index_list_size; ++e)
                       {
                           int i = units.idx_list[e];
                           if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                               units.unit[i].deactivate();
                       }
                       units.unlock();
                   }
               }
               else if (params[0] == "activate")
               {
                   if (selected) // Sur les unités sélectionnées
                   {
                       units.lock();
                       for (unsigned int e = 0; e < units.index_list_size; ++e)
                       {
                           int i = units.idx_list[e];
                           if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                               units.unit[i].activate();
                       }
                       units.unlock();
                   }
               }
               else if (params[0] == "reset_script")							// Réinitialise les scripts
               {
                   if (selected) // Sur les unités sélectionnées
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
               }
               else if (params[0] == "unitinfo")
               {
                   if (selected && cur_sel != -1)	// Sur les unités sélectionnées
                   {
                       static const char *unit_info[] =
                       {
                           "ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS",
                           "HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y","Unit_XZ","Unit_Y",
                           "Unit_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN",
                           "HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
                           "BUGGER_OFF","ARMORED"
                       };
                       units.lock();
                       for (unsigned int e = 0; e < units.index_list_size; ++e)
                       {
                           int i = units.idx_list[e];
                           if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                           {
                               printf("flags=%d\n", units.unit[i].flags);
                               for (int f = 1; f < 21; ++f)
                                   printf("%s=%d\n", unit_info[f-1], units.unit[i].port[f]);
                           }
                       }
                       units.unlock();
                   }
               }
               else if (params[0] == "kill")							// Détruit les unités sélectionnées
               {
                   if (selected) // Sur les unités sélectionnées
                   {
                       units.lock();
                       for (unsigned int e = 0; e < units.index_list_size; ++e)
                       {
                           int i = units.idx_list[e];
                           units.unlock();
                           if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                           {
                               units.kill(i, map.get(), e);
                               --e;
                           }
                           units.lock();
                       }
                       units.unlock();
                       selected=false;
                       cur_sel=-1;
                   }
               }
               else if (params[0] == "shootall")							// Destroy enemy units
               {
                   units.lock();
                   for (int e = 0; e < units.max_unit; ++e)
                   {
                       int i = units.idx_list[e];
                       units.unlock();
                       if ((units.unit[i].flags & 1) && units.unit[i].owner_id != players.local_human_id)
                       {
                           units.kill(i, map.get(), e);
                           --e;
                       }
                       units.lock();
                   }
                   units.unlock();
               }
               else if (params.size() == 3 && params[0] == "start" && params[1] == "script")							// Force l'éxecution d'un script
               {
                   if (selected) // Sur les unités sélectionnées
                   {
                       units.lock();
                       for (int e = 0; e < units.index_list_size; ++e)
                       {
                           const int i = units.idx_list[e];
                           if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
                               units.unit[i].launchScript(UnitScriptInterface::get_script_id(params[2]));
                       }
                       units.unlock();
                   }
               }
               else if (params[0] == "pause")								// Toggle pause mode
                   lp_CONFIG->pause ^= true;
               else if (params[0] == "give")							// Give resources to player_id
               {
                   bool success = false;
                   if (params.size() == 4)
                   {
                       const unsigned int playerID = params[2].to<unsigned int>();
                       if (playerID < players.count())
                       {
                           const int amount = params[3].to<int>();
                           if (params[1] == "metal" || params[1] == "both")
                           {
                               players.metal[playerID] = players.c_metal[playerID] = players.c_metal[playerID] + amount;					// cheat codes
                               success = true;
                           }
                           if (params[1] == "energy" || params[1] == "both")
                           {
                               players.energy[playerID] = players.c_energy[playerID] = players.c_energy[playerID] + amount;					// cheat codes
                               success = true;
                           }
                       }
                   }
                   if (!success)
                       LOG_ERROR("Command error: The correct syntax is: give metal/energy/both player_id amount");

               }
               else if (params[0] == "metal") cheat_metal ^= true;	 // cheat codes
               else if (params[0] == "energy") cheat_energy ^= true; // cheat codes
               // ---------------    Debug commands    ---------------
               else if (params[0] == "lua" && params.size() > 1)
               {
                   if (params[1] == "debug" && params.size() > 2)              // Switch debug context
                   {
                       if (params[2] == "mission")
                           debugInfo.process = &game_script;
                       else if (params[2] == "AI" && params.size() > 3)
                       {
                           int pid = params[3].to<sint32>();
                           if (pid >= 0 && pid < players.count())
							   debugInfo.process = SmartPtr<AiScript>::WeakPointer(players.ai_command[pid].getAiScript());
                           else
                               debugInfo.process = NULL;
                       }
                       else
                       {
                           LOG_INFO(LOG_PREFIX_LUA << "could not find specified LuaThread");
                           debugInfo.process = NULL;
                       }
                   }
                   else if (params[1] == "state" && debugInfo.process)                               // Print LuaThread state
                   {
                      if (debugInfo.process->is_waiting())
                          LOG_INFO(LOG_PREFIX_LUA << "LuaThread is paused");
                      if (debugInfo.process->is_sleeping())
                          LOG_INFO(LOG_PREFIX_LUA << "LuaThread is sleeping");
                      if (debugInfo.process->is_running())
                          LOG_INFO(LOG_PREFIX_LUA << "LuaThread is running");
                      if (debugInfo.process->is_crashed())
                          LOG_INFO(LOG_PREFIX_LUA << "LuaThread is crashed");
                   }
                   else if (params[1] == "load" && params.size() > 2 && debugInfo.process)          // Load a Lua script into the Lua thread
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "Lua script is being loaded");
                       debugInfo.process->load(params[2]);
                   }
                   else if (params[1] == "stop" && debugInfo.process)                               // Stop the LuaThread (it doesn't kill it)
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "LuaThread is being stopped");
                       debugInfo.process->stop();
                   }
                   else if (params[1] == "resume" && debugInfo.process)                             // Resume the LuaThread (it doesn't uncrash it)
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "LuaThread is being resumed");
                       debugInfo.process->resume();
                   }
                   else if (params[1] == "kill" && debugInfo.process)                               // Kill the LuaThread
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "LuaThread is being killed");
                       debugInfo.process->kill();
                   }
                   else if (params[1] == "crash" && debugInfo.process)                              // Crash the LuaThread
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "LuaThread is being crashed");
                       debugInfo.process->crash();
                   }
                   else if (params[1] == "continue" && debugInfo.process)                           // Uncrash the LuaThread
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "LuaThread is being uncrashed");
                       debugInfo.process->uncrash();
                   }
                   else if (params[1] == "run" && debugInfo.process)
                   {
                       String code = cmd.substr(cmd.find("run") + 3, String::npos);
                       LOG_INFO(LOG_PREFIX_LUA << "running : '" << code << "'");
                       if (!debugInfo.process->runCommand(code))
                           LOG_INFO(LOG_PREFIX_LUA << "running given code failed");
                   }
                   else if (params[1] == "memory" && debugInfo.process)                             // Show the memory used by the select Lua VM
                   {
                       LOG_INFO(LOG_PREFIX_LUA << "Lua GC reports " << debugInfo.process->getMem() << " bytes used");
                   }
               }
               // ---------------    OS specific commands    ---------------
#ifdef TA3D_PLATFORM_LINUX
               else if (params.size() == 2 && params[0] == "toggle" && params[1] == "fullscreen")      // This works only on Linux/X11
               {
                   if (SDL_WM_ToggleFullScreen(screen))
                   {
                       lp_CONFIG->fullscreen ^= true;
                   }
               }
#endif
           }
           params.clear();
       }
   }


} // namespace TA3D
