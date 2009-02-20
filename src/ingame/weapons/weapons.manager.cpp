#include "weapons.manager.h"
#include "../../sounds/manager.h"
#include "../../logs/logs.h"


namespace TA3D
{

    WEAPON_MANAGER weapon_manager;


    WEAPON_MANAGER::WEAPON_MANAGER()
        :nb_weapons(0), weapon()
    {
        cannonshell.init();
    }


    void WEAPON_MANAGER::init()
    {
        nb_weapons = 0;
        weapon.clear();
        cannonshell.init();
    }


    WEAPON_MANAGER::~WEAPON_MANAGER()
    {
        destroy();
        weapon_hashtable.emptyHashTable();
    }

    void WEAPON_MANAGER::destroy()
    {
        cannonshell.destroy();
        weapon.clear();
        weapon_hashtable.emptyHashTable();
        weapon_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
        init();
    }



    int WEAPON_MANAGER::add_weapon(const String &name)
    {
        ++nb_weapons;
        weapon.resize( nb_weapons );
        weapon[nb_weapons-1].init();
        weapon[nb_weapons-1].internal_name = name;
        weapon[nb_weapons-1].nb_id = nb_weapons-1;

        weapon_hashtable.insert(String::ToLower(name), nb_weapons);

        return nb_weapons-1;
    }


    void WEAPON_MANAGER::load_tdf(char *data, const int size)
    {
        TDFParser parser;
        parser.setSpecialSection("damage");     // We want to get the list of units in damage sections
        parser.loadFromMemory("weapon",data,size,false,false,true);

        for(int i = 0 ; parser.exists( format("gadget%d", i) ) ; i++)
        {
            String key = format("gadget%d", i);

            int index = add_weapon( parser.pullAsString(key) );
            key << ".";

            if (index >= 0)
            {
                String damage = parser.pullAsString( key + "damage" );
                String::Vector damage_vector;
                damage.split(damage_vector, ",");
                weapon[index].damage_hashtable.emptyHashTable();
                if (damage_vector.size() > 1)
                    weapon[index].damage_hashtable.initTable( damage_vector.size() );
                for(int i = 1 ; i < damage_vector.size() ; i++)        // Since it also contains its name as first element start searching at 1
                    if (damage_vector[i] != "default")
                        weapon[index].damage_hashtable.insert( damage_vector[i], parser.pullAsInt(key + damage_vector[i]) );
                weapon[index].damage = parser.pullAsInt( key + "damage.default", weapon[index].damage );
                weapon[index].name = parser.pullAsString( key + "name", weapon[index].name );
                weapon[index].weapon_id = parser.pullAsInt( key + "id", weapon[index].weapon_id );
                weapon[index].rendertype = parser.pullAsInt( key + "rendertype", weapon[index].rendertype );
                weapon[index].ballistic = parser.pullAsBool( key + "ballistic", weapon[index].ballistic );
                weapon[index].turret = parser.pullAsBool( key + "turret", weapon[index].turret );
                weapon[index].noautorange = parser.pullAsBool( key + "noautorange", weapon[index].noautorange );
                weapon[index].range = parser.pullAsInt( key + "range", weapon[index].range );
                weapon[index].time_to_range *= weapon[index].range;
                weapon[index].reloadtime = parser.pullAsFloat( key + "reloadtime", weapon[index].reloadtime );
                weapon[index].weaponvelocity = parser.pullAsInt( key + "weaponvelocity", weapon[index].weaponvelocity * 2.0f ) * 0.5f;
                weapon[index].time_to_range /= weapon[index].weaponvelocity;
                weapon[index].burst = parser.pullAsInt( key + "burst", weapon[index].burst );
                weapon[index].areaofeffect = parser.pullAsInt( key + "areaofeffect", weapon[index].areaofeffect );
                weapon[index].startsmoke = parser.pullAsBool( key + "startsmoke", weapon[index].startsmoke );
                weapon[index].endsmoke = parser.pullAsBool( key + "endsmoke", weapon[index].endsmoke );
                weapon[index].firestarter = parser.pullAsInt( key + "firestarter", weapon[index].firestarter );
                weapon[index].accuracy = parser.pullAsInt( key + "accuracy", weapon[index].accuracy );
                weapon[index].aimrate = parser.pullAsInt( key + "aimrate", weapon[index].aimrate );
                weapon[index].tolerance = parser.pullAsInt( key + "tolerance", weapon[index].tolerance );
                weapon[index].holdtime = parser.pullAsInt( key + "holdtime", weapon[index].holdtime );
                weapon[index].energypershot = parser.pullAsInt( key + "energypershot", weapon[index].energypershot );
                weapon[index].metalpershot = parser.pullAsInt( key + "metalpershot", weapon[index].metalpershot );
                weapon[index].minbarrelangle = parser.pullAsInt( key + "minbarrelangle", weapon[index].minbarrelangle );
                weapon[index].unitsonly = parser.pullAsBool( key + "unitsonly", weapon[index].unitsonly );
                weapon[index].edgeeffectiveness = parser.pullAsFloat( key + "edgeeffectiveness", weapon[index].edgeeffectiveness );
                weapon[index].lineofsight = parser.pullAsBool( key + "lineofsight", weapon[index].lineofsight );
                {
                    int c = parser.pullAsInt( key + "color" );
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
                    int c = parser.pullAsInt( key + "color2" );
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
                weapon[index].startvelocity = parser.pullAsInt( key + "startvelocity", weapon[index].startvelocity * 2.0f ) * 0.5f;
                weapon[index].weapontimer = parser.pullAsFloat( key + "weapontimer", weapon[index].weapontimer );
                weapon[index].weaponacceleration = parser.pullAsInt( key + "weaponacceleration", weapon[index].weaponacceleration * 2.0f ) * 0.5f;
                weapon[index].turnrate = parser.pullAsInt( key + "turnrate", weapon[index].turnrate );
                weapon[index].model = model_manager.get_model( parser.pullAsString( key + "model" ) );
                weapon[index].smokedelay = parser.pullAsFloat( key + "smokedelay", weapon[index].smokedelay );
                weapon[index].guidance = parser.pullAsInt( key + "guidance", weapon[index].guidance );
                weapon[index].tracks = parser.pullAsBool( key + "tracks", weapon[index].tracks );
                weapon[index].selfprop = parser.pullAsBool( key + "selfprop", weapon[index].selfprop );
                weapon[index].waterweapon = parser.pullAsBool( key + "waterweapon", weapon[index].waterweapon );
                weapon[index].smoketrail = parser.pullAsBool( key + "smoketrail", weapon[index].smoketrail );
                weapon[index].flighttime = parser.pullAsInt( key + "flighttime", weapon[index].flighttime );
                weapon[index].coverage = parser.pullAsInt( key + "coverage", weapon[index].coverage * 2.0f ) * 0.5f;
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
                weapon[index].shakemagnitude = parser.pullAsInt( key + "shakemagnitude", weapon[index].shakemagnitude );
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
            }
        }

//        char *pos=data;
//        char *ligne=NULL;
//        int nb=0;
//        int index=0;
//        char *limit = data + size;
//        char *f;
//        int n_pos=0;
//        do {
//            do
//            {
//                ++nb;
//                if(ligne)
//                    delete[] ligne;
//                ligne = get_line(pos);
//                String lwr_ligne = String::ToLower(ligne);
//#warning FIXME: very UGLY strlwr replacement here
//                memcpy(ligne, lwr_ligne.c_str(), lwr_ligne.size() + 1);
////                strlwr(ligne);
//                while (pos[0]!=0 && pos[0]!=13 && pos[0]!=10)
//                {
//                    pos++;
//                    n_pos++;
//                }
//                while (pos[0]==13 || pos[0]==10)
//                {
//                    ++pos;
//                    ++n_pos;
//                }
//
//                if(strstr(ligne,"//"))
//                    *(strstr(ligne,"//")) = 0;
//                else if( strstr(ligne,"/*") )
//                    *(strstr(ligne,"/*")) = 0;
//                else if( strstr(ligne,"{"))		// Skip comments
//                    *(strstr(ligne,"{")) = 0;
//
//                if(ligne[0]=='[')
//                {
//                    if(strstr(ligne,"]"))
//                        *(strstr(ligne,"]"))=0;
//                    index=add_weapon(ligne+1);
//                }
//                else
//                    if((f=strstr(ligne,"[damage]"))) // Si on a trouvé un paragraphe référant aux dégats
//                    {
//                        do 	// Parcour le paragraphe
//                        {
//                            nb++;
//                            if(ligne)
//                                delete[] ligne;
//                            ligne = get_line(pos);
//                            lwr_ligne = String::ToLower(ligne);
//#warning FIXME: very UGLY strlwr replacement here
//                            memcpy(ligne, lwr_ligne.c_str(), lwr_ligne.size() + 1);
////                            strlwr(ligne);
//                            while(pos[0]!=0 && pos[0]!=13 && pos[0]!=10)	pos++;
//                            while(pos[0]==13 || pos[0]==10)	pos++;
//
//                            if((f=strstr(ligne,"default=")))
//                            {
//                                if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                                weapon[index].damage=atoi(f+8);
//                            }
//                            else
//                                if(( f = strstr(ligne, "=" ))) 	// Read specific damage data
//                                {
//                                    String unit_name = String( ligne );
//                                    unit_name.trim();
//                                    unit_name.resize( unit_name.find( "=" ) );
//                                    int dmg = atoi( f + 1 );
//                                    weapon[index].damage_hashtable.insert(unit_name, dmg);
//                                }
//                        } while(strstr(ligne,"}")==NULL && nb<1000 && pos<limit);
//                    }
//                    else if((f=strstr(ligne,"name="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].name = f+5;
//                    }
//                    else if((f=strstr(ligne,"id="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].weapon_id = atoi(f+3);
//                    }
//                    else if((f=strstr(ligne,"rendertype="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].rendertype = atoi(f+11);
//                    }
//                    else if((f=strstr(ligne,"ballistic="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].ballistic = (f[10]=='1');
//                    }
//                    else if((f=strstr(ligne,"turret="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].turret = (f[7]=='1');
//                    }
//                    else if((f=strstr(ligne,"noautorange="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].noautorange = (f[12]=='1');
//                    }
//                    else if((f=strstr(ligne,"range="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].range = atoi(f+6);
//                        weapon[index].time_to_range*=weapon[index].range;
//                    }
//                    else if((f=strstr(ligne,"reloadtime="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].reloadtime = atof(f+11);
//                    }
//                    else if((f=strstr(ligne,"weaponvelocity="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].weaponvelocity = atoi(f+15)*0.5f;
//                        weapon[index].time_to_range /= weapon[index].weaponvelocity;
//                    }
//                    else if((f=strstr(ligne,"burst="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].burst = atoi(f+6);
//                    }
//                    else if((f=strstr(ligne,"areaofeffect="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].areaofeffect = atoi(f+13);
//                    }
//                    else if((f=strstr(ligne,"startsmoke="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].startsmoke = (f[11]=='1');
//                    }
//                    else if((f=strstr(ligne,"endsmoke="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].endsmoke = (f[9]=='1');
//                    }
//                    else if((f=strstr(ligne,"firestarter="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].firestarter = atoi(f+12);
//                    }
//                    else if((f=strstr(ligne,"accuracy="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].accuracy = atoi(f+9);
//                    }
//                    else if((f=strstr(ligne,"aimrate="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].aimrate = atoi(f+8);
//                    }
//                    else if((f=strstr(ligne,"tolerance="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].tolerance = atoi(f+9);
//                    }
//                    else if((f=strstr(ligne,"holdtime="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].holdtime = atoi(f+8);
//                    }
//                    else if((f=strstr(ligne,"energypershot="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].energypershot = atoi(f+14);
//                    }
//                    else if((f=strstr(ligne,"metalpershot="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].metalpershot = atoi(f+13);
//                    }
//                    else if((f=strstr(ligne,"minbarrelangle="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].minbarrelangle = atoi(f+15);
//                    }
//                    else if((f=strstr(ligne,"unitsonly="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].unitsonly = (f[10]=='1');
//                    }
//                    else if((f=strstr(ligne,"edgeeffectiveness="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].edgeeffectiveness = atof(f+18);
//                    }
//                    else if((f=strstr(ligne,"lineofsight="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].lineofsight = (f[12]=='1');
//                    }
//                    else if((f=strstr(ligne,"color=")))
//                    {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        int c=atoi(f+6);
//                        weapon[index].color[0] = makecol(pal[c].r,pal[c].g,pal[c].b);
//                        weapon[index].color[2] = c;
//                        if( weapon[index].color[2] == 232 && weapon[index].color[3] == 234 )
//                        {
//                            weapon[index].color[0] = makecol( pal[ 180 ].r, pal[ 180 ].g, pal[ 180 ].b );
//                            weapon[index].color[1] = makecol( pal[ 212 ].r, pal[ 212 ].g, pal[ 212 ].b );
//                            weapon[index].color[2] = 180;
//                            weapon[index].color[3] = 212;
//                        }
//                    }
//                    else if((f=strstr(ligne,"color2=")))
//                    {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        int c=atoi(f+7);
//                        weapon[index].color[1] = makecol(pal[c].r,pal[c].g,pal[c].b);
//                        weapon[index].color[3] = c;
//                        if( weapon[index].color[2] == 232 && weapon[index].color[3] == 234 ) {
//                            weapon[index].color[0] = makecol( pal[ 180 ].r, pal[ 180 ].g, pal[ 180 ].b );
//                            weapon[index].color[1] = makecol( pal[ 212 ].r, pal[ 212 ].g, pal[ 212 ].b );
//                            weapon[index].color[2] = 180;
//                            weapon[index].color[3] = 212;
//                        }
//                    }
//                    else if((f=strstr(ligne,"burstrate="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].burstrate=atof(f+10);
//                    }
//                    else if((f=strstr(ligne,"duration="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].duration=atof(f+9);
//                    }
//                    else if((f=strstr(ligne,"beamweapon="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].beamweapon=(f[11]=='1');
//                    }
//                    else if((f=strstr(ligne,"startvelocity="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].startvelocity=atoi(f+14)*0.5f;
//                    }
//                    else if((f=strstr(ligne,"weapontimer="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].weapontimer=atof(f+12);
//                    }
//                    else if((f=strstr(ligne,"weaponacceleration="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].weaponacceleration=atoi(f+19)*0.5f;
//                    }
//                    else if((f=strstr(ligne,"turnrate="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].turnrate=atoi(f+9);
//                    }
//                    else if((f=strstr(ligne,"model="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].model=model_manager.get_model(f+6);
//                    }
//                    else if((f=strstr(ligne,"smokedelay="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].smokedelay=atof(f+11);
//                    }
//                    else if((f=strstr(ligne,"guidance="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].guidance=(f[9]=='1');
//                    }
//                    else if((f=strstr(ligne,"tracks="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].tracks=(f[7]=='1');
//                    }
//                    else if((f=strstr(ligne,"selfprop="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].selfprop=(f[9]=='1');
//                    }
//                    else if((f=strstr(ligne,"waterweapon="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].waterweapon=(f[12]=='1');
//                    }
//                    else if((f=strstr(ligne,"smoketrail="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].smoketrail=(f[11]=='1');
//                    }
//                    else if((f=strstr(ligne,"flighttime="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].flighttime=atoi(f+11);
//                    }
//                    else if((f=strstr(ligne,"coverage="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].coverage=atoi(f+9)*0.5f;
//                    }
//                    else if((f=strstr(ligne,"vlaunch="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].vlaunch=(f[8]=='1');
//                    }
//                    else if((f=strstr(ligne,"paralyzer="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].paralyzer=(f[10]=='1');
//                    }
//                    else if((f=strstr(ligne,"stockpile="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].stockpile=(f[10]=='1');
//                    }
//                    else if((f=strstr(ligne,"targetable="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].targetable=(f[11]=='1');
//                    }
//                    else if((f=strstr(ligne,"interceptor="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].interceptor=(f[12]=='1');
//                    }
//                    else if((f=strstr(ligne,"commandfire="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].commandfire=(f[12]=='1');
//                    }
//                    else if((f=strstr(ligne,"cruise="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].cruise=(f[7]=='1');
//                    }
//                    else if((f=strstr(ligne,"propeller="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].propeller=(f[10]=='1');
//                    }
//                    else if((f=strstr(ligne,"twophase="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].twophase=(f[9]=='1');
//                    }
//                    else if((f=strstr(ligne,"dropped="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].dropped=(f[8]=='1');
//                    }
//                    else if((f=strstr(ligne,"burnblow="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].burnblow=(f[9]=='1');
//                    }
//                    else if((f=strstr(ligne,"toairweapon="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].toairweapon=(f[12]=='1');
//                    }
//                    else if((f=strstr(ligne,"noexplode="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].noexplode=(f[10]=='1');
//                    }
//                    else if((f=strstr(ligne,"shakemagnitude="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].shakemagnitude=atoi(f+15);
//                    }
//                    else if((f=strstr(ligne,"metal="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].metal=atoi(f+6);
//                    }
//                    else if((f=strstr(ligne,"energy="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].energy=atoi(f+7);
//                    }
//                    else if((f=strstr(ligne,"shakeduration="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].shakeduration=atof(f+14);
//                    }
//                    else if((f=strstr(ligne,"waterexplosiongaf="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].waterexplosiongaf = f+18;
//                    }
//                    else if((f=strstr(ligne,"waterexplosionart="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].waterexplosionart = f+18;
//                    }
//                    else if((f=strstr(ligne,"lavaexplosiongaf="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].lavaexplosiongaf = f+17;
//                    }
//                    else if((f=strstr(ligne,"lavaexplosionart="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].lavaexplosionart = f+17;
//                    }
//                    else if((f=strstr(ligne,"explosiongaf="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].explosiongaf = f+13;
//                    }
//                    else if((f=strstr(ligne,"explosionart="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].explosionart = f+13;
//                    }
//                    else if((f=strstr(ligne,"soundtrigger="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].soundtrigger = f+13;
//                        sound_manager->loadSound( weapon[index].soundtrigger , true );
//                    }
//                    else if((f=strstr(ligne,"soundhit="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].soundhit = f+9;
//                        sound_manager->loadSound( weapon[index].soundhit , true );
//                    }
//                    else if((f=strstr(ligne,"soundstart="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].soundstart = f+11;
//                        sound_manager->loadSound( weapon[index].soundstart , true );
//                    }
//                    else if((f=strstr(ligne,"soundwater="))) {
//                        if((strstr(ligne,";")))	*(strstr(ligne,";"))=0;
//                        weapon[index].soundwater = f+11;
//                        sound_manager->loadSound(weapon[index].soundwater , true);
//                    }
//                    else {
//                        if(strlen(ligne)>1 && strstr(ligne,"{")==NULL && strstr(ligne,"}")==NULL)
//                            LOG_WARNING("Unknown weapon: " << ligne);
//                    }
//
//            } while(strstr(ligne,"}")==NULL && pos<limit);
//
//            delete[] ligne;
//            ligne = NULL;
//
//        } while (pos<limit);
    }



} // namespace TA3D
