
#include "gamedata.h"



namespace TA3D
{



    GameData::GameData()
        :map_filename(), nb_players(0)
    {
        saved_file.clear();

        use_only.clear();
        campaign = false;
        fog_of_war = FOW_DISABLED;
        game_script.clear();

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
        }
    }



    GameData::~GameData()
    {
        use_only.clear();
        map_filename.clear();
        game_script.clear();
        nb_players=0;
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
