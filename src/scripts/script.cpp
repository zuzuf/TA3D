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

/*----------------------------------------------------------------------\
  |                                script.cpp                             |
  |      contient les classes nécessaires à la gestion des scripts de     |
  | controle du déroulement de la partie. Les scripts peuvent influencer  |
  | considérablement le déroulement de la partie en manipulant les unités |
  | les ressources mais aussi l'écran et déclenche les signaux de défaite |
  | et de victoire.                                                       |
  \----------------------------------------------------------------------*/

#include "../stdafx.h"
#include "../misc/matrix.h"
#include "../TA3D_NameSpace.h"
#include "../ta3dbase.h"
#include "../EngineClass.h"
#include "../UnitEngine.h"
#include "script.h"
#include "../misc/camera.h"
#include "../languages/i18n.h"
#include <vector>
#include <fstream>



namespace TA3D
{



    LUA_PROGRAM	*lua_program = NULL;
    MAP *lua_map = NULL;

    int function_print_for( lua_State *L )		// ta3d_print_for( x, y, str, player_id )
    {
        const char *str = lua_tostring( L, -2 );		// Read the result
        if( str )
        {
            if( (int) lua_tonumber( L, -1 ) == players.local_human_id || (int) lua_tonumber( L, -1 ) == -1 )
            {
                DRAW_OBJECT draw_obj;
                draw_obj.type = DRAW_TYPE_TEXT;
                draw_obj.r[0] = 1.0f;
                draw_obj.g[0] = 1.0f;
                draw_obj.b[0] = 1.0f;
                draw_obj.x[0] = (float) lua_tonumber( L, -4 );
                draw_obj.y[0] = (float) lua_tonumber( L, -3 );
                draw_obj.text = strdup( I18N::Translate( str ).c_str() );
                lua_program->draw_list.add( draw_obj );
            }

            if( network_manager.isServer() ) {
                struct event print_event;
                print_event.type = EVENT_PRINT;
                print_event.opt1 = ((int) lua_tonumber( L, -1 )) == -1 ? 0xFFFF : (int) lua_tonumber( L, -1 );
                print_event.x = (float) lua_tonumber( L, -4 );
                print_event.y = (float) lua_tonumber( L, -3 );
                memcpy( print_event.str, str, strlen( str ) + 1 );

                network_manager.sendEvent( &print_event );
            }
        }
        lua_pop( L, 4 );
        return 0;
    }

    int function_print( lua_State *L )		// ta3d_print( x, y, str )
    {
        lua_pushnumber( L, -1.0f );
        function_print_for( L );
        return 0;
    }

    int function_logmsg( lua_State *L )		// ta3d_logmsg( str )
    {
        const char *str = lua_tostring( L, -1 );		// Read the result
        if( str )
            Console->AddEntry( str );
        lua_pop( L, 1 );
        return 0;
    }

    int function_line( lua_State *L )		// ta3d_line( x1,y1,x2,y2,r,g,b )
    {
        DRAW_OBJECT draw_obj;
        draw_obj.type = DRAW_TYPE_LINE;
        draw_obj.r[0] = (float) lua_tonumber( L, -3 );
        draw_obj.g[0] = (float) lua_tonumber( L, -2 );
        draw_obj.b[0] = (float) lua_tonumber( L, -1 );
        draw_obj.x[0] = (float) lua_tonumber( L, -7 );
        draw_obj.y[0] = (float) lua_tonumber( L, -6 );
        draw_obj.x[1] = (float) lua_tonumber( L, -5 );
        draw_obj.y[1] = (float) lua_tonumber( L, -4 );
        lua_program->draw_list.add( draw_obj );
        lua_pop( L, 7 );

        return 0;
    }

    int function_point( lua_State *L )		// ta3d_line( x,y,r,g,b )
    {
        DRAW_OBJECT draw_obj;
        draw_obj.type = DRAW_TYPE_POINT;
        draw_obj.r[0] = (float) lua_tonumber( L, -3 );
        draw_obj.g[0] = (float) lua_tonumber( L, -2 );
        draw_obj.b[0] = (float) lua_tonumber( L, -1 );
        draw_obj.x[0] = (float) lua_tonumber( L, -5 );
        draw_obj.y[0] = (float) lua_tonumber( L, -4 );
        lua_program->draw_list.add( draw_obj );
        lua_pop( L, 5 );

        return 0;
    }

    int function_triangle( lua_State *L )		// ta3d_line( x1,y1,x2,y2,x3,y3,r,g,b )
    {
        DRAW_OBJECT draw_obj;
        draw_obj.type = DRAW_TYPE_TRIANGLE;
        draw_obj.r[0] = (float) lua_tonumber( L, -3 );
        draw_obj.g[0] = (float) lua_tonumber( L, -2 );
        draw_obj.b[0] = (float) lua_tonumber( L, -1 );
        draw_obj.x[0] = (float) lua_tonumber( L, -9 );
        draw_obj.y[0] = (float) lua_tonumber( L, -8 );
        draw_obj.x[1] = (float) lua_tonumber( L, -7 );
        draw_obj.y[1] = (float) lua_tonumber( L, -6 );
        draw_obj.x[2] = (float) lua_tonumber( L, -5 );
        draw_obj.y[2] = (float) lua_tonumber( L, -4 );
        lua_program->draw_list.add( draw_obj );
        lua_pop( L, 9 );

        return 0;
    }

    int function_cls_for( lua_State *L )		// ta3d_cls_for( player_id )
    {
        if( (int) lua_tonumber( L, -1 ) == players.local_human_id || (int) lua_tonumber( L, -1 ) == -1 ) {
            lua_program->lock();
            lua_program->draw_list.destroy();
            lua_program->unlock();
        }

        if( network_manager.isServer() ) {
            struct event cls_event;
            cls_event.type = EVENT_CLS;
            cls_event.opt1 = ((int) lua_tonumber( L, -1 )) == -1 ? 0xFFFF : (int) lua_tonumber( L, -1 );

            network_manager.sendEvent( &cls_event );
        }

        lua_pop( L, 1 );

        return 0;
    }

    int function_cls( lua_State *L )		// ta3d_cls()
    {
        lua_pushnumber( L, -1.0f );
        function_cls_for( L );
        return 0;
    }

    int lua_signal = 0;

    int function_signal( lua_State *L )		// ta3d_signal( signal )
    {
        lua_signal = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );
        return 0;
    }

    int function_box( lua_State *L )		// ta3d_box( x1,y1,x2,y2,r,g,b )
    {
        DRAW_OBJECT draw_obj;
        draw_obj.type = DRAW_TYPE_BOX;
        draw_obj.r[0] = (float) lua_tonumber( L, -3 );
        draw_obj.g[0] = (float) lua_tonumber( L, -2 );
        draw_obj.b[0] = (float) lua_tonumber( L, -1 );
        draw_obj.x[0] = (float) lua_tonumber( L, -7 );
        draw_obj.y[0] = (float) lua_tonumber( L, -6 );
        draw_obj.x[1] = (float) lua_tonumber( L, -5 );
        draw_obj.y[1] = (float) lua_tonumber( L, -4 );
        lua_program->draw_list.add( draw_obj );
        lua_pop( L, 7 );

        return 0;
    }

    int function_fillbox( lua_State *L )		// ta3d_fillbox( x1,y1,x2,y2,r,g,b )
    {
        DRAW_OBJECT draw_obj;
        draw_obj.type = DRAW_TYPE_FILLBOX;
        draw_obj.r[0] = (float) lua_tonumber( L, -3 );
        draw_obj.g[0] = (float) lua_tonumber( L, -2 );
        draw_obj.b[0] = (float) lua_tonumber( L, -1 );
        draw_obj.x[0] = (float) lua_tonumber( L, -7 );
        draw_obj.y[0] = (float) lua_tonumber( L, -6 );
        draw_obj.x[1] = (float) lua_tonumber( L, -5 );
        draw_obj.y[1] = (float) lua_tonumber( L, -4 );
        lua_program->draw_list.add( draw_obj );
        lua_pop( L, 7 );

        return 0;
    }

    int function_circle( lua_State *L )		// ta3d_circle( x,y,ray,r,g,b )
    {
        DRAW_OBJECT draw_obj;
        draw_obj.type = DRAW_TYPE_CIRCLE;
        draw_obj.r[0] = (float) lua_tonumber( L, -3 );
        draw_obj.g[0] = (float) lua_tonumber( L, -2 );
        draw_obj.b[0] = (float) lua_tonumber( L, -1 );
        draw_obj.x[0] = (float) lua_tonumber( L, -6 );
        draw_obj.y[0] = (float) lua_tonumber( L, -5 );
        draw_obj.r[1] = (float) lua_tonumber( L, -4 );
        lua_program->draw_list.add( draw_obj );
        lua_pop( L, 6 );

        return 0;
    }

    int function_mouse_x( lua_State *L )		// ta3d_mouse_x()
    {
        lua_pushnumber( L, mouse_x );
        return 1;
    }

    int function_mouse_y( lua_State *L )		// ta3d_mouse_y()
    {
        lua_pushnumber( L, mouse_y );
        return 1;
    }

    int function_mouse_z( lua_State *L )		// ta3d_mouse_z()
    {
        lua_pushnumber( L, mouse_z );
        return 1;
    }

    int function_mouse_b( lua_State *L )		// ta3d_mouse_b()
    {
        lua_pushnumber( L, mouse_b );
        return 1;
    }

    int function_get_key( lua_State *L )		// ta3d_get_key()
    {
        if(keypressed())
            lua_pushnumber( L, readkey() );
        else
            lua_pushnumber( L, 0 );
        return 1;
    }

    int function_time( lua_State *L )		// ta3d_time()
    {
        lua_pushnumber( L, units.current_tick / (float)TICKS_PER_SEC );
        return 1;
    }

    int function_draw_image_for( lua_State *L )		// ta3d_draw_image_for( str image_name, x1, y1, x2, y2, player_id )
    {
        if( (int) lua_tonumber( L, -1 ) == players.local_human_id || (int) lua_tonumber( L, -1 ) == -1 ) {
            DRAW_OBJECT draw_obj;
            draw_obj.type = DRAW_TYPE_BITMAP;
            draw_obj.x[0] = (float) lua_tonumber( L, -5 );
            draw_obj.y[0] = (float) lua_tonumber( L, -4 );
            draw_obj.x[1] = (float) lua_tonumber( L, -3 );
            draw_obj.y[1] = (float) lua_tonumber( L, -2 );
            draw_obj.tex = gfx->load_texture( I18N::Translate( lua_tostring( L, -6 ) ) );
            draw_obj.text = NULL;
            lua_program->draw_list.add( draw_obj );
        }

        if( network_manager.isServer() ) {
            struct event draw_event;
            draw_event.type = EVENT_DRAW;
            draw_event.opt1 = ((int) lua_tonumber( L, -1 )) == -1 ? 0xFFFF : (int) lua_tonumber( L, -1 );
            draw_event.x = (float) lua_tonumber( L, -4 );
            draw_event.y = (float) lua_tonumber( L, -3 );
            draw_event.z = (float) lua_tonumber( L, -2 );
            draw_event.opt3 = (sint32)( ((float) lua_tonumber( L, -1 )) * 16384.0f );
            memcpy( draw_event.str, lua_tostring( L, -5 ), strlen( lua_tostring( L, -5 ) ) + 1 );

            network_manager.sendEvent( &draw_event );
        }

        lua_pop( L, 6 );

        return 0;
    }

    int function_draw_image( lua_State *L )		// ta3d_draw_image( str image_name, x1, y1, x2, y2 )
    {
        lua_pushnumber( L, -1.0f );
        function_draw_image_for( L );
        return 0;
    }

    int function_nb_players( lua_State *L )		// ta3d_nb_players()
    {
        lua_pushnumber( L, NB_PLAYERS );
        return 1;
    }

    int function_get_unit_number_for_player( lua_State *L )		// ta3d_get_unit_number_for_player( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );
        if( player_id >= 0 && player_id < NB_PLAYERS ) {
            int n = 0;
            for( uint16 e = 0 ; e < units.index_list_size ; e++ ) {
                uint16 i = units.idx_list[ e ];
                if( units.unit[i].flags != 0 && units.unit[ i ].owner_id == player_id )
                    n++;
            }
            lua_pushnumber( L, n );
        }
        else
            lua_pushnumber( L, 0 );
        return 1;
    }

    int function_get_unit_owner( lua_State *L )		// ta3d_get_unit_owner( unit_id )
    {
        int unit_idx = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );
        if( unit_idx >= 0 && unit_idx < units.max_unit ) {
            if( units.unit[ unit_idx ].flags )
                lua_pushnumber( L, units.unit[ unit_idx ].owner_id );
            else
                lua_pushnumber( L, -1 );
        }
        else
            lua_pushnumber( L, -1 );
        return 1;
    }

    int function_get_unit_number( lua_State *L )		// ta3d_get_unit_number()
    {
        lua_pushnumber( L, units.nb_unit );
        return 1;
    }

    int function_get_max_unit_number( lua_State *L )		// ta3d_get_max_unit_number()
    {
        lua_pushnumber( L, units.max_unit );
        return 1;
    }

    int function_annihilated( lua_State *L )		// ta3d_annihilated( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );
        if( player_id >= 0 && player_id < NB_PLAYERS )
            lua_pushboolean( L, players.annihilated[ player_id ] );
        else
            lua_pushboolean( L, false );
        return 1;
    }

    int function_has_unit( lua_State *L )		// ta3d_has_unit( player_id, unit_type_id )
    {
        int player_id = (int) lua_tonumber( L, -2 );
        if( player_id >= 0 && player_id < NB_PLAYERS ) {
            int unit_type = lua_isstring( L, -1 ) ? unit_manager.get_unit_index( lua_tostring( L, -1 ) ) : (int) lua_tonumber( L, -1 ) ;
            bool has = false;
            uint16 last_possible_idx = min( (int)units.max_unit, ( player_id + 1 ) * MAX_UNIT_PER_PLAYER);
            for( uint16 i = player_id * MAX_UNIT_PER_PLAYER ; i < last_possible_idx ; i++ )
                if( units.unit[i].flags && units.unit[i].owner_id == player_id && units.unit[i].type_id == unit_type ) {
                    has=true;
                    break;
                }
            lua_pop( L, 2 );
            lua_pushboolean( L, has );
        }
        else {
            lua_pop( L, 2 );
            lua_pushboolean( L, false );
        }
        return 1;
    }

    int function_nb_unit_of_type( lua_State *L )		// ta3d_nb_unit_of_type( player_id, unit_type_id )
    {
        int player_id = (int) lua_tonumber( L, -2 );
        if( player_id >= 0 && player_id < NB_PLAYERS ) {
            int unit_type = lua_isstring( L, -1 ) ? unit_manager.get_unit_index( lua_tostring( L, -1 ) ) : (int) lua_tonumber( L, -1 ) ;
            int nb = 0;
            uint16 last_possible_idx = min( (int)units.max_unit, ( player_id + 1 ) * MAX_UNIT_PER_PLAYER);
            for( uint16 i = player_id * MAX_UNIT_PER_PLAYER ; i < last_possible_idx ; i++ )
                if( units.unit[i].flags && units.unit[i].owner_id == player_id && units.unit[i].type_id == unit_type )
                    nb++;
            lua_pop( L, 2 );
            lua_pushnumber( L, nb );
        }
        else {
            lua_pop( L, 2 );
            lua_pushnumber( L, 0 );
        }
        return 1;
    }

    int function_is_unit_of_type( lua_State *L )		// ta3d_is_unit_of_type( unit_id, unit_type_id )
    {
        int unit_id = (int) lua_tonumber( L, -2 );
        if( unit_id >= 0 && unit_id < units.max_unit ) {
            int unit_type = lua_isstring( L, -1 ) ? unit_manager.get_unit_index( lua_tostring( L, -1 ) ) : (int) lua_tonumber( L, -1 ) ;
            lua_pop( L, 2 );
            lua_pushboolean( L, (units.unit[unit_id].flags & 1) && units.unit[unit_id].type_id == unit_type );
        }
        else {
            lua_pop( L, 2 );
            lua_pushboolean( L, false );
        }
        return 1;
    }

    int function_has_mobile_units( lua_State *L ) 		// ta3d_has_mobile_units( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );
        if( player_id >= 0 && player_id < NB_PLAYERS ) {
            bool result = false;
            for( uint16 e = 0 ; e < units.index_list_size && !result ; e++ ) {
                uint16 i = units.idx_list[ e ];
                if( (units.unit[ i ].flags & 1) && units.unit[ i ].owner_id == player_id ) {
                    int type = units.unit[ i ].type_id;
                    if( type >= 0 && type < unit_manager.nb_unit && unit_manager.unit_type[ type ].canmove && unit_manager.unit_type[ type ].BMcode )
                        result = true;
                }
            }
            lua_pushboolean( L, result );
        }
        else
            lua_pushboolean( L, false );
        return 1;
    }

    int function_move_unit( lua_State *L )		// ta3d_move_unit( unit_id, x, z )
    {
        int unit_id = (int) lua_tonumber( L, -3 );

        if( unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags ) {
            units.lock();

            units.unit[ unit_id ].Pos.x = (float) lua_tonumber( L, -2 );
            units.unit[ unit_id ].Pos.z = (float) lua_tonumber( L, -1 );

            units.unit[ unit_id ].clear_from_map();

            int PX = ((int)(units.unit[ unit_id ].Pos.x + lua_map->map_w_d)>>3);
            int PY = ((int)(units.unit[ unit_id ].Pos.z + lua_map->map_h_d)>>3);
            if( !can_be_there( PX, PY, lua_map, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ) ) {
                bool found = false;
                for( int r = 1 ; r < 120 && !found ; r++ ) {		// Circular check
                    int r2 = r * r;
                    for( int y = 0 ; y <= r ; y++ ) {
                        int x = (int)(sqrt( r2 - y * y ) + 0.5f);
                        if( can_be_there( PX+x, PY+y, lua_map, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ) ) {
                            PX += x;
                            PY += y;
                            found = true;
                            break;
                        }
                        if( can_be_there( PX-x, PY+y, lua_map, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ) ) {
                            PX -= x;
                            PY += y;
                            found = true;
                            break;
                        }
                        if( can_be_there( PX+x, PY-y, lua_map, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ) ) {
                            PX += x;
                            PY -= y;
                            found = true;
                            break;
                        }
                        if( can_be_there( PX-x, PY-y, lua_map, units.unit[ unit_id ].type_id, units.unit[ unit_id ].owner_id ) ) {
                            PX -= x;
                            PY -= y;
                            found = true;
                            break;
                        }
                    }
                }
                if( found ) {
                    units.unit[ unit_id ].Pos.x = (PX<<3) + 8 - lua_map->map_w_d;
                    units.unit[ unit_id ].Pos.z = (PY<<3) + 8 - lua_map->map_h_d;
                    if( units.unit[ unit_id ].mission && (units.unit[ unit_id ].mission->flags & MISSION_FLAG_MOVE) )
                        units.unit[ unit_id ].mission->flags |= MISSION_FLAG_REFRESH_PATH;
                }
                else {
                    int prev = 0;
                    for( int i = 0 ; i < units.nb_unit ; i++ )
                        if( units.idx_list[ i ] == unit_id ) {
                            prev = i;
                            break;
                        }
                    units.kill( unit_id, lua_map, prev);
                    unit_id = -1;
                }
            }
            if( unit_id >= 0 ) {
                units.unit[ unit_id ].cur_px = PX;
                units.unit[ unit_id ].cur_py = PY;

                VECTOR target_pos = units.unit[ unit_id ].Pos;
                target_pos.x=((int)(target_pos.x) + lua_map->map_w_d)>>3;
                target_pos.z=((int)(target_pos.z) + lua_map->map_h_d)>>3;
                target_pos.y = max( lua_map->get_max_rect_h((int)target_pos.x,(int)target_pos.z, unit_manager.unit_type[ units.unit[ unit_id ].type_id ].FootprintX, unit_manager.unit_type[ units.unit[ unit_id ].type_id ].FootprintZ ), lua_map->sealvl);
                units.unit[ unit_id ].Pos.y = target_pos.y;
                units.unit[ unit_id ].draw_on_map();
            }

            units.unlock();
        }
        lua_pop( L, 3 );

        return 0;
    }

    int function_create_unit( lua_State *L )		// ta3d_create_unit( player_id, unit_type_id, x, z )
    {
        int player_id = (int) lua_tonumber( L, -4 );
        int unit_type_id = lua_isstring( L, -3 ) ? unit_manager.get_unit_index( lua_tostring( L, -3 ) ) : (int) lua_tonumber( L, -3 ) ;
        float x = (float) lua_tonumber( L, -2 );
        float z = (float) lua_tonumber( L, -1 );

        lua_pop( L, 4 );

        if( unit_type_id >= 0 && unit_type_id < unit_manager.nb_unit && player_id >= 0 && player_id < NB_PLAYERS ) {
            units.lock();
            VECTOR pos;
            pos.x = x;
            pos.z = z;
            pos.y = max( lua_map->get_max_rect_h((int)x,(int)z, unit_manager.unit_type[ unit_type_id ].FootprintX, unit_manager.unit_type[ unit_type_id ].FootprintZ ), lua_map->sealvl);
            UNIT *unit = (UNIT*)create_unit( unit_type_id, player_id, pos, the_map, true, true );		// Force synchronization
            int idx = unit ? unit->idx : -1;
            if( unit ) {
                unit->lock();
                unit->hp = unit_manager.unit_type[ unit_type_id ].MaxDamage;
                unit->build_percent_left = 0.0f;
                if( unit_manager.unit_type[ unit_type_id ].ActivateWhenBuilt ) {		// Start activated
                    unit->port[ ACTIVATION ] = 0;
                    unit->activate();
                }
                unit->unlock();
            }
            units.unlock();

            if( idx >= 0 && idx < units.max_unit && units.unit[ idx ].flags )
                lua_pushnumber( L, idx );
            else
                lua_pushnumber( L, -1 );
        }
        else
            lua_pushnumber( L, -1 );

        return 1;
    }

    int function_change_unit_owner( lua_State *L )		// ta3d_change_unit_owner( unit_id, player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        int unit_id = (int) lua_tonumber( L, -2 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit && player_id >= 0 && player_id < NB_PLAYERS && units.unit[ unit_id ].flags ) {
            units.lock();
            units.unit[ unit_id ].owner_id = player_id;
            units.unlock();
        }

        return 0;
    }

    int function_set_unit_health( lua_State *L )		// ta3d_set_unit_health( unit_id, health_percentage )
    {
        float health = (float) lua_tonumber( L, -1 ) * 0.01f;
        int unit_id = (int) lua_tonumber( L, -2 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit ) {
            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].hp = health * unit_manager.unit_type[ units.unit[ unit_id ].type_id ].MaxDamage;
            units.unlock();
        }

        return 0;
    }

    int function_add_build_mission( lua_State *L )		// ta3d_add_build_mission( unit_id, pos_x, pos_z, unit_type )
    {
        int unit_id = (int) lua_tonumber( L, -4 );
        float pos_x = (float) lua_tonumber( L, -3 );
        float pos_z = (float) lua_tonumber( L, -2 );
        int unit_type_id = lua_isstring( L, -1 ) ? unit_manager.get_unit_index( lua_tostring( L, -1 ) ) : (int) lua_tonumber( L, -1 ) ;
        lua_pop( L, 4 );

        if( unit_id >= 0 && unit_id < units.max_unit && unit_type_id >= 0 && unit_manager.unit_type[ unit_type_id ].Builder ) {
            VECTOR target;
            target.x=((int)(pos_x)+lua_map->map_w_d)>>3;
            target.z=((int)(pos_z)+lua_map->map_h_d)>>3;
            target.y=max(lua_map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[ unit_type_id ].FootprintX, unit_manager.unit_type[ unit_type_id ].FootprintZ ),lua_map->sealvl);
            target.x=target.x*8.0f-lua_map->map_w_d;
            target.z=target.z*8.0f-lua_map->map_h_d;

            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_BUILD,&target,false,unit_type_id);
            units.unlock();
        }

        return 0;
    }

    int function_add_move_mission( lua_State *L )		// ta3d_add_move_mission( unit_id, pos_x, pos_z )
    {
        int unit_id = (int) lua_tonumber( L, -3 );
        float pos_x = (float) lua_tonumber( L, -2 );
        float pos_z = (float) lua_tonumber( L, -1 );
        lua_pop( L, 3 );

        if( unit_id >= 0 && unit_id < units.max_unit ) {
            VECTOR target;
            target.x = pos_x;
            target.y = lua_map->get_unit_h( pos_x, pos_z );
            target.z = pos_z;

            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_MOVE,&target,false,0);
            units.unlock();
        }

        return 0;
    }

    int function_add_attack_mission( lua_State *L )		// ta3d_add_attack_mission( unit_id, target_id )
    {
        int unit_id = (int) lua_tonumber( L, -2 );
        int target_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit && target_id >= 0 && target_id < units.max_unit ) {
            VECTOR target = units.unit[ target_id ].Pos;

            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_ATTACK,&(target),false,0,&(units.unit[target_id]),NULL,MISSION_FLAG_COMMAND_FIRE );
            units.unlock();
        }

        return 0;
    }

    int function_add_patrol_mission( lua_State *L )		// ta3d_add_patrol_mission( unit_id, pos_x, pos_z )
    {
        int unit_id = (int) lua_tonumber( L, -3 );
        float pos_x = (float) lua_tonumber( L, -2 );
        float pos_z = (float) lua_tonumber( L, -1 );
        lua_pop( L, 3 );

        if( unit_id >= 0 && unit_id < units.max_unit ) {
            VECTOR target;
            target.x = pos_x;
            target.y = lua_map->get_unit_h( pos_x, pos_z );
            target.z = pos_z;

            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_PATROL,&target,false,0);
            units.unlock();
        }

        return 0;
    }

    int function_add_wait_mission( lua_State *L )		// ta3d_add_wait_mission( unit_id, time )
    {
        int unit_id = (int) lua_tonumber( L, -2 );
        float time = (float) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit ) {
            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_WAIT,NULL,false,(int)(time * 1000));
            units.unlock();
        }

        return 0;
    }

    int function_add_wait_attacked_mission( lua_State *L )		// ta3d_add_wait_attacked_mission( unit_id, target_id )
    {
        int unit_id = (int) lua_tonumber( L, -2 );
        int target_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit && target_id >= 0 && target_id < units.max_unit ) {
            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_WAIT_ATTACKED,NULL,false,target_id);
            units.unlock();
        }

        return 0;
    }

    int function_add_guard_mission( lua_State *L )		// ta3d_add_guard_mission( unit_id, target_id )
    {
        int unit_id = (int) lua_tonumber( L, -2 );
        int target_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit && target_id >= 0 && target_id < units.max_unit ) {
            units.lock();
            if( units.unit[ unit_id ].flags )
                units.unit[ unit_id ].add_mission(MISSION_GUARD,&units.unit[ target_id ].Pos,false,0,&(units.unit[ target_id ]),NULL);
            units.unlock();
        }

        return 0;
    }

    int function_set_standing_orders( lua_State *L )		// ta3d_set_standing_orders( unit_id, move_order, fire_order )
    {
        int unit_id = (int) lua_tonumber( L, -3 );
        int move_order = (int) lua_tonumber( L, -2 );
        int fire_order = (int) lua_tonumber( L, -1 );
        lua_pop( L, 3 );

        if( unit_id >= 0 && unit_id < units.max_unit ) {
            units.lock();
            if( units.unit[ unit_id ].flags ) {
                units.unit[ unit_id ].port[ STANDINGMOVEORDERS ] = move_order;
                units.unit[ unit_id ].port[ STANDINGFIREORDERS ] = fire_order;
            }
            units.unlock();
        }

        return 0;
    }

    int function_lock_orders( lua_State *L )		// ta3d_lock_orders( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit )
            units.unit[ unit_id ].lock_command();

        return 0;
    }

    int function_unlock_orders( lua_State *L )		// ta3d_unlock_orders( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit )
            units.unit[ unit_id ].unlock_command();

        return 0;
    }

    int function_get_unit_health( lua_State *L )		// ta3d_get_unit_health( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit ) {
            units.lock();
            if( units.unit[ unit_id ].flags )
                lua_pushnumber( L, units.unit[ unit_id ].hp * 100.0f / unit_manager.unit_type[ units.unit[ unit_id ].type_id ].MaxDamage );
            else
                lua_pushnumber( L, 0 );
            units.unlock();
        }

        return 0;
    }

    int function_local_player( lua_State *L )		// ta3d_local_player()
    {
        lua_pushnumber( L, players.local_human_id );
        return 1;
    }

    int function_map_w( lua_State *L )		// ta3d_map_w()
    {
        lua_pushnumber( L, lua_map->map_w );
        return 1;
    }

    int function_map_h( lua_State *L )		// ta3d_map_h()
    {
        lua_pushnumber( L, lua_map->map_h );
        return 1;
    }

    int function_player_side( lua_State *L )		// ta3d_player_side( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( player_id >= 0 && player_id < NB_PLAYERS )
            lua_pushstring( L, players.side[ player_id ] );
        else
            lua_pushstring( L, "" );

        return 1;
    }

    int function_unit_x( lua_State *L )		// ta3d_unit_x( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags ) {
            units.lock();
            lua_pushnumber( L, units.unit[ unit_id ].Pos.x );
            units.unlock();
        }
        else
            lua_pushnumber( L, 0 );

        return 1;
    }

    int function_unit_y( lua_State *L )		// ta3d_unit_y( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags ) {
            units.lock();
            lua_pushnumber( L, units.unit[ unit_id ].Pos.y );
            units.unlock();
        }
        else
            lua_pushnumber( L, 0 );

        return 1;
    }

    int function_unit_z( lua_State *L )		// ta3d_unit_z( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags ) {
            units.lock();
            lua_pushnumber( L, units.unit[ unit_id ].Pos.z );
            units.unlock();
        }
        else
            lua_pushnumber( L, 0 );

        return 1;
    }

    int function_kill_unit( lua_State *L )		// ta3d_kill_unit( unit_id )
    {
        int unit_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags ) {

            units.unit[ unit_id ].lock();
            if( !network_manager.isConnected() || units.unit[ unit_id ].local )
                units.unit[ unit_id ].hp = 0.0f;
            else {
                struct event event;
                event.type = EVENT_UNIT_DEATH;
                event.opt1 = unit_id;
                network_manager.sendEvent( &event );
            }
            units.unit[ unit_id ].unlock();
        }

        return 0;
    }

    int function_kick_unit( lua_State *L )		// ta3d_kick_unit( unit_id, damage )
    {
        int unit_id = (int) lua_tonumber( L, -2 );
        float damage = (float) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags ) {
            units.lock();

            units.unit[ unit_id ].hp -= damage;

            units.unlock();
        }

        return 0;
    }

    int function_play_for( lua_State *L )		// ta3d_play_for( filename, player_id )
    {
        if( (int) lua_tonumber( L, -1 ) == players.local_human_id || (int) lua_tonumber( L, -1 ) == -1 )
            sound_manager->PlaySound( (char*) lua_tostring( L, -2 ), false );

        if( network_manager.isServer() ) {
            struct event play_event;
            play_event.type = EVENT_PLAY;
            play_event.opt1 = ((int) lua_tonumber( L, -1 )) == -1 ? 0xFFFF : (int) lua_tonumber( L, -1 );
            memcpy( play_event.str, lua_tostring( L, -2 ), strlen( lua_tostring( L, -2 ) ) + 1 );

            network_manager.sendEvent( &play_event );
        }

        lua_pop( L, 2 );

        return 0;
    }

    int function_play( lua_State *L )		// ta3d_play( filename )
    {
        lua_pushnumber( L, -1.0f );
        function_play_for( L );
        return 0;
    }

    int function_set_cam_pos( lua_State *L )		// ta3d_set_cam_pos( player_id,x,z )
    {
        if( (int) lua_tonumber( L, -3 ) == players.local_human_id )
        {
            Camera::inGame->rpos.x = (float) lua_tonumber( L, -2 );
            Camera::inGame->rpos.z = (float) lua_tonumber( L, -1 );
        }
        else
        {
            if (network_manager.isServer())
            {
                struct event cam_event;
                cam_event.type = EVENT_CAMERA_POS;
                cam_event.opt1 = (int) lua_tonumber( L, -3 );
                cam_event.x = (float) lua_tonumber( L, -2 );
                cam_event.z = (float) lua_tonumber( L, -1 );

                network_manager.sendEvent( &cam_event );
            }
        }
        lua_pop( L, 3 );
        return 0;
    }

    int function_set_cam_mode( lua_State *L )		// ta3d_set_cam_mode( mode )	-> uses the signal system
    {
        if( lua_toboolean( L, -1 ) )
            lua_signal = 5;
        else
            lua_signal = 4;
        lua_pop( L, 1 );
        return 0;
    }

    int function_clf( lua_State *L )				// ta3d_clf()
    {
        lua_map->clear_FOW();
        units.lock();
        for( int i = 0 ; i < units.index_list_size ; i++ )
            units.unit[ units.idx_list[ i ] ].old_px = -10000;
        units.unlock();

        if( network_manager.isServer() ) {
            struct event clf_event;
            clf_event.type = EVENT_CLF;
            network_manager.sendEvent( &clf_event );
        }
        return 0;
    }

    int function_start_x( lua_State *L )			// ta3d_start_x( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( player_id >= 0 && player_id < players.nb_player )
            lua_pushnumber( L, (lua_map->ota_data.startX[ player_id ] - lua_map->map_w) * 0.5f );
        else
            lua_pushnumber( L, 0 );
        return 1;
    }

    int function_start_z( lua_State *L )			// ta3d_start_z( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( player_id >= 0 && player_id < players.nb_player )
            lua_pushnumber( L, (lua_map->ota_data.startZ[ player_id ] - lua_map->map_h) * 0.5f );
        else
            lua_pushnumber( L, 0 );
        return 1;
    }

    int function_init_res( lua_State *L )			// ta3d_init_res()
    {
        for( uint16 i = 0 ; i < players.nb_player ; i++ ) {
            players.metal[i] = players.com_metal[i];
            players.energy[i] = players.com_energy[i];
        }

        if( network_manager.isServer() ) {
            struct event init_res_event;
            init_res_event.type = EVENT_INIT_RES;
            network_manager.sendEvent( &init_res_event );
        }

        return 0;
    }

    int function_give_metal( lua_State *L )			// ta3d_give_metal( player_id, amount )
    {
        int player_id = (int) lua_tonumber( L, -2 );
        float amount = (float) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( player_id >= 0 && player_id < players.nb_player ) {
            units.lock();
            players.metal[ player_id ] += amount;
            players.c_metal[ player_id ] = players.metal[ player_id ];
            units.unlock();
        }
        return 0;
    }

    int function_give_energy( lua_State *L )			// ta3d_give_energy( player_id, amount )
    {
        int player_id = (int) lua_tonumber( L, -2 );
        float amount = (float) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( player_id >= 0 && player_id < players.nb_player ) {
            units.lock();
            players.energy[ player_id ] += amount;
            players.c_energy[ player_id ] = players.energy[ player_id ];
            units.unlock();
        }
        return 0;
    }

    int function_commander( lua_State *L )				// ta3d_commander( player_id )
    {
        int player_id = (int) lua_tonumber( L, -1 );
        lua_pop( L, 1 );

        if( player_id >= 0 && player_id < NB_PLAYERS )		// make sure we have a player
            for( int i = 0 ; i < ta3dSideData.nb_side ; i++ )
                if( strcasecmp(ta3dSideData.side_name[ i ], players.side[ player_id ]) == 0  )
                {
                    lua_pushstring( L, ta3dSideData.side_com[ i ] );
                    break;
                }
                else
                    lua_pushstring( L, "" );
        return 1;
    }

    int function_attack( lua_State *L )					// ta3d_attack( attacker_id, target_id )
    {
        int attacker_idx = (int) lua_tonumber( L, -2 );
        int target_idx = (int) lua_tonumber( L, -1 );
        lua_pop( L, 2 );

        if( attacker_idx >= 0 && attacker_idx < units.max_unit && units.unit[ attacker_idx ].flags )		// make sure we have an attacker and a target
            if( target_idx >= 0 && target_idx < units.max_unit && units.unit[ target_idx ].flags ) {
                units.lock();
                units.unit[ attacker_idx ].set_mission( MISSION_ATTACK,&(units.unit[ target_idx ].Pos),false,0,true,&(units.unit[ target_idx ]) );
                units.unlock();
            }
        return 0;
    }

    int function_wait( lua_State *L )			// ta3d_wait()
    {
        lua_program->waiting = true;
        return 0;
    }

    int function_sleep( lua_State *L )			// ta3d_sleep( time )
    {
        lua_program->sleeping = true;
        lua_program->sleep_time = (float) lua_tonumber( L, -1 );
        lua_pop( L, 1 );
        return 0;
    }

    int function_create_feature( lua_State *L )		// ta3d_create_feature( feature_type, x, z )
    {
        int feature_type_id = lua_isstring( L, -3 ) ? feature_manager.get_feature_index( lua_tostring( L, -3 ) ) : (int) lua_tonumber( L, -3 ) ;
        float X = (float) lua_tonumber( L, -2 );
        float Z = (float) lua_tonumber( L, -1 );

        lua_pop( L, 3 );

        if( feature_type_id >= 0 && feature_type_id < feature_manager.nb_features ) {
            int x = (int)(X + lua_map->map_w_d - 8)>>3;
            int y = (int)(Z + lua_map->map_h_d - 8)>>3;
            if(x>0 && y>0 && x<(lua_map->bloc_w<<1) && y<(lua_map->bloc_h<<1))
                if(lua_map->map_data[y][x].stuff==-1) {
                    VECTOR Pos;
                    Pos.x = (x<<3)-lua_map->map_w_d+8.0f;
                    Pos.z = (y<<3)-lua_map->map_h_d+8.0f;
                    Pos.y = lua_map->get_unit_h( Pos.x, Pos.z );
                    lua_map->map_data[y][x].stuff = features.add_feature( Pos, feature_type_id );
                    if(feature_type_id!=-1 && lua_map->map_data[y][x].stuff != -1 && feature_manager.feature[feature_type_id].blocking)
                        lua_map->rect(x-(feature_manager.feature[feature_type_id].footprintx>>1),y-(feature_manager.feature[feature_type_id].footprintz>>1),feature_manager.feature[feature_type_id].footprintx,feature_manager.feature[feature_type_id].footprintz,-2-lua_map->map_data[y][x].stuff);
                }
        }

        return 0;
    }

    int function_send_signal( lua_State *L )		// ta3d_send_signal( player_id, signal )
    {
        int player_id = (int) lua_tonumber( L, -2 );
        int signal_id = (int) lua_tonumber( L, -1 );

        if( player_id == players.local_human_id || player_id == -1 )
            g_ta3d_network->set_signal( signal_id );

        if( network_manager.isServer() ) {
            struct event signal_event;
            signal_event.type = EVENT_SCRIPT_SIGNAL;
            signal_event.opt1 = player_id == -1 ? 0xFFFF : player_id;
            signal_event.opt2 = signal_id;

            network_manager.sendEvent( &signal_event );
        }

        return 0;
    }

    void register_functions( lua_State *L )
    {
        lua_register( L, "ta3d_print", function_print );
        lua_register( L, "ta3d_print_for", function_print_for );
        lua_register( L, "ta3d_logmsg", function_logmsg );
        lua_register( L, "ta3d_line", function_line );
        lua_register( L, "ta3d_cls", function_cls );
        lua_register( L, "ta3d_cls_for", function_cls_for );
        lua_register( L, "ta3d_point", function_point );
        lua_register( L, "ta3d_triangle", function_triangle );
        lua_register( L, "ta3d_signal", function_signal );
        lua_register( L, "ta3d_box", function_box );
        lua_register( L, "ta3d_fillbox", function_fillbox );
        lua_register( L, "ta3d_circle", function_circle );
        lua_register( L, "ta3d_mouse_x", function_mouse_x );
        lua_register( L, "ta3d_mouse_y", function_mouse_y );
        lua_register( L, "ta3d_mouse_z", function_mouse_z );
        lua_register( L, "ta3d_mouse_b", function_mouse_b );
        lua_register( L, "ta3d_get_key", function_get_key );
        lua_register( L, "ta3d_time", function_time );
        lua_register( L, "ta3d_draw_image", function_draw_image );
        lua_register( L, "ta3d_draw_image_for", function_draw_image_for );
        lua_register( L, "ta3d_nb_players", function_nb_players );
        lua_register( L, "ta3d_get_unit_number_for_player", function_get_unit_number_for_player );
        lua_register( L, "ta3d_get_unit_owner", function_get_unit_owner );
        lua_register( L, "ta3d_get_unit_number", function_get_unit_number );
        lua_register( L, "ta3d_get_max_unit_number", function_get_max_unit_number );
        lua_register( L, "ta3d_annihilated", function_annihilated );
        lua_register( L, "ta3d_has_unit", function_has_unit );
        lua_register( L, "ta3d_create_unit", function_create_unit );
        lua_register( L, "ta3d_change_unit_owner", function_change_unit_owner );
        lua_register( L, "ta3d_local_player", function_local_player );
        lua_register( L, "ta3d_map_w", function_map_w );
        lua_register( L, "ta3d_map_h", function_map_h );
        lua_register( L, "ta3d_player_side", function_player_side );
        lua_register( L, "ta3d_unit_x", function_unit_x );
        lua_register( L, "ta3d_unit_y", function_unit_y );
        lua_register( L, "ta3d_unit_z", function_unit_z );
        lua_register( L, "ta3d_move_unit", function_move_unit );
        lua_register( L, "ta3d_kill_unit", function_kill_unit );
        lua_register( L, "ta3d_kick_unit", function_kick_unit );
        lua_register( L, "ta3d_play", function_play );
        lua_register( L, "ta3d_play_for", function_play_for );
        lua_register( L, "ta3d_set_cam_pos", function_set_cam_pos );
        lua_register( L, "ta3d_set_cam_mode", function_set_cam_mode );
        lua_register( L, "ta3d_clf", function_clf );
        lua_register( L, "ta3d_start_x", function_start_x );
        lua_register( L, "ta3d_start_z", function_start_z );
        lua_register( L, "ta3d_init_res", function_init_res );
        lua_register( L, "ta3d_give_metal", function_give_metal );
        lua_register( L, "ta3d_give_energy", function_give_energy );
        lua_register( L, "ta3d_commander", function_commander );
        lua_register( L, "ta3d_attack", function_attack );
        lua_register( L, "ta3d_wait", function_wait );
        lua_register( L, "ta3d_sleep", function_sleep );
        lua_register( L, "ta3d_set_unit_health", function_set_unit_health );
        lua_register( L, "ta3d_get_unit_health", function_get_unit_health );
        lua_register( L, "ta3d_is_unit_of_type", function_is_unit_of_type );
        lua_register( L, "ta3d_add_build_mission", function_add_build_mission );
        lua_register( L, "ta3d_add_move_mission", function_add_move_mission );
        lua_register( L, "ta3d_add_attack_mission", function_add_attack_mission );
        lua_register( L, "ta3d_add_patrol_mission", function_add_patrol_mission );
        lua_register( L, "ta3d_add_wait_mission", function_add_wait_mission );
        lua_register( L, "ta3d_add_wait_attacked_mission", function_add_wait_attacked_mission );
        lua_register( L, "ta3d_add_guard_mission", function_add_guard_mission );
        lua_register( L, "ta3d_set_standing_orders", function_set_standing_orders );
        lua_register( L, "ta3d_unlock_orders", function_unlock_orders );
        lua_register( L, "ta3d_lock_orders", function_lock_orders );
        lua_register( L, "ta3d_nb_unit_of_type", function_nb_unit_of_type );
        lua_register( L, "ta3d_create_feature", function_create_feature );
        lua_register( L, "ta3d_has_mobile_units", function_has_mobile_units );
        lua_register( L, "ta3d_send_signal", function_send_signal );
    }

    LUA_PROGRAM::LUA_PROGRAM()
    {
        lua_program = this;
        init();
    }

    void LUA_PROGRAM::load(char *filename, MAP *map)					// Load a lua script
    {
        destroy();			// Au cas où

        uint32 filesize = 0;
        buffer = HPIManager->PullFromHPI( filename , &filesize );
        if( buffer ) {
            int n = 0;
            char *f = NULL;
            while( ( f = strstr( (char*)buffer, "#include" ) ) != NULL && n < 20 ) {
                char name[101];
                name[0] = 0;
                strcat( name, "scripts/" );
                int i;
                for( i = 0 ; i < 100 & f[ i + 10 ] != '"' ; i++ )
                    name[ i + 8 ] = f[ i + 10 ];
                name[ i + 8 ] = 0;
                uint32 filesize2 = 0;
                byte *buffer2 = HPIManager->PullFromHPI( name, &filesize2 );
                if( buffer2 ) {
                    byte *buffer3 = new byte[ filesize + filesize2 ];
                    memset( buffer3, 0, filesize + filesize2 );
                    memcpy( buffer3, buffer, f - (char*)buffer );
                    memcpy( buffer3 + (f - (char*)buffer), buffer2, filesize2 );
                    memcpy( buffer3 + (f - (char*)buffer) + filesize2, f + i + 11, filesize - ( f + i + 11 - (char*)buffer ) );
                    filesize += filesize2 - i - 11;
                    delete[] buffer;
                    delete[] buffer2;
                    buffer = buffer3;
                }
                else
                    break;
                n++;
            }

            L = lua_open();				// Create a lua state object

            if( L == NULL ) {
                running = false;
                delete[] buffer;
                buffer = NULL;
                return;
            }

            lua_program = this;
            lua_map = map;

            lua_gc( L, LUA_GCSTOP, 0 );		// Load libraries
            luaL_openlibs( L );
            lua_gc( L, LUA_GCRESTART, 0 );

            register_functions( L );

            if( luaL_dobuffer( L, buffer, filesize ) ) {	// Load the lua chunk
                if( lua_tostring( L, -1 ) != "" ) {
                    Console->AddEntry( "LUA ERROR: %s", lua_tostring( L, -1 ) );
                    printf("LUA ERROR: %s\n", lua_tostring( L, -1 ) );
                }

                running = false;
                lua_close( L );
                delete[] buffer;
                L = NULL;
                buffer = NULL;
            }
            else
                running = true;
        }
        else {
            Console->AddEntry("failed opening '%s'", filename );
            running = false;
        }
    }

    int LUA_PROGRAM::run(MAP *map,float dt,int viewer_id)									// Execute le script
    {
        pMutex.lock();
        draw_list.draw(gfx->TA_font);			// Execute la liste de commandes de dessin
        pMutex.unlock();

        if( !running )	return	-1;

        lua_program = this;
        lua_map = map;

        asm_timer += dt;

        if(sleeping) {
            sleep_time-=dt;
            if(sleep_time<=0.0f)
                sleeping=false;
            if(sleeping) {
                return -2;			// Marque une pause
            }
        }
        if(waiting) {
            if(amx!=mouse_x || amy!=mouse_y || amz!=mouse_z || amb!=mouse_b || keypressed()) {
                waiting=false;
            }
            if(waiting)
                return -3;				// Attend un évènement
        }

        lua_signal = 0;

        lua_pushstring( L, "main" );
        lua_gettable( L, LUA_GLOBALSINDEX );
        try {
            if( lua_pcall( L, 0, 1, 0 ) ) {
                if( lua_tostring( L, -1 ) != "" ) {
                    Console->AddEntry( "LUA ERROR: %s", lua_tostring( L, -1 ) );
                    printf("LUA ERROR: %s\n", lua_tostring( L, -1 ) );
                }
                running = false;
                return -1;
            }
        }
        catch(...)
        {
            if( lua_tostring( L, -1 ) != "" ) {
                Console->AddEntry( "LUA ERROR: %s", lua_tostring( L, -1 ) );
                printf("LUA ERROR: %s\n", lua_tostring( L, -1 ) );
            }
            running = false;
            return -1;
        }

        int result = (int) lua_tonumber( L, lua_gettop( L ) );		// Read the result
        lua_pop( L, 1 );

        amx=mouse_x;
        amy=mouse_y;
        amz=mouse_z;
        amb=mouse_b;

        if( lua_signal )
            result = lua_signal;

        return result;
    }

    void DRAW_LIST::add(DRAW_OBJECT &obj)
    {
        lua_program->lock();
        if(next==NULL) {
            next=new DRAW_LIST;
            next->prim=obj;
            next->next=NULL;
        }
        else next->add(obj);
        lua_program->unlock();
    }

    void DRAW_LIST::draw(GfxFont &fnt)
    {
        glPushMatrix();
        glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
        switch(prim.type)
        {
            case DRAW_TYPE_POINT:
                glBegin(GL_POINTS);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glEnd();
                break;
            case DRAW_TYPE_LINE:
                glBegin(GL_LINES);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glEnd();
                break;
            case DRAW_TYPE_CIRCLE:
                glBegin(GL_LINE_STRIP);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                {
                    int max = (int)(sqrt(prim.r[1])*2.0f)*2;
                    if(max>0)
                        for(int i=0;i<=prim.r[1]*10;i++)
                            glVertex2f(prim.x[0]+prim.r[1]*cos(i*6.2831853072f/max),prim.y[0]+prim.r[1]*sin(i*6.2831853072f/max));
                }
                glEnd();
                break;
            case DRAW_TYPE_TRIANGLE:
                glBegin(GL_TRIANGLES);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glVertex2f(prim.x[2],prim.y[2]);
                glEnd();
                break;
            case DRAW_TYPE_BOX:
                glBegin(GL_LINE_STRIP);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glVertex2f(prim.x[0],prim.y[1]);
                glVertex2f(prim.x[0],prim.y[0]);
                glEnd();
                break;
            case DRAW_TYPE_FILLBOX:
                glBegin(GL_QUADS);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glVertex2f(prim.x[0],prim.y[1]);
                glEnd();
                break;
            case DRAW_TYPE_TEXT:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                gfx->print(fnt,prim.x[0],prim.y[0],0.0f,makeacol32((int)(prim.r[0]*255.0f),(int)(prim.g[0]*255.0f),(int)(prim.b[0]*255.0f), 0xFF),prim.text);
                glDisable(GL_BLEND);
                break;
            case DRAW_TYPE_BITMAP:
                if( prim.tex == 0 && prim.text != NULL ) {
                    prim.tex = gfx->load_texture( prim.text );
                    free( prim.text );
                    prim.text = NULL;
                }
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                gfx->drawtexture(prim.tex,prim.x[0],prim.y[0],prim.x[1],prim.y[1]);
                glDisable(GL_BLEND);
                break;
        }
        glPopMatrix();
        if(next)
            next->draw(fnt);
    }

    // Create the script that will do what the mission description .ota file tells us to do
    void generate_script_from_mission( String Filename, cTAFileParser *ota_parser, int schema )
    {
        std::ofstream   m_File;

        m_File.open( Filename.c_str(), std::ios::out | std::ios::trunc );

        if( !m_File.is_open() )	{
            Console->AddEntry("ERROR : could not open file '%s' (%s, %d)", Filename.c_str(), __FILE__, __LINE__ );
            return;
        }

        m_File << "#include \"signals.h\"\n";

        m_File << "\nta3d_clf()\nta3d_init_res()\n";

        m_File << "ta3d_set_cam_pos( 0, ta3d_start_x( 0 ), ta3d_start_z( 0 ) )\n";

        int i = 0;
        String unit_name = "";

        while( !(unit_name = ota_parser->PullAsString( format( "GlobalHeader.Schema %d.units.unit%d.Unitname", schema, i ) ) ).empty() ) {
            String unit_key = format( "GlobalHeader.Schema %d.units.unit%d", schema, i );
            int player_id = ota_parser->PullAsInt( unit_key + ".player" ) - 1;
            float x = ota_parser->PullAsFloat( unit_key + ".XPos" ) * 0.5f;
            float z = ota_parser->PullAsFloat( unit_key + ".ZPos" ) * 0.5f;

            m_File << format( "\nunit_id = ta3d_create_unit( %d, \"", player_id ) << unit_name << format( "\", %f - 0.5 * ta3d_map_w(), %f - 0.5 * ta3d_map_h() )\n", x, z );

            float health = ota_parser->PullAsFloat( unit_key + ".HealthPercentage", -1.0f );
            if( health != -1.0f )
                m_File << format( "ta3d_set_unit_health( unit_id, %f )\n", health );

            String Ident = ota_parser->PullAsString( unit_key + ".Ident" );
            if( !Ident.empty() )
                m_File << Ident << " = unit_id\n";		// Links the unit_id to the given name

            m_File << unit_name << " = unit_id\n";		// Links the unit_id to the given unit_name so it can be used as an identifier

            String::Vector orders;
            ReadVectorString(orders, ota_parser->PullAsString( unit_key + ".InitialMission" ), ",");

            bool selectable = false;
            bool orders_given = false;

            for (int e = 0 ; e < orders.size() ; ++e)	// Converts InitialMission to a mission list
            {
                String::Vector params;
                ReadVectorString(params, orders[ e ], " " );		// Read all the mission parameters
                if( params.size() == 0 )	continue;

                params[ 0 ] = String::ToLower(params[0]);

                if( params[ 0 ][ 0 ] == 'p' && params[ 0 ].size() > 1 ) // something like p3000 2000, convert it to p 3000 2000
                {
                    params.resize( params.size() + 1 );
                    for( int i = params.size() - 1 ; i > 0 ; i++ )
                        if( i == 1 ) {
                            params[ 1 ] = params[ 0 ].substr( 1, params[ 0 ].size() - 1 );
                            params[ 0 ] = params[ 0 ][ 0 ];
                        }
                        else
                            params[ i ] = params[ i - 1 ];
                }

                if( params[ 0 ] == "m" ) {			// Move
                    if( params.size() >= 3 ) {
                        float pos_x = atof( params[ 1 ].c_str() ) * 0.5f;
                        float pos_z = atof( params[ 2 ].c_str() ) * 0.5f;
                        m_File << format( "ta3d_add_move_mission( unit_id, %f - 0.5 * ta3d_map_w(), %f - 0.5 * ta3d_map_h() )\n", pos_x, pos_z );
                    }
                    orders_given = true;
                }
                else if( params[ 0 ] == "a" ) {		// Attack
                    if( params.size() >= 2 )
                        m_File << "ta3d_add_attack_mission( unit_id, " + params[ 1 ] + " )\n";
                    orders_given = true;
                }
                else if( params[ 1 ] == "b" ) {		// Build
                    if( params.size() == 3 ) {			// Factories
                        m_File << "for i = 1, " + params[ 2 ] + " do\n";
                        m_File << "	ta3d_add_build_mission( unit_id, unit_x( unit_id ), unit_z( unit_id ), " + params[ 1 ] + " )\n";
                        m_File << "end\n";
                    }
                    else if( params.size() == 4 ) {		// Mobile builders
                        float pos_x = atof( params[ 2 ].c_str() ) * 0.5f;
                        float pos_z = atof( params[ 3 ].c_str() ) * 0.5f;
                        m_File << format( "ta3d_add_build_mission( unit_id, %f - 0.5 * ta3d_map_w(), %f - 0.5 * ta3d_map_h(), ", pos_x, pos_z ) + params[ 1 ] + " )\n";
                    }
                    orders_given = true;
                }
                else if( params[ 0 ] == "d" )		// Destroy
                    m_File << "ta3d_kill_unit( unit_id )\n";
                else if( params[ 0 ] == "p" ) {		// Patrol
                    int e = 0;
                    while( params.size() >= 3 + e * 2 ) {
                        float pos_x = atof( params[ 2 * e + 1 ].c_str() ) * 0.5f;
                        float pos_z = atof( params[ 2 * e + 2 ].c_str() ) * 0.5f;
                        m_File << format( "ta3d_add_patrol_mission( unit_id, %f - 0.5 * ta3d_map_w(), %f - 0.5 * ta3d_map_h() )\n", pos_x, pos_z );
                        e++;
                    }
                    orders_given = true;
                }
                else if( params[ 0 ] == "w" ) {		// Wait
                    if( params.size() >= 2 ) {
                        float time = atof( params[ 1 ].c_str() );
                        m_File << format( "ta3d_add_wait_mission( unit_id, %f )\n", time );
                    }
                    orders_given = true;
                }
                else if( params[ 0 ] == "wa" ) {		// Wait attacked
                    if( params.size() >= 2 )
                        m_File << "ta3d_add_wait_mission( unit_id, " + params[ 1 ] + " )\n";
                    else
                        m_File << "ta3d_add_wait_mission( unit_id, unit_id )\n";
                    orders_given = true;
                }
                else if( params[ 0 ] == "g" ) {		// Guard
                    if( params.size() >= 2 )
                        m_File << "ta3d_add_guard_mission( unit_id, " + params[ 1 ] + " )\n";
                    orders_given = true;
                }
                else if( params[ 0 ] == "o" ) {		// Set standing orders
                    if( params.size() >= 3 )
                        m_File << "ta3d_set_standing_orders( unit_id, " << params[ 1 ] << ", " << params[ 2 ] << " )\n";
                }
                else if( params[ 0 ] == "s" )		// Make it selectable
                    selectable = true;
            }

            if( !selectable && orders_given )
                m_File << "ta3d_lock_orders( unit_id )\n";

            i++;
        }

        i = 0;
        String feature_name = "";

        while( !(feature_name = ota_parser->PullAsString( format( "GlobalHeader.Schema %d.features.feature%d.Featurename", schema, i ) ) ).empty() ) {
            String unit_key = format( "GlobalHeader.Schema %d.features.feature%d", schema, i );
            float x = ota_parser->PullAsFloat( unit_key + ".XPos" ) * 16.0f;
            float z = ota_parser->PullAsFloat( unit_key + ".ZPos" ) * 16.0f;

            m_File << "\nta3d_create_feature( \"" << feature_name << "\", " << x << " - 0.5 * ta3d_map_w(), " << z << " - 0.5 * ta3d_map_h() )\n";
            i++;
        }

        m_File << "\ntimer = ta3d_time()\nend_signal = 0\n";

        m_File << "check = {}\nfirst_launch = true\n";
        m_File << "for i = 0, ta3d_get_max_unit_number() do\n";
        m_File << "	check[ i ] = false\n";
        m_File << "end\n\n";

        m_File << "pos_x = {}\n";
        m_File << "pos_z = {}\n";
        m_File << "exist = {}\n";
        m_File << "for i = 0, ta3d_get_max_unit_number() do\n";
        m_File << "	if ta3d_get_unit_owner( i ) == -1 then\n";
        m_File << "		exist[ i ] = false\n";
        m_File << "	else\n";
        m_File << "		exist[ i ] = true\n";
        m_File << "		pos_x[ i ] = ta3d_unit_x( i )\n";
        m_File << "		pos_z[ i ] = ta3d_unit_z( i )\n";
        m_File << "	end\n";
        m_File << "end\n";

        if (!ota_parser->PullAsString("GlobalHeader.KillUnitType").empty())
        {
            String::Vector params;
            ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.KillUnitType" ), "," );
            if (params.size() >= 2)
            {
                m_File << "\nKillUnitType_nb = ta3d_nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n" ;
                m_File << "KilledUnitType = 0\n";
            }
        }

        if (!ota_parser->PullAsString( "GlobalHeader.UnitTypeKilled" ).empty())
        {
            String::Vector params;
            ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.UnitTypeKilled" ), ",");
            if (params.size() >= 2)
            {
                m_File << "\nUnitTypeKilled_nb = ta3d_nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n" ;
                m_File << "UnitTypeKilled_count = 0\n";
            }
        }

        m_File << "AnyUnitPassesZ = false\n";
        m_File << "UnitTypePassesZ = false\n";
        m_File << "AnyUnitPassesX = false\n";
        m_File << "UnitTypePassesX = false\n";
        m_File << "BuildUnitType = false\n";
        m_File << "CaptureUnitType = false\n";
        m_File << "KillUnitType = false\n";
        m_File << "KillAllOfType = false\n";
        m_File << "KilledEnemyCommander = false\n";
        m_File << "DestroyAllUnits = false\n";
        m_File << "MoveUnitToRadius = false\n";
        m_File << "KillAllMobileUnits = false\n";

        m_File << "\nfunction main()\n";

        m_File << "	if end_signal ~= 0 and ta3d_time() - timer >= 5 then\n";
        m_File << "		return end_signal\n";
        m_File << "	elseif end_signal ~= 0 then\n";
        m_File << "		return 0\n";
        m_File << "	end\n\n";

        // DEFEAT conditions

        if( ota_parser->PullAsInt( "GlobalHeader.DeathTimerRunsOut" ) > 0 ) {
            m_File << "	if ta3d_time() >= " << ota_parser->PullAsString( "GlobalHeader.DeathTimerRunsOut" ) << " then\n";
            m_File << "		ta3d_print( 288, 236, \"DEFEAT\" )\n		timer = ta3d_time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
            m_File << "	end\n\n";
        }

        if (!ota_parser->PullAsString("GlobalHeader.UnitTypeKilled").empty())
        {
            String::Vector params;
            ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.UnitTypeKilled" ), "," );
            if( params.size() >= 2 )
            {
                m_File << "	new_UnitTypeKilled_nb = ta3d_nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n";
                m_File << "	if UnitTypeKilled_nb > new_UnitTypeKilled_nb then\n";
                m_File << "		UnitTypeKilled_count = UnitTypeKilled_count + UnitTypeKilled_nb - new_UnitTypeKilled_nb\n";
                m_File << "	end\n";
                m_File << "	if UnitTypeKilled_count >= " << params[ 1 ] << " then\n";
                m_File << "		ta3d_print( 288, 236, \"DEFEAT\" )\n		timer = ta3d_time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
                m_File << "	end\n";
                m_File << "	UnitTypeKilled_nb = new_UnitTypeKilled_nb\n\n";
            }
        }

        if( ota_parser->PullAsInt( "GlobalHeader.AllUnitsKilled" ) == 1 )
        {
            m_File << "	if ta3d_annihilated( 0 ) then\n";
            m_File << "		ta3d_print( 288, 236, \"DEFEAT\" )\n		timer = ta3d_time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
            m_File << "	end\n\n";
        }

        if( !ota_parser->PullAsString( "GlobalHeader.AllUnitsKilledOfType" ).empty() ) {
            String type = ota_parser->PullAsString( "GlobalHeader.AllUnitsKilledOfType" );
            m_File << "	if not ta3d_has_unit( 0, \"" << type << "\" ) and not ta3d_has_unit( 1, \"" << type << "\" ) then\n";
            m_File << "		ta3d_print( 288, 236, \"DEFEAT\" )\n		timer = ta3d_time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
            m_File << "	end\n\n";
        }

        if( ota_parser->PullAsInt( "GlobalHeader.CommanderKilled" ) == 1 ) {
            m_File << "	if not ta3d_has_unit( 0, ta3d_commander( 0 ) ) then\n";
            m_File << "		ta3d_print( 288, 236, \"DEFEAT\" )\n		timer = ta3d_time()\n		end_signal = SIGNAL_DEFEAT\n		return 0\n";
            m_File << "	end\n\n";
        }

        // VICTORY conditions

        m_File << "	victory_conditions = 0\n";
        int nb_victory_conditions = 0;

        m_File << "	if UnitTypePassesX then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if UnitTypePassesZ then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if AnyUnitPassesX then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if AnyUnitPassesZ then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if BuildUnitType then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if KillUnitType then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if CaptureUnitType then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if KillAllOfType then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if KilledEnemyCommander then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if DestroyAllUnits then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if MoveUnitToRadius then\n		victory_conditions = victory_conditions + 1\n	end\n";
        m_File << "	if KillAllMobileUnits then\n		victory_conditions = victory_conditions + 1\n	end\n";

        if( ota_parser->PullAsInt( "GlobalHeader.KillAllMobileUnits" ) == 1 )
        {
            m_File << "	if not KillAllMobileUnits and not ta3d_has_mobile_units( 1 ) then\n";
            m_File << "		KillAllMobileUnits = true\n";
            m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "	end\n";
        }

        if( !ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesZ" ).empty()
            || !ota_parser->PullAsString( "GlobalHeader.UnitTypePassesZ" ).empty()
            || !ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesX" ).empty()
            || !ota_parser->PullAsString( "GlobalHeader.UnitTypePassesX" ).empty() )
        {

            if( !ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesZ" ).empty() )
                m_File << "	ZPass0 = 0.5 * ( " << ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesZ" ) << " - ta3d_map_h() )\n";
            if( !ota_parser->PullAsString( "GlobalHeader.UnitTypePassesZ" ).empty() )
            {
                String::Vector params;
                ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.UnitTypePassesZ" ), "," );
                if( params.size() == 2 )
                    m_File << "	ZPass1 = 0.5 * ( " << params[ 1 ] << " - ta3d_map_h() )\n";
            }
            if( !ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesX" ).empty() )
                m_File << "	XPass0 = 0.5 * ( " << ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesX" ) << " - ta3d_map_w() )\n";
            if( !ota_parser->PullAsString( "GlobalHeader.UnitTypePassesX" ).empty() )
            {
                String::Vector params;
                ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.UnitTypePassesX" ), "," );
                if (params.size() == 2)
                    m_File << "	XPass1 = 0.5 * ( " << params[ 1 ] << " - ta3d_map_w() )\n";
            }

            m_File << "	for i = 0, ta3d_get_max_unit_number() do\n";
            m_File << "		unit_z = ta3d_unit_z( i )\n";
            m_File << "		unit_x = ta3d_unit_x( i )\n";
            m_File << "		unit_exist = ( ta3d_get_unit_owner( i ) ~= -1 )\n";
            if( !ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesZ" ).empty() )
            {
                m_File << "		if exist[ i ] and unit_exist and (pos_z[ i ] - ZPass0) * (unit_z - ZPass0) <= 0 and not AnyUnitPassesZ then\n";
                m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
                m_File << "			AnyUnitPassesZ = true\n";
                m_File << "		end\n";
            }
            if( !ota_parser->PullAsString( "GlobalHeader.UnitTypePassesZ" ).empty() )
            {
                String::Vector params;
                ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.UnitTypePassesZ" ), "," );
                if( params.size() == 2 )
                {
                    m_File << "		if exist[ i ] and unit_exist and ta3d_is_unit_of_type( i, \"" << params[ 0 ] << "\" ) and (pos_z[ i ] - ZPass1) * (unit_z - ZPass1) <= 0 and not UnitTypePassesZ then\n";
                    m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
                    m_File << "			UnitTypePassesZ = true\n";
                    m_File << "		end\n";
                }
            }
            if( !ota_parser->PullAsString( "GlobalHeader.AnyUnitPassesX" ).empty() )
            {
                m_File << "		if exist[ i ] and unit_exist and (pos_x[ i ] - XPass0) * (unit_x - XPass0) <= 0 and not AnyUnitPassesX then\n";
                m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
                m_File << "			AnyUnitPassesX = true\n";
                m_File << "		end\n";
            }
            if( !ota_parser->PullAsString( "GlobalHeader.UnitTypePassesX" ).empty() )
            {
                String::Vector params;
                ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.UnitTypePassesX" ), "," );
                if( params.size() == 2 )
                {
                    m_File << "		if exist[ i ] and unit_exist and ta3d_is_unit_of_type( i, \"" << params[ 0 ] << "\" ) and (pos_x[ i ] - XPass1) * (unit_x - XPass1) <= 0 and not UnitTypePassesX then\n";
                    m_File << "			victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
                    m_File << "			UnitTypePassesX = true\n";
                    m_File << "		end\n";
                }
            }
            m_File << "		if unit_exist then\n";
            m_File << "			exist[ i ] = true\n";
            m_File << "			pos_x[ i ] = unit_x\n";
            m_File << "			pos_z[ i ] = unit_z\n";
            m_File << "		else\n";
            m_File << "			exist[ i ] = false\n";
            m_File << "		end\n";
            m_File << "	end\n";
        }

        if( !ota_parser->PullAsString( "GlobalHeader.BuildUnitType" ).empty() )
        {
            m_File << "	if ta3d_has_unit( 0, \"" + ota_parser->PullAsString( "GlobalHeader.BuildUnitType" ) + "\" ) and not BuildUnitType then\n";
            m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "		BuildUnitType = true\n";
            m_File << "	end\n\n";
        }

        if( !ota_parser->PullAsString( "GlobalHeader.CaptureUnitType" ).empty() )
        {
            m_File << "	if ta3d_has_unit( 0, \"" + ota_parser->PullAsString( "GlobalHeader.CaptureUnitType" ) + "\" ) and not CaptureUnitType then\n";
            m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "		CaptureUnitType = true\n";
            m_File << "	end\n\n";
        }

        if( !ota_parser->PullAsString( "GlobalHeader.KillUnitType" ).empty() )
        {
            String::Vector params;
            ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.KillUnitType" ), "," );
            if( params.size() >= 2 )
            {
                m_File << "	new_KillUnitType_nb = ta3d_nb_unit_of_type( 1, \"" << params[ 0 ] << "\" )\n";
                m_File << "	if KillUnitType_nb > new_KillUnitType_nb then\n";
                m_File << "		KilledUnitType = KilledUnitType + KillUnitType_nb - new_KillUnitType_nb\n";
                m_File << "	end\n";
                m_File << "	if KilledUnitType >= " << params[ 1 ] << " and not KillUnitType then\n";
                m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
                m_File << "		KillUnitType = true\n";
                m_File << "	end\n";
                m_File << "	KillUnitType_nb = new_KillUnitType_nb\n\n";
            }
        }

        if( !ota_parser->PullAsString( "GlobalHeader.KillAllOfType" ).empty() )
        {
            m_File << "	if not ta3d_has_unit( 1, \"" + ota_parser->PullAsString( "GlobalHeader.KillAllOfType" ) + "\" ) and not KillAllOfType then\n";
            m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "		KillAllOfType = true\n";
            m_File << "	end\n\n";
        }

        if( ota_parser->PullAsInt( "GlobalHeader.KilledEnemyCommander" ) == 1 )
        {
            m_File << "	if not ta3d_has_unit( 1, ta3d_commander( 1 ) ) and not KilledEnemyCommander then\n";
            m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "		KilledEnemyCommander = true\n";
            m_File << "	end\n\n";
        }

        if( ota_parser->PullAsInt( "GlobalHeader.DestroyAllUnits" ) == 1 )
        {
            m_File << "	annihilated = 0\n";
            m_File << "	for i = 1, ta3d_nb_players() do\n";
            m_File << "		if ta3d_annihilated( i ) then\n";
            m_File << "			annihilated = annihilated + 1\n";
            m_File << "		end\n";
            m_File << "	end\n";
            m_File << "	if annihilated == ta3d_nb_players() - 1 and not DestroyAllUnits then\n";
            m_File << "		victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "		DestroyAllUnits = true\n";
            m_File << "	end\n\n";
        }

        if( !ota_parser->PullAsString( "GlobalHeader.MoveUnitToRadius" ).empty() )
        {
            String::Vector params;
            ReadVectorString(params, ota_parser->PullAsString( "GlobalHeader.MoveUnitToRadius" ) );
            m_File << "	for i = 0, ta3d_get_max_unit_number() do\n";
            if (String::ToLower(params[0]) == "anytype")
                m_File << "		if ta3d_get_unit_owner( i ) == 0 then\n";
            else
                m_File << "		if ta3d_get_unit_owner( i ) == 0 and ta3d_is_unit_of_type( i, \"" << params[0] << "\" ) then\n";
            m_File << "			dx = ta3d_unit_x( i ) + 0.5 * (ta3d_map_w() - " << params[ 1 ] << " )\n";
            m_File << "			dz = ta3d_unit_z( i ) + 0.5 * (ta3d_map_h() - " << params[ 2 ] << " )\n";
            m_File << "			dist = dx * dx + dz * dz\n";
            float dist = atoi( params[ 3 ].c_str() ) * 0.5f;
            m_File << "			if dist <= " << format("%f", dist * dist ) << " then\n";
            m_File << "				if not first_launch and not check[ i ] and not MoveUnitToRadius then\n";
            m_File << "					victory_conditions = victory_conditions + 1\n";	nb_victory_conditions++;
            m_File << "					MoveUnitToRadius = true\n";
            m_File << "				end\n";
            m_File << "				check[ i ] = true\n";
            m_File << "			end\n";
            m_File << "		end\n";
            m_File << "	end\n";
        }

        m_File << "	if victory_conditions == " << nb_victory_conditions << " then\n";
        m_File << "		ta3d_play( \"VICTORY2\" )\n";
        m_File << "		ta3d_draw_image( \"gfx/victory.tga\", 160, 140, 480, 340 )\n";
        m_File << "		timer = ta3d_time()\n";
        m_File << "		end_signal = SIGNAL_VICTORY\n";
        m_File << "	end\n";

        m_File << "	first_launch = false\n";

        m_File << "	return 0\n";
        m_File << "end\n";

        m_File.flush();
        m_File.close();
    }

} // namespace TA3D

