
#include "battle.h"
#include <UnitEngine.h>
#include <languages/table.h>
#include "players.h"
#include <input/keyboard.h>
#include <input/mouse.h>
#include <sounds/manager.h>
#include <console/console.h>


namespace TA3D
{

	void Battle::setCameraDirection(const Vector3D &dir)
	{
		r1 = dir.x;
		r2 = dir.y;
		r3 = dir.z;
	}

	void Battle::setFreeCamera(bool fc)
	{
		freecam = fc;
	}

	void Battle::setTimeFactor(const float f)
	{
		lp_CONFIG->timefactor = f;
		show_timefactor = 1.0f;
	}

	Vector3D Battle::cursorOnMap(const Camera& cam, MAP& map, bool on_mini_map)
	{
		if (on_mini_map) // If the cursor is on the mini_map;
		{
			const float x = float(mouse_x - 64) * 252.0f / 128.0f * (float)map.map_w / (float)map.mini_w;
			const float z = float(mouse_y - 64) * 252.0f / 128.0f * (float)map.map_h / (float)map.mini_h;
			const float y = map.get_unit_h(x, z);
			return Vector3D(x, y, z);
		}
		if (lp_CONFIG->ortho_camera)        // Orthographic camera
		{
			const Vector3D cur_pos = cam.pos + cam.zoomFactor * ( float(mouse_x - gfx->SCREEN_W_HALF) * cam.side
																  - float(mouse_y - gfx->SCREEN_H_HALF) * cam.up );
			return map.hit(cur_pos, cam.dir, true, 2000000000.0f, true);
		}
		// Normal perspective code
		Vector3D cur_dir = cam.dir + cam.widthFactor * 2.0f * float(mouse_x - gfx->SCREEN_W_HALF) * gfx->SCREEN_W_INV * cam.side
						   - 1.5f * float(mouse_y - gfx->SCREEN_H_HALF) * gfx->SCREEN_H_INV * cam.up;
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
			FogFar = lp_CONFIG->far_sight ? std::sqrt(float(map->map_w * map->map_w + map->map_h * map->map_h)) : cam.zfar;
			if (cam.rpos.y > gfx->low_def_limit)
				FogFar *= 2.0f - std::exp(-0.01f * (cam.rpos.y - gfx->low_def_limit));
			FogNear = FogFar * 0.5f;
			FogMode = GL_LINEAR;

			memcpy(FogColor, sky.fogColor(), sizeof(float) * 4);
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
			? std::sqrt(float(map->map_w * map->map_w + map->map_h * map->map_h) + cam.rpos.y * cam.rpos.y)
			: 600.0f + Math::Max((cam_h - 150.0f) * 2.0f, 0.0f);
		// Set View
		cam.setView();
	}



	void Battle::showGameStatus()
	{
		Gui::WND::Ptr statuswnd = pArea.get_wnd("gamestatus");
		if (statuswnd)
			statuswnd->y = (int)((float)SCREEN_H - float(statuswnd->height + 32) * show_gamestatus);

		uint32 game_time = units.current_tick / TICKS_PER_SEC;

		String tmp;

        if (pArea.get_state("gamestatus"))                         // Don't update things if we don't display them
        {
            // Time
			tmp << TranslationTable::gameTime << " : " << (game_time / 3600) << ':';
			int minutes = ((game_time / 60) % 60);
			tmp << (minutes < 10 ? String('0') : String()) << minutes << ':';
			int seconds = (game_time % 60);
			tmp << (seconds < 10 ? String('0') : String()) << seconds;
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
			statuswnd->x = (int)((float)SCREEN_W - float(statuswnd->width + 10) * show_gamestatus);

        if (pArea.get_state("playerstats"))                         // Don't update things if we don't display them
            for (unsigned int i = 0; i < players.count(); ++i)
            {
                tmp.clear();
				tmp << "playerstats.p" << i << "_box";
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
				pArea.caption(String("playerstats.p") << i << "_kills", tmp);
                // Losses
                tmp.clear();
                tmp += players.losses[i];
				pArea.caption(String("playerstats.p") << i << "_losses", tmp);
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

} // namespace TA3D
