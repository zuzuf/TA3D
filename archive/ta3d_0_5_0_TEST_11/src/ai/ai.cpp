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

/*-----------------------------------------------------------------------------\
  |                                     ai.cpp                                   |
  |       Ce module est responsable de l'intelligence artificielle               |
  |                                                                              |
  \-----------------------------------------------------------------------------*/

#include "ai.h"
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




namespace TA3D
{



    //#define AI_DEBUG

    void BRAIN::build(int nb_in,int nb_out,int rg)				// Create the neural network
    {
        destroy();
        q=rg;
        nb_neuron=q+nb_in+nb_out;		// Number of layers * number of input NEURONs + number of output NEURONs
        n=nb_in;
        p=nb_out;
        neuron = new NEURON[nb_neuron];
        n_out = new float[nb_out];
        for(int i = 0; i < nb_neuron; ++i)
        {
            neuron[i].var=0.0f;
            neuron[i].weight=NULL;
            if (i<nb_out)
                n_out[i]=0.0f;
            if (i>=n && i<nb_neuron-p)
            {
                neuron[i].weight = new float[n];
                for(int e=0;e<n;e++)
                    neuron[i].weight[e]=(TA3D_RAND()%2001)*0.001f-1.0f;
            }
            else
            {
                if (i>=n)
                {
                    neuron[i].weight = new float[q];
                    for(int e=0;e<q;e++)
                        neuron[i].weight[e]=(TA3D_RAND()%2001)*0.001f-1.0f;
                }
            }
        }
    }



    void BRAIN::active_neuron(int i)
    {
        if (neuron[i].weight == NULL || i < n)
            return;
        neuron[i].var=0.0f;
        if (i < nb_neuron - p)
        {
            for(int e=0;e<n; ++e)
                neuron[i].var += neuron[e].var * neuron[i].weight[e];
        }
        else
        {
            for(int e = 0; e < q; ++e)
                neuron[i].var += neuron[n+e].var * neuron[i].weight[e];
        }
        neuron[i].var = 1.0f / (1.0f + exp(-neuron[i].var));
    }


    float *BRAIN::work(float entry[],bool seuil)			// Make NEURONs work and return the network results
    {
        if (nb_neuron < 0)
            return NULL;
        int i;
        for(i=0; i < n; ++i) // Prepare the network
            neuron[i].var=entry[i];
        for(i = n; i < nb_neuron; ++i)
            active_neuron(i);
        if (!seuil)
        {
            for(i = 0; i < p; ++i) // get the result
                n_out[i] = neuron[n+q+i].var;
        }
        else
        {
            for(i=0;i<p; ++i) // get the result
                n_out[i] = (neuron[n+q+i].var >= 0.5f) ? 1.0f : 0.0f;
        }
        return n_out;
    }


    void BRAIN::mutation()			// Make some changes to the neural network
    {
        int index = (TA3D_RAND() % (nb_neuron-n)) + n;
        int mod_w = 0;
        if (index < nb_neuron - p)
            mod_w=TA3D_RAND() % n;
        else
            mod_w = TA3D_RAND() % q;
        neuron[index].weight[mod_w] += ((TA3D_RAND() % 200001) - 100000) * 0.00001f;
    }


    void BRAIN::learn(float *result,float coef)		// Make it learn
    {
        for(int i=0;i<p; ++i)
            n_out[i] = (result[i]-n_out[i]) * (n_out[i]+0.01f) * (1.01f-n_out[i]);

        float *diff = new float[q];
        for(int i=0;i<q;++i)
            diff[i]=0.0f;

        for(int i=0;i<p;++i)		// Output layer
        {
            for(int e=0;e<q;e++)
            {
                diff[e]+=(n_out[i]+0.01f)*(1.01f-n_out[i])*neuron[n+q+i].weight[e]*neuron[n+e].var;
                neuron[n+q+i].weight[e]+=coef*n_out[i]*neuron[n+e].var;
            }
        }

        // Middle layers
        for(int i=0;i<q;++i)
        {
            for(int e=0;e<n;e++)
                neuron[n+i].weight[e]+=coef*diff[i]*neuron[e].var;
        }
        delete[] diff;
    }

    void BRAIN::save(FILE *file)		// Save the neural network
    {
        fwrite("BRAIN",5,1,file);			// File format ID
        fwrite(&n,sizeof(int),1,file);		// Inputs
        fwrite(&p,sizeof(int),1,file);		// Outputs
        fwrite(&q,sizeof(int),1,file);		// Size of middle layer

        for(int i=n;i<nb_neuron;++i)		// Save weights
        {
            if (i<n+q)
                fwrite(neuron[i].weight,sizeof(float)*n,1,file);
            else
                fwrite(neuron[i].weight,sizeof(float)*q,1,file);
        }
    }

    int BRAIN::load(TA3D_FILE *file)		// Load the neural network
    {
        char tmp[6];

        ta3d_fread(tmp,5,file); // File format ID
        tmp[5]=0;
        if (strcmp(tmp,"BRAIN") != 0)	// Check if it is what is expected
            return 1;

        destroy();		// clean the object

        ta3d_fread(&n,sizeof(int),file);		// Inputs
        ta3d_fread(&p,sizeof(int),file);		// Outputs
        ta3d_fread(&q,sizeof(int),file);		// Size of middle layer
        nb_neuron=p+q+n;

        neuron = new NEURON[nb_neuron];
        n_out = new float[p];

        for(int i=0;i<p; ++i)
            n_out[i]=0.0f;

        for(int i=0;i<n; ++i)
            neuron[i].weight = NULL;

        for(int i=n;i<nb_neuron; ++i)		// Read weights
        {
            if (i<n+q)
            {
                neuron[i].weight = new float[n];
                ta3d_fread(neuron[i].weight,sizeof(float)*n,file);
            }
            else
            {
                neuron[i].weight = new float[q];
                ta3d_fread(neuron[i].weight,sizeof(float)*q,file);
            }
        }
        return 0;
    }



    BRAIN *copy_brain(BRAIN *brain,BRAIN *dst)			// Make a copy
    {
        BRAIN *copy=dst;
        if (copy==NULL)
            copy = new BRAIN;
        copy->init();
        copy->nb_neuron=brain->nb_neuron;
        copy->n=brain->n;
        copy->p=brain->p;
        copy->q=brain->q;
        copy->neuron = new NEURON[copy->nb_neuron];
        copy->n_out = new float[copy->p];
        for(int i=0;i<brain->nb_neuron; ++i)
        {
            if (i<brain->p)
                copy->n_out[i]=0.0f;
            if (brain->neuron[i].weight==NULL)
                copy->neuron[i].weight=NULL;
            else
            {
                if (i>=copy->n && i<copy->nb_neuron-copy->p)
                {
                    copy->neuron[i].weight = new float[brain->q];
                    for(int e=0;e<brain->q;e++)
                        copy->neuron[i].weight[e]=brain->neuron[i].weight[e];
                }
                else
                {
                    if (i>=copy->n)
                    {
                        copy->neuron[i].weight = new float[brain->n];
                        for(int e=0;e<brain->n;e++)
                            copy->neuron[i].weight[e]=brain->neuron[i].weight[e];
                    }
                    else
                        copy->neuron[i].weight=NULL;
                }
            }
        }
        return copy;
    }

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
            bits[pos++]=((v>>i)&1) ? 1.0f : 0.0f;
        return pos;
    }

    void f_scan_unit( AI_PLAYER *ai )							// Scan the units the AI player currently has
    {
        if (ai->enemy_table == NULL)
        {
            ai->enemy_table = new byte[units.max_unit];
            memset(ai->enemy_table, 0, units.max_unit);
        }

        if (ai->weights == NULL )
        {
            ai->weights = new AI_WEIGHT[ unit_manager.nb_unit ];

            for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i)
                if (strcasecmp( unit_manager.unit_type[i]->side.c_str(), players.side[ ai->player_id ].c_str() ) == 0 )
                {
                    ai->weights[ i ].type = 0;

                    if (unit_manager.unit_type[i]->canattack && unit_manager.unit_type[i]->BMcode && !unit_manager.unit_type[i]->commander )
                    {
                        if (unit_manager.unit_type[i]->canmove )
                            ai->weights[ i ].type |= AI_FLAG_ARMY;
                        else
                            ai->weights[ i ].type |= AI_FLAG_DEFENSE;
                    }

                    if (unit_manager.unit_type[i]->Builder )
                    {
                        if (unit_manager.unit_type[i]->BMcode )
                            ai->weights[ i ].type |= AI_FLAG_BUILDER;
                        else
                            ai->weights[ i ].type |= AI_FLAG_FACTORY;
                    }

                    if (unit_manager.unit_type[i]->MetalMake > 0.0f
                        || unit_manager.unit_type[i]->MakesMetal > 0.0f || unit_manager.unit_type[i]->ExtractsMetal > 0.0f )
                    {
                        ai->weights[ i ].type |= AI_FLAG_METAL_P;
                        ai->weights[ i ].metal_s = unit_manager.unit_type[i]->MetalStorage * 0.001f;
                        ai->weights[ i ].metal_p = (unit_manager.unit_type[i]->MetalMake + unit_manager.unit_type[i]->MakesMetal) * 10.0f
                            + 5000.0f * unit_manager.unit_type[i]->ExtractsMetal - unit_manager.unit_type[i]->EnergyUse;
                    }
                    if (unit_manager.unit_type[i]->MetalStorage )
                    {
                        ai->weights[ i ].type |= AI_FLAG_METAL_S;
                        ai->weights[ i ].metal_s = unit_manager.unit_type[i]->MetalStorage * 0.001f;
                        ai->weights[ i ].metal_p = (unit_manager.unit_type[i]->MetalMake + unit_manager.unit_type[i]->MakesMetal) * 10.0f
                            + 5000.0f * unit_manager.unit_type[i]->ExtractsMetal - unit_manager.unit_type[i]->EnergyUse;
                    }
                    if (unit_manager.unit_type[i]->EnergyMake || unit_manager.unit_type[i]->EnergyUse < 0.0f
                        || unit_manager.unit_type[i]->TidalGenerator || unit_manager.unit_type[i]->WindGenerator )
                    {
                        ai->weights[ i ].type |= AI_FLAG_ENERGY_P;
                        ai->weights[ i ].energy_s = unit_manager.unit_type[i]->EnergyStorage * 0.0001f;
                        ai->weights[ i ].energy_p = (unit_manager.unit_type[i]->EnergyMake + unit_manager.unit_type[i]->TidalGenerator
                                                     + unit_manager.unit_type[i]->WindGenerator - unit_manager.unit_type[i]->EnergyUse) * 0.01f;
                    }
                    if (unit_manager.unit_type[i]->EnergyStorage )
                    {
                        ai->weights[ i ].type |= AI_FLAG_ENERGY_S;
                        ai->weights[ i ].energy_s = unit_manager.unit_type[i]->EnergyStorage * 0.0001f;
                        ai->weights[ i ].energy_p = (unit_manager.unit_type[i]->EnergyMake + unit_manager.unit_type[i]->TidalGenerator
                                                     + unit_manager.unit_type[i]->WindGenerator - unit_manager.unit_type[i]->EnergyUse) * 0.01f;
                    }
                }
                else
                    ai->weights[ i ].type = 0;
            ai->enemy_list.resize( players.nb_player );
            for( uint16 i = 0 ; i < players.nb_player ; ++i )
                ai->enemy_list[ i ].clear();
        }

        if (ai->unit_id == 0 )
        {
            ai->builder_list.clear();
            ai->factory_list.clear();
            ai->army_list.clear();

            for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )			// reset things if needed
                ai->weights[ i ].nb = 0;

            for( uint16 i = 0 ; i < NB_AI_UNIT_TYPE ; ++i )
                ai->nb_units[ i ] = 0;

            for( uint16 i = 0 ; i < 10 ; ++i )
                ai->nb_enemy[ i ] = 0;
        }

        int e;
        units.lock();
        for( e = ai->unit_id ; e < units.nb_unit && e < ai->unit_id + 100 ; e++ )
        {
            int i = units.idx_list[ e ];
            if (i < 0 || i >= units.max_unit )	continue;		// Error
            units.unlock();
            units.unit[i].lock();
            if ((units.unit[ i ].flags & 1) && units.unit[ i ].type_id >= 0 )
            {
                if (units.unit[ i ].owner_id == ai->player_id )
                {
                    ai->weights[ units.unit[ i ].type_id ].nb++;
                    if (ai->weights[ units.unit[ i ].type_id ].type & AI_FLAG_ARMY )
                        ai->army_list.push_back( i );
                    if (ai->weights[ units.unit[ i ].type_id ].type & AI_FLAG_BUILDER )
                        ai->builder_list.push_back( i );
                    if (ai->weights[ units.unit[ i ].type_id ].type & AI_FLAG_FACTORY )
                        ai->factory_list.push_back( i );
                }
                else
                {
                    ai->nb_units[ AI_UNIT_TYPE_ENEMY ]++;
                    ai->nb_enemy[ units.unit[ i ].owner_id ]++;
                    if (!ai->enemy_table[i])
                    {
                        ai->enemy_list[ units.unit[ i ].owner_id ].push_back( WEIGHT_COEF( i, 0 ) );
                        ai->enemy_table[ i ] = true;
                    }
                }
            }
            units.unit[ i ].unlock();
            units.lock();
        }
        units.unlock();
        ai->unit_id = e >= units.nb_unit ? 0 : e;
    }



    void f_refresh_unit_weights( AI_PLAYER *ai )				// Refresh unit weights according to the unit scan and the orders weights
    {
        for(sint8 i = 0 ; i < players.nb_player ; ++i )
            ai->enemy_list[i].sort();

        ai->total_unit = 0;

        for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )
            ai->total_unit += ai->weights[ i ].nb;

        float total_unit_inv = 2.0f / (ai->total_unit + 1);

        for( uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )
        {
            if (ai->weights[ i ].type && !unit_manager.unit_type[i]->not_used )
            {
                if (ai->weights[ i ].type & AI_FLAG_ARMY )
                {
                    ai->nb_units[ AI_UNIT_TYPE_ARMY ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_ARMY ] * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_ARMY ];
                }
                if (ai->weights[ i ].type & AI_FLAG_DEFENSE )
                {
                    ai->nb_units[ AI_UNIT_TYPE_DEFENSE ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_DEFENSE ] * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_DEFENSE ];
                }
                if (ai->weights[ i ].type & AI_FLAG_BUILDER )
                {
                    ai->nb_units[ AI_UNIT_TYPE_BUILDER ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_BUILDER ] * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_BUILDER ];
                }
                if (ai->weights[ i ].type & AI_FLAG_FACTORY )
                {
                    ai->nb_units[ AI_UNIT_TYPE_FACTORY ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_FACTORY ] * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_FACTORY ];
                }
                if (ai->weights[ i ].type & AI_FLAG_METAL_P )
                {
                    ai->nb_units[ AI_UNIT_TYPE_METAL ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_METAL_P ] * ai->weights[ i ].metal_p * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_METAL_P ] * ai->weights[ i ].metal_p;
                }
                if (ai->weights[ i ].type & AI_FLAG_METAL_S )
                {
                    ai->nb_units[ AI_UNIT_TYPE_METAL ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_METAL_S ] * ai->weights[ i ].metal_s * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_METAL_S ] * ai->weights[ i ].metal_s;
                }
                if (ai->weights[ i ].type & AI_FLAG_ENERGY_P )
                {
                    ai->nb_units[ AI_UNIT_TYPE_ENERGY ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_ENERGY_P ] * ai->weights[ i ].energy_p * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_ENERGY_P ] * ai->weights[ i ].energy_p;
                }
                if (ai->weights[ i ].type & AI_FLAG_ENERGY_S )
                {
                    ai->nb_units[ AI_UNIT_TYPE_ENERGY ] += ai->weights[ i ].nb;
                    ai->weights[ i ].w = (ai->weights[ i ].w + ai->order_weight[ ORDER_ENERGY_S ] * ai->weights[ i ].energy_s * 0.1f + 1.0f - ai->weights[ i ].nb * total_unit_inv) * exp( -0.1f * ai->weights[ i ].w ) + 0.1f * ai->order_weight[ ORDER_ENERGY_S ] * ai->weights[ i ].energy_s;
                }
                if (ai->weights[ i ].w < 0.0f )
                    ai->weights[ i ].w = 0.0f;
            }
            else
                ai->weights[ i ].w = 0.0f;
            ai->weights[ i ].o_w = ai->weights[ i ].w;
        }
    }

    void f_think( AI_PLAYER *ai, MAP *map )				// La vrai fonction qui simule l'Intelligence Artificielle / The function that makes Artificial Intelligence work
    {
        srand( msec_timer );

        ai->order_weight[ ORDER_METAL_P ] = Math::Max(0.0f, players.metal_u[ ai->player_id ] - players.metal_t[ai->player_id]) * 10.0f
            + Math::Max(0.0f, players.metal[ ai->player_id ] - (players.metal_s[ ai->player_id ] >> 1) ) * 0.01f;
        ai->order_weight[ ORDER_ENERGY_P ] = Math::Max(0.0f, players.energy_u[ ai->player_id ] - players.energy_t[ai->player_id])
            + Math::Max(0.0f, players.energy[ ai->player_id ] - (players.energy_s[ ai->player_id ] >> 1) ) * 0.001f;
        ai->order_weight[ ORDER_METAL_S ] = Math::Max(0.0f, players.metal[ ai->player_id ] - (players.metal_s[ ai->player_id] * 15 >> 4) ) * 0.001f;
        ai->order_weight[ ORDER_ENERGY_S ] = Math::Max(0.0f, players.energy[ ai->player_id ] - (players.energy_s[ ai->player_id ] * 15 >> 4) ) * 0.001f;

        ai->order_weight[ ORDER_ARMY ] = (ai->nb_units[ AI_UNIT_TYPE_ENEMY ] - ai->nb_units[ AI_UNIT_TYPE_ARMY ]) * 0.1f;
        ai->order_weight[ ORDER_DEFENSE ] = (ai->nb_units[ AI_UNIT_TYPE_ENEMY ] - ai->nb_units[ AI_UNIT_TYPE_DEFENSE ]) * 0.1f;

        for (uint16 i = 0; i < 10; ++i)
        {
            ai->order_attack[i] = ai->nb_enemy[i] > 0 ? (ai->nb_units[AI_UNIT_TYPE_ARMY] - ai->nb_enemy[i]) * 0.1f : 0.0f;
            if (ai->order_attack[i] < 0.0f )
                ai->order_attack[i] = 0.0f;
            ai->order_attack[i] = (1.0f - exp( -ai->order_attack[i])) * 30.0f;
            # ifdef AI_DEBUG
            LOG_DEBUG(LOG_PREFIX_AI << "Attack player " << i << " = " << ai->order_attack[i]);
            # endif
        }


        # ifdef AI_DEBUG
        LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)ai->player_id << "," << msec_timer << ") thinking...");
        # endif

        float sw[ 10000 ];			// Used to compute the units that are built

        {
            sint16 player_target = -1;
            float best_weight = 15.0f;
            for(sint8 e = 0 ; e < players.nb_player; ++e)				// Who can we attack ?
            {
                // Don't attack allies
                if (ai->order_attack[e] > best_weight && !(players.team[ ai->player_id ] & players.team[ e ]))
                {
                    player_target = e;
                    best_weight = ai->order_attack[e];
                }
            }

            if (player_target >= 0 ) // If we've someone to attack
            {
                for (std::list<uint16>::iterator i = ai->army_list.begin() ; i != ai->army_list.end() ; ++i ) // Give instructions to idle army units
                {
                    rest(1);
                    units.unit[ *i ].lock();
                    if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() )
                    {
                        sint16 target_id = -1;
                        while (!ai->enemy_list[ player_target ].empty() && target_id == -1)
                        {
                            target_id = ai->enemy_list[ player_target ].begin()->idx;
                            if (!(units.unit[ target_id ].flags & 1) || units.unit[ target_id ].type_id < 0
                                || units.unit[ target_id ].type_id >= unit_manager.nb_unit || units.unit[ target_id ].owner_id != player_target )
                            {
                                ai->enemy_table[ target_id ] = false;
                                target_id = -1;
                                ai->enemy_list[ player_target ].pop_front();		// Remove what we've just read
                            }
                            else
                            {
                                if (units.unit[ target_id ].cloaked && !units.unit[ target_id ].is_on_radar( 1 << ai->player_id ) ) // This one is cloaked, not on radar
                                {
                                    ai->enemy_table[ target_id ] = false;
                                    target_id = -1;
                                    ai->enemy_list[ player_target ].pop_front();		// Remove what we've just read
                                    continue;
                                }
                                ai->enemy_list[ player_target ].begin()->c++;
                            }
                        }
                        if (target_id >= 0 )
                            units.unit[ *i ].add_mission( MISSION_ATTACK, &units.unit[ target_id ].Pos, false, 0, (&units.unit[ target_id ]), NULL, MISSION_FLAG_COMMAND_FIRE );
                    }
                    units.unit[ *i ].unlock();
                }
            }
        }

        for( std::list<uint16>::iterator i = ai->factory_list.begin() ; i != ai->factory_list.end() ; ++i )	// Give instructions to idle factories
        {
            rest(1);
            units.unit[ *i ].lock();
            if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() && unit_manager.unit_type[units.unit[*i].type_id]->nb_unit > 0 ) {
                short list_size = unit_manager.unit_type[units.unit[*i].type_id]->nb_unit;
                std::vector<short> *BuildList = &(unit_manager.unit_type[units.unit[*i].type_id]->BuildList);
                for( int e = 0 ; e < list_size ; e++ )
                    sw[ e ] = (e > 0 ? sw[ e - 1 ] : 0.0f) + ai->weights[ (*BuildList)[ e ] ].w;
                int selected_idx = -1;
                float selection = (TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
                if (sw[ list_size - 1 ] > 0.1f)
                    for (int e = 0 ; e < list_size ; ++e)
                    {
                        if (selection <= sw[ e ] )
                        {
                            selected_idx = (*BuildList)[e];
                            break;
                        }
                    }
                # ifdef AI_DEBUG
                LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)ai->player_id
                          << "," << msec_timer << ") -> factory " << *i << "building " << selected_idx);
                # endif
                if (selected_idx >= 0)
                {
                    units.unit[ *i ].add_mission( MISSION_BUILD, &units.unit[ *i ].Pos, false, selected_idx );
                    ai->weights[ selected_idx ].w *= 0.8f;
                }
            }
            units.unit[ *i ].unlock();
        }

        // Give instructions to idle builders
        for( std::list<uint16>::iterator i = ai->builder_list.begin() ; i != ai->builder_list.end() ; ++i )
        {
            rest(1);

            units.unit[ *i ].lock();
            if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() && unit_manager.unit_type[units.unit[*i].type_id]->nb_unit > 0)
            {
                short list_size = unit_manager.unit_type[units.unit[*i].type_id]->nb_unit;
                std::vector<short> *BuildList = &(unit_manager.unit_type[units.unit[*i].type_id]->BuildList);
                for (int e = 0; e < list_size; ++e)
                    sw[e] = (e > 0 ? sw[e - 1] : 0.0f) + ai->weights[ (*BuildList)[ e ] ].w;
                int selected_idx = -1;
                float selection = (TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
                if (sw[ list_size - 1 ] > 0.1f)
                    for( int e = 0 ; e < list_size ; e++ )
                    {
                        if (selection <= sw[ e ] )
                        {
                            selected_idx = (*BuildList)[ e ];
                            break;
                        }
                    }
                Vector3D target = units.unit[ *i ].Pos;
                int px = (int)(target.x + map->map_w_d) >> 3;
                int py = (int)(target.z + map->map_h_d) >> 3;

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
                        int x = (int)(sqrt( r2 - y * y ) + 0.5f);

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
                            if (can_be_there_ai( px + cx[e], py + cy[e], map, selected_idx, ai->player_id ))
                            {
                                int stuff_id = -1;
                                int metal_found = extractor ? map->check_metal( px + cx[e], py + cy[e], selected_idx, &stuff_id ) : 0;
                                if ((extractor && metal_found > best_metal) || !extractor)
                                {
                                            // Prevent AI from filling a whole area with metal extractors
                                    if (extractor && stuff_id == -1
                                    && !can_be_there_ai( px + cx[e], py + cy[e], map, selected_idx, ai->player_id, -1, true ))
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
                    target.x = (px << 3) - map->map_w_d;
                    target.z = (py << 3) - map->map_h_d;
                    target.y = Math::Max( map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[selected_idx]->FootprintX, unit_manager.unit_type[selected_idx]->FootprintZ ), map->sealvl);
                    units.unit[ *i ].add_mission( MISSION_BUILD, &target, false, selected_idx );
                    # ifdef AI_DEBUG
                    LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)ai->player_id << "," << msec_timer
                              << ") -> builder " << *i << " building " << selected_idx);
                    # endif
                    ai->weights[ selected_idx ].w *= 0.8f;
                }
                # ifdef AI_DEBUG
                else if (selected_idx >= 0)
                    LOG_WARNING(LOG_PREFIX_AI << "AI(" << (int)ai->player_id << "," << msec_timer
                              << ") -> builder " << *i << " building " << selected_idx << ": No build place found");
                # endif
            }
            units.unit[ *i ].unlock();
        }

        float factory_needed = 0.0f;
        float builder_needed = 0.0f;
        for (uint16 i = 0 ; i < unit_manager.nb_unit; ++i)	// Build required units
            if (ai->weights[i].w >= ai->weights[i].o_w)
            {
                if (ai->weights[i].built_by.empty() )
                {
                    for (uint16 e = 0 ; e < unit_manager.nb_unit; ++e)
                    {
                        bool can_build = false;
                        for (uint16 f = 0; f < unit_manager.unit_type[e]->nb_unit && !can_build ; ++f)
                            can_build = unit_manager.unit_type[e]->BuildList[f] == i;
                        if (can_build )
                            ai->weights[i].built_by.push_back(e);
                    }
                }
                for( std::list<uint16>::iterator e = ai->weights[ i ].built_by.begin() ; e != ai->weights[ i ].built_by.end() ; e++ )
                {
                    if (ai->weights[ *e ].type & AI_UNIT_TYPE_FACTORY )
                        factory_needed += ai->weights[ i ].w;
                    if (ai->weights[ *e ].type & AI_UNIT_TYPE_BUILDER )
                        builder_needed += ai->weights[ i ].w;
                    ai->weights[ *e ].w = ai->weights[ *e ].w < ai->weights[ i ].w ? (ai->weights[ *e ].w + ai->weights[ i ].w) * 0.5f : ai->weights[ *e ].w;
                }
                ai->weights[ i ].w *= 0.5f;				// Don't need to keep trying to build it if we can't
            }

        ai->order_weight[ ORDER_FACTORY ] = factory_needed;
        ai->order_weight[ ORDER_BUILDER ] = builder_needed;

        for(int i = 0 ; i < NB_ORDERS ; ++i)
        {
            if (ai->order_weight[i] < 0.0f )
                ai->order_weight[i] = 0.0f;
            ai->order_weight[i] = (1.0f - exp( -ai->order_weight[i])) * 30.0f;
        }

#ifdef AI_DEBUG
        printf("AI %d :\nARMY = %f\nFACTORY = %f\nDEFENSE = %f\nBUILDER = %f\nMETAL_P = %f\nMETAL_S = %f\nENERGY_P = %f\nENERGY_S = %f\n",
               ai->player_id, ai->order_weight[ ORDER_ARMY ],ai->order_weight[ ORDER_FACTORY ],ai->order_weight[ ORDER_DEFENSE ],
               ai->order_weight[ ORDER_BUILDER ],ai->order_weight[ ORDER_METAL_P ],ai->order_weight[ ORDER_METAL_S ],
               ai->order_weight[ ORDER_ENERGY_P ],ai->order_weight[ ORDER_ENERGY_S ]);
#endif

        return;							// Shortcut to prevent execution of this function because AI will be finished later
    }

    void AI_PLAYER::scan_unit()
    {
        fp_scan_unit(this);
    }

    void AI_PLAYER::refresh_unit_weights()
    {
        fp_refresh_unit_weights(this);
    }

    void AI_PLAYER::think(MAP *map)
    {
        fp_think(this, map);
    }


    int	AI_PLAYER::Run()
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
        LOG_INFO(LOG_PREFIX_AI << "Started for player " << (int)player_id);
        while (!thread_ask_to_stop)
        {
            scan_unit();						// Look at the units

            if (unit_id == 0)	// When unit scanning is done
            {
                refresh_unit_weights();				// Refresh unit weights
                think(the_map);
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
        LOG_INFO(LOG_PREFIX_AI << "Stopped for player " << (int)player_id);
        thread_running = false;
        thread_ask_to_stop = false;
        return 0;
    }


    void AI_PLAYER::SignalExitThread()
    {
        LOG_INFO(LOG_PREFIX_AI << "Stopping for player " << (int)player_id << "...");
        thread_ask_to_stop = true;
        while (thread_running)
            rest(1);
        thread_ask_to_stop = false;
    }


    void AI_PLAYER::monitor()
    {
        if (!thread_running)
        {
            thread_running = true;
            Start();
        }
    }

    void AI_PLAYER::init()
    {
        fp_scan_unit = f_scan_unit;
        fp_refresh_unit_weights = f_refresh_unit_weights;
        fp_think = f_think;

        thread_running = false;
        thread_ask_to_stop = false;

        name = "default ai";
        decide.init();
        anticipate.init();
        player_id = 0;
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

    void AI_PLAYER::destroy()
    {
        DestroyThread();

        builder_list.clear();
        factory_list.clear();
        army_list.clear();
        enemy_list.clear();

        decide.destroy();
        anticipate.destroy();
        player_id=0;
        unit_id=0;
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

    void AI_PLAYER::change_name(const String& newName)		// Change le nom de l'IA (conduit à la création d'un nouveau fichier)
    {
        pMutex.lock();
        name = newName;
        pMutex.unlock();
    }

    void AI_PLAYER::save()
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


    void AI_PLAYER::load(const String& filename, const int id)
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
        player_id = id;
    }



} // namespace TA3D

