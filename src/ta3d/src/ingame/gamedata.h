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
#ifndef __TA3D_INGAME_GAMEDATA_H__
# define __TA3D_INGAME_GAMEDATA_H__

# include <stdafx.h>
# include <misc/string.h>
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

		/*!
		** \brief Save and restore game data as a QString
		** \param serialized data
		** \return serialized data
		*/
		QString serialize() const;
		void unserialize(const QString &data);

    public:
        //! Which map to play
        QString map_filename;

        //! How many players
        int	 nb_players;

        //! How many units per player
        int	 max_unit_per_player;

        //! Name of players
        QStringList player_names;
        //! Side of players
        QStringList player_sides;

        //! Who control them
        std::vector<byte> player_control;

        //! Network ID of players
        std::vector<int> player_network_id;
        //! What's their level (for AI)
        std::vector<QString> ai_level;

        //! How much energy do they have when game starts
        std::vector<uint32> energy;
        //! Idem with metal
        std::vector<uint32> metal;

        //! Teams
        std::vector<uint16> team;
        //! Who is ready ?
        std::vector<byte> ready;
        //! Which script to run
        QString game_script;
        //! flags to configure FOW
        uint8 fog_of_war;

        //! Are we in campaign mode ?
        bool campaign;

        //! The use only file to read
        QString use_only;
        //! If not empty it's the name of the file to load
        QString saved_file;

    }; // class GameData




} // namespace TA3D

#endif // __TA3D_INGAME_GAMEDATA_H__
