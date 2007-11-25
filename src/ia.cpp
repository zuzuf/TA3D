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
|                                     ia.cpp                                   |
|       Ce module est responsable de l'intelligence artificielle               |
|                                                                              |
\-----------------------------------------------------------------------------*/

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"					// Pour la lecture des fichiers 3D
#include "cob.h"					// Pour la lecture et l'éxecution des scripts
#include "tdf.h"					// Pour la gestion des éléments du jeu
#include "EngineClass.h"			// Inclus le moteur
#include "UnitEngine.h"				// Inclus le moteur
//#include "ia.h"

//#define AI_DEBUG

void BRAIN::save(FILE *file)		// Enregistre le réseau de neurones
{
	fwrite("BRAIN",5,1,file);			// Identifiant du format du fichier
	fwrite(&n,sizeof(int),1,file);		// Nombre d'entrées
	fwrite(&p,sizeof(int),1,file);		// Nombre de sorties
	fwrite(&q,sizeof(int),1,file);		// Taille de la couche intermédiaire

	for(int i=n;i<nb_neuron;i++)		// Enregistre les poids
		if(i<n+q)
			fwrite(neuron[i].weight,sizeof(float)*n,1,file);
		else
			fwrite(neuron[i].weight,sizeof(float)*q,1,file);
}

int BRAIN::load(FILE *file)		// Charge le réseau de neurones
{
	char tmp[6];

	fread(tmp,5,1,file);				// Identifiant du format du fichier
	tmp[5]=0;
	if(strcmp(tmp,"BRAIN")!=0)	{		// Vérifie si le fichier est bien du type attendu
		return 1;
		}

	destroy();		// Au cas où

	fread(&n,sizeof(int),1,file);		// Nombre d'entrées
	fread(&p,sizeof(int),1,file);		// Nombre de sorties
	fread(&q,sizeof(int),1,file);		// Taille de la couche intermédiaire
	nb_neuron=p+q+n;

	neuron=(NEURON*) malloc(sizeof(NEURON)*nb_neuron);
	n_out=(float*) malloc(sizeof(float)*p);

	for(int i=0;i<p;i++)
		n_out[i]=0.0f;

	for(int i=0;i<n;i++)
		neuron[i].weight=NULL;

	for(int i=n;i<nb_neuron;i++)		// Lit les poids
		if(i<n+q) {
			neuron[i].weight=(float*) malloc(sizeof(float)*n);
			fread(neuron[i].weight,sizeof(float)*n,1,file);
			}
		else {
			neuron[i].weight=(float*) malloc(sizeof(float)*q);
			fread(neuron[i].weight,sizeof(float)*q,1,file);
			}

	return 0;
}

BRAIN *copy_brain(BRAIN *brain,BRAIN *dst)			// Copie un réseau de neurones
{
	BRAIN *copy=dst;
	if(copy==NULL)
		copy=(BRAIN*) malloc(sizeof(BRAIN));
	copy->init();
	copy->nb_neuron=brain->nb_neuron;
	copy->n=brain->n;
	copy->p=brain->p;
	copy->q=brain->q;
	copy->neuron=(NEURON*) malloc(sizeof(NEURON)*copy->nb_neuron);
	copy->n_out=(float*) malloc(sizeof(float)*copy->p);
	for(int i=0;i<brain->nb_neuron;i++) {
		if(i<brain->p)
			copy->n_out[i]=0.0f;
		if(brain->neuron[i].weight==NULL)
			copy->neuron[i].weight=NULL;
		else {
			if(i>=copy->n && i<copy->nb_neuron-copy->p) {
				copy->neuron[i].weight=(float*) malloc(sizeof(float)*brain->q);
				for(int e=0;e<brain->q;e++)
					copy->neuron[i].weight[e]=brain->neuron[i].weight[e];
				}
			else if(i>=copy->n) {
				copy->neuron[i].weight=(float*) malloc(sizeof(float)*brain->n);
				for(int e=0;e<brain->n;e++)
					copy->neuron[i].weight[e]=brain->neuron[i].weight[e];
				}
			else
				copy->neuron[i].weight=NULL;
			}
		}
	return copy;
}

inline byte int2brain_value(int a)			// Evaluateur de quantité pour réseau de neurones
{
	if(a==0)	return BRAIN_VALUE_NULL;
	if(a<=5)	return BRAIN_VALUE_LOW;
	if(a<=15)	return BRAIN_VALUE_MEDIUM;
	if(a<=50)	return BRAIN_VALUE_HIGH;
	return BRAIN_VALUE_MAX;
}

inline int get_bits(float bits[],byte v,int pos)	// Remplit le tableau de bits (entrée des réseaux de neurones)
{
	for(int i=0;i<BRAIN_VALUE_BITS;i++)
		bits[pos++]=((v>>i)&1) ? 1.0f : 0.0f;
	return pos;
}

void AI_PLAYER::scan_unit()							// Scan the units the AI player currently has
{
	if( weights == NULL ) {
		weights = new AI_WEIGHT[ unit_manager.nb_unit ];

		for( uint16 i = 0 ; i < unit_manager.nb_unit ; i++ )
			if( strcasecmp( unit_manager.unit_type[ i ].side, players.side[ player_id ] ) == 0 ) {
				weights[ i ].type = 0;

				if( unit_manager.unit_type[ i ].canattack && unit_manager.unit_type[ i ].BMcode && !unit_manager.unit_type[ i ].commander ) {
					if( unit_manager.unit_type[ i ].canmove )
						weights[ i ].type |= AI_FLAG_ARMY;
					else
						weights[ i ].type |= AI_FLAG_DEFENSE;
					}

				if( unit_manager.unit_type[ i ].Builder ) {
					if( unit_manager.unit_type[ i ].BMcode )
						weights[ i ].type |= AI_FLAG_BUILDER;
					else
						weights[ i ].type |= AI_FLAG_FACTORY;
					}

				if( unit_manager.unit_type[ i ].MetalMake > 0.0f
				|| unit_manager.unit_type[ i ].MakesMetal > 0.0f || unit_manager.unit_type[ i ].ExtractsMetal > 0.0f ) {
						weights[ i ].type |= AI_FLAG_METAL_P;
						weights[ i ].metal_s = unit_manager.unit_type[ i ].MetalStorage * 0.001f;
						weights[ i ].metal_p = (unit_manager.unit_type[ i ].MetalMake + unit_manager.unit_type[ i ].MakesMetal) * 10.0f
												+ 5000.0f * unit_manager.unit_type[ i ].ExtractsMetal - unit_manager.unit_type[ i ].EnergyUse;
						}
				if( unit_manager.unit_type[ i ].MetalStorage ) {
						weights[ i ].type |= AI_FLAG_METAL_S;
						weights[ i ].metal_s = unit_manager.unit_type[ i ].MetalStorage * 0.001f;
						weights[ i ].metal_p = (unit_manager.unit_type[ i ].MetalMake + unit_manager.unit_type[ i ].MakesMetal) * 10.0f
												+ 5000.0f * unit_manager.unit_type[ i ].ExtractsMetal - unit_manager.unit_type[ i ].EnergyUse;
						}

				if( unit_manager.unit_type[ i ].EnergyMake || unit_manager.unit_type[ i ].EnergyUse < 0.0f
				|| unit_manager.unit_type[ i ].TidalGenerator || unit_manager.unit_type[ i ].WindGenerator ) {
						weights[ i ].type |= AI_FLAG_ENERGY_P;
						weights[ i ].energy_s = unit_manager.unit_type[ i ].EnergyStorage * 0.0001f;
						weights[ i ].energy_p = (unit_manager.unit_type[ i ].EnergyMake + unit_manager.unit_type[ i ].TidalGenerator
												+ unit_manager.unit_type[ i ].WindGenerator - unit_manager.unit_type[ i ].EnergyUse) * 0.01f;
						}
				if( unit_manager.unit_type[ i ].EnergyStorage ) {
						weights[ i ].type |= AI_FLAG_ENERGY_S;
						weights[ i ].energy_s = unit_manager.unit_type[ i ].EnergyStorage * 0.0001f;
						weights[ i ].energy_p = (unit_manager.unit_type[ i ].EnergyMake + unit_manager.unit_type[ i ].TidalGenerator
												+ unit_manager.unit_type[ i ].WindGenerator - unit_manager.unit_type[ i ].EnergyUse) * 0.01f;
						}
				}
			else
				weights[ i ].type = 0;
			enemy_list.resize( players.nb_player );
			for( uint16 i = 0 ; i < players.nb_player ; i++ )
				enemy_list[ i ].clear();
		}

	if( unit_id == 0 ) {
		builder_list.clear();
		factory_list.clear();
		army_list.clear();

		for( uint16 i = 0 ; i < unit_manager.nb_unit ; i++ )			// reset things if needed
			weights[ i ].nb = 0;

		for( uint16 i = 0 ; i < NB_AI_UNIT_TYPE ; i++ )
			nb_units[ i ] = 0;

		for( uint16 i = 0 ; i < 10 ; i++ )
			nb_enemy[ i ] = 0;
		}

//units.EnterCS_from_outside();
	int e;
	for( e = unit_id ; e < units.nb_unit && e < unit_id + 100 ; e++ ) {
		int i = units.idx_list[ e ];
		units.unit[ i ].Lock();
		if( units.unit[ i ].flags && units.unit[ i ].type_id >= 0 ) {
			if( units.unit[ i ].owner_id == player_id ) {
				weights[ units.unit[ i ].type_id ].nb++;
				if( weights[ units.unit[ i ].type_id ].type & AI_FLAG_ARMY )
					army_list.push_back( i );
				if( weights[ units.unit[ i ].type_id ].type & AI_FLAG_BUILDER )
					builder_list.push_back( i );
				if( weights[ units.unit[ i ].type_id ].type & AI_FLAG_FACTORY )
					factory_list.push_back( i );
				}
			else {
				nb_units[ AI_UNIT_TYPE_ENEMY ]++;
				nb_enemy[ units.unit[ i ].owner_id ]++;
				bool found = false;
				for( List<WEIGHT_COEF>::iterator e = enemy_list[ units.unit[ i ].owner_id ].begin() ; e != enemy_list[ units.unit[ i ].owner_id ].end() ; e++ )
					if( e->idx == i ) {
						found = true;
						break;
						}

				if( !found )
					enemy_list[ units.unit[ i ].owner_id ].push_back( WEIGHT_COEF( i, 0 ) );
				}
			}
		units.unit[ i ].UnLock();
		}
	unit_id = e >= units.nb_unit ? 0 : e;
//units.LeaveCS_from_outside();
}

void AI_PLAYER::refresh_unit_weights()				// Refresh unit weights according to the unit scan and the orders weights
{
	for( int i = 0 ; i < players.nb_player ; i++ )
		enemy_list[ i ].sort();

	total_unit = 0;

	for( uint16 i = 0 ; i < unit_manager.nb_unit ; i++ )
		total_unit += weights[ i ].nb;

	float total_unit_inv = 2.0f / (total_unit + 1);

	for( uint16 i = 0 ; i < unit_manager.nb_unit ; i++ ) {
		if( weights[ i ].type )	{
			if( weights[ i ].type & AI_FLAG_ARMY )	{
				nb_units[ AI_UNIT_TYPE_ARMY ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ARMY ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ARMY ];
				}
			if( weights[ i ].type & AI_FLAG_DEFENSE )	{
				nb_units[ AI_UNIT_TYPE_DEFENSE ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_DEFENSE ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_DEFENSE ];
				}
			if( weights[ i ].type & AI_FLAG_BUILDER )	{
				nb_units[ AI_UNIT_TYPE_BUILDER ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_BUILDER ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_BUILDER ];
				}
			if( weights[ i ].type & AI_FLAG_FACTORY )	{
				nb_units[ AI_UNIT_TYPE_FACTORY ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_FACTORY ] * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_FACTORY ];
				}
			if( weights[ i ].type & AI_FLAG_METAL_P )	{
				nb_units[ AI_UNIT_TYPE_METAL ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_METAL_P ] * weights[ i ].metal_p * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_METAL_P ] * weights[ i ].metal_p;
				}
			if( weights[ i ].type & AI_FLAG_METAL_S )	{
				nb_units[ AI_UNIT_TYPE_METAL ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_METAL_S ] * weights[ i ].metal_s * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_METAL_S ] * weights[ i ].metal_s;
				}
			if( weights[ i ].type & AI_FLAG_ENERGY_P )	{
				nb_units[ AI_UNIT_TYPE_ENERGY ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ENERGY_P ] * weights[ i ].energy_p * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ENERGY_P ] * weights[ i ].energy_p;
				}
			if( weights[ i ].type & AI_FLAG_ENERGY_S )	{
				nb_units[ AI_UNIT_TYPE_ENERGY ] += weights[ i ].nb;
				weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ENERGY_S ] * weights[ i ].energy_s * 0.1f + 1.0f - weights[ i ].nb * total_unit_inv) * exp( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ENERGY_S ] * weights[ i ].energy_s;
				}
			if( weights[ i ].w < 0.0f )
				weights[ i ].w = 0.0f;
			}
		else
			weights[ i ].w = 0.0f;
		weights[ i ].o_w = weights[ i ].w;
		}
}

void AI_PLAYER::think(MAP *map)				// La vrai fonction qui simule l'Intelligence Artificielle / The function that makes Artificial Intelligence work
{
	srand( msec_timer );

	order_weight[ ORDER_METAL_P ] = max( 0.0f, players.metal_u[ player_id ] - players.metal_t[ player_id ]) * 10.0f + max( 0.0f, players.metal[ player_id ] - (players.metal_s[ player_id ] >> 1) ) * 0.01f;
	order_weight[ ORDER_ENERGY_P ] = max( 0.0f, players.energy_u[ player_id ] - players.energy_t[ player_id ]) + max( 0.0f, players.energy[ player_id ] - (players.energy_s[ player_id ] >> 1) ) * 0.001f;
	order_weight[ ORDER_METAL_S ] = max( 0.0f, players.metal[ player_id ] - (players.metal_s[ player_id ] * 15 >> 4) ) * 0.001f;
	order_weight[ ORDER_ENERGY_S ] = max( 0.0f, players.energy[ player_id ] - (players.energy_s[ player_id ] * 15 >> 4) ) * 0.001f;

	order_weight[ ORDER_ARMY ] = (nb_units[ AI_UNIT_TYPE_ENEMY ] - nb_units[ AI_UNIT_TYPE_ARMY ]) * 0.1f;
	order_weight[ ORDER_DEFENSE ] = (nb_units[ AI_UNIT_TYPE_ENEMY ] - nb_units[ AI_UNIT_TYPE_DEFENSE ]) * 0.1f;

	for( uint16 i = 0 ; i < 10 ; i++ ) {
		order_attack[ i ] = nb_enemy[ i ] > 0 ? (nb_units[ AI_UNIT_TYPE_ARMY ] - nb_enemy[ i ]) * 0.1f : 0.0f;
		if( order_attack[ i ] < 0.0f )
			order_attack[ i ] = 0.0f;
		order_attack[ i ] = (1.0f - exp( -order_attack[ i ] )) * 30.0f;
#ifdef AI_DEBUG
		printf("attack player %d = %f\n", i, order_attack[ i ] );
#endif
		}


#ifdef AI_DEBUG
	Console->AddEntry( "AI(%d,%d) -> thinking", player_id, msec_timer );
#endif

	float sw[ 10000 ];			// Used to compute the units that are built

//units.EnterCS_from_outside();

	for( List<uint16>::iterator i = army_list.begin() ; i != army_list.end() ; i++ ) {				// Give instructions to idle army units
		units.unit[ *i ].Lock();
		if( units.unit[ *i ].flags && units.unit[ *i ].do_nothing() ) {
			sint16 player_target = -1;
			float best_weight = 15.0f;
			for( int e = 0 ; e < players.nb_player ; e++ )				// Who can we attack ?
				if( order_attack[ e ] > best_weight ) {
					player_target = e;
					best_weight = order_attack[ e ];
					}

			if( player_target >= 0 ) {								// If we've someone to attack
				sint16 target_id = -1;
				while( !enemy_list[ player_target ].empty() && target_id == -1 ) {
					target_id = enemy_list[ player_target ].begin()->idx;
					if( !(units.unit[ target_id ].flags & 1) || units.unit[ target_id ].type_id < 0
					|| units.unit[ target_id ].type_id >= unit_manager.nb_unit || units.unit[ target_id ].owner_id != player_target ) {
						target_id = -1;
						enemy_list[ player_target ].pop_front();		// Remove what we've just read
						}
					else
						enemy_list[ player_target ].begin()->c++;
					}
				if( target_id >= 0 )
					units.unit[ *i ].add_mission( MISSION_ATTACK, &units.unit[ target_id ].Pos, false, 0, (&units.unit[ target_id ]), NULL, MISSION_FLAG_COMMAND_FIRE );
				}
			}
		units.unit[ *i ].UnLock();
		}

	for( List<uint16>::iterator i = factory_list.begin() ; i != factory_list.end() ; i++ ) {				// Give instructions to idle factories
		units.unit[ *i ].Lock();
		if( (units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing() && unit_manager.unit_type[ units.unit[ *i ].type_id ].nb_unit > 0 ) {
			short list_size = unit_manager.unit_type[ units.unit[ *i ].type_id ].nb_unit;
			short *BuildList = unit_manager.unit_type[ units.unit[ *i ].type_id ].BuildList;
			for( int e = 0 ; e < list_size ; e++ )
				sw[ e ] = (e > 0 ? sw[ e - 1 ] : 0.0f) + weights[ BuildList[ e ] ].w;
			int selected_idx = 0;
			float selection = (TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
			for( int e = 0 ; e < list_size ; e++ )
				if( selection <= sw[ e ] ) {
					selected_idx = BuildList[ e ];
					break;
					}
#ifdef AI_DEBUG
			Console->AddEntry( "AI(%d,%d) -> factory %d building %d", player_id, msec_timer, *i, selected_idx );
#endif
			units.unit[ *i ].add_mission( MISSION_BUILD, &units.unit[ *i ].Pos, false, selected_idx );
			weights[ selected_idx ].w *= 0.8f;
			}
		units.unit[ *i ].UnLock();
		}

	for( List<uint16>::iterator i = builder_list.begin() ; i != builder_list.end() ; i++ ) {				// Give instructions to idle builders
		units.unit[ *i ].Lock();
		if( (units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing() && unit_manager.unit_type[ units.unit[ *i ].type_id ].nb_unit > 0 ) {
			short list_size = unit_manager.unit_type[ units.unit[ *i ].type_id ].nb_unit;
			short *BuildList = unit_manager.unit_type[ units.unit[ *i ].type_id ].BuildList;
			for( int e = 0 ; e < list_size ; e++ )
				sw[ e ] = (e > 0 ? sw[ e - 1 ] : 0.0f) + weights[ BuildList[ e ] ].w;
			int selected_idx = 0;
			float selection = (TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
			for( int e = 0 ; e < list_size ; e++ )
				if( selection <= sw[ e ] ) {
					selected_idx = BuildList[ e ];
					break;
					}
			VECTOR target = units.unit[ *i ].Pos;
			int px = (int)(target.x + map->map_w_d) >> 3;
			int py = (int)(target.z + map->map_h_d) >> 3;

			int spx = px;
			int spy = py;
			bool found = false;
			int best_metal = 0;
			for( int r = 5 ; r < 20 && !found ; r++ ) {		// Circular check
				int r2 = r * r;
				for( int y = (r>>1) ; y <= r && !found ; y++ ) {
					int x = (int)(sqrt( r2 - y * y ) + 0.5f);

					rest( 0 );

					int cx[] = { x, -x,  x, -x, y,  y, -y, -y };
					int cy[] = { y,  y, -y, -y, x, -x,  x, -x };
					int rand_t[8];
					int rand_t2[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
					for( int e = 0 ; e < 8 ; e++ ) {
						int t = rand_from_table() % (8 - e);
						rand_t[ e ] = rand_t2[ t ];
						for( int f = t ; f < 7 - e ; f++ )
							rand_t2[ f ] = rand_t2[ f + 1 ];
						}

					for( int f = 0 ; f < 8 ; f++ ) {
						int e = rand_t[ f ];
						if( can_be_there_ai( px + cx[e], py + cy[e], map, selected_idx, player_id ) ) {
							int metal_found = map->check_metal( px + cx[e], py + cy[e], selected_idx );
							if( ( unit_manager.unit_type[ selected_idx ].ExtractsMetal > 0.0f && metal_found > best_metal )
							|| unit_manager.unit_type[ selected_idx ].ExtractsMetal == 0.0f ) {
								spx = px + cx[e];
								spy = py + cy[e];
								if( metal_found > 0 )
									best_metal = metal_found;
								else {
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

			if( found ) {
				target.x = (px << 3) - map->map_w_d;
				target.z = (py << 3) - map->map_h_d;
				target.y = max( map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[ selected_idx ].FootprintX, unit_manager.unit_type[ selected_idx ].FootprintZ ), map->sealvl);
				units.unit[ *i ].add_mission( MISSION_BUILD, &target, false, selected_idx );
#ifdef AI_DEBUG
				Console->AddEntry( "AI(%d,%d) -> builder %d building %d", player_id, msec_timer, *i, selected_idx );
#endif
				weights[ selected_idx ].w *= 0.8f;
				}
#ifdef AI_DEBUG
			else
				Console->AddEntry( "AI(%d,%d) -> builder %d building %d, error: no build place found", player_id, msec_timer, *i, selected_idx );
#endif
			}
		units.unit[ *i ].UnLock();
		}

//units.LeaveCS_from_outside();

	float factory_needed = 0.0f;
	float builder_needed = 0.0f;
	for( uint16 i = 0 ; i < unit_manager.nb_unit ; i++ )				// Build required units
		if( weights[ i ].w >= weights[ i ].o_w ) {
			if( weights[ i ].built_by.empty() )
				for( uint16 e = 0 ; e < unit_manager.nb_unit ; e++ ) {
					bool can_build = false;
					for( uint16 f = 0 ; f < unit_manager.unit_type[ e ].nb_unit && !can_build ; f++ )
						can_build = unit_manager.unit_type[ e ].BuildList[ f ] == i;
					if( can_build )
						weights[ i ].built_by.push_back( e );
					}
			for( List<uint16>::iterator e = weights[ i ].built_by.begin() ; e != weights[ i ].built_by.end() ; e++ ) {
				if( weights[ *e ].type & AI_UNIT_TYPE_FACTORY )
					factory_needed += weights[ i ].w;
				if( weights[ *e ].type & AI_UNIT_TYPE_BUILDER )
					builder_needed += weights[ i ].w;
				weights[ *e ].w = weights[ *e ].w < weights[ i ].w ? (weights[ *e ].w + weights[ i ].w) * 0.5f : weights[ *e ].w;
				}
			weights[ i ].w *= 0.5f;				// Don't need to keep trying to build it if we can't
			}

	order_weight[ ORDER_FACTORY ] = factory_needed;
	order_weight[ ORDER_BUILDER ] = builder_needed;

	for( int i = 0 ; i < NB_ORDERS ; i++ ) {
		if( order_weight[ i ] < 0.0f )
			order_weight[ i ] = 0.0f;
		order_weight[ i ] = (1.0f - exp( -order_weight[ i ] )) * 30.0f;
		}

#ifdef AI_DEBUG
	printf("AI %d :\nARMY = %f\nFACTORY = %f\nDEFENSE = %f\nBUILDER = %f\nMETAL_P = %f\nMETAL_S = %f\nENERGY_P = %f\nENERGY_S = %f\n",
		player_id, order_weight[ ORDER_ARMY ],order_weight[ ORDER_FACTORY ],order_weight[ ORDER_DEFENSE ],
		order_weight[ ORDER_BUILDER ],order_weight[ ORDER_METAL_P ],order_weight[ ORDER_METAL_S ],
		order_weight[ ORDER_ENERGY_P ],order_weight[ ORDER_ENERGY_S ]);
#endif

	return;							// Shortcut to prevent execution of this function because AI will be finished later
}

int	AI_PLAYER::Run()
{
	thread_running = true;
	thread_ask_to_stop = false;
	int speed = 10000;
	switch( AI_type )
	{
	case AI_TYPE_EASY:
		speed = 10000;
		break;
	case AI_TYPE_MEDIUM:
		speed = 5000;
		break;
	case AI_TYPE_HARD:
		speed = 2000;
		break;
	case AI_TYPE_BLOODY:
		speed = 1000;
		break;
	};
	Console->AddEntry("AI thread started for player %d", player_id);
	while( !thread_ask_to_stop ) {

		scan_unit();						// Look at the units

		if( unit_id == 0 ) {				// When unit scanning is done
			refresh_unit_weights();				// Refresh unit weights

			think( the_map );
			}

		float time_factor = units.apparent_timefactor;
		while( time_factor == 0.0f ) {
			time_factor = units.apparent_timefactor;
			rest( 10 );
			}
		int time_to_wait = (int)( speed / ( ( units.nb_unit / 100 + 1 ) * time_factor ) );
		for( int i = 0 ; i < time_to_wait && !thread_ask_to_stop ; i += 100 )		// Wait in order not to use all the CPU
			rest( min( 100, time_to_wait - i ) );									// divide the wait call in order not to wait too much when game ends

		}
	Console->AddEntry("AI thread stopped for player %d", player_id);
	thread_running = false;
	thread_ask_to_stop = false;
	return 0;
}

void AI_PLAYER::SignalExitThread()
{
	Console->AddEntry("AI thread stopping for player %d", player_id);
	thread_ask_to_stop = true;
	while( thread_running )	rest( 1 );
	thread_ask_to_stop = false;
}

void AI_PLAYER::monitor()
{
	if( !thread_running )				// Start the thread if required
		Start();
}

void AI_PLAYER::init()
{
	thread_running = false;
	thread_ask_to_stop = false;

	name = strdup("default ai");
	decider.init();
	anticiper.init();
	player_id = 0;
	unit_id = 0;
	AI_type = AI_TYPE_EASY;
	total_unit = 0;

	weights = NULL;

	builder_list.clear();
	factory_list.clear();
	army_list.clear();

	for( int i = 0 ; i < 10 ; i++ )
		nb_enemy[ i ] = 0;

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

	if(name)	free(name);
	name = NULL;
	decider.destroy();
	anticiper.destroy();
	player_id=0;
	unit_id=0;
	if( weights ) {
		delete[] weights;
		weights = NULL;
		}
}

void AI_PLAYER::change_name(char *new_name)		// Change le nom de l'IA (conduit à la création d'un nouveau fichier)
{
	EnterCS();

	if(name)
		free(name);
	name=strdup(new_name);

	LeaveCS();
}

void AI_PLAYER::save()
{
	String filename = format( "ai/%s.ai", name );
	FILE *file=TA3D_OpenFile(filename,"wb");

	byte l=(byte)strlen(name);
	fwrite(&l,1,1,file);		// Nom de l'IA
	fwrite(name,l,1,file);

	decider.save(file);			// Réseau de décision

	anticiper.save(file);		// Réseau d'analyse

	fclose(file);
}

void AI_PLAYER::load(char *filename,int id)
{
	FILE *file=TA3D_OpenFile(filename,"rb");

	byte l;
	fread(&l,1,1,file);
	if(name)
		free(name);
	name=(char*) malloc(l+1);
	name[l]=0;
	fread(name,l,1,file);

	decider.load(file);

	anticiper.load(file);

	fclose(file);

	player_id=id;
}

