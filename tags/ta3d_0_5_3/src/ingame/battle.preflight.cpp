
#include "battle.h"
#include "../sounds/manager.h"
#include "../UnitEngine.h"


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

		if (!IsOnGUI)
			IsOnGUI = (pArea.check() != 0);
		else        // We need to do it there because AREA::check does it and we do it nowhere else
		{
			poll_mouse();
			poll_keyboard();
		}

		IsOnGUI |= mouse_x < 128; // Priority given to game interface

		if (IsOnMinimap) // Check if we can project the cursor position on the map
		{
			if (fabsf((mouse_x - 64) * 252.0f / 128.0f) > map->mini_w * 0.5f)
				IsOnMinimap = false;
			else
			{
				if (fabsf((mouse_y - 64) * 252.0f / 128.0f) > map->mini_h * 0.5f)
					IsOnMinimap = false;
			}
		}

		if (IsOnMinimap)
			units.pick_minimap(); // Precompute this, we'll need it

		if (video_shoot)
		{
			if ((msec_timer - video_timer) * Conv >= 1.0f / 15.0f)
			{
				video_timer = msec_timer;
				shoot = true;
			}
		}

		// Restore the default cursor
		cursor_type = CURSOR_DEFAULT;
	}


	void Battle::preflightChangeWindSpeedAndDirection()
	{
		wind_t = t;
		map->wind += (Math::RandFromTable() % 2001) - 1000;

		if (map->wind < map->ota_data.minwindspeed)
			map->wind = map->ota_data.minwindspeed;
		else
		{
			if (map->wind > map->ota_data.maxwindspeed)
				map->wind = map->ota_data.maxwindspeed;
		}

		map->wind_dir += (Math::RandFromTable() % 901) * 0.1f - 45.0f;

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
		int delta = IsOnGUI ? 0 : mouse_z -omz;
		camera_zscroll += delta * 2.0f * lp_CONFIG->camera_zoom_speed;
		if (camera_zscroll < -25.0f)
			camera_zscroll = -25.0f;
		else
			if (camera_zscroll > 20.0f) camera_zscroll = 20.0f;

		if ((msec_timer - cam_def_timer) * Conv >= 1.0f && delta != 0
			&& ( ( camera_zscroll > 0.0f && old_zscroll <= 0.0f)
				 || ( camera_zscroll < 0.0f && old_zscroll >= 0.0f)))			// Just to make it lock near def position
		{
			cam_def_timer = msec_timer;
			old_zscroll = 0.0f;
			if (camera_zscroll > -lp_CONFIG->camera_def_angle)
				old_zscroll += 0.00001f;
			else
				old_zscroll -= 0.00001f;
		}

		if ((msec_timer - cam_def_timer) * Conv < 0.5f)
			camera_zscroll = old_zscroll;

        if (lp_CONFIG->ortho_camera)        // 2D zoom with orthographic camera
        {
            r1 = -lp_CONFIG->camera_def_angle;      // angle is constant
            if (r1 > -45.0f) 		r1 = -45.0f;
            else if (r1 < -90.0f)	r1 = -90.0f;
            cam.zoomFactor = 0.5f * expf(-camera_zscroll * 0.04f * logf(Math::Max(map->map_w/SCREEN_W,map->map_h/SCREEN_H)));
            cam_h = lp_CONFIG->camera_def_h * 2.0f * cam.zoomFactor;
        }
        else                                // Mega zoom with a perspective camera
        {
            float angle_factor = Math::Max(fabsf(-lp_CONFIG->camera_def_angle+45.0f) / 20.0f, fabsf(-lp_CONFIG->camera_def_angle+90.0f) / 25.0f);

            r1 = -lp_CONFIG->camera_def_angle + camera_zscroll * angle_factor;
            if (r1 > -45.0f) 		r1 = -45.0f;
            else if (r1 < -90.0f)	r1 = -90.0f;

            cam_h = lp_CONFIG->camera_def_h + (expf(-camera_zscroll * 0.15f) - 1.0f) / (expf(3.75f) - 1.0f) * Math::Max(map->map_w,map->map_h);
            if (delta > 0 && !IsOnGUI)
            {
                if (!cam_has_target || abs( mouse_x - cam_target_mx) > 2 || abs( mouse_y - cam_target_my) > 2)
                {
                    cam_target = cursorOnMap(cam, *map);
                    cam_target_mx = mouse_x;
                    cam_target_my = mouse_y;
                    cam_has_target = true;
                }
            }
        }

		// Save the Z-coordinate
        omz = mouse_z;
	}


	void Battle::preflightFreeCamera()
	{
		int delta = IsOnGUI ? 0 : mouse_z - omz;
		cam.rpos = cam.rpos - 0.5f * delta * cam.dir;
		cam_has_target = false;
		// Save the Z-coordinate
        omz = mouse_z;
	}



} // namespace TA3D
