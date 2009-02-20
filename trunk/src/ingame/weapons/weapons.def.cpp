
#include "weapons.def.h"
#include "../../fbi.h"
#include <vector>


namespace TA3D
{


    WEAPON_DEF::WEAPON_DEF()
        : damage_hashtable(128)
    {
        init();
    }


    WEAPON_DEF::~WEAPON_DEF()
    {
        destroy();
        damage_hashtable.emptyHashTable();
    }

    void WEAPON_DEF::init()
    {
        damage_hashtable.emptyHashTable();

        soundstart.clear();
        soundhit.clear();
        soundwater.clear();
        soundtrigger.clear();

        toairweapon = false;
        time_to_range=2.0f;
        burnblow=false;
        coverage=0.0f;
        interceptor=false;
        metal=0;
        energy=0;
        waterweapon=false;
        explosiongaf.clear();
        explosionart.clear();
        waterexplosiongaf.clear();
        waterexplosionart.clear();
        lavaexplosiongaf.clear();
        lavaexplosionart.clear();
        dropped=false;
        nb_id=0;
        burst=1;
        noexplode=false;
        weapon_id=0;			// Numéro identifiant l'arme
        internal_name.clear();	// Nom interne de l'arme
        name.clear();			// Nom de l'arme
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
        paralyzer=false;
    }


    void WEAPON_DEF::destroy()
    {
        soundstart.clear();
        soundhit.clear();
        soundwater.clear();
        soundtrigger.clear();
        internal_name.clear();
        name.clear();
        init(); // TODO Should be removed
    }


    uint32 WEAPON_DEF::get_damage_for_unit(const String& uname)
    {
        if (damage_hashtable.exists(String::ToLower(uname)))
            return damage_hashtable.find(String::ToLower(uname));
        int unit_type = unit_manager.get_unit_index(uname);
        if (unit_type >= 0 && !unit_manager.unit_type[unit_type]->categories.empty())
        {
            String::Vector::const_iterator i = (unit_manager.unit_type[unit_type]->categories).begin();
            for (; (unit_manager.unit_type[unit_type]->categories).end() != i; ++i)
                if (damage_hashtable.exists(*i))
                    return damage_hashtable.find(*i);
        }
        return damage;
    }



} // namespace TA3D
