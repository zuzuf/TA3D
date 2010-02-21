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

/*-----------------------------------------------------------------------------\
  |                                    menu.cpp                                  |
  |       Ce module contient les routines du menu de TA3D                        |
  |                                                                              |
  \-----------------------------------------------------------------------------*/

#include "stdafx.h"
#include <vector>
#include <list>
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "scripts/cob.h"                    // Pour la lecture et l'éxecution des scripts
#include "EngineClass.h"            // Inclus le moteur
#include "fbi.h"                    // Pour la gestion des unités
#include "tnt.h"                    // Inclus le chargeur de cartes
#include "menu.h"
#include "restore.h"
#include "misc/settings.h"
#include "misc/paths.h"
#include "misc/files.h"
#include "misc/resources.h"
#include "logs/logs.h"
#include "ingame/gamedata.h"
#include "ingame/menus/mapselector.h"
#include "ingame/menus/unitselector.h"
#include "languages/i18n.h"
#include "misc/math.h"
#include "sounds/manager.h"
#include "ingame/players.h"
#include "scripts/script.h"
#include "ingame/battle.h"
#include "ai/ai.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "cache.h"
#include "mods/mods.h"
#include "network/netclient.h"
#include <yuni/core/math.h>
#include <yuni/core/sleep.h>



#define p_size          10.0f

// Some functions from main.cpp used to deal with config file



namespace TA3D
{


	void ReadFileParameter();


	static inline String FixBlank(const String& s)
	{
		String t(s);
		t.replace(' ', char(1));
		return t;
	}

	static inline String UnfixBlank(const String& s)
	{
		String t(s);
		t.replace(char(1), ' ');
		return t;
	}


	void config_menu(void)
	{
		cursor_type = CURSOR_DEFAULT;

		if (lp_CONFIG->restorestart )
		{
			lp_CONFIG->restorestart = false;
			lp_CONFIG->quickstart = false;
		}

		TA3DCONFIG  saved_config = *lp_CONFIG;

		Gui::AREA config_area("config");
		config_area.load_tdf("gui/config.area");
		if (!config_area.background )   config_area.background = gfx->glfond;

		std::vector< String > fps_limits;
		if (config_area.get_object("*.fps_limit") )
		{
			fps_limits = config_area.get_object("*.fps_limit")->Text;
			fps_limits.erase( fps_limits.begin());
		}
		else
		{
			fps_limits.push_back("50");
			fps_limits.push_back("60");
			fps_limits.push_back("70");
			fps_limits.push_back("80");
			fps_limits.push_back("90");
			fps_limits.push_back("100");
			fps_limits.push_back("no limit");
		}
		I18N::Translate(fps_limits);

		int nb_res = 0;
		int res_width[100];
		int res_height[100];
		int res_bpp[100];

		SDL_Rect **mode_list = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);

		if (mode_list == (SDL_Rect**)0)         // No resolution available (normally this shouldn't be possible if we get here)
			nb_res = 0;
		else if (mode_list == (SDL_Rect**)-1)   // Ok, everything is possible so let's use standard sizes
		{
#define ADD_RES(w,h)  \
			res_bpp[nb_res++] = 16;\
			res_width[nb_res++] = w;\
			res_height[nb_res++] = h;\
			res_bpp[nb_res++] = 32;\
			res_width[nb_res++] = w;\
			res_height[nb_res++] = h;

			ADD_RES(640,480)
				ADD_RES(800,480)
				ADD_RES(800,600)
				ADD_RES(1024,768)
				ADD_RES(1024,600)
				ADD_RES(1280,960)
				ADD_RES(1280,1024)
				ADD_RES(1440,900)
				ADD_RES(1680,1050)
				ADD_RES(1600,1200)
				ADD_RES(1920,1200)
				ADD_RES(2560,1600)
		}
		else
		{
			for (int i = 0 ; mode_list[i] ; ++i)
			{
				if (mode_list[i]->w >= 640 && mode_list[i]->h >= 480)
				{
					if(SDL_VideoModeOK(mode_list[i]->w, mode_list[i]->h, 16, SDL_FULLSCREEN | SDL_OPENGL) == 16)
					{
						res_bpp[ nb_res ] = 16;
						res_width[ nb_res ] = mode_list[i]->w;
						res_height[ nb_res++ ] = mode_list[i]->h;
					}
					if(SDL_VideoModeOK(mode_list[i]->w, mode_list[i]->h, 32, SDL_FULLSCREEN | SDL_OPENGL) == 32)
					{
						res_bpp[ nb_res ] = 32;
						res_width[ nb_res ] = mode_list[i]->w;
						res_height[ nb_res++ ] = mode_list[i]->h;
					}
				}
			}
		}

		config_area.set_state("*.showfps", lp_CONFIG->showfps);
		config_area.caption("*.fps_limit", fps_limits[fps_limits.size()-1]);
		for (String::Vector::const_iterator i = fps_limits.begin(); i != fps_limits.end(); ++i)
		{
			if ( (String() << (int)lp_CONFIG->fps_limit) == *i )
				config_area.caption("*.fps_limit", *i);
		}
		config_area.set_value("*.interface_transparency", int(lp_CONFIG->menuTransparency * 255.0f));
		config_area.set_value("*.shadow_map_size", lp_CONFIG->shadowmap_size);
		config_area.set_state("*.grab_inputs", lp_CONFIG->grab_inputs);
		config_area.set_value("*.sound_volume", lp_CONFIG->sound_volume);
		config_area.set_value("*.music_volume", lp_CONFIG->music_volume);
		config_area.set_state("*.far_sight", lp_CONFIG->far_sight);
		config_area.set_value("*.anisotropy", lp_CONFIG->anisotropy);
		config_area.set_value("*.mouse_sensitivity", (int)(lp_CONFIG->mouse_sensivity * 100.0f));
		config_area.set_state("*.disable_perspective", lp_CONFIG->ortho_camera);
		config_area.set_state("*.right_click_interface", lp_CONFIG->right_click_interface);
		config_area.set_state("*.disable_GLSL", lp_CONFIG->disable_GLSL);
		config_area.set_state("*.underwater_bright", lp_CONFIG->underwater_bright);
		config_area.set_state("*.use_texture_compression", lp_CONFIG->use_texture_compression);
		config_area.set_state("*.low_definition_map", lp_CONFIG->low_definition_map);
		config_area.set_state("*.sky", lp_CONFIG->render_sky);
		config_area.set_state("*.particle", lp_CONFIG->particle);
		config_area.set_state("*.explosion_particles", lp_CONFIG->explosion_particles);
		config_area.set_state("*.waves", lp_CONFIG->waves);
		config_area.set_state("*.height_line", lp_CONFIG->height_line);
		config_area.set_state("*.detail_tex", lp_CONFIG->detail_tex);
		config_area.set_state("*.use_texture_cache", lp_CONFIG->use_texture_cache);
		config_area.set_state("*.draw_console_loading", lp_CONFIG->draw_console_loading);
		config_area.set_state("*.fullscreen", lp_CONFIG->fullscreen);
		std::vector<I18N::Language> languageList;
		I18N::Instance()->retrieveAllLanguages(languageList);
		if (config_area.get_object("*.LANG"))
		{
			Gui::GUIOBJ::Ptr objLang = config_area.get_object("*.LANG");
			objLang->Text.clear();
			I18N::Language *l = I18N::Instance()->language(lp_CONFIG->Lang);
			if (l)
				objLang->Text.push_back(l->caption());
			else
				objLang->Text.push_back(lp_CONFIG->Lang);
			for (unsigned int i = 0; i < languageList.size(); i++)
				objLang->Text.push_back(languageList[i].caption());
		}
		if (config_area.get_object("*.camera_zoom") )
			config_area.caption( "*.camera_zoom", config_area.get_object("*.camera_zoom")->Text[1+lp_CONFIG->camera_zoom]);
		config_area.caption( "*.camera_def_angle", String() << lp_CONFIG->camera_def_angle );
		config_area.caption( "*.camera_def_h", String() << lp_CONFIG->camera_def_h );
		config_area.caption( "*.camera_zoom_speed", String() << lp_CONFIG->camera_zoom_speed );
		if (config_area.get_object("*.screenres") )
		{
			Gui::GUIOBJ::Ptr obj = config_area.get_object("*.screenres");
			obj->Text.clear();
			int current = 0;
			while( current < nb_res &&
				   ( res_width[ current ] != lp_CONFIG->screen_width
					 || res_height[ current ] != lp_CONFIG->screen_height
					 || res_bpp[ current ] != lp_CONFIG->color_depth ) )
				current++;
			if (current >= nb_res ) current = 0;
			obj->Text.push_back( String() << res_width[ current ] << "x" << res_height[ current ] << "x" << res_bpp[ current ] );
			for( int i = 0 ; i < nb_res ; i++ )
				obj->Text.push_back( String() << res_width[ i ] << "x" << res_height[ i ] << "x" << res_bpp[ i ] );
		}
		Gui::GUIOBJ::Ptr tmpO = config_area.get_object("*.shadow_quality");
		if (tmpO)
		{
			const unsigned int indx = 1 + Math::Max(0, Math::Min((int)lp_CONFIG->shadow_quality, 3));
			if (indx < tmpO->Text.size())
				config_area.caption("*.shadow_quality", tmpO->Text[indx]);
		}

		config_area.caption("*.timefactor", String() << (int)lp_CONFIG->timefactor);
		switch( lp_CONFIG->fsaa )
		{
			case 2: config_area.caption("*.fsaa", "x2");    break;
			case 4: config_area.caption("*.fsaa", "x4");    break;
			case 6: config_area.caption("*.fsaa", "x6");    break;
			case 8: config_area.caption("*.fsaa", "x8");    break;
			default: config_area.caption("*.fsaa", "no fsaa");
		}
		if (config_area.get_object("*.water_quality"))
		{
			Gui::GUIOBJ::Ptr obj = config_area.get_object("*.water_quality");
			config_area.caption("*.water_quality", obj->Text[1 + lp_CONFIG->water_quality]);
		}

		if (config_area.get_object("*.mod"))
		{
			Gui::GUIOBJ::Ptr obj = config_area.get_object("*.mod");

			if (obj->Text.size() >= 2)
				obj->Text[0] = obj->Text[1];
			else
				obj->Text.resize(1);

			String current_selection = TA3D_CURRENT_MOD.length() > 6 ? TA3D_CURRENT_MOD.substr( 5, TA3D_CURRENT_MOD.length() - 6 ) : "";
			String::List mod_list = Mods::instance()->getModNameList(Mods::MOD_INSTALLED);
			mod_list.sort();
			mod_list.unique();
			for (String::List::iterator i = mod_list.begin() ; i != mod_list.end() ; ++i)
			{
				obj->Text.push_back( *i );
				if (String::ToLower( *i ) == String::ToLower(current_selection))
					obj->Text[0] = *i;
			}
		}

		config_area.caption("*.player_name", lp_CONFIG->player_name);

		if (config_area.get_object("*.skin"))
		{
			Gui::GUIOBJ::Ptr obj = config_area.get_object("*.skin");

			obj->Text.resize(1);
			obj->Text[0] = I18N::Translate( "default.skn");

			String::Vector skin_list;
			VFS::Instance()->getFilelist("gui\\*.skn", skin_list);

			String skin_name;
			const String::Vector::iterator end = skin_list.end();
			for (String::Vector::iterator i = skin_list.begin(); i != end; ++i)
			{
				skin_name = Paths::ExtractFileName(*i, false);
				obj->Text.push_back(skin_name);
				if ("gui/" + String::ToLower(skin_name) == String::ToLower(lp_CONFIG->skin_name))
					obj->Text[0] = skin_name;
			}
		}

		if (config_area.get_object("*.l_files"))
		{
			Gui::GUIOBJ::Ptr obj = config_area.get_object("*.l_files");
			sound_manager->getPlayListFiles(obj->Text);
		}

		if (lp_CONFIG->quickstart)
			I_Msg( TA3D::TA3D_IM_GUI_MSG, "config_confirm.show");

		bool done = false;
		bool save = false;

		int amx = -1;
		int amy = -1;
		int amz = -1;
		int amb = -1;
		uint32 timer = msec_timer;

		do
		{
			bool time_out = false;
			bool key_is_pressed = false;
			do
			{
				config_area.check();
				key_is_pressed = config_area.key_pressed;
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				if (lp_CONFIG->quickstart)
				{
					Gui::GUIOBJ::Ptr pbar = config_area.get_object( "config_confirm.p_wait");
					if (pbar)
					{
						unsigned int new_value = (msec_timer - timer) / 50;
						if (new_value != pbar->Data)
						{
							pbar->Data = new_value;
							key_is_pressed = true;
							if (new_value == 100)
								time_out = true;
						}
					}
				}
			} while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b
				&& !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed
				&& !config_area.scrolling);

			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;

			if (lp_CONFIG->quickstart)
			{
				if (time_out || config_area.get_state("config_confirm.b_cancel_changes" ) || key[KEY_ESC])
				{
					I_Msg( TA3D::TA3D_IM_GUI_MSG, "config_confirm.hide");
					TA3D::Settings::Restore(TA3D::Paths::ConfigFile);
					TA3D::Settings::Load();
					done = true;
					save = false;
					lp_CONFIG->quickstart = false;
					lp_CONFIG->quickrestart = true;
					lp_CONFIG->restorestart = true;
					saved_config = *lp_CONFIG;
				}
				else
					if (config_area.get_state("config_confirm.b_confirm"))
					{
						I_Msg( TA3D::TA3D_IM_GUI_MSG, "config_confirm.hide");
						lp_CONFIG->quickstart = false;
						saved_config.quickstart = false;
						TA3D::Settings::Save();             // Keep settings :)
					}
			}

			if (config_area.get_state( "*.b_activate"))
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object("*.l_files");
				if (obj && obj->Text.size() > obj->Pos)
				{
					sound_manager->setPlayListFileMode( obj->Pos, false, false);
					obj->Text[ obj->Pos ][ 1 ] = '*';
				}
			}
			if (config_area.get_state( "*.b_deactivate"))
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object("*.l_files");
				if (obj && obj->Text.size() > obj->Pos)
				{
					sound_manager->setPlayListFileMode( obj->Pos, false, true);
					obj->Text[ obj->Pos ][ 1 ] = ' ';
				}
			}
			if (config_area.get_state( "*.b_battle" ) )
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object("*.l_files");
				if (obj && obj->Text.size() > obj->Pos)
				{
					sound_manager->setPlayListFileMode(obj->Pos, true, false);
					obj->Text[ obj->Pos ][1] = 'B';
				}
			}


			if (config_area.get_state("*.b_ok"))
			{
				done = true;        // En cas de click sur "OK", on quitte la fenêtre
				save = true;
			}
			if (config_area.get_state( "*.b_cancel" ) ) done=true;      // En cas de click sur "retour", on quitte la fenêtre

			lp_CONFIG->showfps = config_area.get_state( "*.showfps");
			if (config_area.get_value("*.fps_limit") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object("*.fps_limit");
				if (obj && obj->Data != uint32(-1))
				{
					obj->Text[0] = fps_limits[obj->Value];
					switch (obj->Value)
					{
						case 0:  lp_CONFIG->fps_limit = 50;  break;
						case 1:  lp_CONFIG->fps_limit = 60;  break;
						case 2:  lp_CONFIG->fps_limit = 70;  break;
						case 3:  lp_CONFIG->fps_limit = 80;  break;
						case 4:  lp_CONFIG->fps_limit = 90;  break;
						case 5:  lp_CONFIG->fps_limit = 100; break;
						default: lp_CONFIG->fps_limit = -1;
					}
				}
			}

			if (lp_CONFIG->grab_inputs != config_area.get_state("*.grab_inputs"))
			{
				lp_CONFIG->grab_inputs = config_area.get_state("*.grab_inputs");
				grab_mouse(lp_CONFIG->grab_inputs);
			}
			if (lp_CONFIG->sound_volume != config_area.get_value("*.sound_volume"))
			{
				lp_CONFIG->sound_volume = config_area.get_value("*.sound_volume");
				sound_manager->setVolume(lp_CONFIG->sound_volume);
			}
			if (lp_CONFIG->music_volume != config_area.get_value("*.music_volume"))
			{
				lp_CONFIG->music_volume = config_area.get_value("*.music_volume");
				sound_manager->setMusicVolume(lp_CONFIG->music_volume);
			}
			lp_CONFIG->menuTransparency = float(config_area.get_value("*.interface_transparency") / 255.0f);
			lp_CONFIG->shadowmap_size = uint8(config_area.get_value("*.shadow_map_size"));
			lp_CONFIG->far_sight = config_area.get_state("*.far_sight");
			lp_CONFIG->anisotropy = sint16(config_area.get_value("*.anisotropy"));
			lp_CONFIG->mouse_sensivity = float(config_area.get_value("*.mouse_sensitivity")) * 0.01f;
			lp_CONFIG->ortho_camera = config_area.get_state("*.disable_perspective");
			lp_CONFIG->right_click_interface = config_area.get_state("*.right_click_interface");
			lp_CONFIG->disable_GLSL = config_area.get_state("*.disable_GLSL");
			lp_CONFIG->underwater_bright = config_area.get_state("*.underwater_bright");
			lp_CONFIG->use_texture_compression = config_area.get_state("*.use_texture_compression");
			lp_CONFIG->low_definition_map = config_area.get_state("*.low_definition_map");
			lp_CONFIG->render_sky = config_area.get_state( "*.sky");
			lp_CONFIG->particle = config_area.get_state( "*.particle");
			lp_CONFIG->explosion_particles = config_area.get_state( "*.explosion_particles");
			lp_CONFIG->waves = config_area.get_state( "*.waves");
			lp_CONFIG->height_line = config_area.get_state( "*.height_line");
			lp_CONFIG->detail_tex = config_area.get_state( "*.detail_tex");
			lp_CONFIG->draw_console_loading = config_area.get_state( "*.draw_console_loading");
			lp_CONFIG->fullscreen = config_area.get_state( "*.fullscreen");
			lp_CONFIG->use_texture_cache = config_area.get_state( "*.use_texture_cache");
			if (config_area.get_value( "*.camera_zoom" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.camera_zoom");
				if (obj && obj->Value >= -1)
				{
					obj->Text[0] = obj->Text[1 + obj->Value];
					lp_CONFIG->camera_zoom = uint8(obj->Value);
				}
			}
			if (config_area.get_value( "*.camera_def_angle" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.camera_def_angle");
				if (obj && obj->Value >= 0)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value ];
                    lp_CONFIG->camera_def_angle = obj->Text[0].to<float>();
				}
			}
			if (config_area.get_value( "*.camera_def_h" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.camera_def_h");
				if (obj && obj->Value >= 0)
				{
					obj->Text[0] = obj->Text[1 + obj->Value];
                    lp_CONFIG->camera_def_h = obj->Text[0].to<float>();
				}
			}
			if (config_area.get_value( "*.camera_zoom_speed" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.camera_zoom_speed");
				if (obj && obj->Value >= 0)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value ];
                    lp_CONFIG->camera_zoom_speed = obj->Text[0].to<float>();
				}
			}
			if (config_area.get_value( "*.LANG" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.LANG");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[1 + obj->Value];
					lp_CONFIG->Lang = languageList[obj->Value].englishCaption();
				}
			}
			if (config_area.get_value("*.screenres") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.screenres");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value ];
					lp_CONFIG->screen_width = uint16(res_width[ obj->Value ]);
					lp_CONFIG->screen_height = uint16(res_height[ obj->Value ]);
					lp_CONFIG->color_depth = uint8(res_bpp[ obj->Value ]);
				}
			}
			if (config_area.get_value("*.shadow_quality") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object("*.shadow_quality");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[1 + obj->Value];
					lp_CONFIG->shadow_quality = sint16(obj->Value);
				}
			}
			if (config_area.get_value("*.timefactor") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.timefactor");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value ];
					lp_CONFIG->timefactor = float(obj->Value + 1);
				}
			}
			if (config_area.get_value( "*.fsaa" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.fsaa");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value ];
					lp_CONFIG->fsaa = sint16(obj->Value << 1);
				}
			}
			if (config_area.get_value("*.water_quality") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object("*.water_quality");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value ];
					lp_CONFIG->water_quality = sint16(obj->Value);
				}
			}
			if (config_area.get_value("*.mod") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.mod");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[ 1 + obj->Value];
					lp_CONFIG->last_MOD = obj->Value > 0 ? "mods/" + obj->Text[0] + "/" : "";
				}
			}
			if (config_area.get_value( "*.skin" ) >= 0)
			{
				Gui::GUIOBJ::Ptr obj = config_area.get_object( "*.skin");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[1 + obj->Value];
					lp_CONFIG->skin_name = obj->Value >= 0 ? "gui/" + obj->Text[0] : "";
				}
			}

			if (key[KEY_ESC])
				done = true;

			config_area.draw();

			glEnable(GL_TEXTURE_2D);
			gfx->set_color(0xFFFFFFFF);
			draw_cursor();

			// Affiche
			gfx->flip();
		} while(!done);

		if (config_area.background == gfx->glfond ) config_area.background = 0;

		reset_mouse();
		while (key[KEY_ESC])
		{
			SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}

		bool ask_for_quickrestart = lp_CONFIG->quickrestart;

		if (!save)
			*lp_CONFIG = saved_config;
		else
		{
			sound_manager->savePlaylist();              // Save the playlist

			// Check we're not trying to crash the game :P
			gfx->checkConfig();

			if (lp_CONFIG->screen_width != saved_config.screen_width ||
				lp_CONFIG->screen_height != saved_config.screen_height ||
				lp_CONFIG->color_depth != saved_config.color_depth ||
				lp_CONFIG->fsaa != saved_config.fsaa ||
				(lp_CONFIG->fullscreen != saved_config.fullscreen) )            // Need to restart
			{
				lp_CONFIG->quickrestart = true;
			}

			lp_CONFIG->player_name = config_area.caption("*.player_name");

			if (lp_CONFIG->last_MOD != TA3D_CURRENT_MOD) // Refresh the file structure
			{
				TA3D_CURRENT_MOD = lp_CONFIG->last_MOD;
				Cache::Clear(true); // Force cache reset

				VFS::Instance()->reload();
				ta3dSideData.loadData();                // Refresh side data so we load the correct values
				sound_manager = NULL;	// Proper cleaning
				sound_manager = new TA3D::Audio::Manager();
				sound_manager->loadTDFSounds(true);
				sound_manager->loadTDFSounds(false);
			}
		}

		grab_mouse(lp_CONFIG->grab_inputs);
		sound_manager->setVolume(lp_CONFIG->sound_volume);
		sound_manager->setMusicVolume(lp_CONFIG->music_volume);

		lp_CONFIG->quickrestart |= ask_for_quickrestart;

		config_area.destroy();

		I18N::CurrentLanguage(lp_CONFIG->Lang);

		TA3D::Settings::Save();             // Keep settings :)
	}

	void setup_game(bool client, const String& host, String saved_game, bool bNetServer)
	{
		int my_player_id = -1;
		bool advertise = false;
		String status;
		if (!host.empty())
		{
			if (!client)
			{
				if (!bNetServer)
					network_manager.InitBroadcast(1234);      // broadcast mode
				network_manager.HostGame(host, 4242);
			}
			else
				network_manager.Connect(host, 4242);

			my_player_id = network_manager.getMyID();           // Get player id

			if (client)
			{
				status = network_manager.getStatus();
				LOG_DEBUG("client received game status : " << status);
				if (!status.empty())
				{
					status = Paths::Savegames + "multiplayer" + Paths::Separator + status;
					saved_game = status;
				}
				else
				{
					network_manager.sendSpecial("NOTIFY NEW_PLAYER " + FixBlank(lp_CONFIG->player_name));
					SleepMilliSeconds(10);
					network_manager.sendSpecial( "REQUEST GameData" );
				}
			}
		}

		cursor_type = CURSOR_DEFAULT;

		uint16  player_str_n = 4;
		String  player_str[4] = { lp_CONFIG->player_name, I18N::Translate("computer"), I18N::Translate("open"), I18N::Translate("closed") };
		byte    player_control[4] = { PLAYER_CONTROL_LOCAL_HUMAN, PLAYER_CONTROL_LOCAL_AI, PLAYER_CONTROL_NONE, PLAYER_CONTROL_CLOSED };
		int     side_str_n = ta3dSideData.nb_side;
		String::Vector  side_str;
		String::Vector  AI_list = AI_PLAYER::getAvailableAIs();
		AI_list.resize(AI_list.size() + 1);
		for (int i = int(AI_list.size()) - 1 ; i > 0 ; --i)
			AI_list[i] = AI_list[i-1];

		side_str.resize( ta3dSideData.nb_side);
		for (int i = 0; i < ta3dSideData.nb_side; ++i)
			side_str[i] = ta3dSideData.side_name[i];

		GameData game_data;

		if (VFS::Instance()->fileExists(lp_CONFIG->last_map))
			game_data.map_filename = lp_CONFIG->last_map;
		else
		{
			String::Vector map_list;
			const uint32 n = VFS::Instance()->getFilelist("maps\\*.tnt", map_list);

			if (n == 0)
			{
				network_manager.Disconnect();
				TA3D::Gui::AREA::current()->popup(I18N::Translate("Error"),I18N::Translate("No maps found"));
				LOG_ERROR("No map has been found !");
				reset_mouse();
				return;
			}
			game_data.map_filename = *(map_list.begin());
			map_list.clear();
		}
		game_data.nb_players = 2;
		if (VFS::Instance()->fileExists(lp_CONFIG->last_script) && String::ToLower(Paths::ExtractFileExt(lp_CONFIG->last_script)) == ".lua")
			game_data.game_script = lp_CONFIG->last_script;
		else if (VFS::Instance()->fileExists("scripts\\game\\default.lua"))
			game_data.game_script = "scripts\\game\\default.lua";
		else
		{
			String::Vector script_list;
			uint32 n = VFS::Instance()->getFilelist("scripts\\game\\*.lua", script_list);

			if (n == 0)
			{
				network_manager.Disconnect();
				TA3D::Gui::AREA::current()->popup(I18N::Translate("Error"),I18N::Translate("No scripts found"));
				LOG_ERROR("No script has been found!!");
				reset_mouse();
				return;
			}
			for (String::Vector::iterator i = script_list.begin() ; i != script_list.end() ; ++i)
			{
				game_data.game_script = *i;
				if (i->size() > 1 && (*i)[0] != '_')            // Avoid selecting a special file as default script if possible
					break;
			}
			script_list.clear();
		}
		game_data.fog_of_war = lp_CONFIG->last_FOW;

		for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
		{
			game_data.player_names[i] = player_str[2];
			game_data.player_sides[i] = side_str[0];
			game_data.player_control[i] = player_control[2];
			game_data.ai_level[i] = AI_list.empty() ? String("none") : AI_list[0];
		}

		if (!client)
		{
			game_data.player_names[0] = player_str[0];
			game_data.player_sides[0] = side_str[0];
			game_data.player_control[0] = player_control[0];
			game_data.player_network_id[0] = my_player_id;
			game_data.ai_level[0] = AI_list.empty() ? String("none") : AI_list[0];

			if (!host)
			{
				game_data.player_names[1] = player_str[1];
				game_data.player_sides[1] = side_str[1];
				game_data.player_control[1] = player_control[1];
				game_data.ai_level[1] = AI_list.empty() ? String("none") : AI_list[0];
			}
		}

		int net_id_table[10];           // Table used to identify players joining a multiplayer saved game

		if (saved_game.notEmpty())             // We're loading a multiplayer game !!
		{
			load_game_data( saved_game, &game_data, true);      // Override server only access to game information, we're loading now
			int my_old_id = -1;
			for (int i = 0 ; i < 10 ; i++)          // Build the reference table
			{
				net_id_table[i] = game_data.player_network_id[i];
				if (game_data.player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN)
					my_old_id = net_id_table[i];
			}
			network_manager.sendSpecial( String("NOTIFY PLAYER_BACK ") << my_old_id );
			SleepMilliSeconds(10);
			network_manager.sendSpecial( "REQUEST GameData" );
		}

		int dx, dy;
		GLuint glimg = load_tnt_minimap_fast(game_data.map_filename,dx,dy);
		MAP_OTA map_data;
		map_data.load(Paths::Files::ReplaceExtension(game_data.map_filename, ".ota"));
		float ldx = float(dx) * 70.0f / 252.0f;
		float ldy = float(dy) * 70.0f / 252.0f;

		Gui::AREA setupgame_area("setup");
		setupgame_area.load_tdf("gui/setupgame.area");
		if (!setupgame_area.background)
			setupgame_area.background = gfx->glfond;
		for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
		{
			setupgame_area.caption( String("gamesetup.name") << i, game_data.player_names[i]);

			Gui::GUIOBJ::Ptr guiobj = setupgame_area.get_object(String("gamesetup.side") << i);
			if (guiobj)
			{
				guiobj->Text.clear();
				guiobj->Text.push_back( game_data.player_sides[i] );
				guiobj->Text.insert(guiobj->Text.end(), side_str.begin(), side_str.end());
			}
			AI_list[0] = (game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI) ? game_data.ai_level[i] : String("");
			setupgame_area.set_entry( String("gamesetup.ai") << i, AI_list);
                        guiobj = setupgame_area.get_object( String("gamesetup.color") << i);
			if (guiobj)
			{
				guiobj->Flag |= (game_data.player_control[i] == PLAYER_CONTROL_NONE ? FLAG_HIDDEN : 0);
				guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3], player_color[player_color_map[i]*3+1], player_color[player_color_map[i]*3+2]);
			}
			guiobj = setupgame_area.get_object( String("gamesetup.team") << i );
			if (guiobj)
				guiobj->current_state = byte(i);
			setupgame_area.caption( String("gamesetup.energy") << i, String() << game_data.energy[i]);
			setupgame_area.caption( String("gamesetup.metal") << i, String() << game_data.metal[i]);
		}

		if (setupgame_area.get_object("gamesetup.max_units"))
		{
			Gui::GUIOBJ::Ptr obj = setupgame_area.get_object("gamesetup.max_units");
			obj->Text[0] = String() << game_data.max_unit_per_player;
		}

		Gui::GUIOBJ::Ptr minimap_obj = setupgame_area.get_object( "gamesetup.minimap");
		float mini_map_x1 = 0.0f;
		float mini_map_y1 = 0.0f;
		float mini_map_x2 = 0.0f;
		float mini_map_y2 = 0.0f;
		float mini_map_x = 0.0f;
		float mini_map_y = 0.0f;
		if (minimap_obj)
		{
			mini_map_x1 = minimap_obj->x1;
			mini_map_y1 = minimap_obj->y1;
			mini_map_x2 = minimap_obj->x2;
			mini_map_y2 = minimap_obj->y2;
			ldx = float(dx) * ( mini_map_x2 - mini_map_x1 ) / 504.0f;
			ldy = float(dy) * ( mini_map_y2 - mini_map_y1 ) / 504.0f;

			mini_map_x = (mini_map_x1 + mini_map_x2) * 0.5f;
			mini_map_y = (mini_map_y1 + mini_map_y2) * 0.5f;

			minimap_obj->Data = glimg;
			minimap_obj->x1 = mini_map_x - ldx;
			minimap_obj->y1 = mini_map_y - ldy;
			minimap_obj->x2 = mini_map_x + ldx;
			minimap_obj->y2 = mini_map_y + ldy;
			minimap_obj->u2 = float(dx) / 252.0f;
			minimap_obj->v2 = float(dy) / 252.0f;
		}

		Gui::GUIOBJ::Ptr guiobj = setupgame_area.get_object( "scripts.script_list");
		if (guiobj)
		{
			String::Vector script_list;
			VFS::Instance()->getFilelist("scripts\\game\\*.lua", script_list);
			guiobj->Text.clear();
			for (String::Vector::const_iterator i_script = script_list.begin(); i_script != script_list.end(); ++i_script)
				guiobj->Text.push_back(*i_script);
		}
		setupgame_area.caption( "gamesetup.script_name", game_data.game_script);
		{
			Gui::GUIOBJ::Ptr obj = setupgame_area.get_object( "gamesetup.FOW");
			if (obj )
				obj->Text[0] = obj->Text[ 1 + game_data.fog_of_war ];
		}

		{
			String map_info;
			if (!map_data.missionname.empty())
				map_info << map_data.missionname << "\n";
			if (!map_data.numplayers.empty())
				map_info << map_data.numplayers << "\n";
			if (!map_data.missiondescription.empty())
				map_info << map_data.missiondescription;
			setupgame_area.caption("gamesetup.map_info", map_info);
		}

		if (!host)
			for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
				setupgame_area.msg(String("gamesetup.ready") << i << ".hide");
		uint32 player_timer[TA3D_PLAYERS_HARD_LIMIT];
		for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
			player_timer[i] = msec_timer;

		bool done = false;

		if (host.notEmpty() && my_player_id == -1) // Leave now, we aren't connected but we're in network mode
		{
			LOG_ERROR("in network mode without being connected");
			done = true;
		}

		if (saved_game.notEmpty() && game_data.saved_file.empty())     // We're trying to load a multiplayer game we didn't save
		{
			LOG_ERROR("trying to load a multiplayer game we didn't play");
			done = true;
		}

		bool start_game = false;

		int amx = -1;
		int amy = -1;
		int amz = -1;
		int amb = -1;

		String set_map;
		String previous_tnt_port;
		String previous_ota_port;
		String previous_lua_port;

		if (host.notEmpty() && client)
		{
			setupgame_area.msg("gamesetup.b_ok.disable");
			setupgame_area.msg("gamesetup.b_units.disable");
			setupgame_area.msg("gamesetup.max_units.disable");
		}
		else if (saved_game.notEmpty())
		{
			setupgame_area.msg("gamesetup.b_units.disable");
			setupgame_area.msg("gamesetup.change_map.disable");
			setupgame_area.msg("gamesetup.FOW.disable");
			for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
			{
				Gui::GUIOBJ::Ptr obj = setupgame_area.get_object(String("gamesetup.team") << i);
				if (obj)
					obj->Flag &= ~FLAG_CAN_BE_CLICKED;
			}
		}

		int progress_timer = msec_timer;
		int ping_timer = msec_timer;                    // Used to send simple PING requests in order to detect when a connection fails

		bool statusUpdateRequired = true;

		do
		{
			if (host.notEmpty())
			{
				for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
				{
					Gui::GUIOBJ::Ptr obj = setupgame_area.get_object(String("gamesetup.ready") << i);
					if (obj)
					{
						if (game_data.player_control[i] != PLAYER_CONTROL_LOCAL_HUMAN
							&& game_data.player_control[i] != PLAYER_CONTROL_REMOTE_HUMAN)
							obj->Flag |= FLAG_HIDDEN;
						else
							obj->Flag &= ~FLAG_HIDDEN;
						obj->Etat = game_data.ready[i];
					}
				}
			}

			if (statusUpdateRequired && !client)
			{
				statusUpdateRequired = false;
				if (bNetServer)
				{
					uint16 nb_open = 0;
					for (int f = 0; f < TA3D_PLAYERS_HARD_LIMIT; ++f)
					{
						if (setupgame_area.caption(String("gamesetup.name") << f) == player_str[2])
							++nb_open;
					}
					NetClient::instance()->sendMessage("SERVER MAP \"" + Escape(Paths::ExtractFileNameWithoutExtension(game_data.map_filename)) + "\" SLOTS " + String(nb_open));
				}
			}

			if (saved_game.notEmpty())
			{
				for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
				{
					if (game_data.player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN)
					{
						if (!game_data.ready[i])
						{
							setupgame_area.set_state(String("gamesetup.ready") << i,true);
							game_data.ready[i] = true;
							network_manager.sendSpecial("NOTIFY UPDATE");
						}
					}
				}
			}

			bool key_is_pressed = false;
			String broadcast_msg;
			String chat_msg;
			String special_msg;
			struct chat received_chat_msg;
			struct chat received_special_msg;
			bool playerDropped = false;
			do
			{
				if (bNetServer)
					NetClient::instance()->receive();
				playerDropped = network_manager.getPlayerDropped();
				broadcast_msg = network_manager.getNextBroadcastedMessage();
				if (network_manager.getNextChat(&received_chat_msg ) == 0)
					chat_msg = (char*)received_chat_msg.message;
				if (network_manager.getNextSpecial(&received_special_msg ) == 0)
					special_msg = (char*)received_special_msg.message;
				if (host.notEmpty() && !network_manager.isConnected()) // We're disconnected !!
				{
					LOG_DEBUG("disconnected from server");
					done = true;
					break;
				}
				setupgame_area.check();
				key_is_pressed = setupgame_area.key_pressed;
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				if (msec_timer - progress_timer >= 500 && Yuni::Math::Equals(network_manager.getFileTransferProgress(), 100.0f))
					break;
			} while (amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done
				&& !key_is_pressed && !setupgame_area.scrolling && broadcast_msg.empty() && chat_msg.empty() && special_msg.empty() && !playerDropped
				&& ( msec_timer - ping_timer < 2000 || host.empty() || client ));

			//-------------------------------------------------------------- Network Code : syncing information --------------------------------------------------------------

			if (host.notEmpty() && !client && msec_timer - ping_timer >= 2000) // Send a ping request
			{
				statusUpdateRequired = true;
				network_manager.sendPing();
				ping_timer = msec_timer;

				for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i) // ping time out
				{
					if (game_data.player_network_id[i] > 0 && msec_timer - player_timer[i] > 10000)
					{
						LOG_DEBUG("dropping player " << game_data.player_network_id[i] << "[" << i << "] from " << __FILE__ << " l." << __LINE__);
						network_manager.dropPlayer(game_data.player_network_id[i]);
						playerDropped = true;
						player_timer[i] = msec_timer;
					}
				}
			}

            if (network_manager.getFileTransferProgress() < 100.0f)
			{
				progress_timer = msec_timer;
				Gui::GUIOBJ::Ptr obj = setupgame_area.get_object( "gamesetup.p_transfer");
				if (obj)
				{
					obj->Flag &= ~FLAG_HIDDEN;
					int progress = (int)network_manager.getFileTransferProgress();
					obj->Data = progress;
				}
			}
			else
			{
				Gui::GUIOBJ::Ptr obj = setupgame_area.get_object( "gamesetup.p_transfer");
				if (obj)
					obj->Flag |= FLAG_HIDDEN;
			}

			if (playerDropped)
			{
				for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
				{
					if (game_data.player_network_id[i] > 0 && !network_manager.pollPlayer( game_data.player_network_id[i]))
					{
						if (saved_game.notEmpty())
						{
							setupgame_area.set_state(String("gamesetup.ready") << i, false);     // He's not there
							game_data.ready[i] = false;
						}
						else
						{
							game_data.player_names[i] = player_str[2];
							game_data.player_sides[i] = side_str[0];
							game_data.player_control[i] = player_control[2];
							game_data.ai_level[i] = AI_TYPE_EASY;
							game_data.player_network_id[i] = -1;

							setupgame_area.caption( String("gamesetup.name") << i,game_data.player_names[i]);                                 // Update gui
							AI_list[0] = (game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI) ? game_data.ai_level[i] : String();
							setupgame_area.set_entry( String("gamesetup.ai") << i, AI_list);
							setupgame_area.caption( String("gamesetup.side") << i, side_str[0]);                           // Update gui

							Gui::GUIOBJ::Ptr guiobj =  setupgame_area.get_object( String("gamesetup.color") << i );
							if (guiobj)
								guiobj->Flag |= FLAG_HIDDEN;
						}
					}
				}
				network_manager.sendSpecial("NOTIFY UPDATE");
			}

			while (!special_msg.empty()) // Special receiver (sync config data)
			{
				int from = received_special_msg.from;
				String::Vector params;
				LOG_DEBUG(LOG_PREFIX_NET << "parsing '" << (char*)(received_special_msg.message) << "'");
				String((char*)(received_special_msg.message)).explode(params, ' ', true, false, true);
				if (params.size() == 1)
				{
					if (params[0] == "PONG")
					{
						int player_id = game_data.net2id(from);
						if (player_id >= 0 )
							player_timer[ player_id ] = msec_timer;
					}
				}
				else
					if (params.size() == 2)
					{
						if (params[0] == "REQUEST")
						{
							if (params[1] == "PLAYER_ID")                  // Sending player's network ID
								network_manager.sendSpecial( String("RESPONSE PLAYER_ID ") << from, -1, from);
							else
							{
								if (params[1] == "GameData") // Sending game information
								{
									network_manager.sendSpecial(String("SET UNIT LIMIT ") << game_data.max_unit_per_player);
									for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i) // Send player information
									{
										if (client && game_data.player_network_id[i] != my_player_id )  continue;       // We don't send updates about things we won't update
										String msg;                             // SYNTAX: PLAYER_INFO player_id network_id side_id ai_level metal energy player_name ready_flag
										int side_id = String::FindInList(side_str, game_data.player_sides[i]);
										msg << "PLAYER_INFO " << i << " " << game_data.player_network_id[i] << " "
											<< side_id << " "
											<< ((game_data.player_control[i] == PLAYER_CONTROL_NONE || game_data.player_control[i] == PLAYER_CONTROL_CLOSED || game_data.ai_level[i].empty()) ? String("[C]") : FixBlank(game_data.ai_level[i]))
											<< " " << game_data.metal[i] << " " << game_data.energy[i] << " "
											<< FixBlank(game_data.player_names[i]) << " " << (int)game_data.ready[i];
										network_manager.sendSpecial( msg, -1, from);

										Gui::GUIOBJ::Ptr guiobj =  setupgame_area.get_object( String("gamesetup.team") << i);
										if (guiobj)
										{
											msg.clear();
											msg << "CHANGE TEAM " << i << " " << (int)(guiobj->current_state);
											network_manager.sendSpecial( msg, -1, from);
										}
									}
									if (!client)  // Send server to client specific information (player colors, map name, ...)
									{
										String msg("PLAYERCOLORMAP");
										for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
											msg += String(" ") << int(player_color_map[i]);
										network_manager.sendSpecial( msg, -1, from);

										network_manager.sendSpecial(String("SET FOW ") << int(game_data.fog_of_war), -1, from);
										network_manager.sendSpecial("SET SCRIPT " + FixBlank( game_data.game_script), -1, from);
										network_manager.sendSpecial("SET MAP " + FixBlank( game_data.map_filename), -1, from);
									}
								}
								else if (params[1] == "STATUS")
								{
									if (saved_game.notEmpty())
										network_manager.sendSpecial("STATUS SAVED " + FixBlank( Paths::ExtractFileName(saved_game) ), -1, from);
									else
										network_manager.sendSpecial("STATUS NEW", -1, from);
								}
							}
						}
						else
							if (params[0] == "NOTIFY")
							{
								if (params[1] == "UPDATE")
									network_manager.sendSpecial( "REQUEST GameData");           // We're told there are things to update, so ask for update
								else
								{
									if (params[1] == "PLAYER_LEFT")
									{
										LOG_DEBUG("dropping player " << from << " from " << __FILE__ << " l." << __LINE__);
										network_manager.dropPlayer(from);
										network_manager.sendSpecial( "REQUEST GameData");           // We're told there are things to update, so ask for update
										for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
										{
											if (game_data.player_network_id[i] == from)
											{
												if (saved_game.notEmpty())
												{
													setupgame_area.set_state(String("gamesetup.ready") << i,false);
													game_data.ready[i] = false;
												}
												else
												{
													game_data.player_network_id[i] = -1;
													game_data.player_control[i] = player_control[2];
													game_data.player_names[i] = player_str[2];

													setupgame_area.caption(String("gamesetup.name") << i, game_data.player_names[i]);

													Gui::GUIOBJ::Ptr guiobj =  setupgame_area.get_object( String("gamesetup.color") << i );
													if (guiobj)
														guiobj->Flag |= FLAG_HIDDEN;
												}
												break;
											}
										}
									}
									else
									{
										if (params[1] == "START") // Game is starting ...
										{
											clear_keybuf();
											done = true;      // If user click "OK" or hit enter then leave the window
											start_game = true;
										}
									}
								}
							}
					}
					else
						if (params.size() == 3)
						{
							if (params[0] == "NOTIFY")
							{
								if (params[1] == "NEW_PLAYER" && !saved_game) // Add new player
								{
									int slot = -1;
									for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
									{
										if (game_data.player_control[i] == PLAYER_CONTROL_NONE)
										{
											slot = i;
											break;
										}
									}
									if (slot >= 0)
									{
										player_timer[ slot ] = msec_timer;              // If we forget this, player will be droped immediately
										game_data.player_network_id[slot] = from;
										game_data.player_control[slot] = PLAYER_CONTROL_REMOTE_HUMAN;
										game_data.player_names[slot] = UnfixBlank( params[2] );
										setupgame_area.caption( String("gamesetup.name") << slot, game_data.player_names[slot]);                      // Update gui

										Gui::GUIOBJ::Ptr guiobj =  setupgame_area.get_object( String("gamesetup.color") << slot);
										if (guiobj)
										{
											guiobj->Data = gfx->makeintcol( player_color[player_color_map[slot] * 3],
																			player_color[player_color_map[slot] * 3 + 1],
																			player_color[player_color_map[slot] * 3 + 2]);           // Update gui
											guiobj->Flag &= ~FLAG_HIDDEN;
										}
										network_manager.sendSpecial( "NOTIFY UPDATE", from);            // Tell others that things have changed
									}
									else
									{
										LOG_DEBUG("dropping player " << from << " from " << __FILE__ << " l." << __LINE__);
										network_manager.dropPlayer(from);      // No more room for this player !!
									}
								}
								else if (params[1] == "PLAYER_BACK" && saved_game.notEmpty()) // A player is back in the game :), let's find who it is
								{
									LOG_DEBUG("received identifier from " << from << " : " << params[2].to<sint32>());
									int slot = -1;
									for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
									{
										if (net_id_table[i] == params[2].to<sint32>())
										{
											slot = i;
											break;
										}
									}
									if (slot >= 0)
									{
										player_timer[ slot ] = msec_timer;              // If we forget this, player will be droped immediately
										game_data.player_network_id[slot] = from;
										game_data.player_control[slot] = PLAYER_CONTROL_REMOTE_HUMAN;

										network_manager.sendSpecial( "NOTIFY UPDATE", from);            // Tell others that things have changed
									}
									else
									{
										LOG_DEBUG("dropping player " << from << " from " << __FILE__ << " l." << __LINE__ << " because it couldn't be identified");
										network_manager.dropPlayer(from);      // No more room for this player !!
									}
								}
								else if (params[1] == "COLORCHANGE")
								{
									int i = params[2].to<sint32>();
									if (!client) // From client to server only
									{
										byte e = player_color_map[i];
										int f = -1;
										for (int g = 0; g < TA3D_PLAYERS_HARD_LIMIT; ++g) // Look for the next color
										{
											if ((game_data.player_control[g] == PLAYER_CONTROL_NONE || game_data.player_control[g] == PLAYER_CONTROL_CLOSED) && player_color_map[g] > e && (f == -1 || player_color_map[g] < player_color_map[f]) )
												f = g;
										}
										if (f == -1)
										{
											for (int g = 0; g < TA3D_PLAYERS_HARD_LIMIT; ++g)
											{
												if ((game_data.player_control[g] == PLAYER_CONTROL_NONE || game_data.player_control[g] == PLAYER_CONTROL_CLOSED) && (f == -1 || player_color_map[g] < player_color_map[f]) )
													f = g;
											}
										}
										if (f != -1)
										{
											byte g = player_color_map[f];
											player_color_map[i] = g;                                // update game data
											player_color_map[f] = e;

											guiobj =  setupgame_area.get_object( String("gamesetup.color") << i);
											if (guiobj)
												guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3],player_color[player_color_map[i]*3+1],player_color[player_color_map[i]*3+2]);            // Update gui
											guiobj =  setupgame_area.get_object( String("gamesetup.color") << f);
											if (guiobj)
												guiobj->Data = gfx->makeintcol(player_color[player_color_map[f]*3],player_color[player_color_map[f]*3+1],player_color[player_color_map[f]*3+2]);            // Update gui
										}
										network_manager.sendSpecial("NOTIFY UPDATE");
									}
								}
							}
							else if (params[0] == "SET")
							{
								if (params[1] == "FOW")
								{
									int value = params[2].to<sint32>();
									Gui::GUIOBJ::Ptr obj = setupgame_area.get_object( "gamesetup.FOW");
									if (obj && value >= 0 && value < 4)
									{
										obj->Value = value;
										obj->Text[0] = obj->Text[1 + obj->Value];
										game_data.fog_of_war = uint8(obj->Value);
									}
								}
								else if (params[1] == "MAP")
								{
									set_map = UnfixBlank( params[2] );
									if (set_map != game_data.map_filename )
									{
										if (!previous_tnt_port.empty() )
											network_manager.stopFileTransfer( previous_tnt_port);
										if (!previous_ota_port.empty())
											network_manager.stopFileTransfer(previous_ota_port);
										previous_ota_port.empty();
										previous_tnt_port.empty();
										String new_map_name = TA3D::Paths::Files::ReplaceExtension(set_map,".tnt");
										if (client && !VFS::Instance()->fileExists( new_map_name ))
										{
											String sMpN(new_map_name);
											sMpN.replace('\\', '/');
											previous_tnt_port = network_manager.getFile( 1, sMpN);
											network_manager.sendSpecial( "REQUEST FILE " + FixBlank(new_map_name) + " " + previous_tnt_port );
										}

										new_map_name = TA3D::Paths::Files::ReplaceExtension(new_map_name,".ota");

										if (client && !VFS::Instance()->fileExists( new_map_name ))
										{
											String sMpN(new_map_name);
											sMpN.replace('\\', '/');

											previous_ota_port = network_manager.getFile( 1, sMpN);
											network_manager.sendSpecial( "REQUEST FILE " + FixBlank(new_map_name) + " " + previous_ota_port );
										}
									}
								}
								else if (params[1] == "SCRIPT")
								{
									String script_name = UnfixBlank( params[2] );
									if (script_name != game_data.game_script)
									{
										setupgame_area.caption( "gamesetup.script_name", script_name);
										game_data.game_script = script_name;

										if (client && !VFS::Instance()->fileExists( script_name ))
										{
											if (!previous_lua_port.empty())
												network_manager.stopFileTransfer( previous_lua_port);

											String sSpS(script_name);
											sSpS.replace('\\', '/');

											previous_lua_port = network_manager.getFile( 1, sSpS);
											network_manager.sendSpecial("REQUEST FILE " + FixBlank(script_name) + " " + previous_lua_port);
										}
									}
								}
							}
						}
						else if (params.size() == 4)
						{
							if (params[0] == "REQUEST") // REQUEST FILE filename port
							{
								if (params[1] == "FILE")
								{
									String file_name = UnfixBlank( params[2] );
									LOG_DEBUG(LOG_PREFIX_NET << "received file request : '" << file_name << "'");
									network_manager.stopFileTransfer( params[3], from);
									network_manager.sendFile( from, file_name, params[3]);
								}
							}
							else if (params[0] == "CHANGE")
							{
								if (params[1] == "TEAM")
								{
									int i = params[2].to<sint32>();
									int n_team = params[3].to<sint32>();
									if (i >= 0 && i < TA3D_PLAYERS_HARD_LIMIT && (client || from == game_data.player_network_id[i])) // Server doesn't accept someone telling him what to do
									{
										Gui::GUIOBJ::Ptr guiobj = setupgame_area.get_object( String("gamesetup.team") << i );
										if (guiobj)
										{
											guiobj->current_state = byte(n_team);
											game_data.team[i] = short(1 << n_team);
										}
									}
								}
							}
							else if (params[0] == "SET")
							{
								if (params[1] == "UNIT" && params[2] == "LIMIT")
								{
									game_data.max_unit_per_player = params[3].to<sint32>();
									Gui::GUIOBJ::Ptr obj = setupgame_area.get_object("gamesetup.max_units");
									if (obj)
										obj->Text[0] = String() << game_data.max_unit_per_player;
								}
							}
						}
						else if (params.size() == 9)
						{
							if (params[0] == "PLAYER_INFO") // We've received player information, let's update :)
							{
								int i = params[1].to<sint32>();
								int n_id = params[2].to<sint32>();
								if (i >= 0 && i < TA3D_PLAYERS_HARD_LIMIT && (client || from == n_id)) // Server doesn't accept someone telling him what to do
								{
									int side_id  = params[3].to<sint32>();
									int metal_q  = params[5].to<sint32>();
									int energy_q = params[6].to<sint32>();
									bool ready   = params[8].to<sint32>();
									game_data.player_network_id[i] = n_id;
									game_data.player_sides[i] = side_str[ side_id ];
									game_data.ai_level[i] = UnfixBlank( params[4] );
									game_data.metal[i] = metal_q;
									game_data.energy[i] = energy_q;
									game_data.player_names[i] = UnfixBlank( params[7] );
									game_data.ready[i] = ready;
									if (n_id < 0 && game_data.ai_level[i].size() >= 4)
										game_data.player_control[i] = PLAYER_CONTROL_REMOTE_AI;     // AIs are on the server, no need to replicate them
									else if (n_id < 0 && game_data.ai_level[i].size() < 4)
										game_data.player_control[i] = PLAYER_CONTROL_NONE;
									else
										game_data.player_control[i] = (n_id == my_player_id) ? PLAYER_CONTROL_LOCAL_HUMAN : PLAYER_CONTROL_REMOTE_HUMAN;

									setupgame_area.caption( String("gamesetup.name") << i, game_data.player_names[i]);                                 // Update gui
									AI_list[0] = (game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI) ? game_data.ai_level[i] : String();
									setupgame_area.set_entry( String("gamesetup.ai") << i, AI_list);
									setupgame_area.caption( String("gamesetup.side") << i, side_str[side_id]);                         // Update gui
									setupgame_area.caption( String("gamesetup.energy") << i, String() << game_data.energy[i]);         // Update gui
									setupgame_area.caption( String("gamesetup.metal") << i, String() << game_data.metal[i]);               // Update gui
									setupgame_area.set_state( String("gamesetup.ready") << i, ready);                                           // Update gui

									Gui::GUIOBJ::Ptr guiobj =  setupgame_area.get_object( String("gamesetup.color") << i);
									if (guiobj)
									{
										guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3],player_color[player_color_map[i]*3+1],player_color[player_color_map[i]*3+2]);            // Update gui
										if (game_data.player_control[i] == PLAYER_CONTROL_NONE || game_data.player_control[i] == PLAYER_CONTROL_CLOSED )
											guiobj->Flag |= FLAG_HIDDEN;
										else
											guiobj->Flag &= ~FLAG_HIDDEN;
									}
									if (!client)
										network_manager.sendSpecial("NOTIFY UPDATE", from);
								}
								else
									LOG_ERROR("Packet error : " << received_special_msg.message);
							}
						}
						else if (params.size() == 11)
						{
							if (params[0] == "PLAYERCOLORMAP")
							{
								for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
								{
									player_color_map[i] = byte(params[i + 1].to<sint32>());
									Gui::GUIOBJ::Ptr guiobj =  setupgame_area.get_object( String("gamesetup.color") << i);
									if (guiobj)
										guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3],player_color[player_color_map[i]*3+1],player_color[player_color_map[i]*3+2]);            // Update gui
								}
							}
						}

				if (network_manager.getNextSpecial(&received_special_msg) == 0)
					special_msg = (char*)received_special_msg.message;
				else
					special_msg.clear();
			}

			//-------------------------------------------------------------- Network Code : chat system --------------------------------------------------------------

			while (!chat_msg.empty()) // Chat receiver
			{
				Gui::GUIOBJ::Ptr chat_list = setupgame_area.get_object("gamesetup.chat_list");
				if (chat_list)
				{
					chat_list->Text.push_back(chat_msg);
					if (chat_list->Text.size() > 5)
						chat_list->Data++;
					chat_list->Pos = uint32(chat_list->Text.size() - 1);
				}

				if (network_manager.getNextChat( &received_chat_msg ) == 0 )
					chat_msg = (char*)received_chat_msg.message;
				else
					chat_msg.clear();
			}

			//-------------------------------------------------------------- Network Code : advert system --------------------------------------------------------------

			while (!broadcast_msg.empty())  // Broadcast message receiver
			{
				String::Vector params;
				broadcast_msg.explode(params, ' ', true, false, true);
				if (params.size() == 3 && params[0] == "PING" && params[1] == "SERVER")
				{
					if (params[2] == "LIST" && host.notEmpty()) // Sending information about this server
					{
						uint16 nb_open = 0;
						for (int f = 0; f < TA3D_PLAYERS_HARD_LIMIT; ++f)
						{
							if (setupgame_area.caption(String("gamesetup.name") << f) == player_str[2])
								++nb_open;
						}

						String hostFixed(host);
						hostFixed.replace(' ', char(1));

						String engineV(TA3D_ENGINE_VERSION);
						engineV.replace(' ', char(1));
						if (TA3D_CURRENT_MOD.empty())
						{
							network_manager.broadcastMessage(String("PONG SERVER ") << hostFixed << " . " << engineV << " " << nb_open);
						}
						else
						{
							String mod(TA3D_CURRENT_MOD);
							mod.replace(' ', char(1));
							network_manager.broadcastMessage(String("PONG SERVER ") << hostFixed << " " << mod << " " << engineV << " " << nb_open);
						}
					}
				}
				broadcast_msg = network_manager.getNextBroadcastedMessage();
			}

			//-------------------------------------------------------------- End of Network Code --------------------------------------------------------------

			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;

			if (key[KEY_ENTER] && !setupgame_area.caption("gamesetup.t_chat").empty())
			{
				String message;
				message << "<" << lp_CONFIG->player_name << "> " << setupgame_area.caption("gamesetup.t_chat");
				if (host.notEmpty())
				{
					struct chat msg;
					network_manager.sendChat( strtochat( &msg, message ));
				}
				Gui::GUIOBJ::Ptr chat_list = setupgame_area.get_object("gamesetup.chat_list");

				if (chat_list)
				{
					chat_list->Text.push_back( message);
					if (chat_list->Text.size() > 5)
						chat_list->Data++;
					chat_list->Pos = uint32(chat_list->Text.size() - 1);
				}

				setupgame_area.caption("gamesetup.t_chat", "");
			}

			if (setupgame_area.get_value("gamesetup.FOW") >= 0 && !client && !saved_game)
			{
				Gui::GUIOBJ::Ptr obj = setupgame_area.get_object( "gamesetup.FOW");
				if (obj && obj->Value != -1)
				{
					obj->Text[0] = obj->Text[1 + obj->Value];
					game_data.fog_of_war = uint8(obj->Value);
					if (host.notEmpty())
						network_manager.sendSpecial(String("SET FOW ") << obj->Value);
				}
			}

			if (client || saved_game.notEmpty())
				setupgame_area.msg("scripts.hide"); // Hide the scripts window in client mode

			if (setupgame_area.get_state( "scripts.b_ok" ) && !client && !saved_game)
			{
				guiobj = setupgame_area.get_object( "scripts.script_list");
				if (guiobj && guiobj->Pos < guiobj->num_entries())
				{
					setupgame_area.caption( "gamesetup.script_name", guiobj->Text[ guiobj->Pos ]);
					game_data.game_script = guiobj->Text[ guiobj->Pos ];
					if (host.notEmpty())
						network_manager.sendSpecial("SET SCRIPT " + FixBlank(guiobj->Text[guiobj->Pos]));
				}
			}

			if (setupgame_area.get_state( "gamesetup.b_ok" ) && !client)
			{
				bool ready = true;
				if (host.notEmpty())
				{
					for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
					{
						if (game_data.player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN || game_data.player_control[i] == PLAYER_CONTROL_REMOTE_HUMAN )
							ready &= game_data.ready[i];
					}
				}

				if (ready)
				{
					while (key[KEY_ENTER])
					{
						SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
						poll_inputs();
					}
					clear_keybuf();
					done = true;      // If user click "OK" or hit enter then leave the window
					start_game = true;
					network_manager.sendSpecial("NOTIFY START");
				}
			}
			if (setupgame_area.get_state("gamesetup.b_cancel"))
			{
				LOG_DEBUG("leaving game room");
				done = true;      // En cas de click sur "retour", on quitte la fenêtre
			}

			if (!saved_game && setupgame_area.get_value("gamesetup.max_units") >= 0 && !client)
			{
				Gui::GUIOBJ::Ptr obj = setupgame_area.get_object("gamesetup.max_units");
				obj->Text[0] = obj->Text[1+obj->Value];
				game_data.max_unit_per_player = obj->Text[0].to<sint32>();
				network_manager.sendSpecial(String("SET UNIT LIMIT ") << game_data.max_unit_per_player);
			}

			for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
			{
				if (setupgame_area.get_state( String("gamesetup.ready") << i) != game_data.ready[i])
				{
					if (game_data.player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN && !saved_game)
					{
						network_manager.sendSpecial( "NOTIFY UPDATE");
						game_data.ready[i] = !game_data.ready[i];
					}
					else
						setupgame_area.set_state( String("gamesetup.ready") << i, game_data.ready[i]);
				}
				if (saved_game.notEmpty()) continue;            // We mustn't change any thing for a saved game
				guiobj = setupgame_area.get_object( String("gamesetup.team") << i );
				if (guiobj != NULL && (1 << guiobj->current_state) != game_data.team[i])           // Change team
				{
					if ( ((!client && !(game_data.player_control[i] & PLAYER_CONTROL_FLAG_REMOTE)) || (client && game_data.player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN))
						&& game_data.player_control[i] != PLAYER_CONTROL_NONE && game_data.player_control[i] != PLAYER_CONTROL_CLOSED)
					{
						network_manager.sendSpecial( "NOTIFY UPDATE");
						game_data.team[i] = short(1 << guiobj->current_state);
					}
					else
						guiobj->current_state = byte(Math::Log2(game_data.team[i]));
				}
				if (client && game_data.player_network_id[i] != my_player_id )
					continue;                           // You cannot change other player's settings
				if (setupgame_area.get_state(String("gamesetup.b_name") << i) && !client ) // Change player type
				{
					if (game_data.player_network_id[i] >= 0 && game_data.player_network_id[i] != my_player_id ) // Kick player !!
					{
						network_manager.dropPlayer( game_data.player_network_id[i]);
						network_manager.sendSpecial( "NOTIFY UPDATE");
					}
					int e = 0;
					for (int f = 0; f < player_str_n; ++f)
					{
						if (setupgame_area.caption(String("gamesetup.name") << i) == player_str[f])
						{
							e = f;
							break;
						}
					}
					e = (e + 1) % player_str_n;

					if (player_control[e] == PLAYER_CONTROL_LOCAL_HUMAN)// We can only have one local human player ( or it crashes )
					{
						for (int f = 0; f < TA3D_PLAYERS_HARD_LIMIT; ++f)
						{
							if (f!= i && game_data.player_control[f] == PLAYER_CONTROL_LOCAL_HUMAN) // If we already have a local human player pass this player type value
							{
								e = (e + 1) % player_str_n;
								break;
							}
						}
					}

					game_data.player_names[i] = player_str[e];                              // Update game data
					game_data.player_control[i] = player_control[e];
					if (player_control[e] == PLAYER_CONTROL_LOCAL_HUMAN )
						game_data.player_network_id[i] = my_player_id;
					else
						game_data.player_network_id[i] = -1;

					setupgame_area.caption( String("gamesetup.name") << i, player_str[e]);         // Update gui
					AI_list[0] = (game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI) ? game_data.ai_level[i] : String();
					setupgame_area.set_entry( String("gamesetup.ai") << i, AI_list);
					guiobj = setupgame_area.get_object( String("gamesetup.color") << i);
					if (guiobj)
					{
						if (player_control[e] == PLAYER_CONTROL_NONE || player_control[e] == PLAYER_CONTROL_CLOSED)
							guiobj->Flag |= FLAG_HIDDEN;
						else
							guiobj->Flag &= ~FLAG_HIDDEN;
					}
					if (host.notEmpty())
						network_manager.sendSpecial( "NOTIFY UPDATE");
				}
				if (setupgame_area.get_value( String("gamesetup.side") << i) >= 0) // Change player side
				{
					int pos = setupgame_area.get_value( String("gamesetup.side") << i) + 1;
					Gui::GUIOBJ::Ptr guiobj = setupgame_area.get_object(String("gamesetup.side") << i);
					guiobj->Text[0] = guiobj->Text[pos];

					game_data.player_sides[i] = side_str[pos - 1];                                // update game data
					if (host.notEmpty())
						network_manager.sendSpecial( "NOTIFY UPDATE");
				}
				if (!(game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI))
					setupgame_area.set_state(String("gamesetup.ai") << i, false);
				else if (setupgame_area.get_value( String("gamesetup.ai") << i ) >= 0 ) // Change player level (for AI)
				{
					int pos = setupgame_area.get_value( String("gamesetup.ai") << i ) + 1;
					if (pos >= 1 && pos < (int)AI_list.size())
					{
						String AIlevel = AI_list[pos];
						AI_list[0] = (game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI) ? AIlevel : String("");
						setupgame_area.set_entry( String("gamesetup.ai") << i, AI_list);          // Update gui

						game_data.ai_level[i] = AIlevel;                              // update game data
						if (host.notEmpty())
							network_manager.sendSpecial("NOTIFY UPDATE");
					}
				}
				if (setupgame_area.get_state( String("gamesetup.b_color") << i)) // Change player color
				{
					if (client)
						network_manager.sendSpecial(String("NOTIFY COLORCHANGE ") << i);
					byte e = player_color_map[i];
					int f = -1;
					for (int g = 0; g < TA3D_PLAYERS_HARD_LIMIT; ++g) // Look for the next color
					{
						if ((game_data.player_control[g] == PLAYER_CONTROL_NONE || game_data.player_control[g] == PLAYER_CONTROL_CLOSED)
							&& player_color_map[g] > e && (f == -1 || player_color_map[g] < player_color_map[f]))
						{
							f = g;
						}
					}
					if (f == -1)
					{
						for (int g = 0; g < TA3D_PLAYERS_HARD_LIMIT; ++g)
						{
							if ((game_data.player_control[g] == PLAYER_CONTROL_NONE || game_data.player_control[g] == PLAYER_CONTROL_CLOSED) && (f == -1 || player_color_map[g] < player_color_map[f]))
								f = g;
						}
					}
					if (f != -1)
					{
						byte g = player_color_map[f];
						player_color_map[i] = g;                                // update game data
						player_color_map[f] = e;

						guiobj =  setupgame_area.get_object( String("gamesetup.color") << i);
						if (guiobj )
							guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3],player_color[player_color_map[i]*3+1],player_color[player_color_map[i]*3+2]);            // Update gui
						guiobj =  setupgame_area.get_object( String("gamesetup.color") << f);
						if (guiobj )
							guiobj->Data = gfx->makeintcol(player_color[player_color_map[f]*3],player_color[player_color_map[f]*3+1],player_color[player_color_map[f]*3+2]);            // Update gui
					}
					if (host.notEmpty() && !client)
						network_manager.sendSpecial( "NOTIFY UPDATE");
				}
				if (setupgame_area.get_state( String("gamesetup.b_energy") << i ) ) // Change player energy stock
				{
					game_data.energy[i] = (game_data.energy[i] + 500) % 10500;
					if (game_data.energy[i] == 0 ) game_data.energy[i] = 500;

					setupgame_area.caption( String("gamesetup.energy") << i, String() << game_data.energy[i]);         // Update gui
					if (host.notEmpty())
						network_manager.sendSpecial( "NOTIFY UPDATE");
				}
				if (setupgame_area.get_state( String("gamesetup.b_metal") << i ) ) // Change player metal stock
				{
					game_data.metal[i] = (game_data.metal[i] + 500) % 10500;
					if (game_data.metal[i] == 0 ) game_data.metal[i] = 500;

					setupgame_area.caption( String("gamesetup.metal") << i, String() << game_data.metal[i]);           // Update gui
					if (host.notEmpty())
						network_manager.sendSpecial( "NOTIFY UPDATE");
				}
			}

			if (setupgame_area.get_state("gamesetup.b_units") && !client && !saved_game) // Select available units
			{
				Menus::UnitSelector::Execute(game_data.use_only, game_data.use_only);       // Change unit selection
			}

			if (minimap_obj != NULL &&
				( ( ( setupgame_area.get_state( "gamesetup.minimap" ) || setupgame_area.get_state("gamesetup.change_map")) && !client)
				  || ( client && !set_map.empty() ) ) && !saved_game) // Clic on the mini-map or received map set command
			{
				String map_filename;
				String new_map;
				if (!client)
				{
					setupgame_area.caption("popup.msg", I18N::Translate("Loading maps, please wait ..."));       // Show a small popup displaying a wait message
					setupgame_area.title("popup", I18N::Translate("Please wait ..."));
					setupgame_area.msg("popup.show");
					gfx->set_2D_mode();
					setupgame_area.draw();

					glEnable(GL_TEXTURE_2D);
					gfx->set_color(0xFFFFFFFF);
					draw_cursor();

					// Affiche / Show the buffer
					gfx->flip();
					gfx->unset_2D_mode();

					String newMapName;
					Menus::MapSelector::Execute(game_data.map_filename, newMapName);
					new_map = newMapName;
					for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
						player_timer[i] = msec_timer;
					setupgame_area.msg("popup.hide");
				}
				else
					new_map = set_map;

				gfx->SCREEN_W_TO_640 = 1.0f;                // To have mouse sensibility undependent from the resolution
				gfx->SCREEN_H_TO_480 = 1.0f;
				cursor_type=CURSOR_DEFAULT;
				gfx->set_2D_mode();

				if (!new_map.empty() && (set_map.empty() || (client && VFS::Instance()->fileExists( Paths::Files::ReplaceExtension(new_map, ".tnt"))
					&& VFS::Instance()->fileExists( Paths::Files::ReplaceExtension(new_map, ".ota")))))
				{
					set_map.clear();
					if (host.notEmpty() && !client)
					{
						String tmp(new_map);
						tmp.replace(' ', char(1));
						network_manager.sendSpecial("SET MAP " + tmp);
					}

					String new_map_name = new_map;

					gfx->destroy_texture( glimg);

					game_data.map_filename = new_map;
					glimg = load_tnt_minimap_fast(game_data.map_filename,dx,dy);
					ldx = float(dx) * ( mini_map_x2 - mini_map_x1 ) / 504.0f;
					ldy = float(dy) * ( mini_map_y2 - mini_map_y1 ) / 504.0f;
					minimap_obj->x1 = mini_map_x - ldx;
					minimap_obj->y1 = mini_map_y - ldy;
					minimap_obj->x2 = mini_map_x + ldx;
					minimap_obj->y2 = mini_map_y + ldy;
					minimap_obj->u2 = float(dx) / 252.0f;
					minimap_obj->v2 = float(dy) / 252.0f;

					map_data.destroy();
					map_data.load(Paths::Files::ReplaceExtension(game_data.map_filename, ".ota"));
					String map_info;
					if (!map_data.missionname.empty())
						map_info << map_data.missionname << "\n";
					if (!map_data.numplayers.empty())
						map_info << map_data.numplayers << "\n";
					if (!map_data.missiondescription.empty())
						map_info << map_data.missiondescription;
					setupgame_area.caption("gamesetup.map_info", map_info);
				}

				minimap_obj->Data = glimg;      // Synchronize the picture on GUI
			}

			if (key[KEY_ESC])
			{
				LOG_DEBUG("leaving game room");
				done = true;
			}

			setupgame_area.draw();

			glEnable(GL_TEXTURE_2D);
			gfx->set_color(0xFFFFFFFF);
			draw_cursor();

			// Affiche
			gfx->flip();

		} while (!done);

		if (!previous_lua_port.empty() && network_manager.isConnected())
			TA3D::network_manager.stopFileTransfer(previous_lua_port);
		if (!previous_ota_port.empty() && network_manager.isConnected())
			TA3D::network_manager.stopFileTransfer(previous_ota_port);
		if (!previous_tnt_port.empty() && network_manager.isConnected())
			TA3D::network_manager.stopFileTransfer(previous_tnt_port);

		if (setupgame_area.background == gfx->glfond )  setupgame_area.background = 0;
		setupgame_area.destroy();

		map_data.destroy();

		gfx->destroy_texture(glimg);

		reset_mouse();
		while (key[KEY_ESC])
		{
			SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}

		if (start_game)
		{
			if (!game_data.map_filename.empty() && !game_data.game_script.empty())
			{
				lp_CONFIG->last_script = game_data.game_script;     // Remember the last script we played
				lp_CONFIG->last_map = game_data.map_filename;       // Remember the last map we played
				lp_CONFIG->last_FOW = game_data.fog_of_war;

				if (!saved_game)            // For a saved game, we already have everything set
				{
					game_data.nb_players = 0;
					for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i) // Move players to the top of the vector, so it's easier to access data
					{
						if (game_data.player_control[i] != PLAYER_CONTROL_NONE && game_data.player_control[i] != PLAYER_CONTROL_CLOSED)
						{
							if (i != game_data.nb_players)
							{
								game_data.team[game_data.nb_players] = game_data.team[i];
								game_data.player_control[game_data.nb_players] = game_data.player_control[i];
								game_data.player_names[game_data.nb_players] = game_data.player_names[i];
								game_data.player_sides[game_data.nb_players] = game_data.player_sides[i];
								game_data.ai_level[game_data.nb_players] = game_data.ai_level[i];
								game_data.energy[game_data.nb_players] = game_data.energy[i];
								game_data.metal[game_data.nb_players] = game_data.metal[i];
								byte e = player_color_map[game_data.nb_players];
								player_color_map[game_data.nb_players] = player_color_map[i];
								player_color_map[i] = e;
							}
							game_data.nb_players++;
						}
					}
				}

				Battle::Execute(&game_data);
			}

			while (key[KEY_ESC])
			{
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
		}
		else
		{
			if (client)
				network_manager.sendSpecial( "NOTIFY PLAYER_LEFT");
		}
		network_manager.Disconnect();
	}



	/*-----------------------------------------------------------------------------\
	  |                            void network_room(void)                           |
	  |                                                                              |
	  |    Displays the list of available servers and allow to join/host a game      |
	  \-----------------------------------------------------------------------------*/

#define SERVER_LIST_REFRESH_DELAY   5000

	void network_room(void)             // Let players create/join a game
	{
		network_manager.InitBroadcast(1234);      // broadcast mode

		int server_list_timer = msec_timer - SERVER_LIST_REFRESH_DELAY;

		std::list< SERVER_DATA >    servers;                    // the server list

		Gui::AREA networkgame_area("network game area");
		networkgame_area.load_tdf("gui/networkgame.area");
		if (!networkgame_area.background)
			networkgame_area.background = gfx->glfond;

		String sel_index;
		String o_sel;

		bool done=false;

		int amx = -1;
		int amy = -1;
		int amz = -1;
		int amb = -1;

		String join_host;

		do
		{
			bool key_is_pressed = false;
			do
			{
				key_is_pressed = msec_timer - server_list_timer >= SERVER_LIST_REFRESH_DELAY;
				networkgame_area.check();
				key_is_pressed |= networkgame_area.key_pressed;
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			} while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ]
				&& !key[ KEY_ESC ] && !done && !key_is_pressed && !networkgame_area.scrolling && !network_manager.BroadcastedMessages());

			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;

			if (network_manager.BroadcastedMessages())
			{
				String msg = network_manager.getNextBroadcastedMessage();
				while (!msg.empty())
				{
					String::Vector params;
					msg.explode(params, ' ', true, false, true);
					if (params.size() == 6 && params[0] == "PONG" && params[1] == "SERVER") // It looks like "PONG SERVER <name> <mod> <version> <nb open player slots>
					{
						String name = UnfixBlank(params[2]);
						String mod = UnfixBlank(params[3]);
						if (mod == ".")
							mod.clear();
						String version = UnfixBlank(params[4]);
						String host_address = network_manager.getLastMessageAddress();
						int nb_open = params[5].to<sint32>();

						if (version.substr(0, 3) == String(TA3D_ENGINE_VERSION).substr(0, 3) && version.size() == String(TA3D_ENGINE_VERSION).size() && mod == TA3D_CURRENT_MOD && nb_open != 0)
						{
							bool updated = false;
							for (std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ; server_i++ )       // Update the list
							{
								if (server_i->name == name)
								{
									updated = true;
									server_i->timer = msec_timer;
									server_i->nb_open = nb_open;
									server_i->host = host_address;
									if (name == o_sel)
										o_sel += "_";
									break;
								}
							}
							if (!updated)
							{
								SERVER_DATA new_server;
								new_server.internet = false;
								new_server.name = name;
								new_server.timer = msec_timer;
								new_server.nb_open = nb_open;
								new_server.host = host_address;
								servers.push_back( new_server);
							}
						}
					}

					msg = network_manager.getNextBroadcastedMessage();
				}

				for (std::list< SERVER_DATA >::iterator server_i = servers.begin(); server_i != servers.end() ;) // Remove those who timeout
					if (!server_i->internet && msec_timer - server_i->timer >= 30000)
						servers.erase(server_i++);
					else
						server_i++;

				Gui::GUIOBJ::Ptr obj = networkgame_area.get_object("networkgame.server_list");
				if (obj)
				{
					obj->Text.resize( servers.size());
					String::List server_names;
					for (std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ; server_i++ )       // Remove those who timeout
						server_names.push_back( server_i->name);
					server_names.sort();
					int i = 0;
					// Remove those who timeout
					for (String::List::const_iterator server_i = server_names.begin(); server_i != server_names.end(); ++server_i, ++i)
						obj->Text[i] = *server_i;
					if (obj->Text.size() == 0 )
						obj->Text.push_back(I18N::Translate("No server found"));
				}
			}

			bool refresh_all = false;
			if (networkgame_area.get_state("networkgame.b_refresh"))
			{
				refresh_all = true;
				servers.clear();
			}

			if (msec_timer - server_list_timer >= SERVER_LIST_REFRESH_DELAY || refresh_all) // Refresh server list
			{
				for( std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ;)      // Remove those who timeout
				{
					if (!server_i->internet && msec_timer - server_i->timer >= 30000)
						servers.erase( server_i++);
					else
						server_i++;
				}

				Gui::GUIOBJ::Ptr obj = networkgame_area.get_object("networkgame.server_list");
				if (obj)
				{
					obj->Text.resize(servers.size());
					String::List server_names;
					for (std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ; server_i++ )       // Remove those who timeout
						server_names.push_back( server_i->name);
					server_names.sort();
					int i = 0;
					for (String::List::const_iterator server_i = server_names.begin(); server_i != server_names.end(); ++server_i, ++i) // Remove those who timeout
						obj->Text[i] = *server_i;
					if (obj->Text.size() == 0)
						obj->Text.push_back(I18N::Translate("No server found"));
				}

				server_list_timer = msec_timer;
				if (network_manager.broadcastMessage( "PING SERVER LIST" ) )
					printf("error : could not broadcast packet to refresh server list!!\n");
			}

			if (networkgame_area.get_state( "networkgame.b_load"))
			{
				Gui::GUIOBJ::Ptr obj = networkgame_area.get_object("load_menu.l_file");
				if (obj)
				{
					String::List fileList;
					Paths::Glob(fileList, TA3D::Paths::Savegames + "multiplayer" + Paths::Separator + "*.sav");
					fileList.sort();
					obj->Text.clear();
					obj->Text.reserve(fileList.size());
					for (String::List::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
					{
						// Remove the Savegames path, leaving just the bare file names
						obj->Text.push_back(Paths::ExtractFileName(*i));
					}
				}
				else
				{
					LOG_ERROR("Impossible to get an area object : `load_menu.l_file`");
				}
			}

			if (networkgame_area.get_state( "load_menu.b_load" ) || (key[KEY_ENTER] && networkgame_area.get_state( "load_menu" )))    // Loading a game
			{
				Gui::GUIOBJ::Ptr obj = networkgame_area.get_object("load_menu.l_file");
				if (obj && obj->Pos < obj->Text.size())
				{
					GameData game_data;
					String host = obj->Text[obj->Pos];
					bool network = load_game_data(TA3D::Paths::Savegames + "multiplayer" + Paths::Separator + obj->Text[obj->Pos], &game_data);

					if (!game_data.saved_file.empty() && network)
					{
						while (key[KEY_ENTER])
						{
							SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
							poll_inputs();
						}
						clear_keybuf();
						network_manager.Disconnect();

						setup_game(false, host, game_data.saved_file);   // Host a game
						done = true;
					}
				}
			}

			if (networkgame_area.get_state( "hosting.b_ok" ) || ( key[KEY_ENTER] && networkgame_area.get_state( "hosting")))
			{
				while (key[KEY_ENTER])
				{
					SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
					poll_inputs();
				}
				clear_keybuf();
				network_manager.Disconnect();
				String host = networkgame_area.caption( "hosting.t_hostname");

				setup_game(false, host);   // Host a game
				done = true;
			}

			if (networkgame_area.get_state( "joinip.b_ok"))
			{
				join_host = networkgame_area.caption( "joinip.t_hostname");
				done = true;
			}

			if (networkgame_area.get_state( "networkgame.b_join"))
			{
				while (key[KEY_ENTER])
				{
					SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
					poll_inputs();
				}
				clear_keybuf();
				done = true;      // If user click "OK" or hit enter then leave the window
				std::list< SERVER_DATA >::iterator i_server = servers.begin();
				while (i_server != servers.end() && i_server->name != sel_index)
					++i_server;

				if (i_server != servers.end() )     // Server not found !!
					join_host = i_server->host;
				else
					join_host.clear();
			}

			if (networkgame_area.get_state( "networkgame.b_cancel" ) || key[KEY_ESC])
			{
				while (key[KEY_ESC])
				{
					SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
					poll_inputs();
				}
				clear_keybuf();
				done = true;      // If user click "Cancel" or hit ESC then leave the screen returning NULL
				sel_index.clear();
			}

			if (networkgame_area.get_object("networkgame.server_list") && !done )
			{
				Gui::GUIOBJ::Ptr obj = networkgame_area.get_object("networkgame.server_list");
				sel_index = (obj->Pos < obj->Text.size()) ? obj->Text[ obj->Pos ] : String("");

				if (sel_index != o_sel) // Update displayed server info
				{
					o_sel = sel_index;
					std::list< SERVER_DATA >::iterator i_server = servers.begin();
					while( i_server != servers.end() && i_server->name != sel_index )
						++i_server;

					if (i_server != servers.end())
					{
						networkgame_area.caption("networkgame.server_name", i_server->name);
						networkgame_area.caption("networkgame.host", i_server->host);
						networkgame_area.caption("networkgame.open_slots", String() << i_server->nb_open );
					}
				}
			}

			networkgame_area.draw();

			glEnable(GL_TEXTURE_2D);
			gfx->set_color(0xFFFFFFFF);
			draw_cursor();

			// Affiche
			gfx->flip();
		}while(!done);

		if (networkgame_area.background == gfx->glfond )    networkgame_area.background = 0;
		networkgame_area.destroy();

		reset_mouse();
		while (key[KEY_ESC])
		{
			SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}

		network_manager.Disconnect();

		if (!join_host.empty()) // Join a game
		{
			setup_game( true, join_host);
			gfx->set_2D_mode();
			gfx->ReInitTexSys();
		}
	}





	void campaign_main_menu(void)
	{
		cursor_type=CURSOR_DEFAULT;

		Gui::AREA campaign_area("campaign");
		campaign_area.load_tdf("gui/campaign.area");
		if (!campaign_area.background)
			campaign_area.background = gfx->glfond;

		String::List campaign_list;
		VFS::Instance()->getFilelist("camps\\*.tdf", campaign_list);
		campaign_list.sort();
		for (String::List::iterator i = campaign_list.begin(); i != campaign_list.end(); ) // Removes sub directories entries
		{
			if (SearchString(i->substr(6, i->size() - 6), "/", true) != -1 || SearchString(i->substr(6, i->size() - 6), "\\", true ) != -1)
				campaign_list.erase(i++);
			else
				++i;
		}

		if (campaign_area.get_object("campaign.campaign_list") && campaign_list.size() > 0)
		{
			Gui::GUIOBJ::Ptr guiobj = campaign_area.get_object("campaign.campaign_list");
			guiobj->Text.clear();
			guiobj->Text.resize( campaign_list.size());
			int n = 0;
			for (String::List::const_iterator i = campaign_list.begin(); i != campaign_list.end(); ++i, ++n)
				guiobj->Text[n] = i->substr(6, i->size() - 10);
		}

		Gaf::AnimationList side_logos;
		File *file = VFS::Instance()->readFile( "anims\\newgame.gaf");
		side_logos.loadGAFFromRawData(file, true);
		delete file;

		TDFParser::Ptr campaign_parser = NULL;

		bool start_game = false;

		int amx = -1;
		int amy = -1;
		int amz = -1;
		int amb = -1;
		uint32 last_campaign_id = uint32(-1);
		String  campaign_name;
		int mission_id = -1;
		int nb_mission = 0;
		bool done = false;

		do
		{
			bool key_is_pressed = false;
			do
			{
				campaign_area.check();
				key_is_pressed = campaign_area.key_pressed;
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			} while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b
					 && mouse_b == 0 && !key[ KEY_ENTER ] && !key[ KEY_ESC ]
					 && !done && !key_is_pressed && !campaign_area.scrolling);

			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;

			if (campaign_area.get_object("campaign.campaign_list") && campaign_list.size() > 0) // If we don't have campaign data, then load them
			{
				Gui::GUIOBJ::Ptr guiobj = campaign_area.get_object("campaign.campaign_list");
				if (guiobj->Pos < guiobj->Text.size() && last_campaign_id != guiobj->Pos )
				{
					last_campaign_id = guiobj->Pos;
					mission_id = -1;
					campaign_name = "camps\\" + guiobj->Text[ guiobj->Pos ] + ".tdf";
					campaign_parser = new TDFParser( campaign_name);

					guiobj = campaign_area.get_object("campaign.mission_list");
					nb_mission = 0;
					if (guiobj)
					{
						guiobj->Text.clear();
						int i = 0;
						String current_name;
						while (!(current_name = campaign_parser->pullAsString(String("MISSION") << i << ".missionname")).empty())
						{
							guiobj->Text.push_back( current_name);
							++nb_mission;
							++i;
						}
					}

					guiobj = campaign_area.get_object("campaign.logo");
					if (guiobj)
					{
						guiobj->Data = 0;
						for (int i = 0 ; i < ta3dSideData.nb_side ; ++i)
						{
							if (String::ToLower(ta3dSideData.side_name[i] ) == String::ToLower(campaign_parser->pullAsString("HEADER.campaignside")))
							{
								if (side_logos.size() > i)
									guiobj->Data = side_logos[i].glbmp[0];
								break;
							}
						}
					}
				}
			}

			if (campaign_area.get_object("campaign.mission_list"))
			{
				Gui::GUIOBJ::Ptr guiobj = campaign_area.get_object("campaign.mission_list");
				if (guiobj->Pos < guiobj->Text.size())
					mission_id = guiobj->Pos;
			}

			if ((campaign_area.get_state( "campaign.b_ok") || key[KEY_ENTER]) && campaign_list.size())
			{
				while (key[KEY_ENTER])
				{
					SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
					poll_inputs();
				}
				clear_keybuf();
				done = true;      // If user click "OK" or hit enter then leave the window
				start_game = true;
			}
			if (campaign_area.get_state( "campaign.b_cancel")) // En cas de click sur "retour", on quitte la fenêtre
				done = true;

			if (key[KEY_ESC]) // Quitte si on appuie sur echap
				done = true;

			campaign_area.draw();

			glEnable(GL_TEXTURE_2D);
			gfx->set_color(0xFFFFFFFF);
			draw_cursor();

			// Affiche
			gfx->flip();
		} while(!done);

		if (campaign_area.get_object("campaign.logo"))
			campaign_area.get_object("campaign.logo")->Data = 0;

		if (campaign_area.background == gfx->glfond)
			campaign_area.background = 0;
		campaign_area.destroy();

		campaign_parser = NULL;

		reset_mouse();
		while (key[KEY_ESC])
		{
			SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}

		if (start_game) // Open the briefing screen and start playing the campaign
		{
			// The result of the last battle
			Battle::Result exitStatus = Battle::brUnknown;

			while (mission_id < nb_mission && (exitStatus = brief_screen(campaign_name, mission_id)))
			{
				if (Battle::brVictory == exitStatus)
					++mission_id;
			}

			while (key[KEY_ESC])
			{
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
		}
	}






	Battle::Result brief_screen(String campaign_name, int mission_id)
	{
		cursor_type = CURSOR_DEFAULT;

		Gui::AREA brief_area("brief");
		brief_area.load_tdf("gui/brief.area");
		if (!brief_area.background)
			brief_area.background = gfx->glfond;

		TDFParser brief_parser(campaign_name);           // Loads the campaign file

		String map_filename;
		map_filename << "maps\\" << brief_parser.pullAsString(String("MISSION") << mission_id << ".missionfile");
		TDFParser ota_parser(map_filename);

		String narration_file;
		narration_file << "camps\\briefs\\" << ota_parser.pullAsString("GlobalHeader.narration" ) << ".wav"; // The narration file
		String language_suffix = (lp_CONFIG->Lang == "english") ? nullptr : ("-" + lp_CONFIG->Lang);
		String brief_file;
		brief_file << "camps\\briefs" << language_suffix << "\\" << ota_parser.pullAsString("GlobalHeader.brief") << ".txt"; // The brief file

		{
			if (!VFS::Instance()->fileExists( brief_file ) )         // try without the .txt
				brief_file = "camps\\briefs" + language_suffix + "\\" + ota_parser.pullAsString( "GlobalHeader.brief");
			if (!VFS::Instance()->fileExists( brief_file ) )         // try without the suffix if we cannot find it
				brief_file = "camps\\briefs\\" + ota_parser.pullAsString( "GlobalHeader.brief" ) + ".txt";
			if (!VFS::Instance()->fileExists( brief_file ) )         // try without the suffix if we cannot find it
				brief_file = "camps\\briefs\\" + ota_parser.pullAsString( "GlobalHeader.brief");
			File *file = VFS::Instance()->readFile(brief_file);
			if (file)
			{
				String brief_info = (const char*)file->data();
				brief_info.toUTF8();
				brief_area.caption( "brief.info", brief_info);
				delete file;
			}
		}

		Gaf::AnimationList planet_animation;
		String planet_file = ota_parser.pullAsString("GlobalHeader.planet");
		planet_file.toLower();

		if (planet_file == "green planet" )             planet_file = "anims\\greenbrief.gaf";
		else if (planet_file == "archipelago" )         planet_file = "anims\\archibrief.gaf";
		else if (planet_file == "desert" )              planet_file = "anims\\desertbrief.gaf";
		else if (planet_file == "lava" )                planet_file = "anims\\lavabrief.gaf";
		else if (planet_file == "wet desert" )          planet_file = "anims\\wdesertbrief.gaf";
		else if (planet_file == "metal" )               planet_file = "anims\\metalbrief.gaf";
		else if (planet_file == "red planet" )          planet_file = "anims\\marsbrief.gaf";
		else if (planet_file == "lunar" )               planet_file = "anims\\lunarbrief.gaf";
		else if (planet_file == "lush" )                planet_file = "anims\\lushbrief.gaf";
		else if (planet_file == "ice" )                 planet_file = "anims\\icebrief.gaf";
		else if (planet_file == "slate" )               planet_file = "anims\\slatebrief.gaf";
		else if (planet_file == "water world" )         planet_file = "anims\\waterbrief.gaf";

		File *file = VFS::Instance()->readFile(planet_file);
		if (file)
		{
			planet_animation.loadGAFFromRawData(file, true);
			delete file;
		}

		int schema = 0;

		if (brief_area.get_object("brief.schema"))
		{
			Gui::GUIOBJ::Ptr obj = brief_area.get_object("brief.schema");
			obj->Text.clear();
			obj->Text.resize(ota_parser.pullAsInt("GlobalHeader.SCHEMACOUNT") + 1);
			for (unsigned int i = 0 ; i < obj->Text.size() - 1; ++i)
				obj->Text[i + 1] = I18N::Translate(ota_parser.pullAsString(String("GlobalHeader.Schema ") << i << ".Type"));
			if (obj->Text.size() > 1)
				obj->Text[0] = obj->Text[1];
		}

		sound_manager->playSoundFileNow(narration_file);

		bool done=false;

		bool start_game = false;

		int amx = -1;
		int amy = -1;
		int amz = -1;
		int amb = -1;
		float planet_frame = 0.0f;

		int pan_id = 0;
		int rotate_id = 0;
		for (int i = 0; i < planet_animation.size(); ++i)
		{
			if (String::ToLower(String(planet_animation[i].name).substr(String(planet_animation[i].name).size() - 3, 3)) == "pan")
				pan_id = i;
			else
				if (String::ToLower(String(planet_animation[i].name).substr(String(planet_animation[i].name).size() - 6, 6)) == "rotate")
					rotate_id = i;
		}

		float pan_x1 = 0.0f;
		float pan_x2 = 0.0f;

		if (brief_area.get_object("brief.panning0"))
		{
			pan_x1 = brief_area.get_object("brief.panning0")->x1;
			pan_x2 = brief_area.get_object("brief.panning0")->x2;
		}

		int time_ref = msec_timer;

		do
		{
			bool key_is_pressed = false;
			do
			{
				brief_area.check();
				key_is_pressed = brief_area.key_pressed;
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			} while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ]
					 && !key[ KEY_ESC ] && !done && !key_is_pressed && !brief_area.scrolling && (int)planet_frame == (int)(float(msec_timer - time_ref) * 0.01f));

			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;

			planet_frame = float(msec_timer - time_ref) * 0.01f;

			if (brief_area.get_value("brief.schema") >= 0)
			{
				Gui::GUIOBJ::Ptr obj = brief_area.get_object( "brief.schema");
				if (obj->Value != -1)
				{
					obj->Text[0] = obj->Text[obj->Value + 1];
					schema = obj->Value;
				}
			}

			if (brief_area.get_state("brief.info"))
				brief_area.get_object("brief.info")->Pos++;

			if (brief_area.get_object( "brief.planet"))
			{
				Gui::GUIOBJ::Ptr guiobj = brief_area.get_object("brief.planet");
				if (guiobj)
					guiobj->Data = planet_animation[rotate_id].glbmp[((int)planet_frame) % planet_animation[rotate_id].nb_bmp];
			}

			float pan_frame = planet_frame / (pan_x2 - pan_x1);

			if (brief_area.get_object("brief.panning0"))
			{
				Gui::GUIOBJ::Ptr guiobj = brief_area.get_object("brief.panning0");
				if (guiobj)
				{
					guiobj->Data = planet_animation[pan_id].glbmp[ ((int)pan_frame) % planet_animation[pan_id].nb_bmp ];
					guiobj->u1 = Math::Modf(pan_frame);
					guiobj->x2 = pan_x2 + (pan_x1 - pan_x2) * guiobj->u1;
				}
			}

			if (brief_area.get_object( "brief.panning1"))
			{
				Gui::GUIOBJ::Ptr guiobj = brief_area.get_object("brief.panning1");
				if (guiobj)
				{
					guiobj->Data = planet_animation[pan_id].glbmp[ ((int)pan_frame + 1) % planet_animation[pan_id].nb_bmp ];
					guiobj->u2 = Math::Modf(pan_frame);
					guiobj->x1 = pan_x2 + (pan_x1 - pan_x2) * guiobj->u2;
				}
			}

			if (brief_area.get_state( "brief.b_ok" ) || key[KEY_ENTER])
			{
				while (key[KEY_ENTER])
				{
					SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
					poll_inputs();
				}
				clear_keybuf();
				done=true;      // If user click "OK" or hit enter then leave the window
				start_game = true;
			}
			if (brief_area.get_state( "brief.b_cancel"))
				done=true;       // En cas de click sur "retour", on quitte la fenêtre

			if (key[KEY_ESC])
				done=true;         // Quitte si on appuie sur echap

			brief_area.draw();

			glEnable(GL_TEXTURE_2D);
			gfx->set_color(0xFFFFFFFF);
			draw_cursor();

			// Affiche
			gfx->flip();
		}while(!done);

		if (brief_area.get_object( "brief.planet"))     brief_area.get_object( "brief.planet" )->Data = 0;
		if (brief_area.get_object( "brief.panning0"))   brief_area.get_object( "brief.panning0" )->Data = 0;
		if (brief_area.get_object( "brief.panning1"))   brief_area.get_object( "brief.panning1" )->Data = 0;

		if (brief_area.background == gfx->glfond)
			brief_area.background = 0;
		brief_area.destroy();

		sound_manager->stopSoundFileNow();

		reset_mouse();
		while (key[KEY_ESC])
		{
			SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}

		if (start_game)  // Open the briefing screen and start playing the campaign
		{
			GameData game_data;

			// Generate the script which will be removed later
			TA3D::Paths::MakeDir(TA3D::Paths::Resources + "scripts" + Paths::SeparatorAsString + "game");
			TA3D::generate_script_from_mission(TA3D::Paths::Resources + "scripts" + Paths::SeparatorAsString + "game" + Paths::SeparatorAsString + "__campaign_script.lua", ota_parser, schema);

			game_data.game_script = "scripts/game/__campaign_script.lua";
			game_data.map_filename = map_filename.substr( 0, map_filename.size() - 3 ) + "tnt";     // Remember the last map we played
			game_data.fog_of_war = FOW_ALL;

			game_data.nb_players = ota_parser.pullAsInt("GlobalHeader.numplayers", 2);
			if (game_data.nb_players == 0) // Yes it can happen !!
				game_data.nb_players = 2;

			game_data.player_control[0] = PLAYER_CONTROL_LOCAL_HUMAN;
			game_data.player_names[0] = brief_parser.pullAsString( "HEADER.campaignside");
			game_data.player_sides[0] = brief_parser.pullAsString( "HEADER.campaignside");
			game_data.energy[0] = ota_parser.pullAsInt( String("GlobalHeader.Schema ") << schema << ".humanenergy" );
			game_data.metal[0] = ota_parser.pullAsInt( String("GlobalHeader.Schema ") << schema << ".humanmetal" );

			String schema_type = ota_parser.pullAsString(String("GlobalHeader.Schema ") << schema << ".Type").toLower();

			if (schema_type == "easy")
				game_data.ai_level[ 0 ] = "[C] EASY";
			else
			{
				if (schema_type == "medium")
					game_data.ai_level[ 0 ] = "[C] MEDIUM";
				else
					if (schema_type == "hard")
						game_data.ai_level[ 0 ] = "[C] HARD";
			}

			player_color_map[0] = 0;

			for (short int i = 1; i < game_data.nb_players; ++i)
			{
				game_data.player_control[ i ] = PLAYER_CONTROL_LOCAL_AI;
				game_data.player_names[ i ] = brief_parser.pullAsString( "HEADER.campaignside");
				game_data.player_sides[ i ] = brief_parser.pullAsString( "HEADER.campaignside");            // Has no meaning here since we are in campaign mode units are spawned by a script
				game_data.ai_level[ i ] = game_data.ai_level[ 0 ];
				game_data.energy[ i ] = ota_parser.pullAsInt( String("GlobalHeader.Schema ") << schema << ".computerenergy" );
				game_data.metal[ i ] = ota_parser.pullAsInt( String("GlobalHeader.Schema ") << schema << ".computermetal" );

				player_color_map[ i ] = byte(i);
			}

			game_data.campaign = true;
			game_data.use_only = ota_parser.pullAsString( "GlobalHeader.useonlyunits");
			if (!game_data.use_only.empty())
				game_data.use_only = "camps\\useonly\\" + game_data.use_only;

			Battle::Result exit_mode = Battle::Execute(&game_data);

			while (key[KEY_ESC])
			{
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}

			return exit_mode;
		}
		return Battle::brUnknown;
	}

	void wait_room(void *p_game_data)
	{
		GameData *game_data = (GameData*) p_game_data;
		if (!network_manager.isConnected())
			return;

		gfx->set_2D_mode();

		gfx->ReInitTexSys();

		cursor_type=CURSOR_DEFAULT;

		Gui::AREA wait_area("wait");
		wait_area.load_tdf("gui/wait.area");
		if (!wait_area.background)
			wait_area.background = gfx->glfond;

		bool dead_player[TA3D_PLAYERS_HARD_LIMIT];
		uint32 player_timer[TA3D_PLAYERS_HARD_LIMIT];

		for (int i = game_data->nb_players; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
		{
			dead_player[i] = true;
			wait_area.msg(String("wait.name") << i << ".hide");
			wait_area.msg(String("wait.progress") << i << ".hide");
			wait_area.msg(String("wait.ready") << i << ".hide");
		}

		for (int i = 0; i < game_data->nb_players; ++i)
		{
			player_timer[i] = msec_timer;
			dead_player[i] = false;
			if ((game_data->player_control[i] & PLAYER_CONTROL_FLAG_AI) || game_data->player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN )
			{
				wait_area.set_data( String("wait.progress") << i, 100);
				wait_area.set_state( String("wait.ready") << i, true);
			}
			else
				wait_area.set_state( String("wait.ready") << i, false);
			wait_area.caption(String("wait.name") << i, game_data->player_names[i]);
		}

		bool done = false;

		int amx = -1;
		int amy = -1;
		int amz = -1;
		int amb = -1;

		if (network_manager.isServer())
		{                                   // We can only use units available on all clients
			for (int i = 0; i < unit_manager.nb_unit; ++i)
			{
				if (!unit_manager.unit_type[i]->not_used)           // Small check to ensure useonly file has an effect :)
					network_manager.sendAll("USING " + unit_manager.unit_type[i]->Unitname);
			}
			network_manager.sendAll("END USING");           // Ok we've finished sending the available unit list
			network_manager.sendAll("READY");               // Only server can tell he is ready before entering main loop
		}
		else
			for (int i = 0; i < unit_manager.nb_unit; ++i)      // Clients disable all units and wait for server to enable the ones we're going to use
				unit_manager.unit_type[i]->not_used = true;

		int ping_timer = msec_timer;                    // Used to send simple PING requests in order to detect when a connection fails

		do
		{
			bool key_is_pressed = false;
			String special_msg;
			struct chat received_special_msg;
			bool playerDropped = false;
			do
			{
				playerDropped = network_manager.getPlayerDropped();
				if (network_manager.getNextSpecial( &received_special_msg ) == 0)
                    special_msg = (char*)received_special_msg.message;
				if (!network_manager.isConnected()) // We're disconnected !!
				{
					done = true;
					break;
				}
				wait_area.check();
				key_is_pressed = wait_area.key_pressed;
				SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			} while( amx == mouse_x && amy == mouse_y && amz == mouse_z && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done
					 && !key_is_pressed && !wait_area.scrolling && special_msg.empty() && !playerDropped
					 && ( msec_timer - ping_timer < 2000 || !network_manager.isServer() ));

			//-------------------------------------------------------------- Network Code : syncing information --------------------------------------------------------------

			bool check_ready = false;

			if (network_manager.isServer() && msec_timer - ping_timer >= 2000) // Send a ping request
			{
				network_manager.sendPing();
				ping_timer = msec_timer;
				check_ready = true;

				if (network_manager.isServer())
				{
					for (short int i = 0; i < game_data->nb_players; ++i) // Ping time out
					{
						if (game_data->player_network_id[i] > 0 && msec_timer - player_timer[i] > 10000)
							network_manager.dropPlayer(game_data->player_network_id[i]);
					}
				}
			}

			if (playerDropped)
			{
				for (int i = 0 ; i < game_data->nb_players ; i++)
					if (game_data->player_network_id[i] > 0 && !network_manager.pollPlayer(game_data->player_network_id[i]))     // A player is disconnected
					{
						dead_player[i] = true;
						wait_area.msg(String("wait.name") << i << ".hide");
						wait_area.msg(String("wait.progress") << i << ".hide");
						wait_area.msg(String("wait.ready") << i << ".hide");
					}
			}

			while (!special_msg.empty())    // Special receiver (sync config data)
			{
				int from = received_special_msg.from;
				int player_id = game_data->net2id(from);
				String::Vector params;
                String((char*)received_special_msg.message).explode(params, ' ');
				if (params.size() == 1)
				{
					if (params[0] == "PONG" )
					{
						if (player_id >= 0 )
							player_timer[player_id] = msec_timer;
					}
					else
					{
						if (params[0] == "NOT_READY")
							wait_area.set_state(String("wait.ready") << player_id, false);
						else
						{
							if (params[0] == "READY")
							{
								wait_area.set_data(String("wait.progress") << player_id, 100);
								wait_area.set_state(String("wait.ready") << player_id, true);
								check_ready = true;
							}
							else
							{
								if (params[0] == "START")
									done = true;
							}
						}
					}
				}
				else
				{
					if (params.size() == 2)
					{
						if (params[0] == "LOADING")
						{
							int percent = Math::Min(100, Math::Max(0, params[1].to<int>()));
							wait_area.set_data( String("wait.progress") << player_id, percent);
						}
						else
						{
							if (params[0] == "USING")
							{                                   // We can only use units available on all clients, so check the list
								int type_id = unit_manager.get_unit_index(params[1]);
								if (type_id == -1)            // Tell it's missing
									network_manager.sendAll( "MISSING " + params[1]);
								else
									unit_manager.unit_type[type_id]->not_used = false;  // Enable this unit
							}
							else
							{
								if (params[0] == "END" && params[1] == "USING")
									network_manager.sendAll("READY");
								else
									if (params[0] == "MISSING")
									{
										int idx = unit_manager.get_unit_index(params[1]);
										if (idx >= 0)
											unit_manager.unit_type[idx]->not_used = true;
									}
							}
						}
					}
				}

				if (network_manager.getNextSpecial(&received_special_msg) == 0)
					special_msg = received_special_msg.message;
				else
					special_msg.clear();
			}

			if (network_manager.isServer() && check_ready)// If server is late the game should begin once he is there
			{
				bool ready = true;
				for (short int i = 0; i < game_data->nb_players && ready; ++i)
				{
					if (!wait_area.get_state(String("wait.ready") << i) && !dead_player[i] && game_data->player_network_id[i] > 0)
						ready = false;
				}

				if (ready)
				{
					network_manager.sendAll("START");           // Tell everyone to start the game!!
					rest(1);
					done = true;
				}
			}

			//-------------------------------------------------------------- End of Network Code --------------------------------------------------------------

			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;

			//      if (key[KEY_ESC]) done=true;         // Quitte si on appuie sur echap

			wait_area.draw();

			glEnable(GL_TEXTURE_2D);
			gfx->set_color(0xFFFFFFFF);
			draw_cursor();

			// Affiche
			gfx->flip();

		} while (!done);

		lp_CONFIG->timefactor = 1.0f;       // We can't afford running a network game at more than 1x !! (at least at the beginning, and not knowing if there are internet players)

		if (wait_area.background == gfx->glfond)
			wait_area.background = 0;
		wait_area.destroy();

		gfx->unset_2D_mode();
		reset_mouse();
		while (key[KEY_ESC])
		{
			SleepMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
	}


} // namespace TA3D
