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

#include "briefscreen.h"
#include <input/keyboard.h>
#include <input/mouse.h>
#include <vfs/vfs.h>
#include <languages/i18n.h>
#include <ingame/sidedata.h>
#include <misc/paths.h>
#include <misc/timer.h>
#include <sounds/manager.h>
#include <ingame/players.h>

namespace TA3D
{
namespace Menus
{

	Battle::Result BriefScreen::Execute(const QString &campaign_name, const int mission_id)
	{
		BriefScreen m(campaign_name, mission_id);
		m.execute();
		return m.exit_mode;
	}

	BriefScreen::BriefScreen(const QString &campaign_name, const int mission_id)
		: Abstract(), campaign_name(campaign_name), mission_id(mission_id)
	{}


	BriefScreen::~BriefScreen()
	{}


	void BriefScreen::doFinalize()
	{
		// Wait for user to release ESC
		reset_mouse();
		while (key[KEY_ESC])
		{
            QThread::msleep(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		clear_keybuf();

		if (pArea->get_object( "brief.planet"))     pArea->get_object( "brief.planet" )->Data = 0;
		if (pArea->get_object( "brief.panning0"))   pArea->get_object( "brief.panning0" )->Data = 0;
		if (pArea->get_object( "brief.panning1"))   pArea->get_object( "brief.panning1" )->Data = 0;

		pArea->destroy();

		sound_manager->stopSoundFileNow();

		if (start_game)  // Open the briefing screen and start playing the campaign
		{
			GameData game_data;

			// Generate the script which will be removed later
            TA3D::Paths::MakeDir(TA3D::Paths::Resources + "scripts/game");
            TA3D::generate_script_from_mission(TA3D::Paths::Resources + "scripts/game/__campaign_script.lua", ota_parser, schema);

			game_data.game_script = "scripts/game/__campaign_script.lua";
            game_data.map_filename = Substr(map_filename, 0, map_filename.size() - 3 ) + "tnt";     // Remember the last map we played
			game_data.fog_of_war = FOW_ALL;

			game_data.nb_players = ota_parser.pullAsInt("GlobalHeader.numplayers", 2);
			if (game_data.nb_players == 0) // Yes it can happen !!
				game_data.nb_players = 2;

			game_data.player_control[0] = PLAYER_CONTROL_LOCAL_HUMAN;
			game_data.player_names[0] = brief_parser.pullAsString( "HEADER.campaignside");
			game_data.player_sides[0] = brief_parser.pullAsString( "HEADER.campaignside");
            game_data.energy[0] = ota_parser.pullAsInt( QString("GlobalHeader.Schema %1.humanenergy").arg(schema) );
            game_data.metal[0] = ota_parser.pullAsInt( QString("GlobalHeader.Schema %1.humanmetal").arg(schema) );

            const QString &schema_type = ota_parser.pullAsString(QString("GlobalHeader.Schema %1.Type").arg(schema)).toLower();

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

            for (int i = 1; i < game_data.nb_players; ++i)
			{
				game_data.player_control[ i ] = PLAYER_CONTROL_LOCAL_AI;
				game_data.player_names[ i ] = brief_parser.pullAsString( "HEADER.campaignside");
				game_data.player_sides[ i ] = brief_parser.pullAsString( "HEADER.campaignside");            // Has no meaning here since we are in campaign mode units are spawned by a script
				game_data.ai_level[ i ] = game_data.ai_level[ 0 ];
                game_data.energy[ i ] = ota_parser.pullAsInt( QString("GlobalHeader.Schema %1.computerenergy").arg(schema) );
                game_data.metal[ i ] = ota_parser.pullAsInt( QString("GlobalHeader.Schema %1.computermetal").arg(schema) );

				player_color_map[ i ] = byte(i);
			}

			game_data.campaign = true;
			game_data.use_only = ota_parser.pullAsString( "GlobalHeader.useonlyunits");
            if (!game_data.use_only.isEmpty())
                game_data.use_only = "camps/useonly/" + game_data.use_only;

			exit_mode = Battle::Execute(&game_data);

			reset_mouse();
			while (key[KEY_ESC])
			{
                QThread::msleep(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
			clear_keybuf();

			return;
		}
		exit_mode = Battle::brUnknown;
	}


	bool BriefScreen::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MULTIMENU << "Entering...");

		// Loading the area
		loadAreaFromTDF("brief", "gui/brief.area");

		brief_parser.loadFromFile(campaign_name);           // Loads the campaign file

        map_filename = "maps/" + brief_parser.pullAsString(QString("MISSION%1.missionfile").arg(mission_id));
		ota_parser.loadFromFile(map_filename);

        const QString &narration_file = "camps/briefs/" + ota_parser.pullAsString("GlobalHeader.narration") + ".wav"; // The narration file

        const QString &language_suffix = (lp_CONFIG->Lang == "english") ? QString() : ("-" + lp_CONFIG->Lang);
        QString brief_file = "camps/briefs" + language_suffix + "/" + ota_parser.pullAsString("GlobalHeader.brief") + ".txt"; // The brief file

		{
			if (!VFS::Instance()->fileExists( brief_file ) )         // try without the .txt
                brief_file = "camps/briefs" + language_suffix + "/" + ota_parser.pullAsString( "GlobalHeader.brief");
			if (!VFS::Instance()->fileExists( brief_file ) )         // try without the suffix if we cannot find it
                brief_file = "camps/briefs/" + ota_parser.pullAsString( "GlobalHeader.brief" ) + ".txt";
			if (!VFS::Instance()->fileExists( brief_file ) )         // try without the suffix if we cannot find it
                brief_file = "camps/briefs/" + ota_parser.pullAsString( "GlobalHeader.brief");
            QIODevice *file = VFS::Instance()->readFile(brief_file);
			if (file)
			{
                const QString &brief_info = QString::fromLatin1((const char*)file->readAll());
				pArea->caption( "brief.info", brief_info);
				delete file;
			}
		}

        QString planet_file = ota_parser.pullAsString("GlobalHeader.planet").toLower();

        if (planet_file == "green planet" )             planet_file = "anims/greenbrief.gaf";
        else if (planet_file == "archipelago" )         planet_file = "anims/archibrief.gaf";
        else if (planet_file == "desert" )              planet_file = "anims/desertbrief.gaf";
        else if (planet_file == "lava" )                planet_file = "anims/lavabrief.gaf";
        else if (planet_file == "wet desert" )          planet_file = "anims/wdesertbrief.gaf";
        else if (planet_file == "metal" )               planet_file = "anims/metalbrief.gaf";
        else if (planet_file == "red planet" )          planet_file = "anims/marsbrief.gaf";
        else if (planet_file == "lunar" )               planet_file = "anims/lunarbrief.gaf";
        else if (planet_file == "lush" )                planet_file = "anims/lushbrief.gaf";
        else if (planet_file == "ice" )                 planet_file = "anims/icebrief.gaf";
        else if (planet_file == "slate" )               planet_file = "anims/slatebrief.gaf";
        else if (planet_file == "water world" )         planet_file = "anims/waterbrief.gaf";

		if (planet_file.size() > 4)
			planet_animation.loadGAFFromDirectory(Substr(planet_file,0, planet_file.size() - 4), true);
		if (planet_animation.size() == 0)
		{
            QIODevice *file = VFS::Instance()->readFile(planet_file);
			if (file)
			{
				planet_animation.loadGAFFromRawData(file, true);
				delete file;
			}
		}

		schema = 0;

		if (pArea->get_object("brief.schema"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("brief.schema");
			obj->Text.clear();
            const int schema_count = ota_parser.pullAsInt("GlobalHeader.SCHEMACOUNT");
            obj->Text.reserve(schema_count + 1);
            obj->Text.push_back(QString());
            for (int i = 0 ; i < schema_count; ++i)
                obj->Text.push_back(I18N::Translate(ota_parser.pullAsString(QString("GlobalHeader.Schema %1.Type").arg(i))));
			if (obj->Text.size() > 1)
				obj->Text[0] = obj->Text[1];
		}

		sound_manager->playSoundFileNow(narration_file);

		start_game = false;

		planet_frame = 0.0f;

		pan_id = 0;
		rotate_id = 0;
		for (int i = 0; i < planet_animation.size(); ++i)
		{
            if (Substr(planet_animation[i].name,planet_animation[i].name.size() - 3, 3).toLower() == "pan")
				pan_id = i;
			else
                if (Substr(planet_animation[i].name,planet_animation[i].name.size() - 6, 6).toLower() == "rotate")
					rotate_id = i;
		}

		pan_x1 = 0.0f;
		pan_x2 = 0.0f;

		if (pArea->get_object("brief.panning0"))
		{
			pan_x1 = pArea->get_object("brief.panning0")->x1;
			pan_x2 = pArea->get_object("brief.panning0")->x2;
		}

		time_ref = msectimer();

		return true;
	}



	void BriefScreen::waitForEvent()
	{
		do
		{
			// Grab user events
			pArea->check();

			// Wait to reduce CPU consumption
			wait();

		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
				 && mouse_b == 0
				 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
				 && !pArea->key_pressed && !pArea->scrolling
				 && (int)planet_frame == (int)(float(msectimer() - time_ref) * 0.01f));
		planet_frame = float(msectimer() - time_ref) * 0.01f;
	}


	bool BriefScreen::maySwitchToAnotherMenu()
	{
		if (pArea->get_value("brief.schema") >= 0)
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object( "brief.schema");
			if (obj->Value != -1)
			{
				obj->Text[0] = obj->Text[obj->Value + 1];
				schema = obj->Value;
			}
		}

		if (pArea->get_state("brief.info"))
			pArea->get_object("brief.info")->Pos++;

		if (pArea->get_object( "brief.planet"))
		{
			Gui::GUIOBJ::Ptr guiobj = pArea->get_object("brief.planet");
			if (guiobj)
				guiobj->Data = planet_animation[rotate_id].glbmp[((int)planet_frame) % planet_animation[rotate_id].nb_bmp];
		}

		float pan_frame = planet_frame / (pan_x2 - pan_x1);

		if (pArea->get_object("brief.panning0"))
		{
			Gui::GUIOBJ::Ptr guiobj = pArea->get_object("brief.panning0");
			if (guiobj)
			{
				guiobj->Data = planet_animation[pan_id].glbmp[ ((int)pan_frame) % planet_animation[pan_id].nb_bmp ];
				guiobj->u1 = Math::Modf(pan_frame);
				guiobj->x2 = pan_x2 + (pan_x1 - pan_x2) * guiobj->u1;
			}
		}

		if (pArea->get_object( "brief.panning1"))
		{
			Gui::GUIOBJ::Ptr guiobj = pArea->get_object("brief.panning1");
			if (guiobj)
			{
				guiobj->Data = planet_animation[pan_id].glbmp[ ((int)pan_frame + 1) % planet_animation[pan_id].nb_bmp ];
				guiobj->u2 = Math::Modf(pan_frame);
				guiobj->x1 = pan_x2 + (pan_x1 - pan_x2) * guiobj->u2;
			}
		}

		if (pArea->get_state( "brief.b_ok" ) || key[KEY_ENTER])
		{
			start_game = true;
			return true;      // If user click "OK" or hit enter then leave the window
		}
		if (pArea->get_state( "brief.b_cancel"))
			return true;       // En cas de click sur "retour", on quitte la fenêtre

		if (key[KEY_ESC]) // Leave menu on ESC
			return true;

		return false;
	}
}
}
