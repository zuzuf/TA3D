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



namespace TA3D
{



    GameData::GameData()
        :map_filename(), nb_players(0), max_unit_per_player(2000)
    {
        saved_file.clear();

        use_only.clear();
        campaign = false;
        fog_of_war = FOW_DISABLED;
        game_script.clear();

        team.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_names.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_sides.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_control.resize(TA3D_PLAYERS_HARD_LIMIT);
        ai_level.resize(TA3D_PLAYERS_HARD_LIMIT);
        energy.resize(TA3D_PLAYERS_HARD_LIMIT);
        metal.resize(TA3D_PLAYERS_HARD_LIMIT);
        ready.resize(TA3D_PLAYERS_HARD_LIMIT);
        player_network_id.resize(TA3D_PLAYERS_HARD_LIMIT);
        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        {
            energy[i] = metal[i] = 10000;
            player_network_id[i] = -1;
            ready[i] = false;
            team[i] = 1 << i;
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
        for( int i = 0 ; i < nb_players ; ++i)
        {
            if( player_network_id[i] == id )
                return i;
        }
        return -1;
    }



} // namespace TA3D
