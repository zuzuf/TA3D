
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

        player_names.resize(10);
        player_sides.resize(10);
        player_control.resize(10);
        ai_level.resize(10);
        energy.resize(10);
        metal.resize(10);
        ready.resize(10);
        player_network_id.resize(10);
        for( uint16 i = 0 ; i < 10 ; ++i)
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
