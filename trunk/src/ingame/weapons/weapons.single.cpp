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
#include "weapons.h"
#include "../../tdf.h"
#include "../../UnitEngine.h"
#include "../../gfx/fx.h"
#include "../../misc/camera.h"
#include <list>
#include "../../misc/math.h"
#include "../../sounds/manager.h"
#include "../players.h"



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

        WEAPON_DEF *weapon_def = &(weapon_manager.weapon[weapon_id]);

        smoke_time+=dt;
        f_time-=dt;
        a_time+=dt;
        Vector3D A;
        if (weapon_def->twophase && phase == 1)
        {
            if(!dying && a_time>=weapon_def->weapontimer) 	// Entre dans la seconde phase
            {
                phase=2;
                f_time=weapon_def->flighttime;
                stime=0.0f;
            }
            if(weapon_def->vlaunch)
            {
                V.x=0.0f;
                V.z=0.0f;
                if(V.y<weapon_def->weaponvelocity)
                    A.y=weapon_def->weaponacceleration;
                else
                    V.y=weapon_def->weaponvelocity;
            }
        }
        if(!dying && weapon_def->selfprop && f_time<=0.0f && ((weapon_def->twophase && phase==2) || !weapon_def->twophase))	dying=true;
        if(weapon_def->smoketrail && weapon_def->smokedelay<smoke_time) // Trainée de fumée des missiles
        {
            smoke_time=0.0f;
            if(visible)
                particle_engine.make_smoke(Pos,0,1,0.0f,-1.0f, -2.0f, 0.3f);
        }

        Vector3D hit_vec;
        Vector3D OPos(Pos);

        float h=map->get_unit_h(Pos.x,Pos.z);
        if(dying)
            killtime-=dt;
        else
        {
            if ((!weapon_def->lineofsight && (weapon_def->ballistic || weapon_def->dropped))
                || (weapon_def->waterweapon && Pos.y > map->sealvl))		// Arme soumise à la gravité
                A.y -= map->ota_data.gravity;

            if(weapon_def->guidance && ((weapon_def->twophase && phase==2) || !weapon_def->twophase)
               && ((weapon_def->waterweapon && Pos.y<map->sealvl) || !weapon_def->waterweapon))// Traque sa cible
            {
                float speed = V.norm();
                if(weapon_def->tracks && target>=0)
                {
                    Vector3D target_V;
                    if (weapon_def->interceptor && target <= weapons.nb_weapon && weapons.weapon[target].weapon_id!=-1)
                    {
                        target_pos = weapons.weapon[target].Pos;
                        target_V = weapons.weapon[target].V;
                    }
                    else
                        if (!weapon_def->interceptor && target < units.max_unit && (units.unit[target].flags & 1)) // Met à jour les coordonnées de la cible
                        {
                            target_pos = units.unit[target].Pos;
                            target_V = units.unit[target].V;
                        }
                        else
                            target = -1;
                    float speed = V.sq();
                    float target_speed = target_V.sq();
                    if (speed > 0.0f && target_speed > 0.0f) // Make it aim better
                    {
                        float time_to_hit = (target_pos - Pos).sq() / speed;
                        target_pos = target_pos + sqrtf( time_to_hit / target_speed ) * target_V;
                    }
                }
                if (target_pos.y<map->sealvl && !weapon_def->waterweapon)
                    target_pos.y = map->sealvl;
                Vector3D Dir = target_pos - Pos;
                Dir.unit();
                Vector3D I,J,K;			// Crée un trièdre
                I=V;
                I.unit();
                J=I*Dir;
                K=J*I;
                if (speed < weapon_def->weaponvelocity)
                {
                    if( speed > 0.0f )
                        A = A + weapon_def->weaponacceleration * I;
                    else
                        A = A + weapon_def->weaponacceleration * Dir;
                }
                else
                    if (speed > 0.5f * weapon_def->weaponvelocity && (V % Dir) < 0.0f)					// Can slow down if needed
                        A = A - weapon_def->weaponacceleration * I;
                    else
                        if (speed > weapon_def->weaponvelocity)         // Reduce speed
                            speed = weapon_def->weaponvelocity;

                float rotate=dt*weapon_def->turnrate*TA2RAD;
                V=speed*(cosf(rotate)*I+sinf(rotate)*K);
            }
            Pos=Pos+dt*(V+dt*(0.33333333f*A+0.16666666667f*Ac));
            V=V+dt*0.5f*(A+Ac);
            Ac=A;
            stime+=dt;
        }

        if( weapon_def->waterweapon && Pos.y <= map->sealvl && OPos.y > map->sealvl )			// A weapon that gets into water slows down
            V = 0.5f * V;

        float length = ((Vector3D)(OPos - Pos)).norm();
        if(!dying)
        {
            if (weapon_def->waterweapon && Pos.y > map->sealvl && OPos.y <= map->sealvl)			// Une arme aquatique ne sort pas de l'eau
            {
                Pos.y = map->sealvl;
                V.y = 0.0f;
            }
            else
            {
                hit_vec = map->hit(Pos,V,!weapon_def->waterweapon,length);
                if( !weapon_def->waterweapon && Pos.y <= map->sealvl && h < map->sealvl )
                {
                    hit_vec = map->hit(Pos,V,!weapon_def->waterweapon,length);
                    if( V.y != 0.0f )
                        hit_vec = Pos - (map->sealvl - hit_vec.y ) / V.y * V;
                    else
                        hit_vec = Pos;
                    hit_vec.y = map->sealvl;
                }
            }
        }

        if(!dying && weapon_def->cruise && ((weapon_def->twophase && phase==2) || phase==1))
            if(((Vector3D)(target_pos-Pos)).norm()>2.0f*fabsf(Pos.y-h) && V.y<0.0f)
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

        if(weapon_def->interceptor && ((Vector3D)(Pos-target_pos)).sq()<1024.0f)
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
            Vector3D Dir(V);
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
                            Vector3D t_vec;
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
                            if(y==0 && x==0 && t_idx<=-2 && !weapon_def->unitsonly )
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
                    damage = weapon_def->get_damage_for_unit(unit_manager.unit_type[units.unit[hit_idx].type_id]->Unitname) * units.unit[hit_idx].damage_modifier();
                    if (weapon_def->paralyzer)
                    {
                        if (!unit_manager.unit_type[units.unit[hit_idx].type_id]->ImmuneToParalyzer)
                        {
                            units.unit[hit_idx].paralyzed = damage / 60.0f;		// Get paralyzed (900 dmg <-> 15sec according to WEAPONS.TDF)
                            if( network_manager.isConnected() )			// Send damage event
                                g_ta3d_network->sendParalyzeEvent( hit_idx, damage );
                        }
                    }
                    else
                    {
                        units.unit[hit_idx].hp -= damage;		// L'unité touchée encaisse les dégats
                        units.unit[hit_idx].flags&=0xEF;		// This unit must explode if it has been damaged by a weapon even if it is being reclaimed
                        if(ok && shooter_idx >= 0 && shooter_idx < units.max_unit && units.unit[hit_idx].hp<=0.0f && units.unit[shooter_idx].owner_id < players.nb_player
                           && units.unit[hit_idx].owner_id!=units.unit[shooter_idx].owner_id)		// Non,non les unités que l'on se détruit ne comptent pas dans le nombre de tués mais dans les pertes
                        {
                            players.kills[units.unit[shooter_idx].owner_id]++;
                            units.unit[shooter_idx].kills++;
                        }
                        if(units.unit[hit_idx].hp<=0.0f)
                            units.unit[hit_idx].severity = Math::Max(units.unit[hit_idx].severity, (int)damage);

                        if( network_manager.isConnected() )			// Send damage event
                            g_ta3d_network->sendDamageEvent( hit_idx, damage );
                    }

                    Vector3D D = V * RotateY(-units.unit[hit_idx].Angle.y * DEG2RAD);
                    D.unit();
                    int param[] = { (int)(10.0f*DEG2TA*D.z), (int)(10.0f*DEG2TA*D.x) };
                    units.unit[hit_idx].launch_script(SCRIPT_HitByWeapon, 2, param);

                    units.unit[hit_idx].attacked = true;
                }
                units.unit[hit_idx].unlock();
            }
            else if (!weapon_def->paralyzer)       // We can't paralyze features :P
            {
                features.lock();
                if(hit_idx<=-2 && features.feature[-hit_idx-2].type>=0)	// Only local weapons here, otherwise weapons would destroy features multiple times
                {
                    damage = weapon_def->damage;

                    // Start a fire ?
                    if (feature_manager.feature[ features.feature[-hit_idx-2].type ].flamable && !features.feature[-hit_idx-2].burning && weapon_def->firestarter && local )
                    {
                        int starter_score = Math::RandFromTable() % 100;
                        if( starter_score < weapon_def->firestarter )
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
                        Vector3D feature_pos = features.feature[-hit_idx-2].Pos;
                        int feature_type = features.feature[-hit_idx-2].type;
                        features.removeFeatureFromMap( -hit_idx-2 );
                        features.delete_feature(-hit_idx-2);			// Supprime l'objet

                        // Replace the feature if needed
                        if( feature_type!=-1 && !feature_manager.feature[ feature_type ].feature_dead.empty() )
                        {
                            int type = feature_manager.get_feature_index( feature_manager.feature[ feature_type ].feature_dead );
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

        if(hit && weapon_def->areaofeffect>0) // Domages colatéraux
        {
            if( damage < 0 )
                damage = weapon_def->damage;
            int t_idx=-1;
            int py=((int)(OPos.z+map->map_h_d))>>3;
            int px=((int)(OPos.x+map->map_w_d))>>3;
            int s=(weapon_def->areaofeffect+31)>>5;
            int d=(weapon_def->areaofeffect*weapon_def->areaofeffect+15)>>4;
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
                        if (!already)
                        {
                            if (t_idx >= 0)
                            {
                                units.unit[t_idx].lock();
                                if ((units.unit[t_idx].flags & 1) && units.unit[t_idx].local && ((Vector3D)(units.unit[t_idx].Pos - Pos)).sq() <= d)
                                {
                                    oidx.push_back( t_idx );
                                    bool ok = units.unit[ t_idx ].hp > 0.0f;
                                    damage = weapon_def->get_damage_for_unit( unit_manager.unit_type[ units.unit[ t_idx ].type_id ]->Unitname);
                                    float cur_damage = damage * weapon_def->edgeeffectiveness * units.unit[ t_idx ].damage_modifier();
                                    if (weapon_def->paralyzer)
                                    {
                                        if (!unit_manager.unit_type[units.unit[t_idx].type_id]->ImmuneToParalyzer)
                                        {
                                            units.unit[t_idx].paralyzed = cur_damage / 60.0f;		// Get paralyzed (900 <-> 15sec)
                                            if (network_manager.isConnected())			// Send damage event
                                                g_ta3d_network->sendParalyzeEvent(t_idx, cur_damage);
                                        }
                                    }
                                    else
                                    {
                                        units.unit[t_idx].hp -= cur_damage;		// L'unité touchée encaisse les dégats
                                        units.unit[t_idx].flags&=0xEF;		// This unit must explode if it has been damaged by a weapon even if it is being reclaimed
                                        if (ok && shooter_idx >= 0 && shooter_idx < units.max_unit && units.unit[t_idx].hp<=0.0f && units.unit[shooter_idx].owner_id < players.nb_player
                                           && units.unit[t_idx].owner_id!=units.unit[shooter_idx].owner_id)		// Non,non les unités que l'on se détruit ne comptent pas dans le nombre de tués mais dans les pertes
                                        {
                                            players.kills[units.unit[shooter_idx].owner_id]++;
                                            units.unit[shooter_idx].kills++;
                                        }
                                        if (units.unit[t_idx].hp<=0.0f)
                                            units.unit[t_idx].severity = Math::Max(units.unit[t_idx].severity,(int)cur_damage);

                                        if (network_manager.isConnected())			// Send damage event
                                            g_ta3d_network->sendDamageEvent(t_idx, cur_damage);
                                    }

                                    Vector3D D = (units.unit[t_idx].Pos - Pos) * RotateY( -units.unit[t_idx].Angle.y * DEG2RAD );
                                    D.unit();
                                    int param[] = { (int)(10.0f*DEG2TA*D.z), (int)(10.0f*DEG2TA*D.x) };
                                    units.unit[t_idx].launch_script( SCRIPT_HitByWeapon, 2, param);

                                    units.unit[t_idx].attacked = true;
                                }
                                units.unit[t_idx].unlock();
                            }
                            else
                            {
                                features.lock();
                                if(t_idx<-1 && !weapon_def->unitsonly && features.feature[-t_idx-2].type >= 0 && ((Vector3D)(features.feature[-t_idx-2].Pos-Pos)).sq()<=d)
                                {
                                    // Start a fire ?
                                    if( feature_manager.feature[ features.feature[-t_idx-2].type ].flamable && !features.feature[-t_idx-2].burning && weapon_def->firestarter && local )
                                    {
                                        int starter_score = Math::RandFromTable() % 100;
                                        if( starter_score < weapon_def->firestarter ) {
                                            features.burn_feature( -t_idx-2 );
                                            if( network_manager.isConnected() )
                                                g_ta3d_network->sendFeatureFireEvent( -t_idx-2 );
                                        }
                                    }

                                    oidx.push_back( t_idx );
                                    if(!feature_manager.feature[features.feature[-t_idx-2].type].indestructible && !features.feature[-t_idx-2].burning )
                                    {
                                        damage = weapon_def->damage;
                                        float cur_damage = damage * weapon_def->edgeeffectiveness;
                                        features.feature[-t_idx-2].hp -= cur_damage;		// L'objet touché encaisse les dégats
                                        if(features.feature[-t_idx-2].hp<=0.0f && local)
                                        {
                                            if( network_manager.isConnected() )
                                                g_ta3d_network->sendFeatureDeathEvent( -t_idx-2 );
                                            int sx = features.feature[-t_idx-2].px;		// Remove the object
                                            int sy = features.feature[-t_idx-2].py;
                                            Vector3D feature_pos = features.feature[-t_idx-2].Pos;
                                            int feature_type = features.feature[-t_idx-2].type;
                                            features.removeFeatureFromMap( -t_idx-2 );
                                            features.delete_feature(-t_idx-2);			// Supprime l'objet

                                            // Replace the feature if needed
                                            if( feature_type!=-1 && !feature_manager.feature[ feature_type ].feature_dead.empty() )
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

        if(hit && visible && weapon_def->areaofeffect>=256) // Effet de souffle / Shock wave
        {
            fx_manager.addFlash( Pos, weapon_def->areaofeffect >> 1 );
            particle_engine.make_shockwave( Pos,1,weapon_def->areaofeffect,weapon_def->areaofeffect*0.75f);
            particle_engine.make_shockwave( Pos,0,weapon_def->areaofeffect,weapon_def->areaofeffect*0.5f);
            particle_engine.make_nuke( Pos,1,weapon_def->areaofeffect>>1,weapon_def->areaofeffect*0.25f);
        }

        if(hit && weapon_def->interceptor)
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

        if(((stime > 0.5f * weapon_def->time_to_range && (!weapon_def->noautorange || weapon_def->burnblow))
            || hit) && !dying)
        {
            if(hit)
            {
                Pos=hit_vec;
                if (visible)
                    Camera::inGame->setShake( weapon_def->shakeduration, weapon_def->shakemagnitude );
            }
            if (Pos.y == map->sealvl)
            {
                if( !weapon_def->soundwater.empty() )
                    sound_manager->playSound( weapon_def->soundwater , &Pos);
            }
            else
                if( !weapon_def->soundhit.empty() )	sound_manager->playSound( weapon_def->soundhit , &Pos );
            if(hit && !weapon_def->explosiongaf.empty() && !weapon_def->explosionart.empty() && Pos.y!=map->sealvl)
            {
                if( visible && weapon_def->areaofeffect < 256 )		// Nuclear type explosion don't draw sprites :)
                    fx_manager.add(weapon_def->explosiongaf, weapon_def->explosionart, Pos, 1.0f);
            }
            else
                if(hit && Pos.y==map->sealvl)
                {
                    int px=((int)(Pos.x+0.5f)+map->map_w_d)>>4;
                    int py=((int)(Pos.z+0.5f)+map->map_h_d)>>4;
                    Vector3D P = Pos;
                    P.y += 3.0f;
                    if(px>=0 && px<map->bloc_w && py>=0 && py<map->bloc_h)
                    {
                        if(map->bloc[map->bmap[py][px]].lava && !weapon_def->lavaexplosiongaf.empty() && !weapon_def->lavaexplosionart.empty())
                        {
                            if(visible)
                                fx_manager.add(weapon_def->lavaexplosiongaf,weapon_def->lavaexplosionart,Pos,1.0f);
                        }
                        else
                            if(!map->bloc[map->bmap[py][px]].lava && !weapon_def->waterexplosiongaf.empty() && !weapon_def->waterexplosionart.empty())
                                if(visible)
                                    fx_manager.add(weapon_def->waterexplosiongaf,weapon_def->waterexplosionart,Pos,1.0f);
                    }
                    else
                        if(!weapon_def->explosiongaf.empty() && !weapon_def->explosionart.empty())
                            if(visible)
                                fx_manager.add(weapon_def->explosiongaf,weapon_def->explosionart,Pos,1.0f);
                }
            if(weapon_def->endsmoke)
            {
                if(visible)
                    particle_engine.make_smoke( Pos,0,1,0.0f,-1.0f);
            }
            if( weapon_def->noexplode && hit )	// Special flag used by dguns
            {
                dying = false;
                Pos.y += fabsf( 3.0f * dt * V.y );
            }
            else
            {
                if(weapon_def->rendertype==RENDER_TYPE_LASER)
                {
                    dying=true;
                    killtime=weapon_def->duration;
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



    void WEAPON::draw(MAP *map)				// Dessine les objets produits par les armes
    {
        visible = false;
        if(map)
        {
            int px=((int)(Pos.x+0.5f)+map->map_w_d)>>4;
            int py=((int)(Pos.z+0.5f)+map->map_h_d)>>4;
            if(px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)	return;
            byte player_mask = 1 << players.local_human_id;
            if(map->view[py][px]!=1
               || !(SurfaceByte(map->sight_map, px, py) & player_mask))	return;
        }
        glPushMatrix();

        WEAPON_DEF *weapon_def = &(weapon_manager.weapon[weapon_id]);

        visible=true;
        switch(weapon_def->rendertype)
        {
            case RENDER_TYPE_LASER:						// Dessine le laser
                {
                    Vector3D P(Pos);
                    float length=weapon_def->duration;
                    if(weapon_def->duration>stime)
                        length=stime;
                    if(dying && length>killtime)
                        length=killtime;
                    P=P-length*V;
                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);
                    int color0 = weapon_def->color[0];
                    int color1 = weapon_def->color[1];
                    float coef = (cosf(stime*5.0f)+1.0f)*0.5f;
                    int r = (int)(coef*getr(color0)+(1.0f-coef)*getr(color1));
                    int g = (int)(coef*getg(color0)+(1.0f-coef)*getg(color1));
                    int b = (int)(coef*getb(color0)+(1.0f-coef)*getb(color1));
                    Vector3D D(Pos - Camera::inGame->pos);
                    Vector3D Up(D * V);
                    Up.unit();
                    if( damage < 0 )
                        damage = weapon_def->damage;
                    Up = Math::Min(damage / 60.0f + weapon_def->firestarter / 200.0f + weapon_def->areaofeffect / 40.0f, 1.0f) * Up; // Variable width!!
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    glBegin(GL_QUADS);
                    glColor4ub(r,g,b,0);
                    glVertex3f(Pos.x+Up.x,Pos.y+Up.y,Pos.z+Up.z);			glVertex3f(P.x+Up.x,P.y+Up.y,P.z+Up.z);
                    glColor4ub(r,g,b,0xFF);
                    glVertex3f(P.x,P.y,P.z);								glVertex3f(Pos.x,Pos.y,Pos.z);

                    glVertex3f(Pos.x,Pos.y,Pos.z);							glVertex3f(P.x,P.y,P.z);
                    glColor4ub(r,g,b,0);
                    glVertex3f(P.x-Up.x,P.y-Up.y,P.z-Up.z);					glVertex3f(Pos.x-Up.x,Pos.y-Up.y,Pos.z-Up.z);
                    glEnd();
                    glDisable(GL_BLEND);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_MISSILE:					// Dessine le missile
                glTranslatef(Pos.x,Pos.y,Pos.z);
                {
                    Vector3D I, J, Dir;
                    I.y = I.x = 0.0f;
                    I.z = 1.0f;
                    Dir = V;
                    Dir.unit();
                    J = V * I;
                    J.unit();
                    float theta = -acosf( Dir.z ) * RAD2DEG;
                    glRotatef( theta, J.x, J.y, J.z );
                }
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                if(weapon_def->model)
                {
                    glDisable(GL_CULL_FACE);
                    weapon_def->model->draw(0.0f);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_BITMAP:
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glColor4ub(0xFF,0xFF,0xFF,0xFF);
                if(weapon_manager.cannonshell.nb_bmp>0)
                {
                    anim_sprite=((int)(stime*15.0f))%weapon_manager.cannonshell.nb_bmp;
                    gfx->set_alpha_blending();
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D,weapon_manager.cannonshell.glbmp[anim_sprite]);
                    Vector3D A,B,C,D;
                    A = Pos + ((-0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*Camera::inGame->up+(-0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*Camera::inGame->side);
                    B = Pos + ((-0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*Camera::inGame->up+(0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*Camera::inGame->side);
                    C = Pos + ((0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*Camera::inGame->up+(-0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*Camera::inGame->side);
                    D = Pos + ((0.5f*weapon_manager.cannonshell.h[anim_sprite]-weapon_manager.cannonshell.ofs_y[anim_sprite])*Camera::inGame->up+(0.5f*weapon_manager.cannonshell.w[anim_sprite]-weapon_manager.cannonshell.ofs_x[anim_sprite])*Camera::inGame->side);
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
                    Vector3D I, J, Dir;
                    I.y = I.x = 0.0f;
                    I.z = 1.0f;
                    Dir = V;
                    Dir.unit();
                    J = V * I;
                    J.unit();
                    float theta = -acosf( Dir.z ) * RAD2DEG;
                    glRotatef( theta, J.x, J.y, J.z );
                }
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                if(weapon_def->model)
                {
                    glDisable(GL_CULL_FACE);
                    weapon_def->model->draw(0.0f);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_LIGHTNING:
                {
                    Vector3D P=Pos;
                    float length=weapon_def->duration;
                    if(weapon_def->duration>stime)
                        length=stime;
                    if(dying && length>killtime)
                        length=killtime;
                    P=P-length*V;
                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);
                    int color0=weapon_def->color[0];
                    int color1=weapon_def->color[1];
                    float coef=(cosf(stime)+1.0f)*0.5f;
                    int r = (int)(coef*((color0>>16)&0xFF)+coef*((color1>>16)&0xFF));
                    int g = (int)(coef*((color0>>8)&0xFF)+coef*((color1>>8)&0xFF));
                    int b = (int)(coef*(color0&0xFF)+coef*(color1&0xFF));
                    glColor4ub(r,g,b,0xFF);
                    glBegin(GL_LINE_STRIP);
                    for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
                    {
                        float x,y,z;
                        if (i > 0 && i < 9)
                        {
                            x = (((sint32)(Math::RandFromTable() % 2001)) - 1000) * 0.005f;
                            y = (((sint32)(Math::RandFromTable() % 2001)) - 1000) * 0.005f;
                            z = (((sint32)(Math::RandFromTable() % 2001)) - 1000) * 0.005f;
                        }
                        else
                            x = y = z = 0.0f;
                        glVertex3f(Pos.x+(P.x-Pos.x)*i/9+x,Pos.y+(P.y-Pos.y)*i/9+y,Pos.z+(P.z-Pos.z)*i/9+z);
                    }
                    glEnd();
                }
                break;
            case RENDER_TYPE_DGUN:			// Dessine le dgun
                glTranslatef(Pos.x,Pos.y,Pos.z);
                {
                    Vector3D I, J, Dir;
                    I.y = I.x = 0.0f;
                    I.z = 1.0f;
                    Dir = V;
                    Dir.unit();
                    J = V * I;
                    J.unit();
                    float theta = -acosf( Dir.z ) * RAD2DEG;
                    glRotatef( theta, J.x, J.y, J.z );
                }
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                if(weapon_def->model)
                {
                    glDisable(GL_CULL_FACE);
                    weapon_def->model->draw(0.0f);
                    glEnable(GL_CULL_FACE);
                }
                break;
            case RENDER_TYPE_GUN:			// Dessine une "balle"
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_POINTS);
                glColor3ub(0xBF,0xBF,0xBF);
                glVertex3f(Pos.x,Pos.y,Pos.z);
                glEnd();
                break;
            case RENDER_TYPE_PARTICLES:		// Dessine des particules
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_POINTS);
                glColor3ub(0xBF,0xBF,0xBF);
                for (short int i = 0; i < 10; ++i)
                {
                    glVertex3f(Pos.x + (Math::RandFromTable() % 201) * 0.01f - 1.0f,
                               Pos.y + (Math::RandFromTable() % 201) * 0.01f - 1.0f,
                               Pos.z + (Math::RandFromTable() % 201) * 0.01f - 1.0f);
                }
                glEnd();
                break;
        }
        glPopMatrix();
    }




} // namespace TA3D
