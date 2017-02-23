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

# include <stdafx.h>
# include <misc/string.h>
# include "weapons.def.h"


namespace TA3D
{


	/*! \class WeaponManager
    **
    ** \brief Manager for all king of weapons
    */
	class WeaponManager
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
		WeaponManager();
        //! Destructor
		~WeaponManager();
        //@}

        void init();
        void destroy();

        /*!
        ** \brief Add a weapon in the list
        ** \param name
        ** \return
        */
        int add_weapon(const QString &name);

        /*!
        ** \brief Load a TDF file
        */
        void load_tdf(QIODevice *file);

        /*!
        ** \brief
        ** \param name
        */
        int get_weapon_index(const QString &name)
        {
            return (name.isEmpty() || nb_weapons <= 0) ? -1 : (weapon_hashtable[name.toLower()] - 1);
        }


    public:
        //! Count of registered weapons
        int	nb_weapons;
		std::vector< WeaponDef > weapon;
        //! Animation for firing
        Gaf::Animation cannonshell;
		//! hashtable used to speed up operations on WeaponDef objects
		HashMap<int>::Dense  weapon_hashtable;

	}; // class WeaponManager



	extern WeaponManager weapon_manager;

}

#endif // __TA3D_INGAME_WEAPONS_MANAGER_H__
