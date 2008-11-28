#ifndef __TA3D_INGAME_GAMEDATA_H__
# define __TA3D_INGAME_GAMEDATA_H__

# include "../stdafx.h"
# include <vector>

# define FOW_DISABLED  0x0
# define FOW_GREY      0x1
# define FOW_BLACK     0x2
# define FOW_ALL       0x3



namespace TA3D
{

    /*! \class GameData
    **
    ** \brief Game Informations
    */
    class GameData
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        GameData();
        //! Destructor
        ~GameData();
        //@}


        /*!
        ** \brief Get a player index (in vectors) from its id
        ** \param id ID of the player to find out
        ** \return id of the player if found, -1 otherwise
        */
        int net2id(const int id) const;


    public:
        //! Which map to play
        String map_filename;

        //! How many players
        int	 nb_players;

        //! How many units per player
        int	 max_unit_per_player;

        //! Name of players
        String::Vector player_names;
        //! Side of players
        String::Vector player_sides;

        //! Who control them
        std::vector<byte> player_control;

        //! Network ID of players
        std::vector<int> player_network_id;
        //! What's their level (for AI)
        std::vector<byte> ai_level;

        //! How much energy do they have when game starts
        std::vector<uint32> energy;
        //! Idem with metal
        std::vector<uint32> metal;

        //! Teams
        std::vector<uint16> team;
        //! Who is ready ?
        std::vector<byte> ready;
        //! Which script to run
        String game_script;
        //! flags to configure FOW
        uint8 fog_of_war;

        //! Are we in campaign mode ?
        bool campaign;

        //! The use only file to read
        String use_only;
        //! If not empty it's the name of the file to load
        String saved_file;

    }; // class GameData




} // namespace TA3D

#endif // __TA3D_INGAME_GAMEDATA_H__
