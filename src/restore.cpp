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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "matrix.h"

#include "cTA3D_Engine.h"		// The Engine
#include "ta3dbase.h"			// Some core include
#include "EngineClass.h"
#include "UnitEngine.h"

#define SAVE( i )	fwrite( &(i), sizeof( i ), 1, file )

void save_game( const String filename, GAME_DATA *game_data )
{
	FILE *file = TA3D_OpenFile( filename, "wb" );

	if( file ) {

		bool previous_pause_state = lp_CONFIG->pause;
		lp_CONFIG->pause = true;

		while( !lp_CONFIG->paused )	rest( 100 );			// Wait for the engine to enter in pause mode so we can save everything we want
															// without having the engine accessing its data
		fputs( "TA3D SAV", file );

//----- Save game information --------------------------------------------------------------

		fputs( game_data->map_filename, file );	fputc( 0, file );
		fputs( game_data->game_script, file );	fputc( 0, file );

		fputc( game_data->fog_of_war, file );			// flags to configure FOW
		fputc( game_data->campaign, file );				// Are we in campaign mode ?
		if( game_data->use_only )			// The use only file to read
			fputs( game_data->use_only, file );
		fputc( 0, file );

		SAVE( game_data->nb_players );
		
		foreach( game_data->player_names, i )		{	fputs( i->c_str(), file );	fputc( 0, file );	}
		foreach( game_data->player_sides, i )		{	fputs( i->c_str(), file );	fputc( 0, file );	}
		foreach( game_data->player_control, i )		fputc( *i, file );
		foreach( game_data->player_network_id, i )	SAVE( *i );
		foreach( game_data->ai_level, i )			fputc( *i, file );
		foreach( game_data->energy, i )				SAVE( *i );
		foreach( game_data->metal, i )				SAVE( *i );

//----- Save feature information -----------------------------------------------------------

		SAVE( features.nb_features );
		SAVE( features.max_features );
		for( int i = 0 ; i < features.max_features ; i++ ) {
			SAVE( features.feature[i].type );
			if( features.feature[i].type >= 0 ) {
				SAVE( features.feature[i].Pos );
				SAVE( features.feature[i].frame );
				SAVE( features.feature[i].hp );
				SAVE( features.feature[i].angle );
				SAVE( features.feature[i].burning );
				SAVE( features.feature[i].burning_time );
				SAVE( features.feature[i].time_to_burn );
				SAVE( features.feature[i].px );
				SAVE( features.feature[i].py );
				SAVE( features.feature[i].BW_idx );
				SAVE( features.feature[i].weapon_counter );
				SAVE( features.feature[i].last_spread );
				SAVE( features.feature[i].sinking );
				SAVE( features.feature[i].dive_speed );
				SAVE( features.feature[i].dive );
				SAVE( features.feature[i].angle_x );
				}
			}

//----- Save unit information --------------------------------------------------------------

		SAVE( units.nb_unit );
		SAVE( units.max_unit );
		SAVE( units.index_list_size );
		for( int i = 0 ; i < game_data->nb_players ; i++ )		SAVE( units.free_index_size[ i ] );

		foreach( units.repair_pads, pad_list ) {
			int list_size = pad_list->size();
			SAVE( list_size );
			foreach( *pad_list, i )	SAVE( *i );
			}

		for( int e = 0 ; e < units.nb_unit ; e++ ) {
			int i = units.idx_list[ e ];
			SAVE( i );
			SAVE( units.unit[i].flags );
			SAVE( units.unit[i].type_id );
			
			if( units.unit[i].type_id < 0 || !(units.unit[i].flags & 1) )	continue;
			
			int g = units.unit[i].s_var->size();
			SAVE( g );
			foreach( *units.unit[i].s_var, f )	SAVE( f );

			g = units.unit[i].script_val->size();
			SAVE( g );
			foreach( *units.unit[i].script_val, f )	SAVE( f );

			SAVE( units.unit[i].owner_id );
			SAVE( units.unit[i].hp );
			SAVE( units.unit[i].Pos );
			SAVE( units.unit[i].V );
			SAVE( units.unit[i].Angle );
			SAVE( units.unit[i].V_Angle );
			SAVE( units.unit[i].sel );

			fwrite( units.unit[i].port, sizeof( sint16 ), 21, file );

			SAVE( units.unit[i].nb_running );
			SAVE( units.unit[i].c_time );
			SAVE( units.unit[i].h );
			SAVE( units.unit[i].groupe );
			SAVE( units.unit[i].built );
			SAVE( units.unit[i].attacked );
			SAVE( units.unit[i].planned_weapons );
			fwrite( units.unit[i].memory, sizeof( int ), 10, file );
			SAVE( units.unit[i].mem_size );
			SAVE( units.unit[i].attached );
			fwrite( units.unit[i].attached_list, sizeof( short ), 20, file );
			fwrite( units.unit[i].link_list, sizeof( short ), 20, file );
			SAVE( units.unit[i].nb_attached );
			SAVE( units.unit[i].just_created );
			SAVE( units.unit[i].first_move );
			SAVE( units.unit[i].severity );
			SAVE( units.unit[i].cur_px );
			SAVE( units.unit[i].cur_py );
			SAVE( units.unit[i].metal_prod );
			SAVE( units.unit[i].metal_cons );
			SAVE( units.unit[i].energy_prod );
			SAVE( units.unit[i].energy_cons );
			SAVE( units.unit[i].cur_metal_prod );
			SAVE( units.unit[i].cur_metal_cons );
			SAVE( units.unit[i].cur_energy_prod );
			SAVE( units.unit[i].cur_energy_cons );
			for( int f = 0 ; f < 3 ; f++ ) {
				SAVE( units.unit[i].weapon[f].state );
				SAVE( units.unit[i].weapon[f].burst );
				SAVE( units.unit[i].weapon[f].stock );
				SAVE( units.unit[i].weapon[f].delay );
				SAVE( units.unit[i].weapon[f].time );
				SAVE( units.unit[i].weapon[f].target_pos );
				int g = units.unit[i].weapon[f].target ? ( (units.unit[i].weapon[f].state & WEAPON_FLAG_WEAPON) ? ((WEAPON*)units.unit[i].weapon[f].target)->idx : ((UNIT*)units.unit[i].weapon[f].target)->idx ) : 0;
				SAVE( g );
				SAVE( units.unit[i].weapon[f].data );
				SAVE( units.unit[i].weapon[f].flags );
				SAVE( units.unit[i].weapon[f].aim_dir );
				}
			SAVE( units.unit[i].was_moving );
			SAVE( units.unit[i].last_path_refresh );
			SAVE( units.unit[i].shadow_scale_dir );
			SAVE( units.unit[i].hidden );
			SAVE( units.unit[i].flying );
			SAVE( units.unit[i].cloaked );
			SAVE( units.unit[i].cloaking );
			SAVE( units.unit[i].drawn_open );
			SAVE( units.unit[i].drawn_flying );
			SAVE( units.unit[i].drawn_x );
			SAVE( units.unit[i].drawn_y );
			SAVE( units.unit[i].drawn );

			SAVE( units.unit[i].sight );
			SAVE( units.unit[i].radar_range );
			SAVE( units.unit[i].sonar_range );
			SAVE( units.unit[i].radar_jam_range );
			SAVE( units.unit[i].sonar_jam_range );
			SAVE( units.unit[i].old_px );
			SAVE( units.unit[i].old_py );

			SAVE( units.unit[i].move_target_computed );
			SAVE( units.unit[i].was_locked );

			SAVE( units.unit[i].self_destruct );
			SAVE( units.unit[i].build_percent_left );
			SAVE( units.unit[i].metal_extracted );

			SAVE( units.unit[i].requesting_pathfinder );
			SAVE( units.unit[i].pad1 );
			SAVE( units.unit[i].pad2 );
			SAVE( units.unit[i].pad_timer );

			SAVE( units.unit[i].command_locked );

			for( MISSION *start = units.unit[i].mission ; true ; start = units.unit[i].def_mission ) {
				for( MISSION *cur = start ; cur ; cur = cur->next ) {
					fputc( 1, file );
					SAVE( cur->mission );
					SAVE( cur->target );
					SAVE( cur->step );
					SAVE( cur->time );
					SAVE( cur->data );
					SAVE( cur->flags );
					SAVE( cur->last_d );
					SAVE( cur->move_data );
					SAVE( cur->node );
				
					for( PATH_NODE *path = cur->path ; path ; path = path->next ) {
						fputc( 1, file );
						SAVE( path->x );
						SAVE( path->y );
						SAVE( path->Pos );
						SAVE( path->made_direct );
						}
					fputc( 0, file );

					switch( cur->mission )
					{
					case MISSION_ATTACK:
						{
							int p = cur->p ? ( (cur->flags&MISSION_FLAG_TARGET_WEAPON) ? ((WEAPON*)(cur->p))->idx : ((UNIT*)(cur->p))->idx) : -1;
							SAVE( p );
						}
						break;
					case MISSION_BUILD:
					case MISSION_BUILD_2:
					case MISSION_REPAIR:
					case MISSION_GUARD:
					case MISSION_LOAD:
					case MISSION_CAPTURE:
					case MISSION_RECLAIM:
					case MISSION_REVIVE:
					case MISSION_GET_REPAIRED:
						{
							int p = cur->p ? ((UNIT*)(cur->p))->idx : -1;
							SAVE( p );
						}
						break;
					case MISSION_UNLOAD:
					case MISSION_GUARD_NOMOVE:
					case MISSION_PATROL:
					case MISSION_MOVE:
					case MISSION_STOP:
					case MISSION_STANDBY_MINE:
					case MISSION_STANDBY:
					case MISSION_VTOL_STANDBY:
					case MISSION_WAIT:
					case MISSION_WAIT_ATTACKED:
						break;
					};

					}
				fputc( 0, file );
				if( start == units.unit[i].def_mission )	break;
				}

			for( int e = 0 ; e < units.unit[i].nb_running ; e++ ) {
				SCRIPT_ENV *f = &((*units.unit[i].script_env)[e]);
				SAVE( f->wait );
				SAVE( f->running );
				
				for( SCRIPT_STACK *stack = f->stack ; stack ; stack = stack->next ) {
					fputc( 1, file );
					SAVE( stack->val );
					}
				fputc( 0, file );

				for( SCRIPT_ENV_STACK *stack = f->env ; stack ; stack = stack->next ) {
					fputc( 1, file );
					SAVE( stack->cur );
					SAVE( stack->signal_mask );
					fwrite( stack->var, sizeof( int ), 15, file );
					}
				fputc( 0, file );
				}

			SAVE( units.unit[i].data.nb_piece );
			SAVE( units.unit[i].data.explode_time );
			SAVE( units.unit[i].data.explode );
			SAVE( units.unit[i].data.is_moving );

			fwrite( units.unit[i].data.flag, sizeof( short ), units.unit[i].data.nb_piece, file );
			fwrite( units.unit[i].data.explosion_flag, sizeof( short ), units.unit[i].data.nb_piece, file );
			fwrite( units.unit[i].data.pos, sizeof( VECTOR ), units.unit[i].data.nb_piece, file );
			fwrite( units.unit[i].data.dir, sizeof( VECTOR ), units.unit[i].data.nb_piece, file );
			fwrite( units.unit[i].data.matrix, sizeof( MATRIX_4x4 ), units.unit[i].data.nb_piece, file );

			fwrite( units.unit[i].data.axe[0], sizeof( AXE ), units.unit[i].data.nb_piece, file );
			fwrite( units.unit[i].data.axe[1], sizeof( AXE ), units.unit[i].data.nb_piece, file );
			fwrite( units.unit[i].data.axe[2], sizeof( AXE ), units.unit[i].data.nb_piece, file );
			}

//----- Save weapon information ------------------------------------------------------------

		SAVE( weapons.nb_weapon );
		SAVE( weapons.max_weapon );
		SAVE( weapons.index_list_size );
		
		for( int e = 0 ; e < weapons.index_list_size ; e++ ) {
			int i = weapons.idx_list[e];
			SAVE( i );

			SAVE( weapons.weapon[i].weapon_id );
			SAVE( weapons.weapon[i].Pos );
			SAVE( weapons.weapon[i].V );
			SAVE( weapons.weapon[i].Ac );
			SAVE( weapons.weapon[i].target_pos );
			SAVE( weapons.weapon[i].target );
			SAVE( weapons.weapon[i].stime );
			SAVE( weapons.weapon[i].killtime );
			SAVE( weapons.weapon[i].dying );
			SAVE( weapons.weapon[i].smoke_time );
			SAVE( weapons.weapon[i].f_time );
			SAVE( weapons.weapon[i].a_time );
			SAVE( weapons.weapon[i].anim_sprite );
			SAVE( weapons.weapon[i].shooter_idx );
			SAVE( weapons.weapon[i].phase );
			SAVE( weapons.weapon[i].owner );
			SAVE( weapons.weapon[i].damage );
			SAVE( weapons.weapon[i].just_explode );
			}

		fclose( file );

		lp_CONFIG->pause = previous_pause_state;
		}
}

void load_game( const String filename, GAME_DATA *game_data )
{
}
