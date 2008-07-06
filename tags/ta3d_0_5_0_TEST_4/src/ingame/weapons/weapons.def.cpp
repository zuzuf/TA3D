
#include "weapons.def.h"
#include "../../fbi.h"
#include <vector>


namespace TA3D
{


    WEAPON_DEF::WEAPON_DEF()
        :damage_hashtable(NULL)
    {
        init();
    }


    WEAPON_DEF::~WEAPON_DEF()
    {
        destroy();
        delete damage_hashtable; // TODO Should be removed
    }

    void WEAPON_DEF::init()
    {
        damage_hashtable = new cHashTable<int>(128);

        soundstart = NULL;
        soundhit = NULL;
        soundwater = NULL;
        soundtrigger = NULL;

        toairweapon = false;
        time_to_range=2.0f;
        burnblow=false;
        coverage=0.0f;
        interceptor=false;
        metal=0;
        energy=0;
        waterweapon=false;
        explosiongaf=NULL;
        explosionart=NULL;
        waterexplosiongaf=NULL;
        waterexplosionart=NULL;
        lavaexplosiongaf=NULL;
        lavaexplosionart=NULL;
        dropped=false;
        nb_id=0;
        burst=1;
        noexplode=false;
        weapon_id=0;			// Numéro identifiant l'arme
        internal_name=NULL;		// Nom interne de l'arme
        name=NULL;				// Nom de l'arme
        rendertype=RENDER_TYPE_NONE;
        ballistic=false;
        turret=false;
        range=100;				// portée
        reloadtime=0.0f;		// temps de rechargement
        weaponvelocity=0.0f;
        areaofeffect=10;		// zone d'effet
        startsmoke=false;
        endsmoke=false;
        damage=100;				// Dégats causés par l'arme
        firestarter=0;
        accuracy=0;
        aimrate=0;
        tolerance=0;
        holdtime=0;
        energypershot=0;
        metalpershot=0;
        minbarrelangle=0;
        unitsonly=false;
        edgeeffectiveness=1.0f;
        lineofsight=false;
        color[0]=0;
        color[1]=0xFFFFFF;
        color[2]=0;
        color[3]=0;
        burstrate=1.0f;
        duration=0.1f;
        beamweapon=false;
        startvelocity=0;			// Pour les missiles
        weapontimer=0.0f;			// Pour les missiles
        weaponacceleration=0.0f;	// Pour les missiles
        turnrate=1;					// Pour les missiles
        model=NULL;					// Modèle 3D
        smokedelay=0.1f;
        guidance=false;				// Guidage
        tracks=false;
        selfprop=false;
        smoketrail=false;			// Laisse de la fumée en passant
        noautorange=false;
        flighttime=10;
        vlaunch=false;
        stockpile=false;			// Nécessite de fabriquer des munitions / need to make ammo??
        targetable=false;			// On peut viser
        commandfire=false;			// ne tire pas seul
        cruise=false;
        propeller=false;
        twophase=false;
        shakemagnitude=0;
        shakeduration=0.1f;
    }


    void WEAPON_DEF::destroy()
    {
        if(soundstart)
            free(soundstart);
        if(soundhit)
            free(soundhit);
        if(soundwater)
            free(soundwater);
        if(soundtrigger)
            free(soundtrigger);
        if(internal_name)
            free(internal_name);
        if(name)
            free(name);
        if (damage_hashtable)
            delete damage_hashtable;
        init(); // TODO Should be removed
    }


    uint32 WEAPON_DEF::get_damage_for_unit(const String& uname) const
    {
        uint32 dmg = damage_hashtable->Find(Lowercase(uname));
        if(dmg)
            return dmg;
        int unit_type = unit_manager.get_unit_index(uname.c_str());
        if ( unit_type >= 0 && unit_manager.unit_type[ unit_type ].categories)
        {
            std::vector<String>::const_iterator i = (unit_manager.unit_type[ unit_type ].categories)->begin();
            for (; (unit_manager.unit_type[ unit_type ].categories)->end() != i; ++i)
            {
                dmg = damage_hashtable->Find(*i);
                if (dmg)
                    return dmg;
            }
        }
        return damage;
    }



} // namespace TA3D
