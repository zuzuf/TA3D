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
#ifndef __TA3D_INGAME_WEAPONS_MANAGER_H__
# define __TA3D_INGAME_WEAPONS_MANAGER_H__

# include "weapons.def.h"


namespace TA3D
{


    /*! \class WEAPON_MANAGER
    **
    ** \brief Manager for all king of weapons
    */
    class WEAPON_MANAGER
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        WEAPON_MANAGER();
        //! Destructor
        ~WEAPON_MANAGER();
        //@}

        void init();
        void destroy();

        /*!
        ** \brief Add a weapon in the list
        ** \param name
        ** \return
        */
        int add_weapon(const String &name);

        /*!
        ** \brief Load a TDF file
        */
        void load_tdf(char *data, const int size = 99999999);

        /*!
        ** \brief
        ** \param name
        */
        int get_weapon_index(const String &name)
        {
            return (name.empty() || nb_weapons <= 0) ? -1 : (weapon_hashtable.find(String::ToLower(name)) - 1);
        }


    public:
        //! Count of registered weapons
        int	nb_weapons;
        std::vector< WEAPON_DEF > weapon;
        //! Animation for firing
        Gaf::Animation cannonshell;
        //! hashtable used to speed up operations on WEAPON_DEF objects
        cHashTable<int>  weapon_hashtable;

    }; // class WEAPON_MANAGER



    extern WEAPON_MANAGER weapon_manager;

}

#endif // __TA3D_INGAME_WEAPONS_MANAGER_H__
