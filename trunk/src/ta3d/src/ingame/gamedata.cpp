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

#include "gamedata.h"
#include "players.h"
#include <logs/logs.h>
#include <languages/i18n.h>

#define LOG_PREFIX_GAMEDATA "[GameData] "

namespace TA3D
{



    GameData::GameData()
		:map_filename(), nb_players(2), max_unit_per_player(2000)
    {
        saved_file.clear();

        use_only.clear();
        campaign = false;
        fog_of_war = FOW_DISABLED;
		game_script = "scripts\\game\\default.lua";

        team.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_names.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_sides.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_control.resize(TA3D_PLAYERS_HARD_LIMIT);
        ai_level.resize(TA3D_PLAYERS_HARD_LIMIT);
        energy.resize(TA3D_PLAYERS_HARD_LIMIT);
        metal.resize(TA3D_PLAYERS_HARD_LIMIT);
        ready.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_network_id.resize(TA3D_PLAYERS_HARD_LIMIT);
		for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        {
            energy[i] = metal[i] = 10000;
            player_network_id[i] = -1;
            ready[i] = false;
			team[i] = uint16(1 << i);
			player_control[i] = PLAYER_CONTROL_NONE;
			player_names[i] = I18N::Translate("open");
        }
    }



    GameData::~GameData()
    {
        use_only.clear();
        map_filename.clear();
        game_script.clear();
        nb_players=0;
        team.clear();
        player_names.clear();
        player_sides.clear();
        player_control.clear();
        player_network_id.clear();
        ai_level.clear();
        energy.clear();
        metal.clear();
        ready.clear();
        saved_file.clear();
    }



    int GameData::net2id(const int id ) const
    {
		for( int i = 0 ; i < TA3D_PLAYERS_HARD_LIMIT ; ++i)
        {
			if (player_network_id[i] == id && (player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN || player_control[i] == PLAYER_CONTROL_REMOTE_HUMAN))
                return i;
        }
        return -1;
    }

	String GameData::serialize() const
	{
		String data;
		data << max_unit_per_player << ','
			 << map_filename << ','
			 << game_script << ','
			 << use_only << ','
			 << int(fog_of_war) << ','
			 << nb_players << ',';
		for(int i = 0 ; i < nb_players ; ++i)
		{
			data << player_names[i] << ','
				 << player_sides[i] << ','
				 << int(player_control[i]) << ','
				 << ai_level[i] << ','
				 << energy[i] << ','
				 << metal[i] << ','
				 << team[i] << ',';
		}
		return data;
	}

	void GameData::unserialize(const String &data)
	{
		String::Vector args;
		data.explode(args, ',', false, true, true);

		if (args.size() < 6)		// Not enough fields
		{
			LOG_ERROR(LOG_PREFIX_GAMEDATA << "not enought fields");
			return;
		}
		max_unit_per_player = args[0].to<int>();
		map_filename = args[1];
		game_script = args[2];
		use_only = args[3];
		fog_of_war = (uint8)args[4].to<int>();
		nb_players = args[5].to<int>();
		if ((int)args.size() < 6 + nb_players * 7)
		{
			LOG_ERROR(LOG_PREFIX_GAMEDATA << "player data missing");
			return;
		}
		for(int i = 0 ; i < nb_players ; ++i)
		{
			player_names[i] = args[6 + i * 7];
			player_sides[i] = args[7 + i * 7];
			player_control[i] = (byte)args[8 + i * 7].to<int>();
			ai_level[i] = args[9 + i * 7];
			energy[i] = args[10 + i * 7].to<int>();
			metal[i] = args[11 + i * 7].to<int>();
			team[i] = (uint8)args[12 + i * 7].to<int>();
		}
	}
} // namespace TA3D
