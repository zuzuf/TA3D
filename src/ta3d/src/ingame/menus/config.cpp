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

#include "config.h"
#include <input/keyboard.h>
#include <input/mouse.h>
#include <TA3D_NameSpace.h>
#include <mods/mods.h>
#include <misc/paths.h>
#include <cache.h>
#include <sounds/manager.h>
#include <ingame/sidedata.h>
#include <misc/settings.h>

namespace TA3D
{
namespace Menus
{

	bool Config::Execute()
	{
		Config m;
		return m.execute();
	}

	Config::Config()
		:Abstract()
	{
		saved_config = new TA3DCONFIG;
	}


	Config::~Config()
	{
		delete saved_config;
	}


	void Config::doFinalize()
	{
		// Wait for user to release ESC
		reset_mouse();
		while (key[KEY_ESC])
		{
			SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		clear_keybuf();

		bool ask_for_quickrestart = lp_CONFIG->quickrestart;

		if (!save)
			*lp_CONFIG = *saved_config;
		else
		{
			sound_manager->savePlaylist();              // Save the playlist

			// Check we're not trying to crash the game :P
			gfx->checkConfig();

			if (lp_CONFIG->screen_width != saved_config->screen_width ||
				lp_CONFIG->screen_height != saved_config->screen_height ||
				lp_CONFIG->color_depth != saved_config->color_depth ||
				lp_CONFIG->fsaa != saved_config->fsaa ||
				(lp_CONFIG->fullscreen != saved_config->fullscreen) )            // Need to restart
			{
				lp_CONFIG->quickrestart = true;
			}

			lp_CONFIG->player_name = pArea->caption("*.player_name");

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

			if ((lp_CONFIG->developerMode ^ saved_config->developerMode)
				|| lp_CONFIG->unitTextureQuality != saved_config->unitTextureQuality)
				Cache::Clear(true); // Force cache reset
		}

		grab_mouse(lp_CONFIG->grab_inputs);
		sound_manager->setVolume(lp_CONFIG->sound_volume);
		sound_manager->setMusicVolume(lp_CONFIG->music_volume);

		lp_CONFIG->quickrestart |= ask_for_quickrestart;

		pArea->destroy();

		I18N::CurrentLanguage(lp_CONFIG->Lang);

		TA3D::Settings::Save();             // Keep settings :)
	}


	bool Config::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MULTIMENU << "Entering...");

		// Loading the area
		loadAreaFromTDF("config", "gui/config.area");

		if (lp_CONFIG->restorestart)
		{
			lp_CONFIG->restorestart = false;
			lp_CONFIG->quickstart = false;
		}

		*saved_config = *lp_CONFIG;

		if (pArea->get_object("*.fps_limit") )
		{
			fps_limits = pArea->get_object("*.fps_limit")->Text;
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

		nb_res = 0;

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
			for (unsigned int i = 0; mode_list[i] != NULL; ++i)
			{
				// Reference to the current SDL Rect
				const SDL_Rect& rect = *(mode_list[i]);

				if (rect.w >= 640 && rect.h >= 480)
				{
					# ifndef YUNI_OS_MAC
					if(SDL_VideoModeOK(rect.w, rect.h, 16, SDL_FULLSCREEN | SDL_OPENGL) == 16)
					{
						res_bpp[nb_res]    = 16;
						res_width[nb_res ] = rect.w;
						res_height[nb_res] = rect.h;
						++nb_res;
					}
					# endif
					if (SDL_VideoModeOK(rect.w, rect.h, 32, SDL_FULLSCREEN | SDL_OPENGL) == 32)
					{
						res_bpp[nb_res]    = 32;
						res_width[ nb_res] = rect.w;
						res_height[nb_res] = rect.h;
						++nb_res;
					}
				}
			}
		}

		pArea->set_state("*.showfps", lp_CONFIG->showfps);
		pArea->caption("*.fps_limit", fps_limits[fps_limits.size()-1]);
		for (QStringList::const_iterator i = fps_limits.begin(); i != fps_limits.end(); ++i)
		{
			if ( (QString() << (int)lp_CONFIG->fps_limit) == *i )
				pArea->caption("*.fps_limit", *i);
		}
		pArea->set_value("*.texture_quality", lp_CONFIG->unitTextureQuality);
		pArea->set_value("*.interface_transparency", int(lp_CONFIG->menuTransparency * 255.0f));
		pArea->set_value("*.shadow_map_size", lp_CONFIG->shadowmap_size);
		pArea->set_state("*.grab_inputs", lp_CONFIG->grab_inputs);
		pArea->set_value("*.sound_volume", lp_CONFIG->sound_volume);
		pArea->set_value("*.music_volume", lp_CONFIG->music_volume);
		pArea->set_state("*.far_sight", lp_CONFIG->far_sight);
		pArea->set_value("*.anisotropy", lp_CONFIG->anisotropy);
		pArea->set_value("*.mouse_sensitivity", (int)(lp_CONFIG->mouse_sensivity * 100.0f));
		pArea->set_state("*.disable_perspective", lp_CONFIG->ortho_camera);
		pArea->set_state("*.right_click_interface", lp_CONFIG->right_click_interface);
		pArea->set_state("*.disable_GLSL", lp_CONFIG->disable_GLSL);
		pArea->set_state("*.underwater_bright", lp_CONFIG->underwater_bright);
		pArea->set_state("*.use_texture_compression", lp_CONFIG->use_texture_compression);
		pArea->set_state("*.low_definition_map", lp_CONFIG->low_definition_map);
		pArea->set_state("*.sky", lp_CONFIG->render_sky);
		pArea->set_state("*.particle", lp_CONFIG->particle);
		pArea->set_state("*.explosion_particles", lp_CONFIG->explosion_particles);
		pArea->set_state("*.waves", lp_CONFIG->waves);
		pArea->set_state("*.height_line", lp_CONFIG->height_line);
		pArea->set_state("*.detail_tex", lp_CONFIG->detail_tex);
		pArea->set_state("*.use_texture_cache", lp_CONFIG->use_texture_cache);
		pArea->set_state("*.draw_console_loading", lp_CONFIG->draw_console_loading);
		pArea->set_state("*.fullscreen", lp_CONFIG->fullscreen);
		pArea->set_state("*.developer_mode", lp_CONFIG->developerMode);
		pArea->set_state("*.tool_tips", lp_CONFIG->tooltips);
		I18N::Instance()->retrieveAllLanguages(languageList);
		if (pArea->get_object("*.LANG"))
		{
			Gui::GUIOBJ::Ptr objLang = pArea->get_object("*.LANG");
			objLang->Text.clear();
			I18N::Language *l = I18N::Instance()->language(lp_CONFIG->Lang);
			if (l)
				objLang->Text.push_back(l->caption());
			else
				objLang->Text.push_back(lp_CONFIG->Lang);
			for (unsigned int i = 0; i < languageList.size(); i++)
				objLang->Text.push_back(languageList[i].caption());
		}
		if (pArea->get_object("*.camera_zoom") )
			pArea->caption( "*.camera_zoom", pArea->get_object("*.camera_zoom")->Text[1+lp_CONFIG->camera_zoom]);
		pArea->caption( "*.camera_def_angle", QString() << lp_CONFIG->camera_def_angle );
		pArea->caption( "*.camera_def_h", QString() << lp_CONFIG->camera_def_h );
		pArea->caption( "*.camera_zoom_speed", QString() << lp_CONFIG->camera_zoom_speed );
		if (pArea->get_object("*.screenres") )
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.screenres");
			obj->Text.clear();
			int current = 0;
			while( current < nb_res &&
				   ( res_width[ current ] != lp_CONFIG->screen_width
					 || res_height[ current ] != lp_CONFIG->screen_height
					 || res_bpp[ current ] != lp_CONFIG->color_depth ) )
				current++;
			if (current >= nb_res ) current = 0;
			obj->Text.push_back( QString() << res_width[ current ] << "x" << res_height[ current ] << "x" << res_bpp[ current ] );
			for( int i = 0 ; i < nb_res ; i++ )
				obj->Text.push_back( QString() << res_width[ i ] << "x" << res_height[ i ] << "x" << res_bpp[ i ] );
		}
		Gui::GUIOBJ::Ptr tmpO = pArea->get_object("*.shadow_quality");
		if (tmpO)
		{
			const unsigned int indx = 1 + Math::Max(0, Math::Min((int)lp_CONFIG->shadow_quality, 3));
			if (indx < tmpO->Text.size())
				pArea->caption("*.shadow_quality", tmpO->Text[indx]);
		}

		pArea->caption("*.timefactor", QString() << (int)lp_CONFIG->timefactor);
		switch( lp_CONFIG->fsaa )
		{
			case 2: pArea->caption("*.fsaa", "x2");    break;
			case 4: pArea->caption("*.fsaa", "x4");    break;
			case 6: pArea->caption("*.fsaa", "x6");    break;
			case 8: pArea->caption("*.fsaa", "x8");    break;
			default: pArea->caption("*.fsaa", I18N::Translate("no fsaa"));
		}
		if (pArea->get_object("*.water_quality"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.water_quality");
			pArea->caption("*.water_quality", obj->Text[1 + lp_CONFIG->water_quality]);
		}

		if (pArea->get_object("*.mod"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.mod");

			if (obj->Text.size() >= 2)
				obj->Text[0] = obj->Text[1];
			else
				obj->Text.resize(1);

			QString current_selection = TA3D_CURRENT_MOD.length() > 6 ? Substr(TA3D_CURRENT_MOD, 5, TA3D_CURRENT_MOD.length() - 6 ) : "";
			QStringList mod_list = Mods::instance()->getModNameList(Mods::MOD_INSTALLED);
			mod_list.sort();
			mod_list.unique();
			for (QStringList::iterator i = mod_list.begin() ; i != mod_list.end() ; ++i)
			{
				obj->Text.push_back( *i );
				if (ToLower( *i ) == ToLower(current_selection))
					obj->Text[0] = *i;
			}
		}

		pArea->caption("*.player_name", lp_CONFIG->player_name);

		if (pArea->get_object("*.skin"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.skin");

			obj->Text.resize(1);
			obj->Text[0] = I18N::Translate( "default.skn");

			QStringList skin_list;
			VFS::Instance()->getFilelist("gui\\*.skn", skin_list);

			QString skin_name;
			const QStringList::iterator end = skin_list.end();
			for (QStringList::iterator i = skin_list.begin(); i != end; ++i)
			{
				skin_name = Paths::ExtractFileName(*i, false);
				obj->Text.push_back(skin_name);
				if (QString("gui/") << ToLower(skin_name) == ToLower(lp_CONFIG->skin_name))
					obj->Text[0] = skin_name;
			}
		}

		if (pArea->get_object("*.l_files"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.l_files");
			sound_manager->getPlayListFiles(obj->Text);
		}

		if (lp_CONFIG->quickstart)
			I_Msg( TA3D::TA3D_IM_GUI_MSG, "config_confirm.show");

		save = false;
		timer = msec_timer;

		return true;
	}



	void Config::waitForEvent()
	{
		time_out = false;
		do
		{
			// Grab user events
			pArea->check();

			// Wait to reduce CPU consumption
			wait();

			if (lp_CONFIG->quickstart)
			{
				Gui::GUIOBJ::Ptr pbar = pArea->get_object( "config_confirm.p_wait");
				if (pbar)
				{
					const uint32 new_value = (msec_timer - timer) / 50;
					if (new_value != pbar->Data)
					{
						pbar->Data = new_value;
						if (new_value >= 100)
							time_out = true;
						break;
					}
				}
			}
		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
				 && mouse_b == 0
				 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
				 && !pArea->key_pressed && !pArea->scrolling);
	}


	bool Config::maySwitchToAnotherMenu()
	{
		if (lp_CONFIG->quickstart)
		{
			if (time_out || pArea->get_state("config_confirm.b_cancel_changes" ) || key[KEY_ESC])
			{
				I_Msg( TA3D::TA3D_IM_GUI_MSG, "config_confirm.hide");
				TA3D::Settings::Restore(TA3D::Paths::ConfigFile);
				TA3D::Settings::Load();
				save = false;
				lp_CONFIG->quickstart = false;
				lp_CONFIG->quickrestart = true;
				lp_CONFIG->restorestart = true;
				*saved_config = *lp_CONFIG;
				return true;
			}
			else
				if (pArea->get_state("config_confirm.b_confirm"))
				{
					I_Msg( TA3D::TA3D_IM_GUI_MSG, "config_confirm.hide");
					lp_CONFIG->quickstart = false;
					saved_config->quickstart = false;
					TA3D::Settings::Save();             // Keep settings :)
				}
		}

		if (pArea->get_state( "*.b_activate"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.l_files");
			if (obj && obj->Text.size() > obj->Pos)
			{
				sound_manager->setPlayListFileMode( obj->Pos, false, false);
				obj->Text[ obj->Pos ][ 1 ] = '*';
			}
		}
		if (pArea->get_state( "*.b_deactivate"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.l_files");
			if (obj && obj->Text.size() > obj->Pos)
			{
				sound_manager->setPlayListFileMode( obj->Pos, false, true);
				obj->Text[ obj->Pos ][ 1 ] = ' ';
			}
		}
		if (pArea->get_state( "*.b_battle" ) )
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.l_files");
			if (obj && obj->Text.size() > obj->Pos)
			{
				sound_manager->setPlayListFileMode(obj->Pos, true, false);
				obj->Text[ obj->Pos ][1] = 'B';
			}
		}


		if (pArea->get_state("*.b_ok"))
		{
			save = true;
			return true;        // On "OK", leave the menu
		}
		if (pArea->get_state( "*.b_cancel" ) )      // On "cancel", leave
			return true;

		lp_CONFIG->showfps = pArea->get_state( "*.showfps");
		if (pArea->get_value("*.fps_limit") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.fps_limit");
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

		if (lp_CONFIG->grab_inputs != pArea->get_state("*.grab_inputs"))
		{
			lp_CONFIG->grab_inputs = pArea->get_state("*.grab_inputs");
			grab_mouse(lp_CONFIG->grab_inputs);
		}
		if (lp_CONFIG->sound_volume != pArea->get_value("*.sound_volume"))
		{
			lp_CONFIG->sound_volume = pArea->get_value("*.sound_volume");
			sound_manager->setVolume(lp_CONFIG->sound_volume);
		}
		if (lp_CONFIG->music_volume != pArea->get_value("*.music_volume"))
		{
			lp_CONFIG->music_volume = pArea->get_value("*.music_volume");
			sound_manager->setMusicVolume(lp_CONFIG->music_volume);
		}
		lp_CONFIG->unitTextureQuality = pArea->get_value("*.texture_quality");
		lp_CONFIG->menuTransparency = float(pArea->get_value("*.interface_transparency")) / 255.0f;
		lp_CONFIG->shadowmap_size = uint8(pArea->get_value("*.shadow_map_size"));
		lp_CONFIG->far_sight = pArea->get_state("*.far_sight");
		lp_CONFIG->anisotropy = sint16(pArea->get_value("*.anisotropy"));
		lp_CONFIG->mouse_sensivity = float(pArea->get_value("*.mouse_sensitivity")) * 0.01f;
		lp_CONFIG->ortho_camera = pArea->get_state("*.disable_perspective");
		lp_CONFIG->right_click_interface = pArea->get_state("*.right_click_interface");
		lp_CONFIG->disable_GLSL = pArea->get_state("*.disable_GLSL");
		lp_CONFIG->underwater_bright = pArea->get_state("*.underwater_bright");
		lp_CONFIG->use_texture_compression = pArea->get_state("*.use_texture_compression");
		lp_CONFIG->low_definition_map = pArea->get_state("*.low_definition_map");
		lp_CONFIG->render_sky = pArea->get_state( "*.sky");
		lp_CONFIG->particle = pArea->get_state( "*.particle");
		lp_CONFIG->explosion_particles = pArea->get_state( "*.explosion_particles");
		lp_CONFIG->waves = pArea->get_state( "*.waves");
		lp_CONFIG->height_line = pArea->get_state( "*.height_line");
		lp_CONFIG->detail_tex = pArea->get_state( "*.detail_tex");
		lp_CONFIG->draw_console_loading = pArea->get_state( "*.draw_console_loading");
		lp_CONFIG->fullscreen = pArea->get_state( "*.fullscreen");
		lp_CONFIG->use_texture_cache = pArea->get_state( "*.use_texture_cache");
		lp_CONFIG->developerMode = pArea->get_state("*.developer_mode");
		lp_CONFIG->tooltips = pArea->get_state("*.tool_tips");
		if (pArea->get_value( "*.camera_zoom" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.camera_zoom");
			if (obj && obj->Value >= -1)
			{
				obj->Text[0] = obj->Text[1 + obj->Value];
				lp_CONFIG->camera_zoom = uint8(obj->Value);
			}
		}
		if (pArea->get_value( "*.camera_def_angle" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.camera_def_angle");
			if (obj && obj->Value >= 0)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value ];
				lp_CONFIG->camera_def_angle = obj->Text[0].toFloat();
			}
		}
		if (pArea->get_value( "*.camera_def_h" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.camera_def_h");
			if (obj && obj->Value >= 0)
			{
				obj->Text[0] = obj->Text[1 + obj->Value];
				lp_CONFIG->camera_def_h = obj->Text[0].toFloat();
			}
		}
		if (pArea->get_value( "*.camera_zoom_speed" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.camera_zoom_speed");
			if (obj && obj->Value >= 0)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value ];
				lp_CONFIG->camera_zoom_speed = obj->Text[0].toFloat();
			}
		}
		if (pArea->get_value( "*.LANG" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.LANG");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[1 + obj->Value];
				lp_CONFIG->Lang = languageList[obj->Value].englishCaption();
			}
		}
		if (pArea->get_value("*.screenres") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.screenres");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value ];
				lp_CONFIG->screen_width = uint16(res_width[ obj->Value ]);
				lp_CONFIG->screen_height = uint16(res_height[ obj->Value ]);
				lp_CONFIG->color_depth = uint8(res_bpp[ obj->Value ]);
			}
		}
		if (pArea->get_value("*.shadow_quality") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.shadow_quality");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[1 + obj->Value];
				lp_CONFIG->shadow_quality = sint16(obj->Value);
			}
		}
		if (pArea->get_value("*.timefactor") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.timefactor");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value ];
				lp_CONFIG->timefactor = float(obj->Value + 1);
			}
		}
		if (pArea->get_value( "*.fsaa" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.fsaa");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value ];
				lp_CONFIG->fsaa = sint16(obj->Value << 1);
			}
		}
		if (pArea->get_value("*.water_quality") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("*.water_quality");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value ];
				lp_CONFIG->water_quality = sint16(obj->Value);
			}
		}
		if (pArea->get_value("*.mod") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.mod");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[ 1 + obj->Value];
				lp_CONFIG->last_MOD = obj->Value > 0 ? QString("mods/") << obj->Text[0] << '/' : QString();
			}
		}
		if (pArea->get_value( "*.skin" ) >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "*.skin");
			if (obj && obj->Value != -1)
			{
				obj->Text[0] = obj->Text[1 + obj->Value];
				lp_CONFIG->skin_name = obj->Value >= 0 ? QString("gui/") << obj->Text[0] : QString();
			}
		}

		if (key[KEY_ESC]) // Leave menu on ESC
			return true;

		return false;
	}
}
}
