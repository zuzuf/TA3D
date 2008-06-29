#ifndef __TA3D_INGAME_GAMEDATA_H__
# define __TA3D_INGAME_GAMEDATA_H__

# include "../stdafx.h"


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
        //{
        //! Default constructor
        GameData();
        //! Destructor
        ~GameData();
        //}
    

        /*!
        ** \brief Get a player index (in vectors) from its id
        ** \param id ID of the player to find out
        ** \return id of the player if found, -1 otherwise
        */
        int net2id(const int id) const;
        

    public:
        //! Which map to play
        char* map_filename;	

        //! How many players
        int	 nb_players;

        //! Name of players
        Vector<String> player_names;
        //! Side of players
        Vector<String> player_sides;

        //! Who control them
        Vector<byte> player_control;
        
        //! Network ID of players
        Vector<int> player_network_id;
        //! What's their level (for AI)
        Vector<byte> ai_level;

        //! How much energy do they have when game starts
        Vector<uint32> energy; 
        //! Idem with metal
        Vector<uint32> metal;

        //! Who is ready ?
        Vector<byte> ready;
        //! Which script to run
        char* game_script;
        //! flags to configure FOW
        uint8 fog_of_war;

        //! Are we in campaign mode ?
        bool campaign;

        //! The use only file to read
        char* use_only;
        //! If not empty it's the name of the file to load
        String saved_file;

    }; // class GameData




} // namespace TA3D

#endif // __TA3D_INGAME_GAMEDATA_H__
