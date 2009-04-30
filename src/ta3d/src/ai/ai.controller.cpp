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

#include "../misc/matrix.h"
#include "../TA3D_NameSpace.h"
#include "../ta3dbase.h"
#include "../3do.h"             // To read 3D files
#include "../scripts/cob.h"             // To read and execute scripts
#include "../EngineClass.h"
#include "../UnitEngine.h"
#include "../misc/paths.h"
#include "../misc/math.h"
#include "../logs/logs.h"
#include "../ingame/players.h"
#include "ai.controller.h"
#include "ai.h"

namespace TA3D
{



    //#define AI_DEBUG

    inline byte int2brain_value(int a)	// Convert an integer into a value a neural network can understand
    {
        if (a==0)	return BRAIN_VALUE_NULL;
        if (a<=5)	return BRAIN_VALUE_LOW;
        if (a<=15)	return BRAIN_VALUE_MEDIUM;
        if (a<=50)	return BRAIN_VALUE_HIGH;
        return BRAIN_VALUE_MAX;
    }

    inline int get_bits(float bits[], byte v, int pos)	// Fill the array (neural network input)
    {
        for(int i = 0; i < BRAIN_VALUE_BITS; ++i)
            bits[pos++] = ((v>>i)&1) ? 1.0f : 0.0f;
        return pos;
    }

    void AI_CONTROLLER::scan_unit()							// Scan the units the AI player currently has
    {
        if (enemy_table == NULL)
        {
            enemy_table = new byte[units.max_unit];
            memset(enemy_table, 0, units.max_unit);
        }

        if (weights == NULL)
        {
            weights = new AI_WEIGHT[ unit_manager.nb_unit ];

            for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i)
                if (strcasecmp( unit_manager.unit_type[i]->side.c_str(), players.side[ playerID ].c_str() ) == 0)
                {
                    weights[ i ].type = 0;

                    if (unit_manager.unit_type[i]->canattack && unit_manager.unit_type[i]->BMcode && !unit_manager.unit_type[i]->commander)
                    {
                        if (unit_manager.unit_type[i]->canmove)
                            weights[ i ].type |= AI_FLAG_ARMY;
                        else
                            weights[ i ].type |= AI_FLAG_DEFENSE;
                    }

                    if (unit_manager.unit_type[i]->Builder)
                    {
                        if (unit_manager.unit_type[i]->BMcode)
                            weights[ i ].type |= AI_FLAG_BUILDER;
                        else
                            weights[ i ].type |= AI_FLAG_FACTORY;
                    }

                    if (unit_manager.unit_type[i]->MetalMake > 0.0f
                        || unit_manager.unit_type[i]->MakesMetal > 0.0f || unit_manager.unit_type[i]->ExtractsMetal > 0.0f)
                    {
                        weights[ i ].type |= AI_FLAG_METAL_P;
                        weights[ i ].metal_s = unit_manager.unit_type[i]->MetalStorage * 0.001f;
                        weights[ i ].metal_p = (unit_manager.unit_type[i]->MetalMake + unit_manager.unit_type[i]->MakesMetal) * 10.0f
                            + 5000.0f * unit_manager.unit_type[i]->ExtractsMetal - unit_manager.unit_type[i]->EnergyUse;
                    }
                    if (unit_manager.unit_type[i]->MetalStorage)
                    {
                        weights[ i ].type |= AI_FLAG_METAL_S;
                        weights[ i ].metal_s = unit_manager.unit_type[i]->MetalStorage * 0.001f;
                        weights[ i ].metal_p = (unit_manager.unit_type[i]->MetalMake + unit_manager.unit_type[i]->MakesMetal) * 10.0f
                            + 5000.0f * unit_manager.unit_type[i]->ExtractsMetal - unit_manager.unit_type[i]->EnergyUse;
                    }
                    if (unit_manager.unit_type[i]->EnergyMake || unit_manager.unit_type[i]->EnergyUse < 0.0f
                        || unit_manager.unit_type[i]->TidalGenerator || unit_manager.unit_type[i]->WindGenerator)
                    {
                        weights[ i ].type |= AI_FLAG_ENERGY_P;
                        weights[ i ].energy_s = unit_manager.unit_type[i]->EnergyStorage * 0.0001f;
                        weights[ i ].energy_p = (unit_manager.unit_type[i]->EnergyMake + unit_manager.unit_type[i]->TidalGenerator
                                                     + unit_manager.unit_type[i]->WindGenerator - unit_manager.unit_type[i]->EnergyUse) * 0.01f;
                    }
                    if (unit_manager.unit_type[i]->EnergyStorage)
                    {
                        weights[ i ].type |= AI_FLAG_ENERGY_S;
                        weights[ i ].energy_s = unit_manager.unit_type[i]->EnergyStorage * 0.0001f;
                        weights[ i ].energy_p = (unit_manager.unit_type[i]->EnergyMake + unit_manager.unit_type[i]->TidalGenerator
                                                     + unit_manager.unit_type[i]->WindGenerator - unit_manager.unit_type[i]->EnergyUse) * 0.01f;
                    }
                }
                else
                    weights[ i ].type = 0;
            enemy_list.resize( players.nb_player );
            for( uint16 i = 0 ; i < players.nb_player ; ++i )
                enemy_list[ i ].clear();
        }

        if (unit_id == 0)
        {
            builder_list.clear();
            factory_list.clear();
            army_list.clear();

            for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )			// reset things if needed
                weights[ i ].nb = 0;

            for( uint16 i = 0 ; i < NB_AI_UNIT_TYPE ; ++i )
                nb_units[ i ] = 0;

            for( uint16 i = 0 ; i < 10 ; ++i )
                nb_enemy[ i ] = 0;
        }

        int e;
        units.lock();
        for( e = unit_id ; e < units.nb_unit && e < unit_id + 100 ; e++ )
        {
            int i = units.idx_list[ e ];
            if (i < 0 || i >= units.max_unit)	continue;		// Error
            units.unlock();
            units.unit[i].lock();
            if ((units.unit[ i ].flags & 1) && units.unit[ i ].type_id >= 0)
            {
                if (units.unit[ i ].owner_id == playerID)
                {
                    weights[ units.unit[ i ].type_id ].nb++;
                    if (weights[ units.unit[ i ].type_id ].type & AI_FLAG_ARMY)
                        army_list.push_back( i );
                    if (weights[ units.unit[ i ].type_id ].type & AI_FLAG_BUILDER)
                        builder_list.push_back( i );
                    if (weights[ units.unit[ i ].type_id ].type & AI_FLAG_FACTORY)
                        factory_list.push_back( i );
                }
                else
                {
                    nb_units[ AI_UNIT_TYPE_ENEMY ]++;
                    nb_enemy[ units.unit[ i ].owner_id ]++;
                    if (!enemy_table[i])
                    {
                        enemy_list[ units.unit[ i ].owner_id ].push_back( WEIGHT_COEF( i, 0 ) );
                        enemy_table[ i ] = true;
                    }
                }
            }
            units.unit[ i ].unlock();
            units.lock();
        }
        units.unlock();
        unit_id = (e >= units.nb_unit) ? 0 : e;
    }



    void AI_CONTROLLER::refresh_unit_weights()				// Refresh unit weights according to the unit scan and the orders weights
    {
        for(sint8 i = 0 ; i < players.nb_player ; ++i )
            enemy_list[i].sort();

        total_unit = 0;

        for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )
            total_unit += weights[ i ].nb;

        float total_unit_inv = 2.0f / (total_unit + 1);

        for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )
        {
            if (weights[ i ].type && !unit_manager.unit_type[i]->not_used)
            {
                if (weights[ i ].type & AI_FLAG_ARMY)
                {
                    nb_units[ AI_UNIT_TYPE_ARMY ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ARMY ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ARMY ];
                }
                if (weights[ i ].type & AI_FLAG_DEFENSE)
                {
                    nb_units[ AI_UNIT_TYPE_DEFENSE ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_DEFENSE ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_DEFENSE ];
                }
                if (weights[ i ].type & AI_FLAG_BUILDER)
                {
                    nb_units[ AI_UNIT_TYPE_BUILDER ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_BUILDER ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_BUILDER ];
                }
                if (weights[ i ].type & AI_FLAG_FACTORY)
                {
                    nb_units[ AI_UNIT_TYPE_FACTORY ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_FACTORY ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_FACTORY ];
                }
                if (weights[ i ].type & AI_FLAG_METAL_P)
                {
                    nb_units[ AI_UNIT_TYPE_METAL ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_METAL_P ] * weights[ i ].metal_p * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_METAL_P ] * weights[ i ].metal_p;
                }
                if (weights[ i ].type & AI_FLAG_METAL_S)
                {
                    nb_units[ AI_UNIT_TYPE_METAL ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_METAL_S ] * weights[ i ].metal_s * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_METAL_S ] * weights[ i ].metal_s;
                }
                if (weights[ i ].type & AI_FLAG_ENERGY_P)
                {
                    nb_units[ AI_UNIT_TYPE_ENERGY ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ENERGY_P ] * weights[ i ].energy_p * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ENERGY_P ] * weights[ i ].energy_p;
                }
                if (weights[ i ].type & AI_FLAG_ENERGY_S)
                {
                    nb_units[ AI_UNIT_TYPE_ENERGY ] += weights[ i ].nb;
                    weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ENERGY_S ] * weights[ i ].energy_s * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ENERGY_S ] * weights[ i ].energy_s;
                }
                if (weights[ i ].w < 0.0f)
                    weights[ i ].w = 0.0f;
            }
            else
                weights[ i ].w = 0.0f;
            weights[ i ].o_w = weights[ i ].w;
        }
    }

    void AI_CONTROLLER::think()				// La vrai fonction qui simule l'Intelligence Artificielle / The function that makes Artificial Intelligence work
    {
        srand( msec_timer );

        order_weight[ ORDER_METAL_P ] = Math::Max(0.0f, players.metal_u[ playerID ] - players.metal_t[playerID]) * 10.0f
            + Math::Max(0.0f, players.metal[ playerID ] - (players.metal_s[ playerID ] >> 1) ) * 0.01f;
        order_weight[ ORDER_ENERGY_P ] = Math::Max(0.0f, players.energy_u[ playerID ] - players.energy_t[playerID])
            + Math::Max(0.0f, players.energy[ playerID ] - (players.energy_s[ playerID ] >> 1) ) * 0.001f;
        order_weight[ ORDER_METAL_S ] = Math::Max(0.0f, players.metal[ playerID ] - (players.metal_s[ playerID] * 15 >> 4) ) * 0.001f;
        order_weight[ ORDER_ENERGY_S ] = Math::Max(0.0f, players.energy[ playerID ] - (players.energy_s[ playerID ] * 15 >> 4) ) * 0.001f;

        order_weight[ ORDER_ARMY ] = (nb_units[ AI_UNIT_TYPE_ENEMY ] - nb_units[ AI_UNIT_TYPE_ARMY ]) * 0.1f;
        order_weight[ ORDER_DEFENSE ] = (nb_units[ AI_UNIT_TYPE_ENEMY ] - nb_units[ AI_UNIT_TYPE_DEFENSE ]) * 0.1f;

        for (uint16 i = 0; i < 10; ++i)
        {
            order_attack[i] = nb_enemy[i] > 0 ? (nb_units[AI_UNIT_TYPE_ARMY] - nb_enemy[i]) * 0.1f : 0.0f;
            if (order_attack[i] < 0.0f)
                order_attack[i] = 0.0f;
            order_attack[i] = (1.0f - expf( -order_attack[i])) * 30.0f;
            # ifdef AI_DEBUG
            LOG_DEBUG(LOG_PREFIX_AI << "Attack player " << i << " = " << order_attack[i]);
            # endif
        }


        # ifdef AI_DEBUG
        LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)playerID << "," << msec_timer << ") thinking...");
        # endif

        float sw[ 10000 ];			// Used to compute the units that are built

        {
            sint16 player_target = -1;
            float best_weight = 15.0f;
            for(sint8 e = 0 ; e < players.nb_player; ++e)				// Who can we attack ?
            {
                // Don't attack allies
                if (order_attack[e] > best_weight && !(players.team[ playerID ] & players.team[ e ]))
                {
                    player_target = e;
                    best_weight = order_attack[e];
                }
            }

            if (player_target >= 0 ) // If we've someone to attack
            {
                for (std::list<uint16>::iterator i = army_list.begin() ; i != army_list.end() ; ++i ) // Give instructions to idle army units
                {
                    rest(1);
                    units.unit[ *i ].lock();
                    if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() )
                    {
                        sint16 target_id = -1;
                        while (!enemy_list[ player_target ].empty() && target_id == -1)
                        {
                            target_id = enemy_list[ player_target ].begin()->idx;
                            if (!(units.unit[ target_id ].flags & 1) || units.unit[ target_id ].type_id < 0
                                || units.unit[ target_id ].type_id >= unit_manager.nb_unit || units.unit[ target_id ].owner_id != player_target )
                            {
                                enemy_table[ target_id ] = false;
                                target_id = -1;
                                enemy_list[ player_target ].pop_front();		// Remove what we've just read
                            }
                            else
                            {
                                if (units.unit[ target_id ].cloaked && !units.unit[ target_id ].is_on_radar( 1 << playerID ) ) // This one is cloaked, not on radar
                                {
                                    enemy_table[ target_id ] = false;
                                    target_id = -1;
                                    enemy_list[ player_target ].pop_front();		// Remove what we've just read
                                    continue;
                                }
                                enemy_list[ player_target ].begin()->c++;
                            }
                        }
                        if (target_id >= 0 )
                            units.unit[ *i ].add_mission( MISSION_ATTACK, &units.unit[ target_id ].Pos, false, 0, (&units.unit[ target_id ]), NULL, MISSION_FLAG_COMMAND_FIRE );
                    }
                    units.unit[ *i ].unlock();
                }
            }
        }

        for( std::list<uint16>::iterator i = factory_list.begin() ; i != factory_list.end() ; ++i )	// Give instructions to idle factories
        {
            rest(1);
            units.unit[ *i ].lock();
            if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() && unit_manager.unit_type[units.unit[*i].type_id]->nb_unit > 0)
            {
                short list_size = unit_manager.unit_type[units.unit[*i].type_id]->nb_unit;
                std::vector<short> *BuildList = &(unit_manager.unit_type[units.unit[*i].type_id]->BuildList);
                for( int e = 0 ; e < list_size ; e++ )
                    sw[ e ] = (e > 0 ? sw[ e - 1 ] : 0.0f) + weights[ (*BuildList)[ e ] ].w;
                int selected_idx = -1;
                float selection = (TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
                if (sw[ list_size - 1 ] > 0.1f)
                    for (int e = 0 ; e < list_size ; ++e)
                    {
                        if (selection <= sw[ e ])
                        {
                            selected_idx = (*BuildList)[e];
                            break;
                        }
                    }
                # ifdef AI_DEBUG
                LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)playerID
                          << "," << msec_timer << ") -> factory " << *i << "building " << selected_idx);
                # endif
                if (selected_idx >= 0)
                {
                    units.unit[ *i ].add_mission( MISSION_BUILD, &units.unit[ *i ].Pos, false, selected_idx );
                    weights[ selected_idx ].w *= 0.8f;
                }
            }
            units.unit[ *i ].unlock();
        }

        // Give instructions to idle builders
        for( std::list<uint16>::iterator i = builder_list.begin() ; i != builder_list.end() ; ++i )
        {
            rest(1);

            units.unit[ *i ].lock();
            if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() && unit_manager.unit_type[units.unit[*i].type_id]->nb_unit > 0)
            {
                short list_size = unit_manager.unit_type[units.unit[*i].type_id]->nb_unit;
                std::vector<short> *BuildList = &(unit_manager.unit_type[units.unit[*i].type_id]->BuildList);
                for (int e = 0; e < list_size; ++e)
                    sw[e] = (e > 0 ? sw[e - 1] : 0.0f) + weights[ (*BuildList)[ e ] ].w;
                int selected_idx = -1;
                float selection = (TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
                if (sw[ list_size - 1 ] > 0.1f)
                    for( int e = 0 ; e < list_size ; e++ )
                    {
                        if (selection <= sw[ e ])
                        {
                            selected_idx = (*BuildList)[ e ];
                            break;
                        }
                    }
                Vector3D target = units.unit[ *i ].Pos;
                int px = (int)(target.x + the_map->map_w_d) >> 3;
                int py = (int)(target.z + the_map->map_h_d) >> 3;

                int spx = px;
                int spy = py;
                bool found = selected_idx < 0;
                int best_metal = 0;
                int metal_stuff_id = -1;
                bool extractor = selected_idx >= 0 ? unit_manager.unit_type[selected_idx]->ExtractsMetal > 0.0f : false;
                for( int r = 5 ; r < 50 && !found ; r++ ) 	// Circular check
                {
                    int r2 = r * r;
                    for( int y = (r>>1) ; y <= r && !found ; y++ )
                    {
                        int x = (int)(sqrtf( r2 - y * y ) + 0.5f);

                        int cx[] = { x, -x,  x, -x, y,  y, -y, -y };
                        int cy[] = { y,  y, -y, -y, x, -x,  x, -x };
                        int rand_t[8];
                        int rand_t2[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
                        for( int e = 0 ; e < 8 ; ++e)
                        {
                            int t = Math::RandFromTable() % (8 - e);
                            rand_t[e] = rand_t2[t];
                            for (int f = t; f < 7 - e; ++f)
                                rand_t2[f] = rand_t2[f + 1];
                        }

                        for( int f = 0 ; f < 8 ; f++ )
                        {
                            int e = rand_t[ f ];
                            if (can_be_there_ai( px + cx[e], py + cy[e], the_map, selected_idx, playerID ))
                            {
                                int stuff_id = -1;
                                int metal_found = extractor ? the_map->check_metal( px + cx[e], py + cy[e], selected_idx, &stuff_id ) : 0;
                                if ((extractor && metal_found > best_metal) || !extractor)
                                {
                                            // Prevent AI from filling a whole area with metal extractors
                                    if (extractor && stuff_id == -1
                                    && !can_be_there_ai( px + cx[e], py + cy[e], the_map, selected_idx, playerID, -1, true ))
                                        continue;
                                    spx = px + cx[e];
                                    spy = py + cy[e];
                                    if (metal_found > 0 && extractor)
                                    {
                                        best_metal = metal_found;
                                        metal_stuff_id = stuff_id;
                                        if (metal_stuff_id != -1)
                                            break;
                                    }
                                    else
                                    {
                                        found = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                px = spx;
                py = spy;
                found |= best_metal > 0;

                if (found && selected_idx >= 0)
                {
                    if (metal_stuff_id >= 0)        // We have a valid metal patch
                    {
                        px = features.feature[ metal_stuff_id ].px;
                        py = features.feature[ metal_stuff_id ].py;
                    }
                    target.x = (px << 3) - the_map->map_w_d;
                    target.z = (py << 3) - the_map->map_h_d;
                    target.y = Math::Max( the_map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[selected_idx]->FootprintX, unit_manager.unit_type[selected_idx]->FootprintZ ), the_map->sealvl);
                    units.unit[ *i ].add_mission( MISSION_BUILD, &target, false, selected_idx );
                    # ifdef AI_DEBUG
                    LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)playerID << "," << msec_timer
                              << ") -> builder " << *i << " building " << selected_idx);
                    # endif
                    weights[ selected_idx ].w *= 0.8f;
                }
                # ifdef AI_DEBUG
                else if (selected_idx >= 0)
                    LOG_WARNING(LOG_PREFIX_AI << "AI(" << (int)playerID << "," << msec_timer
                              << ") -> builder " << *i << " building " << selected_idx << ": No build place found");
                # endif
            }
            units.unit[ *i ].unlock();
        }

        float factory_needed = 0.0f;
        float builder_needed = 0.0f;
        for (uint16 i = 0 ; i < unit_manager.nb_unit; ++i)	// Build required units
            if (weights[i].w >= weights[i].o_w)
            {
                if (weights[i].built_by.empty())
                {
                    for (uint16 e = 0 ; e < unit_manager.nb_unit; ++e)
                    {
                        bool can_build = false;
                        for (uint16 f = 0; f < unit_manager.unit_type[e]->nb_unit && !can_build ; ++f)
                            can_build = unit_manager.unit_type[e]->BuildList[f] == i;
                        if (can_build)
                            weights[i].built_by.push_back(e);
                    }
                }
                for( std::list<uint16>::iterator e = weights[ i ].built_by.begin() ; e != weights[ i ].built_by.end() ; e++ )
                {
                    if (weights[ *e ].type & AI_UNIT_TYPE_FACTORY)
                        factory_needed += weights[ i ].w;
                    if (weights[ *e ].type & AI_UNIT_TYPE_BUILDER)
                        builder_needed += weights[ i ].w;
                    weights[ *e ].w = weights[ *e ].w < weights[ i ].w ? (weights[ *e ].w + weights[ i ].w) * 0.5f : weights[ *e ].w;
                }
                weights[ i ].w *= 0.5f;				// Don't need to keep trying to build it if we can't
            }

        order_weight[ ORDER_FACTORY ] = factory_needed;
        order_weight[ ORDER_BUILDER ] = builder_needed;

        for(int i = 0 ; i < NB_ORDERS ; ++i)
        {
            if (order_weight[i] < 0.0f )
                order_weight[i] = 0.0f;
            order_weight[i] = (1.0f - expf( -order_weight[i])) * 30.0f;
        }

#ifdef AI_DEBUG
        printf("AI %d :\nARMY = %f\nFACTORY = %f\nDEFENSE = %f\nBUILDER = %f\nMETAL_P = %f\nMETAL_S = %f\nENERGY_P = %f\nENERGY_S = %f\n",
               playerID, order_weight[ ORDER_ARMY ],order_weight[ ORDER_FACTORY ],order_weight[ ORDER_DEFENSE ],
               order_weight[ ORDER_BUILDER ],order_weight[ ORDER_METAL_P ],order_weight[ ORDER_METAL_S ],
               order_weight[ ORDER_ENERGY_P ],order_weight[ ORDER_ENERGY_S ]);
#endif

        return;							// Shortcut to prevent execution of this function because AI will be finished later
    }

    void	AI_CONTROLLER::proc(void*)
    {
        thread_running = true;
        thread_ask_to_stop = false;
        int speed = 10000;
        switch (AI_type)
        {
            case AI_TYPE_EASY    :speed = 10000; break;
            case AI_TYPE_MEDIUM  :speed = 5000;  break;
            case AI_TYPE_HARD    :speed = 2000;  break;
            case AI_TYPE_BLOODY  :speed = 1000;  break;
        }
        LOG_INFO(LOG_PREFIX_AI << "Started for player " << (int)playerID);
        while (!thread_ask_to_stop)
        {
            scan_unit();						// Look at the units

            if (unit_id == 0)	// When unit scanning is done
            {
                refresh_unit_weights();				// Refresh unit weights
                think();
            }

            float time_factor = units.apparent_timefactor;
            while ((time_factor == 0.0f || lp_CONFIG->pause) && !thread_ask_to_stop)
            {
                time_factor = units.apparent_timefactor;
                rest(10);
            }
            int time_to_wait = Math::Min( (int)(speed / ((units.nb_unit / 100 + 1) * time_factor)), 1 );
            for (int i = 0 ; i < time_to_wait && !thread_ask_to_stop; i += 100)		// Wait in order not to use all the CPU
                rest(Math::Min(100, time_to_wait - i));									// divide the wait call in order not to wait too much when game ends

        }
        LOG_INFO(LOG_PREFIX_AI << "Stopped for player " << (int)playerID);
        thread_running = false;
        thread_ask_to_stop = false;
    }


    void AI_CONTROLLER::signalExitThread()
    {
        LOG_INFO(LOG_PREFIX_AI << "Stopping for player " << (int)playerID << "...");
        thread_ask_to_stop = true;
        while (thread_running)
            rest(1);
        thread_ask_to_stop = false;
    }


    void AI_CONTROLLER::monitor()
    {
        if (!thread_running)
        {
            thread_running = true;
            start();
        }
    }

    void AI_CONTROLLER::init()
    {
        thread_running = false;
        thread_ask_to_stop = false;

        name = "default ai";
        decide.init();
        anticipate.init();
        playerID = 0;
        unit_id = 0;
        AI_type = AI_TYPE_EASY;
        total_unit = 0;

        weights = NULL;
        enemy_table = NULL;

        builder_list.clear();
        factory_list.clear();
        army_list.clear();

        for (int i = 0 ; i < 10 ; ++i)
            nb_enemy[i] = 0;

        order_weight[ORDER_ARMY] = 0.5f;
        order_weight[ORDER_METAL_P] = 2.0f;
        order_weight[ORDER_ENERGY_P] = 3.0f;
        order_weight[ORDER_METAL_S] = 0.0f;
        order_weight[ORDER_ENERGY_S] = 0.0f;
        order_weight[ORDER_DEFENSE] = 0.0f;
        order_weight[ORDER_FACTORY] = 5.0f;
        order_weight[ORDER_BUILDER] = 5.0f;
    }

    void AI_CONTROLLER::destroy()
    {
        destroyThread();

        builder_list.clear();
        factory_list.clear();
        army_list.clear();
        enemy_list.clear();

        decide.destroy();
        anticipate.destroy();
        playerID = 0;
        unit_id = 0;
        if (enemy_table)
        {
            delete[] enemy_table;
            enemy_table = NULL;
        }
        if (weights)
        {
            delete[] weights;
            weights = NULL;
        }
    }

    void AI_CONTROLLER::changeName(const String& newName)		// Change le nom de l'IA (conduit à la création d'un nouveau fichier)
    {
        pMutex.lock();
        name = newName;
        pMutex.unlock();
    }

    void AI_CONTROLLER::save()
    {
        String filename;
        Paths::MakeDir( Paths::Resources + "ai" );
        filename << Paths::Resources << "ai" << Paths::Separator << name << TA3D_AI_FILE_EXTENSION;
        FILE* file = TA3D_OpenFile(filename, "wb");

        byte l = (byte)name.size();
        fwrite(&l, 1, 1, file);		// Nom de l'IA
        fwrite(name.c_str(), l, 1, file);
        decide.save(file);			// Réseau de décision
        anticipate.save(file);		// Réseau d'analyse
        fclose(file);
    }


    void AI_CONTROLLER::loadAI(const String& filename, const int id)
    {
        TA3D_FILE* file = ta3d_fopen(filename);

        // Length of the name
        byte l;
        ta3d_fread(&l,1,file);

        // Reading the name
        char* n = new char[l+1];
        n[l]=0;
        ta3d_fread(n, l, file);
        name = n;
        delete[] n;

        decide.load(file);
        anticipate.load(file);
        ta3d_fclose(file);
        playerID = id;
    }

    AI_CONTROLLER::AI_CONTROLLER() : builder_list(), factory_list(), army_list(), enemy_list()
    {
        init();
    }

    AI_CONTROLLER::~AI_CONTROLLER()
    {
        destroy();
    }

    void AI_CONTROLLER::setPlayerID(int id)
    {
        lock();
        playerID = id;
        unlock();
    }

    int AI_CONTROLLER::getPlayerID()
    {
        return playerID;
    }

    void AI_CONTROLLER::setType(int type)
    {
        lock();
        AI_type = type;
        unlock();
    }

    int AI_CONTROLLER::getType()
    {
        return AI_type;
    }
} // namespace TA3D

