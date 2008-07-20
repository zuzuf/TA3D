#include "weapons.h"
#include "../../tdf.h"
#include "../../UnitEngine.h"
#include "../../gfx/fx.h"
#include "../../misc/camera.h"
#include <list>
#include "../../misc/math.h"
#include "../../sounds/manager.h"



namespace TA3D
{


    void WEAPON::init()
    {
        last_timestamp = 0;
        just_explode = false;
        damage = -1;
        visible=true;
        idx=0;
        phase=1;
        a_time=0.0f;
        f_time=0.0f;
        shooter_idx=-1;
        anim_sprite=0;
        weapon_id=-1;		// Non défini
        Pos.x=Pos.y=Pos.z=0.0f;
        Ac=V=Pos;
        target_pos=Pos;
        target=-1;			// Pas de cible
        stime=0.0f;
        killtime=0.0f;
        dying=false;
        smoke_time=0.0f;
        owner=0;
        local = true;
    }




    void WEAPON::move(const float dt,MAP *map)				// Anime les armes
    {
        if( weapon_id < 0 )
            return;

        smoke_time+=dt;
        f_time-=dt;
        a_time+=dt;
        VECTOR A;
        A.x=A.y=A.z=0.0f;
        if(weapon_manager.weapon[weapon_id].twophase && phase==1)
        {
            if(!dying && a_time>=weapon_manager.weapon[weapon_id].weapontimer) 	// Entre dans la seconde phase
            {
                phase=2;
                f_time=weapon_manager.weapon[weapon_id].flighttime;
                stime=0.0f;
            }
            if(weapon_manager.weapon[weapon_id].vlaunch)
            {
                V.x=0.0f;
                V.z=0.0f;
                if(V.y<weapon_manager.weapon[weapon_id].weaponvelocity)
                    A.y=weapon_manager.weapon[weapon_id].weaponacceleration;
                else
                    V.y=weapon_manager.weapon[weapon_id].weaponvelocity;
            }
        }
        if(!dying && weapon_manager.weapon[weapon_id].selfprop && f_time<=0.0f && ((weapon_manager.weapon[weapon_id].twophase && phase==2) || !weapon_manager.weapon[weapon_id].twophase))	dying=true;
        if(weapon_manager.weapon[weapon_id].smoketrail && weapon_manager.weapon[weapon_id].smokedelay<smoke_time) // Trainée de fumée des missiles
        {
            smoke_time=0.0f;
            if(visible)
                particle_engine.make_smoke(Pos,0,1,0.0f,-1.0f, -2.0f, 0.3f);
        }

        VECTOR hit_vec;
        VECTOR OPos = Pos;

        float h=map->get_unit_h(Pos.x,Pos.z);
        if(dying)
            killtime-=dt;
        else
        {
            if(weapon_manager.weapon[weapon_id].lineofsight)
            {
                // TODO Damien Mssing code here ?
            }
            else
                if(weapon_manager.weapon[weapon_id].ballistic || weapon_manager.weapon[weapon_id].dropped)		// Arme soumise à la gravité
                    A.y-=map->ota_data.gravity;

            if(weapon_manager.weapon[weapon_id].guidance && ((weapon_manager.weapon[weapon_id].twophase && phase==2) || !weapon_manager.weapon[weapon_id].twophase)
               && ((weapon_manager.weapon[weapon_id].waterweapon && Pos.y<map->sealvl) || !weapon_manager.weapon[weapon_id].waterweapon))// Traque sa cible
            {
                float speed = V.norm();
                if(weapon_manager.weapon[weapon_id].tracks && target>=0)
                {
                    VECTOR target_V;
                    if(weapon_manager.weapon[weapon_id].interceptor && target <= weapons.nb_weapon && weapons.weapon[target].weapon_id!=-1)
                    {
                        target_pos = weapons.weapon[target].Pos;
                        target_V = weapons.weapon[target].V;
                    }
                    else if(!weapon_manager.weapon[weapon_id].interceptor && target<units.max_unit && (units.unit[target].flags & 1) ) {		// Met à jour les coordonnées de la cible
                        target_pos = units.unit[target].Pos;
                        target_V = units.unit[target].V;
                    }
                    else
                        target = -1;
                    float speed = V.sq();
                    float target_speed = target_V.sq();
                    if( speed > 0.0f && target_speed > 0.0f ) {					// Make it aim better
                        float time_to_hit = (target_pos - Pos).sq() / speed;
                        target_pos = target_pos + sqrt( time_to_hit / target_speed ) * target_V;
                    }
                }
                if(target_pos.y<map->sealvl && !weapon_manager.weapon[weapon_id].waterweapon)
                    target_pos.y=map->sealvl;
                VECTOR Dir=target_pos-Pos;
                Dir.unit();
                VECTOR I,J,K;			// Crée un trièdre
                I=V;
                I.unit();
                J=I*Dir;
                K=J*I;
                if( speed < weapon_manager.weapon[weapon_id].weaponvelocity )
                {
                    if( speed > 0.0f )
                        A = A + weapon_manager.weapon[weapon_id].weaponacceleration * I;
                    else
                        A = A + weapon_manager.weapon[weapon_id].weaponacceleration * Dir;
                }
                else 
                    if( speed > 0.5f * weapon_manager.weapon[weapon_id].weaponvelocity && (V % Dir) < 0.0f )					// Can slow down if needed
                        A = A - weapon_manager.weapon[weapon_id].weaponacceleration * I;

                float rotate=dt*weapon_manager.weapon[weapon_id].turnrate*TA2RAD;
                V=speed*(cos(rotate)*I+sin(rotate)*K);
            }
            Pos=Pos+dt*(V+dt*(0.33333333f*A+0.16666666667f*Ac));
            V=V+dt*0.5f*(A+Ac);
            Ac=A;
            stime+=dt;
        }

        float length = ((VECTOR)(OPos - Pos)).norm();
        if(!dying)
        {
            hit_vec = map->hit(Pos,V,!weapon_manager.weapon[weapon_id].waterweapon,length);

            if( weapon_manager.weapon[weapon_id].waterweapon && ( Pos.y > map->sealvl && V.y > 0.0f ) )			// Une arme aquatique ne sort pas de l'eau
                hit_vec = Pos;
            else if( !weapon_manager.weapon[weapon_id].waterweapon && Pos.y <= map->sealvl && h < map->sealvl )
            {
                if( V.y != 0.0f )
                    hit_vec = Pos - (map->sealvl - hit_vec.y ) / V.y * V;
                else
                    hit_vec = Pos;
                hit_vec.y = map->sealvl;
            }
        }

        if(!dying && weapon_manager.weapon[weapon_id].cruise && ((weapon_manager.weapon[weapon_id].twophase && phase==2) || phase==1))
            if(((VECTOR)(target_pos-Pos)).norm()>2.0f*fabs(Pos.y-h) && V.y<0.0f)
                V.y=0.0f;

        bool hit=false;
        if(!dying)
            hit=((hit_vec-Pos)%V)<=0.0f && ((hit_vec-OPos)%V>=0.0f);
        bool u_hit=false;

        if( just_explode )
        {
            hit_vec = Pos;
            hit = true;
            just_explode = false;
        }

        if(weapon_manager.weapon[weapon_id].interceptor && ((VECTOR)(Pos-target_pos)).sq()<1024.0f)
        {
            hit=true;
            hit_vec=Pos;
            if(target >= 0 && target <= weapons.nb_weapon && weapons.weapon[target].weapon_id != -1)
            {
                weapons.weapon[target].dying=true;
                weapons.weapon[target].killtime=0.0f;
            }
        }

        int hit_idx=-1;
        if(!dying && !hit)
        {
            int t_idx=-1;
            int py=((int)(OPos.z)+map->map_h_d)>>3;
            int px=((int)(OPos.x)+map->map_w_d)>>3;
            int oidx=-1;
            VECTOR Dir=V;
            Dir.unit();
            for(int y=-5;y<=5;y++)
                for(int x=-5;x<=5;x++)
                {
                    if(px+x<0 || px+x>=map->bloc_w_db)	continue;
                    if(py+y<0 || py+y>=map->bloc_h_db)	continue;

                    bool land_test = true;

                    std::list< uint32 > air_list;

                    map->lock();
                    for( IDX_LIST_NODE *cur = map->map_data[py+y][px+x].air_idx.head ; cur != NULL ; cur = cur->next )
                        air_list.push_back( cur->idx );
                    map->unlock();

                    std::list< uint32 >::iterator cur = air_list.begin();

                    for( ; land_test || cur != air_list.end() ; )
                    {
                        if( land_test )
                        {
                            t_idx = map->map_data[py+y][px+x].unit_idx;
                            land_test = false;
                        }
                        else
                        {
                            t_idx = *cur;
                            ++cur;
                        }

                        if(t_idx == -1 || t_idx == oidx || t_idx == shooter_idx || t_idx == hit_idx)
                            continue;
                        if( t_idx >= 0 && t_idx < units.max_unit && ( units.unit[t_idx].owner_id != owner || target == t_idx ) && (units.unit[ t_idx ].flags & 1) ) // No Friendly Fire
                        {
                            VECTOR t_vec;
                            t_vec.x = t_vec.y=t_vec.z=0.0f;
                            u_hit = units.unit[t_idx].hit_fast(OPos,Dir,&t_vec, length);
                            if(u_hit)
                            {
                                if ((t_vec-Pos)%V<=0.0f) // Touché
                                {
                                    if(!hit)
                                    {
                                        hit_vec=t_vec;
                                        hit_idx=t_idx;
                                    }
                                    else 
                                        if(hit_vec%Dir>=t_vec%Dir) 
                                        {
                                            hit_vec=t_vec;
                                            hit_idx=t_idx;
                                        }
                                        else
                                            u_hit=false;
                                }
                                else
                                    u_hit=false;
                            }
                            hit|=u_hit;
                        }
                        else 
                        {
                            if(y==0 && x==0 && t_idx<=-2 && !weapon_manager.weapon[weapon_id].unitsonly )
                            {
                                if(!hit && -t_idx-2<features.max_features && features.feature[-t_idx-2].type>=0 && features.feature[-t_idx-2].Pos.y+feature_manager.feature[features.feature[-t_idx-2].type].height*0.5f>OPos.y)
                                {
                                    hit=true;
                                    hit_vec=OPos;
                                    hit_idx=t_idx;
                                }
                            }
                        }
                        oidx=t_idx;
                    }
                }
            if(hit_idx>=0)
            {
                units.unit[hit_idx].lock();
                if( (units.unit[hit_idx].flags & 1) && units.unit[hit_idx].local )
                {
                    bool ok = units.unit[hit_idx].hp>0.0f;		// Juste pour identifier l'assassin...
                    damage = weapon_manager.weapon[weapon_id].get_damage_for_unit( unit_manager.unit_type[ units.unit[hit_idx].type_id ].Unitname ) * units.unit[ hit_idx ].damage_modifier();
                    units.unit[hit_idx].hp -= damage;		// L'unité touchée encaisse les dégats
                    units.unit[hit_idx].flags&=0xEF;		// This unit must explode if it has been damaged by a weapon even if it is being reclaimed
                    if(ok && shooter_idx >= 0 && shooter_idx < units.max_unit && units.unit[hit_idx].hp<=0.0f && units.unit[shooter_idx].owner_id < players.nb_player
                       && units.unit[hit_idx].owner_id!=units.unit[shooter_idx].owner_id)		// Non,non les unités que l'on se détruit ne comptent pas dans le nombre de tués mais dans les pertes
                        players.kills[units.unit[shooter_idx].owner_id]++;
                    if(units.unit[hit_idx].hp<=0.0f)
                        units.unit[hit_idx].severity = Math::Max(units.unit[hit_idx].severity, (int)damage);

                    if( network_manager.isConnected() )			// Send damage event
                        g_ta3d_network->sendDamageEvent( hit_idx, damage );

                    VECTOR D = V * RotateY( -units.unit[hit_idx].Angle.y * DEG2RAD );
                    D.unit();
                    int param[] = { (int)(10.0f*DEG2TA*D.z), (int)(10.0f*DEG2TA*D.x) };
                    units.unit[hit_idx].launch_script(units.unit[hit_idx].get_script_index("hitbyweapon"),2,param,true);

                    units.unit[hit_idx].attacked=true;
                }
                units.unit[hit_idx].unlock();
            }
            else
            {
                features.lock();
                if(hit_idx<=-2 && features.feature[-hit_idx-2].type>=0)	// Only local weapons here, otherwise weapons would destroy features multiple times
                {
                    damage = weapon_manager.weapon[weapon_id].damage;

                    // Start a fire ?
                    if (feature_manager.feature[ features.feature[-hit_idx-2].type ].flamable && !features.feature[-hit_idx-2].burning && weapon_manager.weapon[weapon_id].firestarter && local )
                    {
                        int starter_score = rand_from_table() % 100;
                        if( starter_score < weapon_manager.weapon[weapon_id].firestarter )
                        {
                            features.burn_feature( -hit_idx-2 );
                            if( network_manager.isConnected() )
                                g_ta3d_network->sendFeatureFireEvent( -hit_idx-2 );
                        }
                    }

                    features.feature[-hit_idx-2].hp -= damage;		// The feature hit is taking damage
                    if(features.feature[-hit_idx-2].hp<=0.0f && !features.feature[-hit_idx-2].burning && local)
                    {
                        if( network_manager.isConnected() )
                            g_ta3d_network->sendFeatureDeathEvent( -hit_idx-2 );

                        int sx = features.feature[-hit_idx-2].px;		// Delete the feature
                        int sy = features.feature[-hit_idx-2].py;
                        VECTOR feature_pos = features.feature[-hit_idx-2].Pos;
                        int feature_type = features.feature[-hit_idx-2].type;
                        features.removeFeatureFromMap( -hit_idx-2 );
                        features.delete_feature(-hit_idx-2);			// Supprime l'objet

                        // Replace the feature if needed
                        if( feature_type!=-1 && feature_manager.feature[ feature_type ].feature_dead )
                        {
                            int type=feature_manager.get_feature_index( feature_manager.feature[ feature_type ].feature_dead );
                            if( type >= 0 )
                            {
                                map->map_data[sy][sx].stuff = features.add_feature(feature_pos,type);
                                features.drawFeatureOnMap( map->map_data[sy][sx].stuff );
                                if( network_manager.isConnected() )
                                    g_ta3d_network->sendFeatureCreationEvent( map->map_data[sy][sx].stuff );
                            }
                        }
                    }
                }
                features.unlock();
            }
        }

        if(hit && weapon_manager.weapon[weapon_id].areaofeffect>0) // Domages colatéraux
        {
            if( damage < 0 )
                damage = weapon_manager.weapon[weapon_id].damage;
            int t_idx=-1;
            int py=((int)(OPos.z+map->map_h_d))>>3;
            int px=((int)(OPos.x+map->map_w_d))>>3;
            int s=weapon_manager.weapon[weapon_id].areaofeffect+31>>5;
            int d=weapon_manager.weapon[weapon_id].areaofeffect*weapon_manager.weapon[weapon_id].areaofeffect+15>>4;
            std::list< int > oidx;
            for(int y=-s;y<=s;y++)
                for(int x=-s;x<=s;x++)
                {
                    if(px+x<0 || px+x>=map->bloc_w_db)	continue;
                    if(py+y<0 || py+y>=map->bloc_h_db)	continue;

                    bool land_test = true;

                    std::list< uint32 > air_list;

                    map->lock();
                    for( IDX_LIST_NODE *cur = map->map_data[py+y][px+x].air_idx.head ; cur != NULL ; cur = cur->next )
                        air_list.push_back( cur->idx );
                    map->unlock();

                    std::list< uint32 >::iterator cur = air_list.begin();

                    for( ; land_test || cur != air_list.end() ; )
                    {
                        if( land_test )
                        {
                            t_idx = map->map_data[py+y][px+x].unit_idx;
                            land_test = false;
                        }
                        else
                        {
                            t_idx = *cur;
                            cur++;
                        }
                        if(t_idx==-1)
                            continue;
                        if( t_idx >= 0 )
                        {
                            units.unit[ t_idx ].lock();
                            if( !(units.unit[ t_idx ].flags & 1) )
                            {
                                units.unit[ t_idx ].unlock();
                                continue;
                            }
                            units.unit[ t_idx ].unlock();
                        }
                        bool already=(t_idx==shooter_idx || t_idx==hit_idx || t_idx >= units.max_unit);
                        if(!already)
                        {
                            for (std::list<int>::const_iterator i = oidx.begin(); i != oidx.end(); ++i)
                            {
                                if( t_idx == *i )
                                {
                                    already=true;
                                    break;
                                }
                            }
                        }
                        if(!already)
                        {
                            if(t_idx>=0)
                            {
                                units.unit[t_idx].lock();
                                if( (units.unit[t_idx].flags & 1) && units.unit[t_idx].local && ((VECTOR)(units.unit[t_idx].Pos-Pos)).sq()<=d) {
                                    oidx.push_back( t_idx );
                                    bool ok = units.unit[ t_idx ].hp > 0.0f;
                                    damage = weapon_manager.weapon[weapon_id].get_damage_for_unit( unit_manager.unit_type[ units.unit[ t_idx ].type_id ].Unitname );
                                    float cur_damage = damage * weapon_manager.weapon[weapon_id].edgeeffectiveness * units.unit[ t_idx ].damage_modifier();
                                    units.unit[t_idx].hp -= cur_damage;		// L'unité touchée encaisse les dégats
                                    units.unit[t_idx].flags&=0xEF;		// This unit must explode if it has been damaged by a weapon even if it is being reclaimed
                                    if(ok && shooter_idx >= 0 && shooter_idx < units.max_unit && units.unit[t_idx].hp<=0.0f && units.unit[shooter_idx].owner_id < players.nb_player
                                       && units.unit[t_idx].owner_id!=units.unit[shooter_idx].owner_id)		// Non,non les unités que l'on se détruit ne comptent pas dans le nombre de tués mais dans les pertes
                                        players.kills[units.unit[shooter_idx].owner_id]++;
                                    if(units.unit[t_idx].hp<=0.0f)
                                        units.unit[t_idx].severity = Math::Max(units.unit[t_idx].severity,(int)cur_damage);

                                    if( network_manager.isConnected() )			// Send damage event
                                        g_ta3d_network->sendDamageEvent( t_idx, cur_damage );

                                    VECTOR D = (units.unit[t_idx].Pos - Pos) * RotateY( -units.unit[t_idx].Angle.y * DEG2RAD );
                                    D.unit();
                                    int param[] = { (int)(10.0f*DEG2TA*D.z), (int)(10.0f*DEG2TA*D.x) };
                                    units.unit[t_idx].launch_script(units.unit[t_idx].get_script_index("hitbyweapon"),2,param,true);

                                    units.unit[t_idx].attacked=true;
                                }
                                units.unit[t_idx].unlock();
                            }
                            else
                            {
                                features.lock();
                                if(t_idx<-1 && !weapon_manager.weapon[weapon_id].unitsonly && ((VECTOR)(features.feature[-t_idx-2].Pos-Pos)).sq()<=d)
                                {
                                    // Start a fire ?
                                    if( feature_manager.feature[ features.feature[-t_idx-2].type ].flamable && !features.feature[-t_idx-2].burning && weapon_manager.weapon[weapon_id].firestarter && local )
                                    {
                                        int starter_score = rand_from_table() % 100;
                                        if( starter_score < weapon_manager.weapon[weapon_id].firestarter ) {
                                            features.burn_feature( -t_idx-2 );
                                            if( network_manager.isConnected() )
                                                g_ta3d_network->sendFeatureFireEvent( -t_idx-2 );
                                        }
                                    }

                                    oidx.push_back( t_idx );
                                    if(features.feature[-t_idx-2].type>=0 && !feature_manager.feature[features.feature[-t_idx-2].type].indestructible && !features.feature[-t_idx-2].burning )
                                    {
                                        damage = weapon_manager.weapon[weapon_id].damage;
                                        float cur_damage = damage * weapon_manager.weapon[weapon_id].edgeeffectiveness;
                                        features.feature[-t_idx-2].hp -= cur_damage;		// L'objet touché encaisse les dégats
                                        if(features.feature[-t_idx-2].hp<=0.0f && local)
                                        {
                                            if( network_manager.isConnected() )
                                                g_ta3d_network->sendFeatureDeathEvent( -t_idx-2 );
                                            int sx = features.feature[-t_idx-2].px;		// Remove the object
                                            int sy = features.feature[-t_idx-2].py;
                                            VECTOR feature_pos = features.feature[-t_idx-2].Pos;
                                            int feature_type = features.feature[-t_idx-2].type;
                                            features.removeFeatureFromMap( -t_idx-2 );
                                            features.delete_feature(-t_idx-2);			// Supprime l'objet

                                            // Replace the feature if needed
                                            if( feature_type!=-1 && feature_manager.feature[ feature_type ].feature_dead )
                                            {
                                                int type=feature_manager.get_feature_index( feature_manager.feature[ feature_type ].feature_dead );
                                                if( type >= 0 )
                                                {
                                                    map->map_data[sy][sx].stuff = features.add_feature(feature_pos,type);
                                                    features.drawFeatureOnMap( map->map_data[sy][sx].stuff );
                                                    if( network_manager.isConnected() )
                                                        g_ta3d_network->sendFeatureCreationEvent( map->map_data[sy][sx].stuff );
                                                }
                                            }
                                        }
                                    }
                                }
                                features.unlock();
                            }
                        }
                    }
                }
            oidx.clear();
        }

        if(hit && visible && weapon_manager.weapon[weapon_id].areaofeffect>=256) // Effet de souffle / Shock wave
        {
            fx_manager.addFlash( Pos, weapon_manager.weapon[weapon_id].areaofeffect >> 1 );
            particle_engine.make_shockwave( Pos,1,weapon_manager.weapon[weapon_id].areaofeffect,weapon_manager.weapon[weapon_id].areaofeffect*0.75f);
            particle_engine.make_shockwave( Pos,0,weapon_manager.weapon[weapon_id].areaofeffect,weapon_manager.weapon[weapon_id].areaofeffect*0.5f);
            particle_engine.make_nuke( Pos,1,weapon_manager.weapon[weapon_id].areaofeffect>>1,weapon_manager.weapon[weapon_id].areaofeffect*0.25f);
        }

        if(hit && weapon_manager.weapon[weapon_id].interceptor)
        {
            units.unit[shooter_idx].lock();
            if(units.unit[shooter_idx].flags & 1)
            {
                int e=0;
                for(int i=0;i+e<units.unit[shooter_idx].mem_size;i++)
                {
                    if(units.unit[shooter_idx].memory[i+e]==target)
                    {
                        e++;
                        i--;
                        continue;
                    }
                    units.unit[shooter_idx].memory[i]=units.unit[shooter_idx].memory[i+e];
                }
                units.unit[shooter_idx].mem_size-=e;
            }
            units.unit[shooter_idx].unlock();
        }

        if(((stime > 0.5f * weapon_manager.weapon[weapon_id].time_to_range && (!weapon_manager.weapon[weapon_id].noautorange || weapon_manager.weapon[weapon_id].burnblow))
            || hit) && !dying)
        {
            if(hit)
            {
                Pos=hit_vec;
                if (visible)
                    Camera::inGame->setShake( weapon_manager.weapon[weapon_id].shakeduration, weapon_manager.weapon[weapon_id].shakemagnitude );
            }
            if (Pos.y == map->sealvl)
            {
                if( weapon_manager.weapon[weapon_id].soundwater )
                    sound_manager->playSound( weapon_manager.weapon[weapon_id].soundwater , &Pos);
            }
            else
                if( weapon_manager.weapon[weapon_id].soundhit )	sound_manager->playSound( weapon_manager.weapon[weapon_id].soundhit , &Pos );
            if(hit && weapon_manager.weapon[weapon_id].explosiongaf!=NULL && weapon_manager.weapon[weapon_id].explosionart!=NULL && Pos.y!=map->sealvl)
            {
                if( visible && weapon_manager.weapon[weapon_id].areaofeffect < 256 )		// Nuclear type explosion don't draw sprites :)
                    fx_manager.add(weapon_manager.weapon[weapon_id].explosiongaf, weapon_manager.weapon[weapon_id].explosionart, Pos, 1.0f);
            }
            else 
                if(hit && Pos.y==map->sealvl)
                {
                    int px=(int)(Pos.x+0.5f)+map->map_w_d>>4;
                    int py=(int)(Pos.z+0.5f)+map->map_h_d>>4;
                    VECTOR P = Pos;
                    P.y += 3.0f;
                    if(px>=0 && px<map->bloc_w && py>=0 && py<map->bloc_h)
                    {
                        if(map->bloc[map->bmap[py][px]].lava && weapon_manager.weapon[weapon_id].lavaexplosiongaf!=NULL && weapon_manager.weapon[weapon_id].lavaexplosionart!=NULL)
                        {
                            if(visible)
                                fx_manager.add(weapon_manager.weapon[weapon_id].lavaexplosiongaf,weapon_manager.weapon[weapon_id].lavaexplosionart,Pos,1.0f);
                        }
                        else 
                            if(!map->bloc[map->bmap[py][px]].lava && weapon_manager.weapon[weapon_id].waterexplosiongaf!=NULL && weapon_manager.weapon[weapon_id].waterexplosionart!=NULL)
                                if(visible)
                                    fx_manager.add(weapon_manager.weapon[weapon_id].waterexplosiongaf,weapon_manager.weapon[weapon_id].waterexplosionart,Pos,1.0f);
                    }
                    else 
                        if(weapon_manager.weapon[weapon_id].explosiongaf!=NULL && weapon_manager.weapon[weapon_id].explosionart!=NULL)
                            if(visible)
                                fx_manager.add(weapon_manager.weapon[weapon_id].explosiongaf,weapon_manager.weapon[weapon_id].explosionart,Pos,1.0f);
                }
            if(weapon_manager.weapon[weapon_id].endsmoke)
            {
                if(visible)
                    particle_engine.make_smoke( Pos,0,1,0.0f,-1.0f);
            }
            if( weapon_manager.weapon[weapon_id].noexplode && hit )	// Special flag used by dguns
            {
                dying = false;
                Pos.y += fabs( 3.0f * dt * V.y );
            }
            else
            {
                if(weapon_manager.weapon[weapon_id].rendertype==RENDER_TYPE_LASER)
                {
                    dying=true;
                    killtime=weapon_manager.weapon[weapon_id].duration;
                }
                else
                    weapon_id=-1;
            }
        }
        else 
            if(dying && killtime<=0.0f)
                weapon_id = -1;
            else 
                if( Pos.x < -map->map_w_d || Pos.x > map->map_w_d || Pos.z < -map->map_h_d || Pos.z > map->map_h_d )		// We're out of the map
                    weapon_id = -1;
    }



    void WEAPON::draw(Camera *cam,MAP *map)				// Dessine les objets produits par les armes
    {
        visible = false;
        if(map)
        {
            int px=(int)(Pos.x+0.5f)+map->map_w_d>>4;
            int py=(int)(Pos.z+0.5f)+map->map_h_d>>4;
            if(px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)	return;
            byte player_mask = 1 << players.local_human_id;
            if(map->view[py][px]!=1
               || !(map->sight_map->line[py][px]&player_mask))	return;
        }
        glPushMatrix();

        visible=true;
        switch(weapon_manager.weapon[weapon_id].rendertype)
        {
            case RENDER_TYPE_LASER:						// Dessine le laser
                {
                    VECTOR P=Pos;
                    float length=weapon_manager.weapon[weapon_id].duration;
                    if(weapon_manager.weapon[weapon_id].duration>stime)
                        length=stime;
                    if(dying && length>killtime)
                        length=killtime;
                    P=P-length*V;
                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);
                    int color0 = weapon_manager.weapon[weapon_id].color[0];
                    int color1 = weapon_manager.weapon[weapon_id].color[1];
                    float coef = (cos(stime*5.0f)+1.0f)*0.5f;
                    float r=(coef*getr(color0)+(1.0f-coef)*getr(color1))/255.0f;
                    float g=(coef*getg(color0)+(1.0f-coef)*getg(color1))/255.0f;
                    float b=(coef*getb(color0)+(1.0f-coef)*getb(color1))/255.0f;
                    VECTOR D=Pos-cam->pos;
                    VECTOR Up=D*V;
                    Up.unit();
                    if( damage < 0 )
                        damage = weapon_manager.weapon[weapon_id].damage;
                    Up = Math::Min(damage / 60.0f + weapon_manager.weapon[weapon_id].firestarter / 200.0f + weapon_manager.weapon[weapon_id].areaofeffect / 40.0f, 1.0f) * Up; // Variable width!!
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    glBegin(GL_QUADS);
                    glColor4f(r,g,b,0.0f);
                    glVertex3f(Pos.x+Up.x,Pos.y+Up.y,Pos.z+Up.z);			glVertex3f(P.x+Up.x,P.y+Up.y,P.z+Up.z);
                    glColor4f(r,g,b,1.0f);
                    glVertex3f(P.x,P.y,P.z);								glVertex3f(Pos.x,Pos.y,Pos.z);

                    glVertex3f(Pos.x,Pos.y,Pos.z);							glVertex3f(P.x,P.y,P.z);				
                    glColor4f(r,g,b,0.0f);
                    glVertex3f(P.x-Up.x,P.y-Up.y,P.z-Up.z);					glVertex3f(Pos.x-Up.x,Pos.y-Up.y,Pos.z-Up.z);
                    glEnd();
                    glDisable(GL_BLEND);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_MISSILE:					// Dessine le missile
                glTranslatef(Pos.x,Pos.y,Pos.z);
                {
                    VECTOR I, J, Dir;
                    I.y = I.x = 0.0f;
                    I.z = 1.0f;
                    Dir = V;
                    Dir.unit();
                    J = V * I;
                    J.unit();
                    float theta = -acos( Dir.z ) * RAD2DEG;
                    glRotatef( theta, J.x, J.y, J.z );
                }
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                if(weapon_manager.weapon[weapon_id].model)
                {
                    glDisable(GL_CULL_FACE);
                    weapon_manager.weapon[weapon_id].model->draw(0.0f);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_BITMAP:
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glColor4f(1.0f,1.0f,1.0f,1.0f);
                if(weapon_manager.cannonshell.nb_bmp>0)
                {
                    anim_sprite=((int)(stime*15.0f))%weapon_manager.cannonshell.nb_bmp;
                    gfx->set_alpha_blending();
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D,weapon_manager.cannonshell.glbmp[anim_sprite]);
                    VECTOR A,B,C,D;
                    A=Pos+((-0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*cam->up+(-0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*cam->side);
                    B=Pos+((-0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*cam->up+(0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*cam->side);
                    C=Pos+((0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*cam->up+(-0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*cam->side);
                    D=Pos+((0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*cam->up+(0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*cam->side);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0f,0.0f);		glVertex3f(A.x,A.y,A.z);
                    glTexCoord2f(1.0f,0.0f);		glVertex3f(B.x,B.y,B.z);
                    glTexCoord2f(1.0f,1.0f);		glVertex3f(D.x,D.y,D.z);
                    glTexCoord2f(0.0f,1.0f);		glVertex3f(C.x,C.y,C.z);
                    glEnd();
                    gfx->unset_alpha_blending();
                }
                else
                {
                    glBegin(GL_QUADS);
                    glVertex3f(Pos.x-2.5f,Pos.y-2.5f,Pos.z);
                    glVertex3f(Pos.x+2.5f,Pos.y-2.5f,Pos.z);
                    glVertex3f(Pos.x+2.5f,Pos.y+2.5f,Pos.z);
                    glVertex3f(Pos.x-2.5f,Pos.y+2.5f,Pos.z);
                    glEnd();
                }
                glEnable(GL_LIGHTING);
                break;
            case RENDER_TYPE_BOMB:
                glTranslatef(Pos.x,Pos.y,Pos.z);
                {
                    VECTOR I, J, Dir;
                    I.y = I.x = 0.0f;
                    I.z = 1.0f;
                    Dir = V;
                    Dir.unit();
                    J = V * I;
                    J.unit();
                    float theta = -acos( Dir.z ) * RAD2DEG;
                    glRotatef( theta, J.x, J.y, J.z );
                }
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                if(weapon_manager.weapon[weapon_id].model)
                {
                    glDisable(GL_CULL_FACE);
                    weapon_manager.weapon[weapon_id].model->draw(0.0f);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_LIGHTNING:
                {
                    VECTOR P=Pos;
                    float length=weapon_manager.weapon[weapon_id].duration;
                    if(weapon_manager.weapon[weapon_id].duration>stime)
                        length=stime;
                    if(dying && length>killtime)
                        length=killtime;
                    P=P-length*V;
                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);
                    int color0=weapon_manager.weapon[weapon_id].color[0];
                    int color1=weapon_manager.weapon[weapon_id].color[1];
                    float coef=(cos(stime)+1.0f)*0.5f;
                    float r=(coef*((color0>>16)&0xFF)+coef*((color1>>16)&0xFF))/255.0f;
                    float g=(coef*((color0>>8)&0xFF)+coef*((color1>>8)&0xFF))/255.0f;
                    float b=(coef*(color0&0xFF)+coef*(color1&0xFF))/255.0f;
                    glColor4f(r,g,b,1.0f);
                    glBegin(GL_LINE_STRIP);
                    for(int i=0;i<10;i++)
                    {
                        float x,y,z;
                        if(i>0 && i<9)
                        {
                            x=((rand_from_table()%2001)-1000)*0.005f;
                            y=((rand_from_table()%2001)-1000)*0.005f;
                            z=((rand_from_table()%2001)-1000)*0.005f;
                        }
                        else
                            x=y=z=0.0f;
                        glVertex3f(Pos.x+(P.x-Pos.x)*i/9+x,Pos.y+(P.y-Pos.y)*i/9+y,Pos.z+(P.z-Pos.z)*i/9+z);
                    }
                    glEnd();
                }
                break;
            case RENDER_TYPE_DGUN:			// Dessine le dgun
                glTranslatef(Pos.x,Pos.y,Pos.z);
                {
                    VECTOR I, J, Dir;
                    I.y = I.x = 0.0f;
                    I.z = 1.0f;
                    Dir = V;
                    Dir.unit();
                    J = V * I;
                    J.unit();
                    float theta = -acos( Dir.z ) * RAD2DEG;
                    glRotatef( theta, J.x, J.y, J.z );
                }
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                if(weapon_manager.weapon[weapon_id].model)
                {
                    glDisable(GL_CULL_FACE);
                    weapon_manager.weapon[weapon_id].model->draw(0.0f);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_GUN:			// Dessine une "balle"
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_POINTS);
                glColor3f(0.75f,0.75f,0.75f);
                glVertex3f(Pos.x,Pos.y,Pos.z);
                glEnd();
                break;
            case RENDER_TYPE_PARTICLES:		// Dessine des particules
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_POINTS);
                glColor3f(0.75f,0.75f,0.75f);
                for(int i=0;i<10;i++)
                    glVertex3f(Pos.x+(rand_from_table()%201)*0.01f-1.0f,Pos.y+(rand_from_table()%201)*0.01f-1.0f,Pos.z+(rand_from_table()%201)*0.01f-1.0f);
                glEnd();
                break;
        }
        glPopMatrix();
    }




} // namespace TA3D
