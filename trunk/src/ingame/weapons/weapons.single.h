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

# include "../../stdafx.h"
# include "../../EngineClass.h"
# include "../../misc/camera.h"


namespace TA3D
{


    /*!
    ** \brief
    */
    class WEAPON						// Objet arme utilisé pendant le jeu
    {
    public:

        //! Default constructor
        WEAPON() { init(); }

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        ** \param dt
        ** \param map
        */
        void move(const float dt, MAP* map);

        /*!
        ** \brief
        */
        void draw(MAP *map = NULL);


    public:
        //! Weapon unique id
        short weapon_id;
        //! Position
        Vector3D Pos;
        //! Speed
        Vector3D V;
        //! Acceleration
        Vector3D Ac;

        //!
        Vector3D target_pos;			// Position ciblée
        //!
        short target;				// Unité ciblée (dans le tableau des unités)
        //!
        float stime;				// Temps écoulé depuis le lancement
        //!
        float killtime;			// Temps écoulé depuis la destruction de l'arme
        //!
        bool dying;
        //!
        float smoke_time;			// Temps écoulé depuis la dernière émission de fumée
        //!
        float f_time;				// Temps de vol
        //!
        float a_time;				// Temps d'activité
        //!
        short anim_sprite;		// Position dans l'animation
        //!
        short shooter_idx;		// Unité qui a tiré l'arme (ne peut pas se tirer dessus)
        //!
        byte phase;
        //!
        byte owner;
        //!
        uint32 idx;
        //!
        bool visible;
        //!
        float damage;
        //!
        bool just_explode;		// When set the weapon behaves as if it had hit something
        //!
        bool local;
        //!
        uint32 last_timestamp;

    }; // class WEAPON




}


#endif // __TA3D_INGAME_WEAPONS_SINGLE_H__
