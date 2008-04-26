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

//----- Save feature information -----------------------------------------------------------
		
		fclose( file );

		lp_CONFIG->pause = previous_pause_state;
		}
}

void load_game( const String filename, GAME_DATA *game_data )
{
}
