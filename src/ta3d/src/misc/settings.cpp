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

#include "settings.h"
#include <logs/logs.h>
#include "paths.h"
#include "files.h"
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <languages/i18n.h>
#include "tdf.h"



namespace TA3D
{
namespace Settings
{



	bool Backup(const QString& filename)
	{
		LOG_INFO(LOG_PREFIX_SETTINGS << "Making a backup for `" << filename << "`...");
        if (Paths::Files::Copy(filename, filename + ".bak"))
		{
			LOG_INFO(LOG_PREFIX_SETTINGS << "The backup is done.");
			return true;
		}
		LOG_WARNING(LOG_PREFIX_SETTINGS << "Impossible to make the backup.");
		return false;
	}


	bool Save()
	{
		if (!TA3D::VARS::lp_CONFIG)
			return false;

		// Make a copy that can be restored if TA3D does not start any more
		TA3D::Settings::Backup(TA3D::Paths::ConfigFile);

		TA3D::VARS::lp_CONFIG->last_MOD = TA3D::VARS::TA3D_CURRENT_MOD;

        QString s;
        QTextStream stream(&s);
        stream << "// TA3D Settings\n"
               << "\n"
               << "[TA3D]\n"
               << "{\n"
               << "                  Version = " << TA3D_VERSION_HI << "." << TA3D_VERSION_LO << ";\n"
               << "                FPS Limit = " << TA3D::VARS::lp_CONFIG->fps_limit << "; // <= 0 means `unlimited`\n"
               << "              Time Factor = " << TA3D::VARS::lp_CONFIG->timefactor << ";\n"
               << "           Shadow Quality = " << TA3D::VARS::lp_CONFIG->shadow_quality << "; // 0 -> none, 1 -> low (shadow volumes), 2 -> normal (shadow maps), 3 -> high (shadow maps)\n"
               << "           ShadowMap Size = " << (int)TA3D::VARS::lp_CONFIG->shadowmap_size << "; // 0 -> lower (256x256), 1 -> low (512x512), 2 -> normal (1024x1024), 3 -> high (2048x2048)\n"
               << "           Priority Level = " << TA3D::VARS::lp_CONFIG->priority_level << "; // 0, 1, 2\n"
               << "                     FSAA = " << TA3D::VARS::lp_CONFIG->fsaa << ";\n"
               << "               Anisotropy = " << TA3D::VARS::lp_CONFIG->anisotropy << ";\n"
               << "                 Language = " << TA3D::VARS::lp_CONFIG->Lang << ";\n"
               << "            Water Quality = " << TA3D::VARS::lp_CONFIG->water_quality << "; // 0..5\n"
               << "             Screen Width = " << TA3D::VARS::lp_CONFIG->screen_width << ";\n"
               << "            Screen Height = " << TA3D::VARS::lp_CONFIG->screen_height << ";\n"
               << "              Color Depth = " << (int)TA3D::VARS::lp_CONFIG->color_depth << ";\n"
               << "                 Show FPS = " << TA3D::VARS::lp_CONFIG->showfps << ";\n"
               << "           Show Wireframe = " << TA3D::VARS::lp_CONFIG->wireframe << ";\n"
               << "           Show particles = " << TA3D::VARS::lp_CONFIG->particle << ";\n"
               << " Show explosion particles = " << TA3D::VARS::lp_CONFIG->explosion_particles << ";\n"
               << "               Show Waves = " << TA3D::VARS::lp_CONFIG->waves << ";\n"
               << "        Show Height Lines = " << TA3D::VARS::lp_CONFIG->height_line << ";\n"
               << "          Show FullScreen = " << TA3D::VARS::lp_CONFIG->fullscreen << ";\n"
               << "           Detail Texture = " << TA3D::VARS::lp_CONFIG->detail_tex << ";\n"
               << "     Draw Console Loading = " << TA3D::VARS::lp_CONFIG->draw_console_loading << ";\n"
               << "                Game Data = " << TA3D::VARS::lp_CONFIG->serializedGameData << ";\n"
               << "                 Last MOD = " << TA3D::VARS::lp_CONFIG->last_MOD << ";\n"
               << "              Player name = " << TA3D::VARS::lp_CONFIG->player_name << ";\n"
               << "         Camera Zoom Mode = " << (int)TA3D::VARS::lp_CONFIG->camera_zoom << ";\n"
               << "     Camera Default Angle = " << TA3D::VARS::lp_CONFIG->camera_def_angle << ";\n"
               << "    Camera Default Height = " << TA3D::VARS::lp_CONFIG->camera_def_h << ";\n"
               << "        Camera Zoom Speed = " << TA3D::VARS::lp_CONFIG->camera_zoom_speed << ";\n"
               << "   Interface Transparency = " << TA3D::VARS::lp_CONFIG->menuTransparency << "; // 0 -> fully opaque, 1 -> fully transparent, default : 0\n"
               << "                     Skin = " << TA3D::VARS::lp_CONFIG->skin_name << ";\n"
               << "        Use Texture Cache = " << TA3D::VARS::lp_CONFIG->use_texture_cache << ";\n"
               << "               Net Server = " << TA3D::VARS::lp_CONFIG->net_server << "; // default: " << TA3D_DEFAULT_SERVER_HOSTNAME << "\n"
               << "               Render Sky = " << TA3D::VARS::lp_CONFIG->render_sky << ";\n"
               << "       Low Definition Map = " << TA3D::VARS::lp_CONFIG->low_definition_map << ";\n"
               << "  Use Texture Compression = " << TA3D::VARS::lp_CONFIG->use_texture_compression << ";\n"
               << "        Underwater Bright = " << TA3D::VARS::lp_CONFIG->underwater_bright << ";\n"
               << "             Disable GLSL = " << TA3D::VARS::lp_CONFIG->disable_GLSL << ";\n"
               << "    Right Click Interface = " << TA3D::VARS::lp_CONFIG->right_click_interface << ";\n"
               << "      Orthographic Camera = " << TA3D::VARS::lp_CONFIG->ortho_camera << ";\n"
               << "        Mouse Sensitivity = " << TA3D::VARS::lp_CONFIG->mouse_sensivity << ";\n"
               << "                Far Sight = " << TA3D::VARS::lp_CONFIG->far_sight << ";\n"
               << "             Sound Volume = " << TA3D::VARS::lp_CONFIG->sound_volume << ";\n"
               << "             Music Volume = " << TA3D::VARS::lp_CONFIG->music_volume << ";\n"
               << "              Grab Inputs = " << TA3D::VARS::lp_CONFIG->grab_inputs << ";\n"
               << "               7z command = " << TA3D::VARS::lp_CONFIG->system7zCommand << ";\n"
               << "           Resource Paths = " << TA3D::VARS::lp_CONFIG->resourcePaths << ";\n"
               << "                 Tooltips = " << TA3D::VARS::lp_CONFIG->tooltips << ";\n"
               << "           Developer Mode = " << TA3D::VARS::lp_CONFIG->developerMode << ";\n"
               << "          Texture Quality = " << TA3D::VARS::lp_CONFIG->unitTextureQuality << ";\n"
               << "}\n";

        if (Paths::Files::SaveToFile(TA3D::Paths::ConfigFile, s.toUtf8()))
		{
			LOG_INFO(LOG_PREFIX_SETTINGS << "The settings has been saved.");
			return true;
		}
		LOG_ERROR(LOG_PREFIX_SETTINGS << "Impossible to write settings: `" << TA3D::Paths::ConfigFile << "`");
		return false;
	}



	bool Restore(const QString& filename)
	{
		LOG_INFO(LOG_PREFIX_SETTINGS << "Restoring the backup for `" << filename << "`...");
        if (Paths::Files::Copy(filename + ".bak", filename))
		{
			LOG_INFO(LOG_PREFIX_SETTINGS << "The settings have been restored.");
			return true;
		}
		LOG_WARNING(LOG_PREFIX_SETTINGS << "Impossible to restore the settings.");
		return false;
	}


	bool Load()
	{
		TDFParser cfgFile;
		if (!cfgFile.loadFromFile(TA3D::Paths::ConfigFile,false,false,false,true))      // Load this from real file system since it has nothing to do with game content
		{
			LOG_ERROR(LOG_PREFIX_SETTINGS << "Impossible to load the settings from `" << TA3D::Paths::ConfigFile << "`");

			lp_CONFIG->Lang = "english";     // Set default language to English
			lp_CONFIG->first_start = true;	// No config file -> guess best settings
			// Apply settings for the current language
			I18N::Instance()->currentLanguage(lp_CONFIG->Lang);

			return false;
		}

		TA3D::VARS::lp_CONFIG->fps_limit = cfgFile.pullAsFloat("TA3D.FPS Limit");
		TA3D::VARS::lp_CONFIG->timefactor = cfgFile.pullAsFloat("TA3D.Time Factor");

		TA3D::VARS::lp_CONFIG->shadow_quality = sint16(cfgFile.pullAsInt("TA3D.Shadow Quality"));
		TA3D::VARS::lp_CONFIG->shadowmap_size = uint8(cfgFile.pullAsInt("TA3D.ShadowMap Size", 2));
		TA3D::VARS::lp_CONFIG->priority_level = sint16(cfgFile.pullAsInt("TA3D.Priority Level"));
		TA3D::VARS::lp_CONFIG->fsaa = sint16(cfgFile.pullAsInt("TA3D.FSAA"));
		TA3D::VARS::lp_CONFIG->anisotropy = sint16(cfgFile.pullAsInt("TA3D.Anisotropy", 1));
        TA3D::VARS::lp_CONFIG->Lang = cfgFile.pullAsString("TA3D.Language").toLower();
		TA3D::VARS::lp_CONFIG->water_quality = sint16(cfgFile.pullAsInt("TA3D.Water Quality"));
		TA3D::VARS::lp_CONFIG->screen_width = uint16(cfgFile.pullAsInt("TA3D.Screen Width"));
		TA3D::VARS::lp_CONFIG->screen_height = uint16(cfgFile.pullAsInt("TA3D.Screen Height"));
		TA3D::VARS::lp_CONFIG->color_depth = uint8(cfgFile.pullAsInt("TA3D.Color Depth", 32));

		TA3D::VARS::lp_CONFIG->showfps = cfgFile.pullAsBool("TA3D.Show FPS");
		TA3D::VARS::lp_CONFIG->wireframe = cfgFile.pullAsBool("TA3D.Show Wireframe");
		TA3D::VARS::lp_CONFIG->explosion_particles = cfgFile.pullAsBool("TA3D.Show explosion particles", true);
		TA3D::VARS::lp_CONFIG->particle = cfgFile.pullAsBool("TA3D.Show particles");
		TA3D::VARS::lp_CONFIG->waves = cfgFile.pullAsBool("TA3D.Show Waves");
		TA3D::VARS::lp_CONFIG->height_line = cfgFile.pullAsBool("TA3D.Show Height Lines");
		TA3D::VARS::lp_CONFIG->fullscreen = cfgFile.pullAsBool("TA3D.Show FullScreen", false);
		TA3D::VARS::lp_CONFIG->detail_tex = cfgFile.pullAsBool("TA3D.Detail Texture");
		TA3D::VARS::lp_CONFIG->draw_console_loading = cfgFile.pullAsBool("TA3D.Draw Console Loading");
		TA3D::VARS::lp_CONFIG->tooltips = cfgFile.pullAsBool("TA3D.Tooltips");

        TA3D::VARS::lp_CONFIG->serializedGameData = cfgFile.pullAsString("TA3D.Game Data", QString());
        TA3D::VARS::lp_CONFIG->last_MOD = cfgFile.pullAsString("TA3D.Last MOD", "");

		TA3D::VARS::lp_CONFIG->camera_zoom = uint8(cfgFile.pullAsInt("TA3D.Camera Zoom Mode", ZOOM_NORMAL));
		TA3D::VARS::lp_CONFIG->camera_def_angle = cfgFile.pullAsFloat("TA3D.Camera Default Angle", 63.44f);
		TA3D::VARS::lp_CONFIG->camera_def_h = cfgFile.pullAsFloat("TA3D.Camera Default Height", 200.0f);
		TA3D::VARS::lp_CONFIG->camera_zoom_speed = cfgFile.pullAsFloat("TA3D.Camera Zoom Speed", 1.0f);

		TA3D::VARS::lp_CONFIG->menuTransparency = cfgFile.pullAsFloat("TA3D.Interface Transparency", 0.0f);

		TA3D::VARS::lp_CONFIG->use_texture_cache = cfgFile.pullAsBool("TA3D.Use Texture Cache", false);

        TA3D::VARS::lp_CONFIG->skin_name = cfgFile.pullAsString("TA3D.Skin", "");

        TA3D::VARS::lp_CONFIG->net_server = cfgFile.pullAsString("TA3D.Net Server", TA3D_DEFAULT_SERVER_HOSTNAME);

		TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD;

        TA3D::VARS::lp_CONFIG->player_name = cfgFile.pullAsString("TA3D.Player name", "player");

		TA3D::VARS::lp_CONFIG->render_sky = cfgFile.pullAsBool("TA3D.Render Sky", true);

		TA3D::VARS::lp_CONFIG->low_definition_map = cfgFile.pullAsBool("TA3D.Low Definition Map", false);

		TA3D::VARS::lp_CONFIG->use_texture_compression = cfgFile.pullAsBool("TA3D.Use Texture Compression", true);

		TA3D::VARS::lp_CONFIG->underwater_bright = cfgFile.pullAsBool("TA3D.Underwater Bright", false);

		TA3D::VARS::lp_CONFIG->disable_GLSL = cfgFile.pullAsBool("TA3D.Disable GLSL", false);

		TA3D::VARS::lp_CONFIG->right_click_interface = cfgFile.pullAsBool("TA3D.Right Click Interface", false);

		TA3D::VARS::lp_CONFIG->ortho_camera = cfgFile.pullAsBool("TA3D.Orthographic Camera", false);

		TA3D::VARS::lp_CONFIG->mouse_sensivity = cfgFile.pullAsFloat("TA3D.Mouse Sensitivity", 1.0f);

		TA3D::VARS::lp_CONFIG->far_sight = cfgFile.pullAsBool("TA3D.Far Sight", true);

		TA3D::VARS::lp_CONFIG->sound_volume = cfgFile.pullAsInt("TA3D.Sound Volume", 128);
		TA3D::VARS::lp_CONFIG->music_volume = cfgFile.pullAsInt("TA3D.Music Volume", 128);

		TA3D::VARS::lp_CONFIG->grab_inputs = cfgFile.pullAsBool("TA3D.Grab Inputs", false);

        TA3D::VARS::lp_CONFIG->system7zCommand = cfgFile.pullAsString("TA3D.7z command", "7z");
        TA3D::VARS::lp_CONFIG->resourcePaths = cfgFile.pullAsString("TA3D.Resource Paths");

		TA3D::VARS::lp_CONFIG->developerMode = cfgFile.pullAsBool("TA3D.Developer Mode");

		TA3D::VARS::lp_CONFIG->unitTextureQuality = cfgFile.pullAsInt("TA3D.Texture Quality", 3);

        QString cfg_version = cfgFile.pullAsString("TA3D.Version");
        QString ref_version = QString("%1.%1").arg(TA3D_VERSION_HI).arg(TA3D_VERSION_LO);
		if (cfg_version != ref_version)     // Update ?
		{
			lp_CONFIG->net_server = TA3D_DEFAULT_SERVER_HOSTNAME;
            if (cfg_version.isEmpty())        // Pre-SDL versions
			{
				lp_CONFIG->first_start = true;		// First start of a >= 0.6 release
				lp_CONFIG->shadow_quality = sint16(cfgFile.pullAsInt("TA3D.Show Shadows"));
                int langID = lp_CONFIG->Lang.toInt(nullptr, 0);     // TA3D used to store language ID instead of language
				switch( langID )
				{
					case 0:     lp_CONFIG->Lang = "english";    break;
					case 1:     lp_CONFIG->Lang = "french";     break;
					case 2:     lp_CONFIG->Lang = "german";     break;
					case 3:     lp_CONFIG->Lang = "spanish";    break;
					case 4:     lp_CONFIG->Lang = "italian";    break;
					case 5:     lp_CONFIG->Lang = "japanese";   break;
					default:
								lp_CONFIG->Lang = "english";
				};
			}
		}

		// Apply settings for the current language
        if (!lp_CONFIG->Lang.isEmpty())
			I18N::Instance()->currentLanguage(lp_CONFIG->Lang);

		LOG_INFO(LOG_PREFIX_SETTINGS << "Loaded from `" << TA3D::Paths::ConfigFile << "`");
		return true;
	}



} // namespace Settings
} // namespace TA3D
