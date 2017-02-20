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
#ifndef __TA3D_INGAME_WEAPONS_DEF_H__
# define __TA3D_INGAME_WEAPONS_DEF_H__

# include <stdafx.h>
# include <mesh/mesh.h>
# include <misc/string.h>



# define RENDER_TYPE_LASER	        0x0
# define RENDER_TYPE_MISSILE		0x1
# define RENDER_TYPE_GUN			0x2
# define RENDER_TYPE_DGUN		    0x3
# define RENDER_TYPE_BITMAP		    0x4
# define RENDER_TYPE_PARTICLES	    0x5
# define RENDER_TYPE_BOMB	        0x6
# define RENDER_TYPE_LIGHTNING      0x7
# define RENDER_TYPE_NONE           0x8




namespace TA3D
{

	class WeaponDef				// Classe définissant un type d'arme
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
		WeaponDef();
        //! Destructor
		~WeaponDef();
        //@}


        /*!
        ** \brief Init or ReInit all data
        */
        void init();

        /*!
        ** \brief Release all resources
        */
        void destroy();

        /*!
        ** \brief
        **
        ** \param uname Name of the unit
        ** \return
        */
		uint32	get_damage_for_unit(const QString &uname) const;

    public:
        //!
        short weapon_id;			// Numéro identifiant l'arme
        //!
        QString internal_name;		// Nom interne de l'arme
        //!
        QString name;				// Nom de l'arme

        //!
        byte rendertype;
        //!
        bool ballistic;
        //!
        bool dropped;
        //!
        bool turret;
        //!
        int range;				// portée
        //!
        float reloadtime;			// temps de rechargement
        //!
        float weaponvelocity;
        //!
        float time_to_range;		// Time needed to get to range
        //!
        short areaofeffect;		// zone d'effet
        //!
        bool startsmoke;
        //!
        bool endsmoke;
        //!
        int	damage;				// Dégats causés par l'arme
        //!
        byte firestarter;
        //!
        int accuracy;
        //!
        int aimrate;
        //!
        int tolerance;
        //!
        short holdtime;
        //!
        int energypershot;
        //!
        int metalpershot;
        //!
        int minbarrelangle;
        //!
        bool unitsonly;
        //!
        float edgeeffectiveness;
        //!
        bool lineofsight;
        //!
        int	 color[4];
        //!
        short burst;
        //!
        float burstrate;
        //!
        float duration;
        //!
        bool beamweapon;
        //!
        bool burnblow;
        //!
        bool paralyzer;
        //! (for Missiles)
        float startvelocity;
        //! (for Missiles)
        float weapontimer;
        //! Acceleration (for missiles)
        float weaponacceleration;
        //! Rotation speed (for missiles)
        int turnrate;
        //! The 3D Model that is associated to this weapon
		Model* model;
        //!
        float smokedelay;
        //!
        bool guidance;
        //!
        bool tracks;
        //!
        bool selfprop;
        //! Produces smoke if enabled
        bool smoketrail;
        //!
        bool noautorange;
        //!
        bool noexplode;
        //!
        short flighttime;
        //!
        bool vlaunch;

        //!
        bool stockpile;
        //! Can be targeted
        bool targetable;
        //! Can shoot by itself
        bool commandfire;
        //!
        bool cruise;
        //!
        bool propeller;
        //!
        bool twophase;
        //!
        short shakemagnitude;
        //!
        float shakeduration;

        //! Sound to play when the weapon is firing
        QString soundstart;
        //! Sound to play when the weapon is exploding
        QString soundhit;
        //! Sound to play when the water is reached
        QString soundwater;
        //! Sound to play when the weapon is firing (in fast mode)
        QString soundtrigger;
        //!
        QString explosiongaf;
        //!
        QString explosionart;
        //!
        QString waterexplosiongaf;
        //!
        QString waterexplosionart;
        //!
        QString lavaexplosiongaf;
        //!
        QString lavaexplosionart;

        //!
        short nb_id;
        //!
        bool waterweapon;

        //!
        int	metal;
        //!
        int	energy;

        //!
        bool interceptor;			// Prend pour cible des armes / Target weapons only
        //!
        float coverage;				// Zone de protection
        //! Can only attack flying units
        bool toairweapon;
        //! hashtable used to get specific damages quickly
		HashMap<int>::Dense damage_hashtable;

		//! Textures for lasers
		GLuint laserTex1;
		GLuint laserTex2;
	}; // class WeaponDef



} // namespace TA3D


#endif // __TA3D_INGAME_WEAPONS_DEF_H__
