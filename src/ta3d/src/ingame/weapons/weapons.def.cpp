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

#include "weapons.def.h"
#include <fbi.h>
#include <vector>


namespace TA3D
{


	WeaponDef::WeaponDef()
    {
        init();
    }


	WeaponDef::~WeaponDef()
    {
        destroy();
		damage_hashtable.clear();
    }

	void WeaponDef::init()
    {
		laserTex1 = 0;
		laserTex2 = 0;

		damage_hashtable.clear();

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


	void WeaponDef::destroy()
    {
        laserTex1 = nullptr;
        laserTex2 = nullptr;
		soundstart.clear();
        soundhit.clear();
        soundwater.clear();
        soundtrigger.clear();
        internal_name.clear();
        name.clear();
        init(); // TODO Should be removed
    }


	uint32 WeaponDef::get_damage_for_unit(const QString& uname) const
    {
		HashMap<int>::Dense::const_iterator it = damage_hashtable.find(ToLower(uname));
		if (it != damage_hashtable.end())
			return *it;
        int unit_type = unit_manager.get_unit_index(uname);
        if (unit_type >= 0 && !unit_manager.unit_type[unit_type]->categories.empty())
        {
            QStringList::const_iterator i = (unit_manager.unit_type[unit_type]->categories).begin();
            for (; (unit_manager.unit_type[unit_type]->categories).end() != i; ++i)
			{
				it = damage_hashtable.find(*i);
				if (it != damage_hashtable.end())
					return *it;
			}
        }
        return damage;
    }



} // namespace TA3D
