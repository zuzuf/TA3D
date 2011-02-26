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
#ifndef __TA3D_INGAME_WEAPONS_SINGLE_H__
# define __TA3D_INGAME_WEAPONS_SINGLE_H__

# include <stdafx.h>
# include <misc/camera.h>


namespace TA3D
{


    /*!
    ** \brief
    */
	class Weapon						// Objet arme utilisé pendant le jeu
    {
    public:

        //! Default constructor
		Weapon() { init(); }

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        ** \param dt
        ** \param map
        */
		void move(const float dt);

        /*!
        ** \brief
        */
		void draw();


    public:
        //! Position
        Vector3D Pos;
        //! Speed
        Vector3D V;
		//! Start position
		Vector3D start_pos;
		//!
		Vector3D target_pos;			// Position ciblée

		//! Weapon unique id
		int weapon_id;
		//!
		int target;				// Unité ciblée (dans le tableau des unités)
		//!
		float stime;				// Temps écoulé depuis le lancement
		//!
		float killtime;			// Temps écoulé depuis la destruction de l'arme
		//!
		float smoke_time;			// Temps écoulé depuis la dernière émission de fumée
		//!
		float f_time;				// Temps de vol
		//!
		float a_time;				// Temps d'activité
		//!
		int anim_sprite;		// Position dans l'animation
		//!
		int shooter_idx;		// Unité qui a tiré l'arme (ne peut pas se tirer dessus)
		//!
		float damage;
		//!
		uint32 last_timestamp;
		//!
		uint32 idx;
		//!
		uint32 ticks_to_compute;	// How many ticks to compute in order to synchronize the weapon (multiplayer, 0 for local objects)

        //!
        byte phase;
        //!
        byte owner;
        //!
        bool visible;
        //!
        bool just_explode;		// When set the weapon behaves as if it had hit something
        //!
        bool local;
		//!
		bool bInit;
		//!
		bool dying;

	}; // class Weapon




}


#endif // __TA3D_INGAME_WEAPONS_SINGLE_H__
