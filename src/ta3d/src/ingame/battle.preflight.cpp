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

#include "battle.h"
#include <sounds/manager.h>
#include <UnitEngine.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <misc/timer.h>
#include <misc/timer.h>


namespace TA3D
{


	void Battle::preflightVars()
	{
		can_be_there = false;

		// Used to handle on mini map commands
		IsOnMinimap = mouse_x < 128 && mouse_y < 128;
		// Priority given to game interface
		IsOnGUI = (mouse_x < 128 && ( mouse_y >= SCREEN_H - 64 || mouse_y < 128)) || mouse_y < 32
			|| mouse_y >= SCREEN_H - 32;

		IsOnGUI |= (pArea.check() != 0);

		IsOnGUI |= mouse_x < 128; // Priority given to game interface

		if (IsOnMinimap) // Check if we can project the cursor position on the map
		{
            if (std::fabs(float(mouse_x - 64) * 252.0f / 128.0f) > (float)map->mini_w * 0.5f)
				IsOnMinimap = false;
			else
			{
                if (std::fabs(float(mouse_y - 64) * 252.0f / 128.0f) > (float)map->mini_h * 0.5f)
					IsOnMinimap = false;
			}
		}

		if (IsOnMinimap)
			units.pick_minimap(); // Precompute this, we'll need it

		if (video_shoot)
		{
            if (msectimer() - video_timer >= 1000 / 15)
			{
				video_timer = msectimer();
				shoot = true;
			}
		}

		// Restore the default cursor
		cursor_type = CURSOR_DEFAULT;
	}


	void Battle::preflightChangeWindSpeedAndDirection()
	{
		wind_t = t;
		map->wind += float((Math::RandomTable() % 2001) - 1000);

		if (map->wind < map->ota_data.minwindspeed)
			map->wind = (float)map->ota_data.minwindspeed;
		else
		{
			if (map->wind > map->ota_data.maxwindspeed)
				map->wind = (float)map->ota_data.maxwindspeed;
		}

		map->wind_dir += float(Math::RandomTable() % 901) * 0.1f - 45.0f;

		if (map->wind_dir < 0.0f)
			map->wind_dir += 360.0f;
		else
		{
			if (map->wind_dir >= 360.0f)
				map->wind_dir -= 360.0f;
		}

		map->wind_vec.y = 0.0f;
		map->wind_vec.x = 0.01f * map->wind * cosf(map->wind_dir * DEG2RAD);
		map->wind_vec.z = 0.01f * map->wind * sinf(map->wind_dir * DEG2RAD);
		units.set_wind_change();
	}


	void Battle::preflightUpdate3DSounds()
	{
		if (units.nb_attacked/(units.nb_attacked + units.nb_built + 1) >= 0.75f)
			sound_manager->setMusicMode(true);
		else
		{
			if (units.nb_attacked / (units.nb_attacked + units.nb_built + 1) <= 0.25f)
				sound_manager->setMusicMode(false);
		}
		sound_manager->setListenerPos(cam.rpos);
		sound_manager->update3DSound();
	}


	void Battle::preflightAutomaticCamera()
	{
		float old_zscroll = camera_zscroll;
		float delta = IsOnGUI ? 0.0f : float(mouse_z - omz);
        if (key[KEY_PAGEUP])
            delta = -10.0f * dt;
        else if (key[KEY_PAGEDOWN])
            delta = 10.0f * dt;
        if (lp_CONFIG->ortho_camera)        // 2D zoom with orthographic camera
        {
            camera_zscroll += delta * lp_CONFIG->camera_zoom_speed;
            if (camera_zscroll < -50.0f)
                camera_zscroll = -50.0f;
            else
                if (camera_zscroll > 50.0f) camera_zscroll = 50.0f;
        }
        else
        {
            camera_zscroll += delta * 2.0f * lp_CONFIG->camera_zoom_speed;
            if (camera_zscroll < -25.0f)
                camera_zscroll = -25.0f;
            else
                if (camera_zscroll > 20.0f) camera_zscroll = 20.0f;
        }

		if (msectimer() - cam_def_timer >= 1000 && !Math::Zero(delta)
			&& ( ( camera_zscroll > 0.0f && old_zscroll <= 0.0f)
				 || ( camera_zscroll < 0.0f && old_zscroll >= 0.0f)))			// Just to make it lock near def position
		{
			cam_def_timer = msectimer();
			old_zscroll = 0.0f;
			if (camera_zscroll > -lp_CONFIG->camera_def_angle)
				old_zscroll += 0.00001f;
			else
				old_zscroll -= 0.00001f;
		}

        if (msectimer() - cam_def_timer < 500)
			camera_zscroll = old_zscroll;

		if (lp_CONFIG->ortho_camera)        // 2D zoom with orthographic camera
		{
			r1 = -lp_CONFIG->camera_def_angle;      // angle is constant
			if (r1 > -45.0f) 		r1 = -45.0f;
			else if (r1 < -90.0f)	r1 = -90.0f;
			float zoom_min = Math::Max(((float)map->map_w) / (float)SCREEN_W, ((float)map->map_h) / (float)SCREEN_H);     // ==> x2
            float zoom_max = 0.02f;                                                                          // ==> x0
            float zoom_med = 0.5f;
            float x0 = -50.0f;
            float x2 = 50.0f;
            float sm = 0.01f;
			float x1 = x0 + std::log((zoom_med - zoom_min) / (zoom_max - zoom_min) * (std::exp(sm * (x2 - x0)) - 1.0f) + 1.0f) / sm;
//			cam.zoomFactor = 0.5f * expf(-camera_zscroll * 0.05f * logf(Math::Max(map->map_w / SCREEN_W, map->map_h / SCREEN_H)));
            float x = camera_zscroll;
            float xt = (x2 + x0) * 0.5f;
            float z(0.0f);
            if (x <= xt)
                z = x0 + (x1 - x0) * (x - x0) / (xt - x0);
            else
                z = x1 + (x2 - x1) * (x - xt) / (x2 - xt);
//            z = x;//Math::Max(Math::Min(x2, xt + x), x0);
//            = x0 + (x1 - x0) * (x - x0) * (x - x2) / ((x1 - x0) * (x1 - x2))
//                               + (x2 - x0) * (x - x0) * (x - x1) / ((x2 - x0) * (x2 - x1));
			cam.zoomFactor = zoom_min + (zoom_max - zoom_min) * (std::exp(sm * (z - x0)) - 1.0f) / (std::exp(sm * (x2 - x0)) - 1.0f);
            cam_h = lp_CONFIG->camera_def_h * 2.0f * cam.zoomFactor;
		}
		else                                // Mega zoom with a perspective camera
		{
			float angle_factor = Math::Max(fabsf(-lp_CONFIG->camera_def_angle + 45.0f) / 20.0f, fabsf(-lp_CONFIG->camera_def_angle + 90.0f) / 25.0f);

			r1 = -lp_CONFIG->camera_def_angle + camera_zscroll * angle_factor;
			if (r1 > -45.0f) 		r1 = -45.0f;
			else if (r1 < -90.0f)	r1 = -90.0f;

			cam_h = lp_CONFIG->camera_def_h + (std::exp(-camera_zscroll * 0.15f) - 1.0f) / (std::exp(3.75f) - 1.0f) * (float)Math::Max(map->map_w, map->map_h);
		}
		if (delta > 0 && !IsOnGUI)
		{
			if (!cam_has_target || abs( mouse_x - cam_target_mx) > 2 || abs( mouse_y - cam_target_my) > 2)
			{
				cam_target = cursorOnMap(cam, *map);
				if (cam_target.x < -map->map_w_d)
					cam_target.x = (float)-map->map_w_d;
				else if (cam_target.x > map->map_w_d)
					cam_target.x = (float)map->map_w_d;
				if (cam_target.z < -map->map_h_d)
					cam_target.z = (float)-map->map_h_d;
				else if (cam_target.z > map->map_h_d)
					cam_target.z = (float)map->map_h_d;
				cam_target_mx = mouse_x;
				cam_target_my = mouse_y;
				cam_has_target = true;
			}
		}

		// Save the Z-coordinate
		omz = mouse_z;
	}


	void Battle::preflightFreeCamera()
	{
		const int delta = IsOnGUI ? 0 : mouse_z - omz;
		cam.rpos = cam.rpos - 0.5f * (float)delta * cam.dir;
		cam_has_target = false;
		// Save the Z-coordinate
		omz = mouse_z;
	}



} // namespace TA3D
