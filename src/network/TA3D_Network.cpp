/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "TA3D_Network.h"
#include "../EngineClass.h"
#include "../UnitEngine.h"
#include "../scripts/script.h"
#include "../misc/camera.h"
#include "../ingame/sidedata.h"
#include <vector>
#include "../languages/i18n.h"
#include "../misc/math.h"
#include "../sounds/manager.h"
#include "../console.h"
#include "../ingame/players.h"
#include "../misc/paths.h"
#include "../misc/files.h"
#include "../restore.h"


#define CHAT_MESSAGE_TIMEOUT	10000
#define CHAT_MAX_MESSAGES       10

namespace TA3D
{


    TA3DNetwork *g_ta3d_network = NULL;

    const float	tick_conv = 1.0f / TICKS_PER_SEC;


    void TA3DSock::makeTick(int from)
    {
        if (tcpinbuf[0] != 'T')
        {
            LOG_ERROR(LOG_PREFIX_NET_SOCKET << "The data doesn't start with an 'T'. Impossible to read tick data");
            return;
        }
        if (tiremain == -1)
            return;

        tibrp = 1;
        if (g_ta3d_network && g_ta3d_network->game_data)
        {
            int player_id = g_ta3d_network->game_data->net2id( from );
            if (player_id >= 0)
            {
                units.client_tick[ player_id ] = getLong() * 1000;
                units.client_speed[ player_id ] = getShort();
            }
        }
        tibp = 0;
        tiremain = -1;
    }


    TA3DNetwork::TA3DNetwork( AREA *area, GameData *game_data )
        :messages()
    {
        enter = false;
        this->area = area;
        this->game_data = game_data;
        signal = 0;
    }

    TA3DNetwork::~TA3DNetwork()
    {
        for (std::list<NetworkMessage>::iterator i = messages.begin(); i != messages.end(); ++i)
            i->text.clear();
        messages.clear();
    }

    void TA3DNetwork::check()
    {
        if (!network_manager.isConnected())
        {
            lp_CONFIG->enable_shortcuts = true;
            return;		// Only works in network mode
        }

        if( key[KEY_ENTER] && !console.activated())
        {
            if( !enter )
            {
                if( area->get_state("chat") ) // Chat is visible, so hide it and send the message is not empty
                {
                    area->msg("chat.hide");
                    String msg = area->get_caption("chat.msg");
                    area->set_caption("chat.msg", "");

                    if( !msg.empty() ) // If not empty send the message
                    {
                        struct chat chat;
                        strtochat( &chat, msg );
                        network_manager.sendChat( &chat );

                        int player_id = game_data->net2id( chat.from );
                        if( player_id >= 0 )
                        {
                            pMutex.lock();
                            if( messages.size() > CHAT_MAX_MESSAGES )		// Prevent flooding the screen with chat messages
                                messages.pop_front();
                            msg = "<" + game_data->player_names[ player_id ] + "> " + msg;
                            messages.push_back( NetworkMessage( msg, msec_timer ) );
                            pMutex.unlock();
                        }
                    }
                }
                else
                {								// Chat is hidden, so show it
                    area->set_caption("chat.msg", "");
                    area->msg("chat.show");
                    area->msg("chat.msg.focus");
                }
            }
            enter = true;
        }
        else
            enter = false;
        if( area->get_state("chat") )		// Chat is visible, so give it the focus so we can type the message
        {
            area->msg("chat.focus");
            lp_CONFIG->enable_shortcuts = false;
        }
        else
            lp_CONFIG->enable_shortcuts = true;

        int n = 5;
        while(n--) // Chat receiver
        {
            String chat_msg;
            struct chat received_chat_msg;

            if( network_manager.getNextChat( &received_chat_msg ) == 0 )
                chat_msg = received_chat_msg.message;
            else
                break;

            int player_id = game_data->net2id( received_chat_msg.from );
            if( player_id >= 0 )
            {
                pMutex.lock();
                if( messages.size() > CHAT_MAX_MESSAGES )		// Prevent flooding the screen with chat messages
                    messages.pop_front();
                chat_msg = "<" + game_data->player_names[ player_id ] + "> " + chat_msg;
                messages.push_back( NetworkMessage( chat_msg, msec_timer ) );
                pMutex.unlock();
            }
        }

        pMutex.lock();
        for (std::list<NetworkMessage>::iterator i = messages.begin(); i != messages.end(); )
        {
            if( msec_timer - i->timer >= CHAT_MESSAGE_TIMEOUT )
                messages.erase(i++);
            else
                ++i;
        }
        pMutex.unlock();

        n = 100;
        while(--n)	// Special message receiver
        {
            String special_msg;
            special received_special_msg;

            if( network_manager.getNextSpecial( &received_special_msg ) == 0 )
                special_msg = received_special_msg.message;
            else
                break;

            int player_id = game_data->net2id( received_special_msg.from );

            String::Vector params;
            special_msg.split(params, " ");
            if (params.size() == 1)
            {
                if (params[0] == "GONE")       // Someone tell us he's gone !! Remove it from remote players otherwise game
                {                                   // will freeze
                    if (network_manager.isServer())     // Hum currently we only handle the case where a client leaves ... server must stay!!
                    {
                        network_manager.dropPlayer(received_special_msg.from);      // Remove it from socket list
                        if (game_data)
                        {
                            game_data->player_control[ player_id ] = PLAYER_CONTROL_NONE;       // Remove control data
                            players.control[ player_id ] = PLAYER_CONTROL_NONE;
                        }
                    }
                }
                else if (params[0] == "PAUSE")
                {
                    lp_CONFIG->pause = true;
                    area->msg("esc_menu.show");
                    area->msg("esc_menu.b_pause.hide");
                    area->msg("esc_menu.b_resume.show");
                }
                else if (params[0] == "RESUME")
                {
                    lp_CONFIG->pause = false;
                    area->msg("esc_menu.hide");
                    area->msg("esc_menu.b_pause.show");
                    area->msg("esc_menu.b_resume.hide");
                }
            }
            else if (params.size() == 2)
            {
                if (params[0] == "USING")           // End of available units synchronization ... easier that way and the game doesn't start syncing before it's finished
                                                    // so game doesn't start before this :P
                {                                   // We can only use units available on all clients, so check the list
                    int type_id = unit_manager.get_unit_index(params[1]);
                    if (type_id == -1 || unit_manager.unit_type[type_id]->not_used)            // Tell it's missing
                        network_manager.sendAll( "MISSING " + params[1]);
                }
                else if (params[0] == "MISSING")
                {
                    int idx = unit_manager.get_unit_index(params[1]);
                    if (idx >= 0)
                        unit_manager.unit_type[idx]->not_used = true;
                }
                else if (params[0] == "SAVE")           // Server order to save the game
                {
                    String filename = Paths::Savegames + "multiplayer" + Paths::Separator + Paths::Files::ReplaceExtension(ReplaceChar( params[1], 1, ' ' ), ".sav");
                    save_game(filename, game_data); // Save the game using filename given by server
                }
            }
        }

        n = 100;
        while(--n) // Sync message receiver
        {
            struct sync sync_msg;

            if( network_manager.getNextSync( &sync_msg ) )
                break;

            if( sync_msg.unit < units.max_unit )
            {
                units.unit[sync_msg.unit].lock();
                if( !(units.unit[sync_msg.unit].flags & 1) || units.unit[sync_msg.unit].exploding || units.unit[sync_msg.unit].last_synctick[0] >= sync_msg.timestamp )
                {
                    units.unit[sync_msg.unit].unlock();
                    continue;
                }

                units.unit[sync_msg.unit].flying = sync_msg.flags & SYNC_FLAG_FLYING;
                units.unit[sync_msg.unit].cloaking = sync_msg.flags & SYNC_FLAG_CLOAKING;

                units.unit[sync_msg.unit].last_synctick[0] = sync_msg.timestamp;
                units.unit[sync_msg.unit].Pos.x = sync_msg.x;
                units.unit[sync_msg.unit].Pos.y = sync_msg.y;
                units.unit[sync_msg.unit].Pos.z = sync_msg.z;

                units.unit[sync_msg.unit].V.x = sync_msg.vx;
                units.unit[sync_msg.unit].V.z = sync_msg.vz;
                units.unit[sync_msg.unit].V.y = 0.0f;

                // Guess where the unit should be now
                units.unit[sync_msg.unit].Pos = units.unit[sync_msg.unit].Pos + ((((int)units.current_tick) - (int)sync_msg.timestamp) * tick_conv) * units.unit[sync_msg.unit].V;

                units.unit[sync_msg.unit].cur_px = ((int)(units.unit[sync_msg.unit].Pos.x)+the_map->map_w_d+4)>>3;
                units.unit[sync_msg.unit].cur_py = ((int)(units.unit[sync_msg.unit].Pos.z)+the_map->map_h_d+4)>>3;


                units.unit[sync_msg.unit].Angle.y = sync_msg.orientation * 360.0f / 65536.0f;

                units.unit[sync_msg.unit].hp = sync_msg.hp;
                units.unit[sync_msg.unit].build_percent_left = sync_msg.build_percent_left / 2.55f;

                units.unit[sync_msg.unit].unlock();
                units.unit[sync_msg.unit].draw_on_map();
            }
        }

        n = 100;
        while(--n) // Event message receiver
        {
            struct event event_msg;

            if( network_manager.getNextEvent( &event_msg ) )
                break;

            switch( event_msg.type )
            {
                case EVENT_UNIT_NANOLATHE:				// Sync nanolathe effect
                    if( event_msg.opt1 < units.max_unit && (units.unit[ event_msg.opt1 ].flags & 1) ) {
                        units.unit[ event_msg.opt1 ].lock();
                        if( event_msg.opt2 & 4 )		// Stop nanolathing
                            units.unit[ event_msg.opt1 ].nanolathe_target = -1;
                        else {							// Start nanolathing
                            units.unit[ event_msg.opt1 ].nanolathe_reverse = event_msg.opt2 & 2;
                            units.unit[ event_msg.opt1 ].nanolathe_feature = event_msg.opt2 & 1;
                            if( event_msg.opt2 & 1 ) {		// It's a feature
                                int sx = event_msg.opt3;
                                int sy = event_msg.opt4;
                                units.unit[ event_msg.opt1 ].nanolathe_target = the_map->map_data[sy][sx].stuff;
                            }
                            else							// It's a unit
                                units.unit[ event_msg.opt1 ].nanolathe_target = event_msg.opt3;
                        }
                        units.unit[ event_msg.opt1 ].unlock();
                    }
                    break;
                case EVENT_FEATURE_CREATION:
                    {
                        int sx = event_msg.opt3;		// Burn the object
                        int sy = event_msg.opt4;
                        if( sx < the_map->bloc_w_db && sy < the_map->bloc_h_db )
                        {
                            int type = feature_manager.get_feature_index( (const char*)(event_msg.str) );
                            if( type >= 0)
                            {
                                Feature *feature = feature_manager.getFeaturePointer(type);
                                Vector3D feature_pos( event_msg.x, event_msg.y, event_msg.z );
                                the_map->map_data[sy][sx].stuff = features.add_feature( feature_pos, type );
                                if(feature->blocking)
                                    the_map->rect(sx - (feature->footprintx >> 1), sy - (feature->footprintz >> 1), feature->footprintx, feature->footprintz, -2 - the_map->map_data[sy][sx].stuff);
                            }
                        }
                    }
                    break;
                case EVENT_FEATURE_FIRE:
                    {
                        int sx = event_msg.opt3;		// Burn the object
                        int sy = event_msg.opt4;
                        if( sx < the_map->bloc_w_db && sy < the_map->bloc_h_db ) {
                            int idx = the_map->map_data[sy][sx].stuff;
                            if( !features.feature[idx].burning )
                                features.burn_feature( idx );
                        }
                    }
                    break;
                case EVENT_FEATURE_DEATH:
                    {
                        int sx = event_msg.opt3;		// Remove the object
                        int sy = event_msg.opt4;
                        if (sx < the_map->bloc_w_db && sy < the_map->bloc_h_db)
                        {
                            int idx = the_map->map_data[sy][sx].stuff;
                            if (idx >= 0)
                            {
                                Feature *feature = feature_manager.getFeaturePointer(features.feature[idx].type);
                                the_map->rect(sx - (feature->footprintx >> 1), sy - (feature->footprintz >> 1), feature->footprintx, feature->footprintz, -1);
                                features.delete_feature(idx);			// Delete it
                            }
                        }
                    }
                    break;
                case EVENT_SCRIPT_SIGNAL:
                    if( event_msg.opt1 == players.local_human_id || event_msg.opt1 == 0xFFFF )				// Do it only if the packet is for us
                        g_ta3d_network->set_signal( event_msg.opt2 );
                    break;
                case EVENT_UNIT_EXPLODE:				// BOOOOM and corpse creation :)
                    if( event_msg.opt1 < units.max_unit && ( units.unit[ event_msg.opt1 ].flags & 1 ) ) {		// If it's false then game is out of sync !!
                        units.unit[ event_msg.opt1 ].lock();

                        units.unit[ event_msg.opt1 ].severity = event_msg.opt2;
                        units.unit[ event_msg.opt1 ].Pos.x = event_msg.x;
                        units.unit[ event_msg.opt1 ].Pos.y = event_msg.y;
                        units.unit[ event_msg.opt1 ].Pos.z = event_msg.z;

                        units.unit[ event_msg.opt1 ].explode();			// BOOM :)

                        units.unit[ event_msg.opt1 ].unlock();
                    }
                    break;
                case EVENT_CLS:
                    if( event_msg.opt1 == players.local_human_id || event_msg.opt1 == 0xFFFF ) {			// Do it only if the packet is for us
                        LuaProgram::inGame->lock();
                        LuaProgram::inGame->draw_list.destroy();
                        LuaProgram::inGame->unlock();
                    }
                    break;
                case EVENT_DRAW:
                    if( event_msg.opt1 == players.local_human_id || event_msg.opt1 == 0xFFFF ) {			// Do it only if the packet is for us
                        DrawObject draw_obj;
                        draw_obj.type = DRAW_TYPE_BITMAP;
                        draw_obj.x[0] = event_msg.x;
                        draw_obj.y[0] = event_msg.y;
                        draw_obj.x[1] = event_msg.z;
                        draw_obj.y[1] = event_msg.opt3 / 16384.0f;
                        draw_obj.text = strdup( I18N::Translate( (char*)event_msg.str ).c_str() );		// We can't load it now because of thread safety
                        draw_obj.tex = 0;
                        LuaProgram::inGame->draw_list.add( draw_obj );
                    }
                    break;
                case EVENT_PRINT:
                    if (event_msg.opt1 == players.local_human_id || event_msg.opt1 == 0xFFFF) // Do it only if the packet is for us
                    {
                        DrawObject draw_obj;
                        draw_obj.type = DRAW_TYPE_TEXT;
                        draw_obj.r[0] = 1.0f;
                        draw_obj.g[0] = 1.0f;
                        draw_obj.b[0] = 1.0f;
                        draw_obj.x[0] = event_msg.x;
                        draw_obj.y[0] = event_msg.y;
                        draw_obj.text = strdup( I18N::Translate( (char*)event_msg.str ).c_str() );
                        LuaProgram::inGame->draw_list.add( draw_obj );
                    }
                    break;
                case EVENT_PLAY:
                    if (event_msg.opt1 == players.local_human_id || event_msg.opt1 == 0xFFFF) // Do it only if the packet is for us
                        sound_manager->playSound((char*)event_msg.str, false);
                    break;
                case EVENT_CLF:
                    the_map->clear_FOW();
                    units.lock();
                    for( int i = 0 ; i < units.index_list_size ; i++ )
                        units.unit[ units.idx_list[ i ] ].old_px = -10000;
                    units.unlock();
                    break;
                case EVENT_INIT_RES:
                    for( uint16 i = 0 ; i < players.nb_player ; i++ ) {
                        players.metal[i] = players.com_metal[i];
                        players.energy[i] = players.com_energy[i];
                    }
                    break;
                case EVENT_CAMERA_POS:
                    if( event_msg.opt1 == players.local_human_id ) 	// Move the camera only if the packet is for us
                    {
                        Camera::inGame->rpos.x = event_msg.x;
                        Camera::inGame->rpos.z = event_msg.z;
                    }
                    break;
                case EVENT_UNIT_SYNCED:
                    if( event_msg.opt1 < units.max_unit && ( units.unit[ event_msg.opt1 ].flags & 1 ) ) {
                        units.unit[ event_msg.opt1 ].lock();

                        int player_id = game_data->net2id( event_msg.opt2 );

                        if( player_id >= 0 )
                            units.unit[ event_msg.opt1 ].last_synctick[player_id] = event_msg.opt3;

                        units.unit[ event_msg.opt1 ].unlock();
                    }
                    break;
                case EVENT_UNIT_PARALYZE:
                    if (event_msg.opt1 < units.max_unit && (units.unit[event_msg.opt1].flags & 1))
                    {
                        units.unit[ event_msg.opt1 ].lock();

                        if (units.unit[ event_msg.opt1 ].exploding)
                        {
                            units.unit[ event_msg.opt1 ].unlock();
                            break;
                        }
                        float damage = event_msg.opt2 / 3600.0f;

                        units.unit[ event_msg.opt1 ].paralyzed = damage;

                        units.unit[ event_msg.opt1 ].unlock();
                    }
                    break;
                case EVENT_UNIT_DAMAGE:
                    if (event_msg.opt1 < units.max_unit && (units.unit[event_msg.opt1].flags & 1))
                    {
                        units.unit[ event_msg.opt1 ].lock();

                        if (units.unit[ event_msg.opt1 ].exploding)
                        {
                            units.unit[ event_msg.opt1 ].unlock();
                            break;
                        }
                        float damage = event_msg.opt2 / 16.0f;

                        units.unit[ event_msg.opt1 ].hp -= damage;

                        units.unit[ event_msg.opt1 ].flags &= 0xEF;		// This unit must explode if it has been damaged by a weapon even if it is being reclaimed
                        if( units.unit[ event_msg.opt1 ].hp <= 0.0f )
                            units.unit[ event_msg.opt1 ].severity = Math::Max(units.unit[event_msg.opt1].severity, (int)damage);

                        units.unit[ event_msg.opt1 ].unlock();
                    }
                    break;
                case EVENT_WEAPON_CREATION:
                    {
                        Vector3D target_pos( event_msg.x, event_msg.y, event_msg.z );
                        Vector3D Dir( event_msg.dx / 16384.0f, event_msg.dy / 16384.0f, event_msg.dz / 16384.0f );
                        Vector3D startpos( event_msg.vx, event_msg.vy, event_msg.vz );

                        int w_type = weapon_manager.get_weapon_index( (char*)event_msg.str );
                        if (w_type >= 0)
                        {
                            weapons.lock();
                            int w_idx = weapons.add_weapon(w_type,event_msg.opt1);
                            int player_id = event_msg.opt5;

                            if(weapon_manager.weapon[w_type].startsmoke)
                                particle_engine.make_smoke(startpos,0,1,0.0f,-1.0f,0.0f, 0.3f);

                            if (w_idx >= 0)
                            {
                                weapons.weapon[w_idx].local = false;

                                weapons.weapon[w_idx].damage = event_msg.opt4;
                                weapons.weapon[w_idx].Pos = startpos;
                                weapons.weapon[w_idx].local = false;
                                if(weapon_manager.weapon[w_type].startvelocity==0.0f && !weapon_manager.weapon[w_type].selfprop)
                                    weapons.weapon[w_idx].V = weapon_manager.weapon[w_type].weaponvelocity*Dir;
                                else
                                    weapons.weapon[w_idx].V = weapon_manager.weapon[w_type].startvelocity*Dir;
                                if( weapon_manager.weapon[w_type].dropped || !(weapon_manager.weapon[w_type].rendertype & RENDER_TYPE_LASER) ) {
                                    units.unit[ event_msg.opt1 ].lock();
                                    if( (units.unit[ event_msg.opt1 ].flags & 1) )
                                        weapons.weapon[w_idx].V = weapons.weapon[w_idx].V + units.unit[ event_msg.opt1 ].V;
                                    units.unit[ event_msg.opt1 ].unlock();
                                }
                                weapons.weapon[w_idx].owner = player_id;
                                weapons.weapon[w_idx].target = event_msg.opt2;
                                if( event_msg.opt2 < weapons.weapon.size() ) {
                                    if(weapon_manager.weapon[w_type].interceptor)
                                        weapons.weapon[w_idx].target_pos = weapons.weapon[event_msg.opt2].Pos;
                                    else
                                        weapons.weapon[w_idx].target_pos = target_pos;
                                }
                                else
                                    weapons.weapon[w_idx].target_pos = target_pos;
                                weapons.weapon[w_idx].stime = 0.0f;
                                weapons.weapon[w_idx].visible = true;

                                for( uint32 i = event_msg.opt3 ; i < units.current_tick && weapons.weapon[w_idx].weapon_id >= 0 ; i++ )		// Guess what happened (compensate latency)
                                    weapons.weapon[w_idx].move(tick_conv,the_map);
                            }
                            else
                                LOG_WARNING(LOG_PREFIX_NET << "couldn't create weapon '" << (char*)event_msg.str << "'" );
                            weapons.unlock();
                        }
                        else
                            LOG_WARNING(LOG_PREFIX_NET << "couldn't identify weapon '" << (char*)event_msg.str << "'" );
                    }
                    break;
                case EVENT_UNIT_SCRIPT:
                    if( event_msg.opt1 < units.max_unit && (units.unit[ event_msg.opt1 ].flags & 1) )
                        units.unit[ event_msg.opt1 ].launch_script( event_msg.opt2, event_msg.opt3, (int*)event_msg.str );
                    break;
                case EVENT_UNIT_DEATH:
                    {
                        int e = -1;
                        units.lock();

                        for( int i = 0 ; i < units.max_unit ; i++ )
                            if (units.idx_list[ i ] == event_msg.opt1)
                            {
                                e = i;
                                break;
                            }

                        units.unlock();

                        units.kill( event_msg.opt1, the_map, e, false );
                    }
                    break;
                case EVENT_UNIT_CREATION:
                    {
                        units.lock();

                        int idx = unit_manager.get_unit_index( (char*)event_msg.str );
                        if (idx >= 0)
                        {
                            Vector3D pos;
                            pos.x = event_msg.x;
                            pos.z = event_msg.z;
                            pos.y = the_map->get_unit_h( pos.x, pos.z );
                            Unit *unit = (Unit*)create_unit( idx, (event_msg.opt2 & 0xFF),pos,the_map,false);		// We don't want to send sync data for this ...
                            if (unit)
                            {
                                unit->lock();

                                if (event_msg.opt2 & 0x1000) // Created by a script, so give it 100% HP
                                {
                                    unit->hp = unit_manager.unit_type[idx]->MaxDamage;
                                    unit->built = false;
                                    unit->build_percent_left = 0.0f;
                                }
                                else
                                {
                                    unit->hp = 0.001f;
                                    unit->built = true;
                                    unit->build_percent_left = 100.0f;
                                }
                                unit->unlock();
                            }
                            else
                                LOG_ERROR("Cannot create unit of type `" << event_msg.str << "`");
                        }
                        else
                            LOG_ERROR("Cannot create unit, `" << event_msg.str << "` not found");

                        units.unlock();
                    }
                    break;
            };
        }
    }

    void TA3DNetwork::draw()
    {
        if( !network_manager.isConnected() )	return;		// Only works in network mode

        pMutex.lock();
        if( !messages.empty() )
        {
            const float Y_ref = 32 + gfx->TA_font->height();
            float Y = Y_ref;
            for (std::list<NetworkMessage>::const_iterator i = messages.begin(); i != messages.end(); ++i)
            {
                int color = 0xFFFFFFFF;
                if( (int)(msec_timer - i->timer) - CHAT_MESSAGE_TIMEOUT + 1000 >= 0 )
                {
                    color = makeacol( 0xFF, 0xFF, 0xFF, 255 - Math::Min(255, ((int)(msec_timer - i->timer) - CHAT_MESSAGE_TIMEOUT + 1000) * 255 / 1000));
                    Y -= Math::Min(1.0f, ((int)(msec_timer - i->timer) - CHAT_MESSAGE_TIMEOUT + 1000) * 0.001f) * (gfx->TA_font->height() + Y - Y_ref);
                }
                gfx->print( gui_font, 136, Y, 0.0f, color, i->text );
                Y += gui_font->height();
            }
        }
        pMutex.unlock();
    }

    bool TA3DNetwork::isLocal( int player_id )
    {
        return !(game_data->player_control[ player_id ] & PLAYER_CONTROL_FLAG_REMOTE);
    }

    bool TA3DNetwork::isRemoteHuman( int player_id )
    {
        return game_data->player_control[ player_id ] == PLAYER_CONTROL_REMOTE_HUMAN;
    }

    void TA3DNetwork::sendDamageEvent( int idx, float damage )
    {
        struct event event;
        event.type = EVENT_UNIT_DAMAGE;
        event.opt1 = idx;
        event.opt2 = (int)(damage * 16.0f);
        network_manager.sendEvent( &event );
    }

    void TA3DNetwork::sendParalyzeEvent( int idx, float damage )
    {
        struct event event;
        event.type = EVENT_UNIT_PARALYZE;
        event.opt1 = idx;
        event.opt2 = (int)(damage * 60.0f);
        network_manager.sendEvent( &event );
    }

    void TA3DNetwork::sendFeatureCreationEvent( int idx )
    {
        if( idx < 0 || features.feature[ idx ].type < 0 )	return;
        struct event event;
        event.type = EVENT_FEATURE_CREATION;
        event.opt3 = features.feature[ idx ].px;
        event.opt4 = features.feature[ idx ].py;
        event.x = features.feature[ idx ].Pos.x;
        event.y = features.feature[ idx ].Pos.y;
        event.z = features.feature[ idx ].Pos.z;
        String name = feature_manager.getFeaturePointer( features.feature[ idx ].type )->name;
        if (!name.empty())
        {
            memcpy( event.str, name.c_str(), name.size() + 1 ) ;
            network_manager.sendEvent( &event );
        }
    }

    void TA3DNetwork::sendFeatureDeathEvent( int idx )
    {
        if( idx < 0 || features.feature[ idx ].type < 0 )	return;

        struct event event;
        event.type = EVENT_FEATURE_DEATH;
        event.opt3 = features.feature[ idx ].px;
        event.opt4 = features.feature[ idx ].py;
        network_manager.sendEvent( &event );
    }

    void TA3DNetwork::sendFeatureFireEvent( int idx )
    {
        if( idx < 0 || features.feature[ idx ].type < 0 )	return;

        struct event event;
        event.type = EVENT_FEATURE_FIRE;
        event.opt3 = features.feature[ idx ].px;
        event.opt4 = features.feature[ idx ].py;
        network_manager.sendEvent( &event );
    }

    void TA3DNetwork::sendUnitNanolatheEvent( int idx, int target, bool feature, bool reverse )
    {
        if( idx < 0 || idx >= units.max_unit || !( units.unit[ idx ].flags & 1 ) )	return;

        struct event event;
        event.type = EVENT_UNIT_NANOLATHE;
        event.opt1 = idx;
        event.opt2 = (reverse ? 1 : 0) | (feature ? 2 : 0) | (target < 0 ? 4 : 0);
        if( feature ) {
            if( target < 0 || target >= features.max_features || features.feature[ target ].type < 0 )	return;
            event.opt3 = features.feature[ target ].px;
            event.opt4 = features.feature[ target ].py;
        }
        else
            event.opt3 = target;
        network_manager.sendEvent( &event );
    }

    int TA3DNetwork::getNetworkID( int unit_id )
    {
        if( unit_id >= units.max_unit )	return -1;
        units.unit[ unit_id ].lock();
        if (!(units.unit[ unit_id ].flags & 1))
        {
            units.unit[ unit_id ].unlock();
            return -1;
        }
        int result = game_data->player_network_id[ units.unit[ unit_id ].owner_id ];
        units.unit[ unit_id ].unlock();
        return result;
    }


} // namespace TA3D

