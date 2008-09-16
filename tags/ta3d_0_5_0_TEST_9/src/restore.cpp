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
#include <list>
#include <vector>
#include "TA3D_NameSpace.h"
#include "misc/matrix.h"
#include "cTA3D_Engine.h"		// The Engine
#include "ta3dbase.h"			// Some core include
#include "EngineClass.h"
#include "UnitEngine.h"
#include "restore.h"
#include "ingame/players.h"



#define SAVE( i )	fwrite( &(i), sizeof( i ), 1, file )
#define LOAD( i )	fread( &(i), sizeof( i ), 1, file )
inline char *readstring( char *str, int limit, FILE *file )
{
    for (int f = 0; f < limit - 1; ++f)
    {
        str[f] = fgetc(file);
        if (str[f] == 0)
            break;
    }
    return str;
}


void save_game( const String filename, GameData *game_data )
{
    FILE *file = TA3D_OpenFile( filename, "wb" );

    if( file == NULL )
        return;

    bool previous_pause_state = lp_CONFIG->pause;
    lp_CONFIG->pause = true;

    while (!lp_CONFIG->paused)
        rest( 100 );			// Wait for the engine to enter in pause mode so we can save everything we want
    // without having the engine accessing its data
    fputs( "TA3D SAV", file );

    if (network_manager.isConnected())     // Multiplayer game ?
    {
        fputc( 'M', file );                 // Save connection data
        if (network_manager.isServer())    // Are we the server ? (only server can resume the game)
            fputc( 1, file );
        else
            fputc( 0, file );
    }
    else
        fputc( 'S', file );

    //----- Save game information --------------------------------------------------------------

    fputs( game_data->map_filename.c_str(), file );	fputc( 0, file );
    fputs( game_data->game_script.c_str(), file );	fputc( 0, file );

    fputc( game_data->fog_of_war, file );			// flags to configure FOW
    fputc( game_data->campaign, file );				// Are we in campaign mode ?
    if( !game_data->use_only.empty() )			// The use only file to read
        fputs( game_data->use_only.c_str(), file );
    fputc( 0, file );

    SAVE( game_data->nb_players );

    // Palyer names
    for (String::Vector::const_iterator i = game_data->player_names.begin(); i != game_data->player_names.end(); ++i)
    {
        fputs(i->c_str(), file);
        fputc(0, file );
    }
    // Player sides
    for (String::Vector::const_iterator i = game_data->player_sides.begin(); i != game_data->player_sides.end(); ++i)
    {
        fputs(i->c_str(), file);
        fputc( 0, file );
    }
    // Player control
    for (std::vector<byte>::const_iterator i = game_data->player_control.begin(); i != game_data->player_control.end(); ++i)
        fputc(*i, file);
    // Player network ID
    for (std::vector<int>::const_iterator i = game_data->player_network_id.begin(); i != game_data->player_network_id.end(); ++i)
        SAVE(*i);
    // Teams
    for (std::vector<uint16>::const_iterator i = game_data->team.begin(); i != game_data->team.end(); ++i)
        SAVE(*i);
    // AI Levels
    for (std::vector<byte>::const_iterator i = game_data->ai_level.begin(); i != game_data->ai_level.end(); ++i)
        fputc(*i, file);
    // Energy
    for (std::vector<uint32>::const_iterator i = game_data->energy.begin(); i != game_data->energy.end(); ++i)
        SAVE(*i);
    // Metal
    for (std::vector<uint32>::const_iterator i = game_data->metal.begin(); i != game_data->metal.end(); ++i)
        SAVE(*i);

    // Color map
    for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        SAVE(player_color_map[i]);

    for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
    {
        SAVE( players.energy[i] );
        SAVE( players.metal[i] );
        SAVE( players.metal_u[i] );
        SAVE( players.energy_u[i] );
        SAVE( players.metal_t[i] );
        SAVE( players.energy_t[i] );
        SAVE( players.kills[i] );
        SAVE( players.losses[i] );
        SAVE( players.energy_s[i] );
        SAVE( players.metal_s[i] );
        SAVE( players.com_metal[i] );
        SAVE( players.com_energy[i] );
        SAVE( players.commander[i] );
        SAVE( players.annihilated[i] );
        SAVE( players.nb_unit[i] );

        //		Variables used to compute the data we need ( because of threading )

        SAVE( players.c_energy[i] );
        SAVE( players.c_metal[i] );
        SAVE( players.c_energy_s[i] );
        SAVE( players.c_metal_s[i] );
        SAVE( players.c_commander[i] );
        SAVE( players.c_annihilated[i] );
        SAVE( players.c_nb_unit[i] );
        SAVE( players.c_metal_u[i] );
        SAVE( players.c_energy_u[i] );
        SAVE( players.c_metal_t[i] );
        SAVE( players.c_energy_t[i] );

        // For statistic purpose only
        SAVE( players.energy_total[i] );
        SAVE( players.metal_total[i] );
    }
    //----- Save feature information -----------------------------------------------------------

    SAVE( features.nb_features );
    SAVE( features.max_features );
    for( int i = 0 ; i < features.max_features ; i++ )
    {
        SAVE( features.feature[i].type );
        if( features.feature[i].type >= 0 )
        {
            fputs( feature_manager.feature[features.feature[i].type].name.c_str(), file );		// Store the name so it doesn't rely on the feature order
            fputc( 0, file );
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

    //----- Save weapon information ------------------------------------------------------------

    SAVE( weapons.nb_weapon );
    SAVE( weapons.max_weapon );
    SAVE( weapons.index_list_size );

    for(unsigned int e = 0 ; e < weapons.index_list_size ; ++e)
    {
        int i = weapons.idx_list[e];
        SAVE( i );

        SAVE( weapons.weapon[i].weapon_id );
        if( weapons.weapon[i].weapon_id == -1 )	continue;

        fputs( weapon_manager.weapon[weapons.weapon[i].weapon_id].internal_name.c_str(), file );
        fputc( 0, file );

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

    //----- Save unit information --------------------------------------------------------------

    SAVE( units.nb_unit );
    SAVE( units.max_unit );
    SAVE( units.next_unit_ID );

    for (INGAME_UNITS::RepairPodsList::const_iterator pad_list = units.repair_pads.begin(); units.repair_pads.end() != pad_list; ++pad_list)
    {
        int list_size = pad_list->size();
        SAVE(list_size);
        for (std::list<uint16>::const_iterator i = pad_list->begin(); pad_list->end() != i; ++i)
            SAVE(*i);
    }

    for (int e = 0 ; e < units.nb_unit ; ++e)
    {
        int i = units.idx_list[ e ];
        SAVE( i );
        SAVE( units.unit[i].flags );
        SAVE( units.unit[i].type_id );

        if (units.unit[i].type_id < 0 || !(units.unit[i].flags & 1))
            continue;

        SAVE( units.unit[i].ID );		// Store its ID so we don't lose its "name"

        fputs( unit_manager.unit_type[units.unit[i].type_id]->Unitname.c_str(), file );		// Store the name so it doesn't rely on the feature order
        fputc( 0, file );

        int g = units.unit[i].s_var->size();
        SAVE( g );
        for (std::vector<int>::const_iterator f = (units.unit[i].s_var)->begin(); (units.unit[i].s_var)->end() != f; ++f)
            SAVE(*f);

        g = units.unit[i].script_val->size();
        SAVE( g );
        for (std::vector<short>::const_iterator f = (units.unit[i].script_val)->begin(); (units.unit[i].script_val)->end() != f; ++f)
            SAVE(*f);

        SAVE( units.unit[i].owner_id );
        SAVE( units.unit[i].hp );
        SAVE( units.unit[i].Pos );
        SAVE( units.unit[i].V );
        SAVE( units.unit[i].Angle );
        SAVE( units.unit[i].V_Angle );
        SAVE( units.unit[i].sel );
        SAVE( units.unit[i].death_delay );
        SAVE( units.unit[i].paralyzed );

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
        for (short int f = 0; f < units.unit[i].weapon.size(); ++f)
        {
            SAVE( units.unit[i].weapon[f].state );
            SAVE( units.unit[i].weapon[f].burst );
            SAVE( units.unit[i].weapon[f].stock );
            SAVE( units.unit[i].weapon[f].delay );
            SAVE( units.unit[i].weapon[f].time );
            SAVE( units.unit[i].weapon[f].target_pos );
            int g = units.unit[i].weapon[f].target ? (int)( (units.unit[i].weapon[f].state & WEAPON_FLAG_WEAPON) ? ((WEAPON*)units.unit[i].weapon[f].target)->idx : ((UNIT*)units.unit[i].weapon[f].target)->idx ) : -1;
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

        for (MISSION *start = units.unit[i].mission; true; start = units.unit[i].def_mission)
        {
            for (MISSION *cur = start; cur; cur = cur->next)
            {
                fputc( 1, file );
                SAVE( cur->mission );
                SAVE( cur->target );
                SAVE( cur->target_ID );
                SAVE( cur->step );
                SAVE( cur->time );
                SAVE( cur->data );
                SAVE( cur->flags );
                SAVE( cur->last_d );
                SAVE( cur->move_data );
                SAVE( cur->node );

                for( PATH_NODE *path = cur->path; path ; path = path->next)
                {
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
                            int p = cur->p ? ( (cur->flags&MISSION_FLAG_TARGET_WEAPON) ? (int)((WEAPON*)(cur->p))->idx : ((UNIT*)(cur->p))->idx) : -1;
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
                            int p = cur->p ? (int)((UNIT*)(cur->p))->idx : -1;
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
                }

            }
            fputc( 0, file );
            if( start == units.unit[i].def_mission )	break;
        }

        for (int e = 0; e < units.unit[i].nb_running; ++e)
        {
            SCRIPT_ENV *f = &((*units.unit[i].script_env)[e]);
            SAVE( f->wait );
            SAVE( f->running );

            for( SCRIPT_STACK *stack = f->stack ; stack ; stack = stack->next)
            {
                fputc( 1, file );
                SAVE( stack->val );
            }
            fputc( 0, file );

            for (SCRIPT_ENV_STACK *stack = f->env; stack; stack = stack->next)
            {
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
        fwrite( units.unit[i].data.pos, sizeof( Vector3D), units.unit[i].data.nb_piece, file );
        fwrite( units.unit[i].data.dir, sizeof( Vector3D), units.unit[i].data.nb_piece, file );
        fwrite( units.unit[i].data.matrix, sizeof( MATRIX_4x4 ), units.unit[i].data.nb_piece, file );

        fwrite( units.unit[i].data.axe[0], sizeof( AXE ), units.unit[i].data.nb_piece, file );
        fwrite( units.unit[i].data.axe[1], sizeof( AXE ), units.unit[i].data.nb_piece, file );
        fwrite( units.unit[i].data.axe[2], sizeof( AXE ), units.unit[i].data.nb_piece, file );
    }

    SAVE( units.current_tick );     // We'll need this for multiplayer games

    if (game_data->fog_of_war)      // Save fog of war state
    {
        for (int y = 0 ; y < the_map->view_map->h ; y++)
            fwrite( the_map->view_map->line[y], bitmap_color_depth(the_map->view_map) >> 3, the_map->view_map->w, file );

        for (int y = 0 ; y < the_map->sight_map->h ; y++)
            fwrite( the_map->sight_map->line[y], bitmap_color_depth(the_map->sight_map) >> 3, the_map->sight_map->w, file );

        for (int y = 0 ; y < the_map->radar_map->h ; y++)
            fwrite( the_map->radar_map->line[y], bitmap_color_depth(the_map->radar_map) >> 3, the_map->radar_map->w, file );

        for (int y = 0 ; y < the_map->sonar_map->h ; y++)
            fwrite( the_map->sonar_map->line[y], bitmap_color_depth(the_map->sonar_map) >> 3, the_map->sonar_map->w, file );
    }

    fclose( file );

    lp_CONFIG->pause = previous_pause_state;
}

bool load_game_data( const String filename, GameData *game_data, bool loading )
{
    FILE *file = TA3D_OpenFile( filename, "rb" );

    if( file == NULL )	return false;

    char tmp[1024];
    tmp[8] = 0;

    fread( tmp, 8, 1, file );
    if (strcmp(tmp, "TA3D SAV"))// Check format identifier
    {
        fclose( file );
        return false;
    }

    bool network = false;

    if (fgetc(file) == 'M')         // Multiplayer saved game
    {
        network = true;
        if (fgetc(file))            // We are server
        {
        }
        else                        // We are client
        {
            if (!loading)
            {
                fclose( file );
                return true;
            }
        }
    }

    //----- Load game information --------------------------------------------------------------

    readstring( tmp, 1024, file );
    strcat( tmp, ".tnt" );
    game_data->map_filename = tmp;
    game_data->game_script = readstring( tmp, 1024, file );

    game_data->fog_of_war = fgetc( file );			// flags to configure FOW
    game_data->campaign = fgetc( file );			// Are we in campaign mode ?
    readstring( tmp, 1024, file );
    if( tmp[0] )
        game_data->use_only = tmp;
    else
        game_data->use_only = "";

    LOAD( game_data->nb_players );

    // Palyer names
    for (String::Vector::iterator i = game_data->player_names.begin(); i != game_data->player_names.end(); ++i)
        *i = readstring( tmp, 1024, file );
    // Player sides
    for (String::Vector::iterator i = game_data->player_sides.begin(); i != game_data->player_sides.end(); ++i)
        *i = readstring( tmp, 1024, file );
    // Player control
    for (std::vector<byte>::iterator i = game_data->player_control.begin(); i != game_data->player_control.end(); ++i)
        *i = fgetc( file );
    // Player network ID
    for (std::vector<int>::iterator i = game_data->player_network_id.begin(); i != game_data->player_network_id.end(); ++i)
        LOAD(*i);
    // Teams
    for (std::vector<uint16>::iterator i = game_data->team.begin(); i != game_data->team.end(); ++i)
        LOAD(*i);
    // AI Levels
    for (std::vector<byte>::iterator i = game_data->ai_level.begin(); i != game_data->ai_level.end(); ++i)
        *i = fgetc( file );
    // Energy
    for (std::vector<uint32>::iterator i = game_data->energy.begin(); i != game_data->energy.end(); ++i)
        LOAD(*i);
    // Metal
    for (std::vector<uint32>::iterator i = game_data->metal.begin(); i != game_data->metal.end(); ++i)
        LOAD(*i);

    for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        LOAD( player_color_map[i] );

    game_data->saved_file = filename;

    fclose(file);
    return network;
}

void load_game( GameData *game_data )
{
    FILE *file = TA3D_OpenFile( game_data->saved_file, "rb" );

    if( file == NULL )	return;

    char tmp[1024];
    tmp[8] = 0;

    fread( tmp, 8, 1, file );
    if( strcmp( tmp, "TA3D SAV" ) )	// Check format identifier
    {
        fclose( file );
        return;
    }

    if (fgetc(file) == 'M')         // Multiplayer saved game
    {
        if (fgetc(file))            // We are server
        {
        }
        else                        // We are client
        {
        }
    }

    //----- Load game information --------------------------------------------------------------

    readstring( tmp, 1024, file );				// map
    readstring( tmp, 1024, file );				// game script

    fgetc( file );			// flags to configure FOW
    fgetc( file );			// Are we in campaign mode ?
    readstring( tmp, 1024, file );		// Useonly file

    LOAD( game_data->nb_players );		// nb players


    // Palyer names
    for (String::Vector::iterator i = game_data->player_names.begin(); i != game_data->player_names.end(); ++i)
        *i = readstring( tmp, 1024, file );
    // Player sides
    for (String::Vector::iterator i = game_data->player_sides.begin(); i != game_data->player_sides.end(); ++i)
        *i = readstring( tmp, 1024, file );
    // Player control
    for (std::vector<byte>::iterator i = game_data->player_control.begin(); i != game_data->player_control.end(); ++i)
        *i = fgetc( file );
    // Player network ID
    for (std::vector<int>::iterator i = game_data->player_network_id.begin(); i != game_data->player_network_id.end(); ++i)
        LOAD(*i);
    // Teams
    for (std::vector<uint16>::iterator i = game_data->team.begin(); i != game_data->team.end(); ++i)
        LOAD(*i);
    // AI Levels
    for (std::vector<byte>::iterator i = game_data->ai_level.begin(); i != game_data->ai_level.end(); ++i)
        *i = fgetc( file );
    // Energy
    for (std::vector<uint32>::iterator i = game_data->energy.begin(); i != game_data->energy.end(); ++i)
        LOAD(*i);
    // Metal
    for (std::vector<uint32>::iterator i = game_data->metal.begin(); i != game_data->metal.end(); ++i)
        LOAD(*i);


    for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        LOAD( player_color_map[i] );

    for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
    {
        LOAD( players.energy[i] );
        LOAD( players.metal[i] );
        LOAD( players.metal_u[i] );
        LOAD( players.energy_u[i] );
        LOAD( players.metal_t[i] );
        LOAD( players.energy_t[i] );
        LOAD( players.kills[i] );
        LOAD( players.losses[i] );
        LOAD( players.energy_s[i] );
        LOAD( players.metal_s[i] );
        LOAD( players.com_metal[i] );
        LOAD( players.com_energy[i] );
        LOAD( players.commander[i] );
        LOAD( players.annihilated[i] );
        LOAD( players.nb_unit[i] );

        //		Variables used to compute the data we need ( because of threading )

        LOAD( players.c_energy[i] );
        LOAD( players.c_metal[i] );
        LOAD( players.c_energy_s[i] );
        LOAD( players.c_metal_s[i] );
        LOAD( players.c_commander[i] );
        LOAD( players.c_annihilated[i] );
        LOAD( players.c_nb_unit[i] );
        LOAD( players.c_metal_u[i] );
        LOAD( players.c_energy_u[i] );
        LOAD( players.c_metal_t[i] );
        LOAD( players.c_energy_t[i] );

        // For statistic purpose only
        LOAD( players.energy_total[i] );
        LOAD( players.metal_total[i] );
    }

    the_map->clean_map();			// Reset yardmap

    //----- Load feature information -----------------------------------------------------------

    features.destroy();

    LOAD( features.nb_features );
    LOAD( features.max_features );

    features.feature = new FEATURE_DATA[features.max_features];
    for (int i = features.nb_features - 1; i < features.max_features; ++i)
    {
        features.feature[i].type = -1;
        features.feature[i].shadow_dlist = 0;
        features.feature[i].delete_shadow_dlist = false;
    }
    features.resetListOfItemsToDisplay();

    for (int i = 0 ; i < features.max_features ; ++i)
    {
        LOAD( features.feature[i].type );
        if( features.feature[i].type >= 0 )
        {
            readstring( tmp, 1024, file );
            features.feature[i].type = feature_manager.get_feature_index( tmp );

            LOAD( features.feature[i].Pos );
            LOAD( features.feature[i].frame );
            LOAD( features.feature[i].hp );
            LOAD( features.feature[i].angle );
            LOAD( features.feature[i].burning );
            LOAD( features.feature[i].burning_time );
            LOAD( features.feature[i].time_to_burn );
            LOAD( features.feature[i].px );
            LOAD( features.feature[i].py );
            LOAD( features.feature[i].BW_idx );
            LOAD( features.feature[i].weapon_counter );
            LOAD( features.feature[i].last_spread );
            LOAD( features.feature[i].sinking );
            LOAD( features.feature[i].dive_speed );
            LOAD( features.feature[i].dive );
            LOAD( features.feature[i].angle_x );

            if( features.feature[i].px < 0 || features.feature[i].py < 0
                || features.feature[i].px >= the_map->bloc_w_db || features.feature[i].py >= the_map->bloc_h_db ) // Out of the map ?
            {
                features.feature[i].type = -1;
                features.nb_features--;
                continue;
            }

            if( features.feature[i].burning )	features.burning_features.push_back( i );
            if( features.feature[i].sinking )	features.sinking_features.push_back( i );

            the_map->map_data[features.feature[i].py][features.feature[i].px].stuff = i;
            features.drawFeatureOnMap( i );
        }
    }

    //----- Load weapon information ------------------------------------------------------------

    weapons.lock();

    if( weapons.weapon )	delete[] weapons.weapon;			// Tableau regroupant les armes
    if( weapons.idx_list )	delete[] weapons.idx_list;
    if( weapons.free_idx )	delete[] weapons.free_idx;

    LOAD( weapons.nb_weapon );
    LOAD( weapons.max_weapon );
    LOAD( weapons.index_list_size );

    weapons.weapon = new WEAPON[weapons.max_weapon];

    weapons.idx_list = new uint32[weapons.max_weapon];
    weapons.free_idx = new uint32[weapons.max_weapon];
    weapons.free_index_size = 0;
    for(unsigned int e = 0 ; e < weapons.max_weapon ; ++e)
        weapons.weapon[e].weapon_id = -1;

    for (unsigned int e = 0 ; e < weapons.index_list_size ; ++e)
    {
        int i;
        LOAD( i );
        weapons.idx_list[e] = i;

        LOAD( weapons.weapon[i].weapon_id );
        if( weapons.weapon[i].weapon_id == -1 )	continue;
        readstring( tmp, 1024, file );
        weapons.weapon[i].weapon_id = weapon_manager.get_weapon_index( tmp );

        LOAD( weapons.weapon[i].Pos );
        LOAD( weapons.weapon[i].V );
        LOAD( weapons.weapon[i].Ac );
        LOAD( weapons.weapon[i].target_pos );
        LOAD( weapons.weapon[i].target );
        LOAD( weapons.weapon[i].stime );
        LOAD( weapons.weapon[i].killtime );
        LOAD( weapons.weapon[i].dying );
        LOAD( weapons.weapon[i].smoke_time );
        LOAD( weapons.weapon[i].f_time );
        LOAD( weapons.weapon[i].a_time );
        LOAD( weapons.weapon[i].anim_sprite );
        LOAD( weapons.weapon[i].shooter_idx );
        LOAD( weapons.weapon[i].phase );
        LOAD( weapons.weapon[i].owner );
        LOAD( weapons.weapon[i].damage );
        LOAD( weapons.weapon[i].just_explode );
    }

    for (unsigned int e = 0 ; e < weapons.max_weapon ; ++e)
        if( weapons.weapon[e].weapon_id == -1 )
            weapons.free_idx[ weapons.free_index_size++ ] = e;

    weapons.unlock();

    //----- Load unit information --------------------------------------------------------------

    units.destroy(false);

    LOAD( units.nb_unit );
    LOAD( units.max_unit );
    LOAD( units.next_unit_ID );

    for (INGAME_UNITS::RepairPodsList::iterator pad_list = units.repair_pads.begin(); units.repair_pads.end() != pad_list; ++pad_list)
    {
        int list_size;
        LOAD( list_size );
        pad_list->resize( list_size );
        for (std::list<uint16>::iterator i = pad_list->begin(); pad_list->end() != i; ++i)
            LOAD( *i );
    }

    units.lock();

    units.mini_col = new uint32[ units.max_unit ];
    units.mini_pos = new float[ units.max_unit * 2 ];

    units.unit =  new UNIT[units.max_unit];
    units.idx_list = new uint16[units.max_unit];
    units.free_idx = new uint16[units.max_unit];

    for (short int i = 0; i < 10; ++i)
        units.free_index_size[i] = 0;
    for(int i = 0 ; i < units.max_unit; ++i)
    {
        units.unit[i].init(-1,-1,true);
        units.unit[i].flags = 0;
        units.unit[i].idx = i;
    }

    units.index_list_size = 0;
    for (int e = 0 ; e < units.nb_unit ; ++e)
    {
        int i;
        LOAD( i );
        LOAD( units.unit[i].flags );
        LOAD( units.unit[i].type_id );
        int player_id = i / MAX_UNIT_PER_PLAYER;

        if( units.unit[i].type_id < 0 || !(units.unit[i].flags & 1) )
            continue;
        units.idx_list[units.index_list_size++] = i;

        uint32 ID;
        LOAD( ID );

        readstring( tmp, 1024, file );
        units.unit[i].type_id = unit_manager.get_unit_index( tmp );

        units.unit[i].init( units.unit[i].type_id, player_id, false, true );

        units.unit[i].ID = ID;

        int g;
        LOAD( g );
        units.unit[i].s_var->resize(g);
        for (std::vector<int>::iterator f = (units.unit[i].s_var)->begin(); (units.unit[i].s_var)->end() != f; ++f)
            LOAD( *f );

        LOAD( g );
        units.unit[i].script_val->resize(g);
        for (std::vector<short>::iterator f = (units.unit[i].script_val)->begin(); (units.unit[i].script_val)->end() != f; ++f)
            LOAD( *f );

        LOAD( units.unit[i].owner_id );
        LOAD( units.unit[i].hp );
        LOAD( units.unit[i].Pos );
        LOAD( units.unit[i].V );
        LOAD( units.unit[i].Angle );
        LOAD( units.unit[i].V_Angle );
        LOAD( units.unit[i].sel );
        LOAD( units.unit[i].death_delay );
        LOAD( units.unit[i].paralyzed );

        fread( units.unit[i].port, sizeof( sint16 ), 21, file );

        LOAD( units.unit[i].nb_running );
        LOAD( units.unit[i].c_time );
        LOAD( units.unit[i].h );
        LOAD( units.unit[i].groupe );
        LOAD( units.unit[i].built );
        LOAD( units.unit[i].attacked );
        LOAD( units.unit[i].planned_weapons );
        fread( units.unit[i].memory, sizeof( int ), 10, file );
        LOAD( units.unit[i].mem_size );
        LOAD( units.unit[i].attached );
        fread( units.unit[i].attached_list, sizeof( short ), 20, file );
        fread( units.unit[i].link_list, sizeof( short ), 20, file );
        LOAD( units.unit[i].nb_attached );
        LOAD( units.unit[i].just_created );
        LOAD( units.unit[i].first_move );
        LOAD( units.unit[i].severity );
        LOAD( units.unit[i].cur_px );
        LOAD( units.unit[i].cur_py );
        LOAD( units.unit[i].metal_prod );
        LOAD( units.unit[i].metal_cons );
        LOAD( units.unit[i].energy_prod );
        LOAD( units.unit[i].energy_cons );
        LOAD( units.unit[i].cur_metal_prod );
        LOAD( units.unit[i].cur_metal_cons );
        LOAD( units.unit[i].cur_energy_prod );
        LOAD( units.unit[i].cur_energy_cons );
        for (short int f = 0; f < units.unit[i].weapon.size(); ++f)
        {
            LOAD( units.unit[i].weapon[f].state );
            LOAD( units.unit[i].weapon[f].burst );
            LOAD( units.unit[i].weapon[f].stock );
            LOAD( units.unit[i].weapon[f].delay );
            LOAD( units.unit[i].weapon[f].time );
            LOAD( units.unit[i].weapon[f].target_pos );
            int g;
            LOAD( g );
            units.unit[i].weapon[f].target = (g == -1) ? NULL :	( (units.unit[i].weapon[f].state & WEAPON_FLAG_WEAPON) ? (void*)&(weapons.weapon[g]) : (void*)&(units.unit[g]) );
            LOAD( units.unit[i].weapon[f].data );
            LOAD( units.unit[i].weapon[f].flags );
            LOAD( units.unit[i].weapon[f].aim_dir );
        }
        LOAD( units.unit[i].was_moving );
        LOAD( units.unit[i].last_path_refresh );
        LOAD( units.unit[i].shadow_scale_dir );
        LOAD( units.unit[i].hidden );
        LOAD( units.unit[i].flying );
        LOAD( units.unit[i].cloaked );
        LOAD( units.unit[i].cloaking );
        LOAD( units.unit[i].drawn_open );
        LOAD( units.unit[i].drawn_flying );
        LOAD( units.unit[i].drawn_x );
        LOAD( units.unit[i].drawn_y );
        LOAD( units.unit[i].drawn );

        LOAD( units.unit[i].sight );
        LOAD( units.unit[i].radar_range );
        LOAD( units.unit[i].sonar_range );
        LOAD( units.unit[i].radar_jam_range );
        LOAD( units.unit[i].sonar_jam_range );
        LOAD( units.unit[i].old_px );
        LOAD( units.unit[i].old_py );

        LOAD( units.unit[i].move_target_computed );
        LOAD( units.unit[i].was_locked );

        LOAD( units.unit[i].self_destruct );
        LOAD( units.unit[i].build_percent_left );
        LOAD( units.unit[i].metal_extracted );

        LOAD( units.unit[i].requesting_pathfinder );
        LOAD( units.unit[i].pad1 );
        LOAD( units.unit[i].pad2 );
        LOAD( units.unit[i].pad_timer );

        LOAD( units.unit[i].command_locked );

        for( MISSION **start = &(units.unit[i].mission) ; true ; start = &(units.unit[i].def_mission) )
        {
            byte c = fgetc( file );
            if( c == 0 )
                *start = NULL;
            else
            {
                MISSION **cur = start;
                do
                {
                    *cur = new MISSION;
                    (*cur)->next = NULL;
                    LOAD( (*cur)->mission );
                    LOAD( (*cur)->target );
                    LOAD( (*cur)->target_ID );
                    LOAD( (*cur)->step );
                    LOAD( (*cur)->time );
                    LOAD( (*cur)->data );
                    LOAD( (*cur)->flags );
                    LOAD( (*cur)->last_d );
                    LOAD( (*cur)->move_data );
                    LOAD( (*cur)->node );

                    (*cur)->path = NULL;
                    PATH_NODE **path = &((*cur)->path);
                    while( fgetc( file ) )
                    {
                        *path = new PATH_NODE;
                        (*path)->next = NULL;
                        LOAD( (*path)->x );
                        LOAD( (*path)->y );
                        LOAD( (*path)->Pos );
                        LOAD( (*path)->made_direct );
                        path = &((*path)->next);
                    }

                    switch( (*cur)->mission )
                    {
                        case MISSION_ATTACK:
                            {
                                int p;
                                LOAD( p );
                                (*cur)->p = (p == -1) ? NULL : ( ((*cur)->flags&MISSION_FLAG_TARGET_WEAPON) ? (void*)&(weapons.weapon[p]) : (void*)&(units.unit[p]) );
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
                                int p;
                                LOAD( p );
                                (*cur)->p = (p == -1) ? NULL : (void*)&(units.unit[p]);
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
                            (*cur)->p = NULL;
                            break;
                    };

                    cur = &((*cur)->next);

                }while( fgetc( file ) );
            }
            if( start == &(units.unit[i].def_mission) )	break;
        }

        units.unit[i].script_env->resize( units.unit[i].nb_running );
        for( int e = 0 ; e < units.unit[i].nb_running ; e++ )
        {
            SCRIPT_ENV *f = &((*units.unit[i].script_env)[e]);
            LOAD( f->wait );
            LOAD( f->running );

            {
                f->stack = NULL;
                SCRIPT_STACK **stack = &(f->stack);
                while( fgetc( file ) ) {
                    *stack = new SCRIPT_STACK;
                    LOAD( (*stack)->val );
                    (*stack)->next = NULL;
                    stack = &((*stack)->next);
                }
            }

            {
                f->env = NULL;
                SCRIPT_ENV_STACK **stack = &(f->env);
                while( fgetc( file ) )
                {
                    *stack = new SCRIPT_ENV_STACK;
                    LOAD( (*stack)->cur );
                    LOAD( (*stack)->signal_mask );
                    fread( (*stack)->var, sizeof( int ), 15, file );
                    (*stack)->next = NULL;
                    stack = &((*stack)->next);
                }
            }
        }

        LOAD( units.unit[i].data.nb_piece );
        LOAD( units.unit[i].data.explode_time );
        LOAD( units.unit[i].data.explode );
        LOAD( units.unit[i].data.is_moving );

        fread( units.unit[i].data.flag, sizeof( short ), units.unit[i].data.nb_piece, file );
        fread( units.unit[i].data.explosion_flag, sizeof( short ), units.unit[i].data.nb_piece, file );
        fread( units.unit[i].data.pos, sizeof( Vector3D), units.unit[i].data.nb_piece, file );
        fread( units.unit[i].data.dir, sizeof( Vector3D), units.unit[i].data.nb_piece, file );
        fread( units.unit[i].data.matrix, sizeof( MATRIX_4x4 ), units.unit[i].data.nb_piece, file );

        fread( units.unit[i].data.axe[0], sizeof( AXE ), units.unit[i].data.nb_piece, file );
        fread( units.unit[i].data.axe[1], sizeof( AXE ), units.unit[i].data.nb_piece, file );
        fread( units.unit[i].data.axe[2], sizeof( AXE ), units.unit[i].data.nb_piece, file );

        if( units.unit[i].drawn )
        {
            units.unit[i].drawn = false;
            units.unit[i].draw_on_map();
        }
    }

    for( int i = 0 ; i < units.max_unit ; i++ )	// Build the free index list
    {
        int player_id = i / MAX_UNIT_PER_PLAYER;
        if( units.unit[i].type_id < 0 || !(units.unit[i].flags & 1) )
            units.free_idx[ player_id * MAX_UNIT_PER_PLAYER + (units.free_index_size[player_id]++) ] = i;
    }

    LOAD( units.current_tick );     // We'll need this for multiplayer games

    units.unlock();

    if (game_data->fog_of_war)      // Load fog of war state
    {
        for (int y = 0 ; y < the_map->view_map->h ; y++)
            fread( the_map->view_map->line[y], bitmap_color_depth(the_map->view_map) >> 3, the_map->view_map->w, file );

        for (int y = 0 ; y < the_map->sight_map->h ; y++)
            fread( the_map->sight_map->line[y], bitmap_color_depth(the_map->sight_map) >> 3, the_map->sight_map->w, file );

        for (int y = 0 ; y < the_map->radar_map->h ; y++)
            fread( the_map->radar_map->line[y], bitmap_color_depth(the_map->radar_map) >> 3, the_map->radar_map->w, file );

        for (int y = 0 ; y < the_map->sonar_map->h ; y++)
            fread( the_map->sonar_map->line[y], bitmap_color_depth(the_map->sonar_map) >> 3, the_map->sonar_map->w, file );
    }

    game_data->saved_file.clear();

    fclose( file );
}



