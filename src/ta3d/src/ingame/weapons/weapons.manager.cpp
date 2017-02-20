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
#include "weapons.manager.h"
#include <sounds/manager.h>
#include <logs/logs.h>
#include <vfs/file.h>

namespace TA3D
{

	WeaponManager weapon_manager;


	WeaponManager::WeaponManager()
        :nb_weapons(0), weapon()
    {
        cannonshell.init();
    }


	void WeaponManager::init()
    {
        nb_weapons = 0;
        weapon.clear();
        cannonshell.init();
    }


	WeaponManager::~WeaponManager()
    {
        destroy();
		weapon_hashtable.clear();
    }

	void WeaponManager::destroy()
    {
        cannonshell.destroy();
        weapon.clear();
		weapon_hashtable.clear();
        init();
    }



	int WeaponManager::add_weapon(const QString &name)
    {
        ++nb_weapons;
        weapon.resize( nb_weapons );
        weapon[nb_weapons-1].init();
        weapon[nb_weapons-1].internal_name = name;
		weapon[nb_weapons-1].nb_id = short(nb_weapons - 1);

		weapon_hashtable[ToLower(name)] = nb_weapons;

        return nb_weapons-1;
    }


	void WeaponManager::load_tdf(File *file)
    {
        TDFParser parser;
        parser.setSpecialSection("damage");     // We want to get the list of units in damage sections
		parser.loadFromMemory("weapon",file->data(),file->size(),false,false,true);
		file->close();

        for (int i = 0; parser.exists(QString("gadget%1").arg(i)); ++i)
        {
            QString key = QString("gadget%1").arg(i);

            int index = add_weapon( parser.pullAsString(key) );
            key += ".";

            if (index >= 0)
            {
                QString damage = parser.pullAsString( key + "damage" );
                const QStringList &damage_vector = damage.split(",", QString::SkipEmptyParts);
				weapon[index].damage_hashtable.clear();
				for (uint32 i = 1; i < damage_vector.size() ; ++i)        // Since it also contains its name as first element start searching at 1
				{
					if (damage_vector[i] != "default")
                        weapon[index].damage_hashtable[ damage_vector[i] ] = parser.pullAsInt(key + "damage." + damage_vector[i]);
				}
                weapon[index].damage = parser.pullAsInt( key + "damage.default", weapon[index].damage );
                weapon[index].name = parser.pullAsString( key + "name", weapon[index].name );
                weapon[index].weapon_id = short(parser.pullAsInt( key + "id", weapon[index].weapon_id ));
                weapon[index].rendertype = byte(parser.pullAsInt( key + "rendertype", weapon[index].rendertype ));
                weapon[index].ballistic = parser.pullAsBool( key + "ballistic", weapon[index].ballistic );
                weapon[index].turret = parser.pullAsBool( key + "turret", weapon[index].turret );
                weapon[index].noautorange = parser.pullAsBool( key + "noautorange", weapon[index].noautorange );
                weapon[index].range = parser.pullAsInt( key + "range", weapon[index].range );
				weapon[index].time_to_range *= float(weapon[index].range);
                weapon[index].reloadtime = parser.pullAsFloat( key + "reloadtime", weapon[index].reloadtime );
                weapon[index].weaponvelocity = parser.pullAsFloat( key + "weaponvelocity", weapon[index].weaponvelocity * 2.0f ) * 0.5f;
                weapon[index].time_to_range /= weapon[index].weaponvelocity;
                weapon[index].burst = short(parser.pullAsInt( key + "burst", weapon[index].burst ));
                weapon[index].areaofeffect = short(parser.pullAsInt( key + "areaofeffect", weapon[index].areaofeffect ));
                weapon[index].startsmoke = parser.pullAsBool( key + "startsmoke", weapon[index].startsmoke );
                weapon[index].endsmoke = parser.pullAsBool( key + "endsmoke", weapon[index].endsmoke );
                weapon[index].firestarter = byte(parser.pullAsInt( key + "firestarter", weapon[index].firestarter ));
                weapon[index].accuracy = parser.pullAsInt( key + "accuracy", weapon[index].accuracy );
                weapon[index].aimrate = parser.pullAsInt( key + "aimrate", weapon[index].aimrate );
                weapon[index].tolerance = parser.pullAsInt( key + "tolerance", weapon[index].tolerance );
                weapon[index].holdtime = short(parser.pullAsInt( key + "holdtime", weapon[index].holdtime ));
                weapon[index].energypershot = parser.pullAsInt( key + "energypershot", weapon[index].energypershot );
                weapon[index].metalpershot = parser.pullAsInt( key + "metalpershot", weapon[index].metalpershot );
                weapon[index].minbarrelangle = parser.pullAsInt( key + "minbarrelangle", weapon[index].minbarrelangle );
                weapon[index].unitsonly = parser.pullAsBool( key + "unitsonly", weapon[index].unitsonly );
                weapon[index].edgeeffectiveness = parser.pullAsFloat( key + "edgeeffectiveness", weapon[index].edgeeffectiveness );
                weapon[index].lineofsight = parser.pullAsBool( key + "lineofsight", weapon[index].lineofsight );
                {
                    const int c = parser.pullAsInt( key + "color" );
                    weapon[index].color[0] = makecol(pal[c].r,pal[c].g,pal[c].b);
                    weapon[index].color[2] = c;
                    if( weapon[index].color[2] == 232 && weapon[index].color[3] == 234 )
                    {
                        weapon[index].color[0] = makecol( pal[ 180 ].r, pal[ 180 ].g, pal[ 180 ].b );
                        weapon[index].color[1] = makecol( pal[ 212 ].r, pal[ 212 ].g, pal[ 212 ].b );
                        weapon[index].color[2] = 180;
                        weapon[index].color[3] = 212;
                    }
                }
                {
                    const int c = parser.pullAsInt( key + "color2" );
                    weapon[index].color[1] = makecol(pal[c].r,pal[c].g,pal[c].b);
                    weapon[index].color[3] = c;
                    if( weapon[index].color[2] == 232 && weapon[index].color[3] == 234 ) {
                        weapon[index].color[0] = makecol( pal[ 180 ].r, pal[ 180 ].g, pal[ 180 ].b );
                        weapon[index].color[1] = makecol( pal[ 212 ].r, pal[ 212 ].g, pal[ 212 ].b );
                        weapon[index].color[2] = 180;
                        weapon[index].color[3] = 212;
                    }
                }
                weapon[index].burstrate = parser.pullAsFloat( key + "burstrate", weapon[index].burstrate );
                weapon[index].duration = parser.pullAsFloat( key + "duration", weapon[index].duration );
                weapon[index].beamweapon = parser.pullAsBool( key + "beamweapon", weapon[index].beamweapon );
                weapon[index].startvelocity = parser.pullAsFloat( key + "startvelocity", weapon[index].startvelocity * 2.0f ) * 0.5f;
                weapon[index].weapontimer = parser.pullAsFloat( key + "weapontimer", weapon[index].weapontimer );
                weapon[index].weaponacceleration = parser.pullAsFloat( key + "weaponacceleration", weapon[index].weaponacceleration * 2.0f ) * 0.5f;
                weapon[index].turnrate = parser.pullAsInt( key + "turnrate", weapon[index].turnrate );
                weapon[index].model = model_manager.get_model( parser.pullAsString( key + "model" ) );
                weapon[index].smokedelay = parser.pullAsFloat( key + "smokedelay", weapon[index].smokedelay );
                weapon[index].guidance = parser.pullAsInt( key + "guidance", weapon[index].guidance );
                weapon[index].tracks = parser.pullAsBool( key + "tracks", weapon[index].tracks );
                weapon[index].selfprop = parser.pullAsBool( key + "selfprop", weapon[index].selfprop );
                weapon[index].waterweapon = parser.pullAsBool( key + "waterweapon", weapon[index].waterweapon );
                weapon[index].smoketrail = parser.pullAsBool( key + "smoketrail", weapon[index].smoketrail );
                weapon[index].flighttime = short(parser.pullAsInt( key + "flighttime", weapon[index].flighttime ));
                weapon[index].coverage = parser.pullAsFloat( key + "coverage", weapon[index].coverage * 2.0f ) * 0.5f;
                weapon[index].vlaunch = parser.pullAsBool( key + "vlaunch", weapon[index].vlaunch );
                weapon[index].paralyzer = parser.pullAsBool( key + "paralyzer", weapon[index].paralyzer );
                weapon[index].stockpile = parser.pullAsBool( key + "stockpile", weapon[index].stockpile );
                weapon[index].targetable = parser.pullAsBool( key + "targetable", weapon[index].targetable );
                weapon[index].interceptor = parser.pullAsBool( key + "interceptor", weapon[index].interceptor );
                weapon[index].commandfire = parser.pullAsBool( key + "commandfire", weapon[index].commandfire );
                weapon[index].cruise = parser.pullAsBool( key + "cruise", weapon[index].cruise );
                weapon[index].propeller = parser.pullAsBool( key + "propeller", weapon[index].propeller );
                weapon[index].twophase = parser.pullAsBool( key + "twophase", weapon[index].twophase );
                weapon[index].dropped = parser.pullAsBool( key + "dropped", weapon[index].dropped );
                weapon[index].burnblow = parser.pullAsBool( key + "burnblow", weapon[index].burnblow );
                weapon[index].toairweapon = parser.pullAsBool( key + "toairweapon", weapon[index].toairweapon );
                weapon[index].noexplode = parser.pullAsBool( key + "noexplode", weapon[index].noexplode );
                weapon[index].shakemagnitude = short(parser.pullAsInt( key + "shakemagnitude", weapon[index].shakemagnitude ));
                weapon[index].metal = parser.pullAsInt( key + "metal", weapon[index].metal );
                weapon[index].energy = parser.pullAsInt( key + "energy", weapon[index].energy );
                weapon[index].shakeduration = parser.pullAsFloat( key + "shakeduration", weapon[index].shakeduration );
                weapon[index].waterexplosiongaf = parser.pullAsString( key + "waterexplosiongaf", weapon[index].waterexplosiongaf );
                weapon[index].waterexplosionart = parser.pullAsString( key + "waterexplosionart", weapon[index].waterexplosionart );
                weapon[index].lavaexplosiongaf = parser.pullAsString( key + "lavaexplosiongaf", weapon[index].lavaexplosiongaf );
                weapon[index].lavaexplosionart = parser.pullAsString( key + "lavaexplosionart", weapon[index].lavaexplosionart );
                weapon[index].explosiongaf = parser.pullAsString( key + "explosiongaf", weapon[index].explosiongaf );
                weapon[index].explosionart = parser.pullAsString( key + "explosionart", weapon[index].explosionart );
                weapon[index].soundtrigger = parser.pullAsString( key + "soundtrigger", weapon[index].soundtrigger );
                sound_manager->loadSound( weapon[index].soundtrigger , true );
                weapon[index].soundhit = parser.pullAsString( key + "soundhit", weapon[index].soundhit );
                sound_manager->loadSound( weapon[index].soundhit , true );
                weapon[index].soundstart = parser.pullAsString( key + "soundstart", weapon[index].soundstart );
                sound_manager->loadSound( weapon[index].soundstart , true );
                weapon[index].soundwater = parser.pullAsString( key + "soundwater", weapon[index].soundwater );
                sound_manager->loadSound(weapon[index].soundwater , true);

				if (weapon[index].rendertype == RENDER_TYPE_LASER)
				{
                    weapon[index].laserTex1 = gfx->load_texture(parser.pullAsString( key + "lasertexture1", "gfx/weapons/laser1normal.png" ), FILTER_TRILINEAR, NULL, NULL, true, 0);
                    weapon[index].laserTex2 = gfx->load_texture(parser.pullAsString( key + "lasertexture2", "gfx/weapons/laser2normal.png" ), FILTER_TRILINEAR, NULL, NULL, true, 0);
				}
            }
        }
    }



} // namespace TA3D
