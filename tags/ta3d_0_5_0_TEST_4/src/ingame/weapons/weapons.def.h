#ifndef __TA3D_INGAME_WEAPONS_DEF_H__
# define __TA3D_INGAME_WEAPONS_DEF_H__

# include "../../stdafx.h"
# include "../../3do.h"



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

    class WEAPON_DEF				// Classe définissant un type d'arme
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        WEAPON_DEF();
        //! Destructor
        ~WEAPON_DEF();
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
        uint32	get_damage_for_unit(const String &uname) const;

    public:
        //!
        short weapon_id;			// Numéro identifiant l'arme
        //!
        char* internal_name;		// Nom interne de l'arme
        //!
        char* name;				// Nom de l'arme

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
        //! (for Missiles)
        float startvelocity;
        //! (for Missiles)
        float weapontimer;
        //! Acceleration (for missiles)
        float weaponacceleration;
        //! Rotation speed (for missiles)
        int turnrate;
        //! The 3D Model that is associated to this weapon
        MODEL* model;
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
        char* soundstart;
        //! Sound to play when the weapon is exploding
        char* soundhit;
        //! Sound to play when the water is reached
        char* soundwater;
        //! Sound to play when the weapon is firing (in fast mode)
        char* soundtrigger;
        //!
        char* explosiongaf;
        //!
        char* explosionart;
        //!
        char* waterexplosiongaf;
        //!
        char* waterexplosionart;
        //!
        char* lavaexplosiongaf;
        //!
        char* lavaexplosionart;

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
        cHashTable<int>* damage_hashtable;

    }; // class WEAPON_DEF



} // namespace TA3D


#endif // __TA3D_INGAME_WEAPONS_DEF_H__
