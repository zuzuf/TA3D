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
    
#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "TA3D_Network.h"
#include "EngineClass.h"
#include "UnitEngine.h"

#define CHAT_MESSAGE_TIMEOUT	10000

TA3DNetwork *g_ta3d_network = NULL;

const float	tick_conv = 1.0f / TICKS_PER_SEC;

TA3DNetwork::TA3DNetwork( AREA *area, GAME_DATA *game_data ) : messages()
{
	enter = false;
	this->area = area;
	this->game_data = game_data;
	CreateCS();
}

TA3DNetwork::~TA3DNetwork()
{
	foreach( messages, i )	i->text.clear();
	messages.clear();
	DeleteCS();
}

void TA3DNetwork::check()
{
	if( !network_manager.isConnected() )	return;		// Only works in network mode
	if( key[KEY_ENTER] && !Console->activated() ) {
		if( !enter ) {
			if( area->get_state("chat") ) {		// Chat is visible, so hide it and send the message is not empty
				area->msg("chat.hide");
				String msg = area->get_caption("chat.msg");
				area->set_caption("chat.msg", "");
				
				if( !msg.empty() ) {		// If not empty send the message
					struct chat chat;
					strtochat( &chat, msg );
					network_manager.sendChat( &chat );

					int player_id = game_data->net2id( chat.from );
					if( player_id >= 0 ) {
						EnterCS();
						if( messages.size() > 10 )		// Prevent flooding the screen with chat messages
							messages.pop_front();
						msg = "<" + game_data->player_names[ player_id ] + "> " + msg;
						messages.push_back( NetworkMessage( msg, msec_timer ) );
						LeaveCS();
						}
					}
				}
			else {								// Chat is hidden, so show it
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
		area->msg("chat.focus");

	int n = 5;
	while( n-- ) {													// Chat receiver
		String chat_msg;
		struct chat received_chat_msg;

		if( network_manager.getNextChat( &received_chat_msg ) == 0 )
			chat_msg = received_chat_msg.message;
		else
			break;

		int player_id = game_data->net2id( received_chat_msg.from );
		if( player_id >= 0 ) {
			if( messages.size() > 10 )		// Prevent flooding the screen with chat messages
				messages.pop_front();
			chat_msg = "<" + game_data->player_names[ player_id ] + "> " + chat_msg;
			messages.push_back( NetworkMessage( chat_msg, msec_timer ) );
			}
		}

	foreach_( messages, i )
		if( msec_timer - i->timer >= CHAT_MESSAGE_TIMEOUT )
			messages.erase( i++ );
		else
			i++;

	n = 100;
	while( n-- ) {													// Special message receiver
		String special_msg;
		special received_special_msg;

		if( network_manager.getNextSpecial( &received_special_msg ) == 0 )
			special_msg = received_special_msg.message;
		else
			break;

		int player_id = game_data->net2id( received_special_msg.from );

		Vector<String> params = ReadVectorString( special_msg, " " );
		if( params.size() == 2 ) {
			if( params[0] == "TICK" ) {
				if( player_id >= 0 )
					units.client_tick[ player_id ] = atoi( params[1].c_str() );
				}
			}
		}

	n = 100;
	while( n-- ) {													// Special order receiver
		struct order order_msg;

		if( network_manager.getNextOrder( &order_msg ) )
			break;
		}

	n = 100;
	while( n-- ) {													// Special sync receiver
		struct sync sync_msg;

		if( network_manager.getNextSync( &sync_msg ) )
			break;

		if( units.current_tick - sync_msg.timestamp >= 10 )		continue;		// It's too old

		if( sync_msg.hp == 0 )	{		// It's a weapon
			if( sync_msg.unit < weapons.max_weapon ) {
				weapons.Lock();

				if( weapons.weapon[ sync_msg.unit ].weapon_id < 0 || weapons.weapon[ sync_msg.unit ].last_timestamp >= sync_msg.timestamp )	{
					weapons.UnLock();
					continue;
					}

				weapons.weapon[ sync_msg.unit ].last_timestamp = sync_msg.timestamp;

				weapons.weapon[ sync_msg.unit ].Pos.x = sync_msg.x;
				weapons.weapon[ sync_msg.unit ].Pos.y = sync_msg.y;
				weapons.weapon[ sync_msg.unit ].Pos.z = sync_msg.z;

				weapons.weapon[ sync_msg.unit ].V.x = sync_msg.vx;
				weapons.weapon[ sync_msg.unit ].V.z = sync_msg.vz;
				weapons.weapon[ sync_msg.unit ].V.y = sync_msg.vy;

					// Guess where the weapon should be now
				weapons.weapon[ sync_msg.unit ].Pos = weapons.weapon[ sync_msg.unit ].Pos + ((units.current_tick - sync_msg.timestamp) * tick_conv) * weapons.weapon[ sync_msg.unit ].V;

				weapons.UnLock();
				}
			}
		else if( sync_msg.unit < units.max_unit ) {
			units.unit[sync_msg.unit].Lock();
			if( !(units.unit[sync_msg.unit].flags & 1) || units.unit[sync_msg.unit].last_synctick[0] >= sync_msg.timestamp )	{
				units.unit[sync_msg.unit].UnLock();
				continue;
				}

			units.unit[sync_msg.unit].last_synctick[0] = sync_msg.timestamp;
			units.unit[sync_msg.unit].Pos.x = sync_msg.x;
			units.unit[sync_msg.unit].Pos.y = sync_msg.y;
			units.unit[sync_msg.unit].Pos.z = sync_msg.z;

			units.unit[sync_msg.unit].V.x = sync_msg.vx;
			units.unit[sync_msg.unit].V.z = sync_msg.vz;
			units.unit[sync_msg.unit].V.y = 0.0f;

				// Guess where the unit should be now
			units.unit[sync_msg.unit].Pos = units.unit[sync_msg.unit].Pos + ((units.current_tick - sync_msg.timestamp) * tick_conv) * units.unit[sync_msg.unit].V;

			units.unit[sync_msg.unit].Angle.y = sync_msg.orientation * 360.0f / 65536.0f;

			units.unit[sync_msg.unit].hp = sync_msg.hp;
			units.unit[sync_msg.unit].build_percent_left = sync_msg.build_percent_left / 2.55f;

			units.unit[sync_msg.unit].UnLock();
			
			struct event sync_event;
			sync_event.type = EVENT_UNIT_SYNCED;
			sync_event.opt1 = sync_msg.unit;
			sync_event.opt2 = network_manager.getMyID();
			sync_event.x = sync_msg.timestamp;
			
			network_manager.sendEventUDP( &sync_event );
			}
		}

	n = 100;
	while( n-- ) {													// Special sync receiver
		struct event event_msg;

		if( network_manager.getNextEvent( &event_msg ) )
			break;

		switch( event_msg.type )
		{
		case EVENT_UNIT_SYNCED:
			if( event_msg.opt1 < units.max_unit && ( units.unit[ event_msg.opt1 ].flags & 1 ) ) {
				units.unit[ event_msg.opt1 ].Lock();

				int player_id = game_data->net2id( event_msg.opt2 );

				if( player_id >= 0 )
					units.unit[ event_msg.opt1 ].last_synctick[player_id] = event_msg.x;

				units.unit[ event_msg.opt1 ].UnLock();
				}
			break;
		case EVENT_UNIT_DAMAGE:
			if( event_msg.opt1 < units.max_unit && ( units.unit[ event_msg.opt1 ].flags & 1 ) ) {
				units.unit[ event_msg.opt1 ].Lock();

				float damage = event_msg.opt2 / 16.0f;

				units.unit[ event_msg.opt1 ].hp -= damage;

				units.unit[ event_msg.opt1 ].flags &= 0xEF;		// This unit must explode if it has been damaged by a weapon even if it is being reclaimed
				if( units.unit[ event_msg.opt1 ].hp <= 0.0f )
					units.unit[ event_msg.opt1 ].severity = max( units.unit[ event_msg.opt1 ].severity, (int)damage );
				
				units.unit[ event_msg.opt1 ].UnLock();

				printf("(%d), received order to damage %d\n", units.current_tick, event_msg.opt1 );
				}
			break;
		case EVENT_WEAPON_CREATION:
			if( event_msg.opt1 < units.max_unit && ( units.unit[ event_msg.opt1 ].flags & 1 ) ) {
				VECTOR target_pos( event_msg.x / 32768.0f, event_msg.y / 32768.0f, event_msg.z / 32768.0f );
				VECTOR Dir( ((sint16*)event_msg.str)[6] / 16384.0f, ((sint16*)event_msg.str)[7] / 16384.0f, ((sint16*)event_msg.str)[8] / 16384.0f );
				VECTOR startpos( ((sint32*)event_msg.str)[0] / 32768.0f, ((sint32*)event_msg.str)[1] / 32678.0f, ((sint32*)event_msg.str)[2] / 32768.0f );
				
				units.unit[ event_msg.opt1 ].Lock();
				int w_idx = units.unit[ event_msg.opt1 ].shoot( event_msg.opt2, startpos, Dir, ((sint16*)event_msg.str)[9], target_pos );
				units.unit[ event_msg.opt1 ].UnLock();

				if( w_idx >= 0 ) {
					weapons.Lock();
					weapons.weapon[w_idx].local = false;
					weapons.UnLock();
					}

				printf("(%d), received order to shoot from %d\n", units.current_tick, event_msg.opt1 );
			}
			break;
		case EVENT_UNIT_SCRIPT:
			if( event_msg.opt1 < units.max_unit && (units.unit[ event_msg.opt1 ].flags & 1) )
				units.unit[ event_msg.opt1 ].launch_script( event_msg.x, event_msg.z, (int*)event_msg.str, event_msg.opt2 );
			break;
		case EVENT_UNIT_DEATH:
			{
				int e = -1;
				units.EnterCS_from_outside();

				for( int i = 0 ; i < units.max_unit ; i++ )
					if( units.idx_list[ i ] == event_msg.opt1 ) {
						e = i;
						break;
						}
						
				printf("(%d), received order to kill %d\n", units.current_tick, event_msg.opt1 );

				units.LeaveCS_from_outside();

				units.kill( event_msg.opt1, the_map, e );
			}
			break;
		case EVENT_UNIT_CREATION:
			{
				units.EnterCS_from_outside();

				int idx = unit_manager.get_unit_index( (char*)event_msg.str );
				if( idx >= 0 ) {
					VECTOR pos;
					pos.x = event_msg.x / 65536.0f;
					pos.z = event_msg.z / 65536.0f;
					pos.y = the_map->get_unit_h( pos.x, pos.z );
					UNIT *unit = (UNIT*)create_unit( idx, (event_msg.opt2 & 0xFF),pos,the_map,false);		// We don't want to send sync data for this ...
					if( unit ) {
						unit->Lock();
						printf("(%d), created unit (%s) idx = %d\n", units.current_tick, event_msg.str, unit->idx );

						if( event_msg.opt2 & 0x1000 ) {								// Created by a script, so give it 100% HP
							unit->hp = unit_manager.unit_type[ idx ].MaxDamage;
							unit->built = false;
							unit->build_percent_left = 0.0f;
							}
						else {
							unit->hp = 0.001f;
							unit->built = true;
							unit->build_percent_left = 100.0f;
							}
						unit->UnLock();
						}
					else
						Console->AddEntry("Error: cannot create unit of type %s", event_msg.str);
					}
				else
					Console->AddEntry("Error: cannot create unit, %s not found", event_msg.str);

				units.LeaveCS_from_outside();
			}
			break;
		};
		}
}

void TA3DNetwork::draw()
{
	if( !network_manager.isConnected() )	return;		// Only works in network mode
	
	EnterCS();
	if( !messages.empty() ) {
		float Y = SCREEN_H * 0.5f;
		foreach( messages, i ) {
			int color = 0xFFFFFFFF;
			if( (int)(msec_timer - i->timer) - CHAT_MESSAGE_TIMEOUT + 1000 >= 0 ) {
				color = makeacol( 0xFF, 0xFF, 0xFF, 255 - min( 255, ((int)(msec_timer - i->timer) - CHAT_MESSAGE_TIMEOUT + 1000) * 255 / 1000 ) );
				Y -= min( 1.0f, ((int)(msec_timer - i->timer) - CHAT_MESSAGE_TIMEOUT + 1000) * 0.001f ) * (gfx->TA_font.height() + Y - SCREEN_H * 0.5f);
				}
			gfx->print( gfx->TA_font, 136, Y, 0.0f, color, i->text );
			Y += gfx->TA_font.height();
			}
		}
	LeaveCS();
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
	event.opt2 = damage * 16.0f;
	network_manager.sendEvent( &event );
}

int TA3DNetwork::getNetworkID( int unit_id )
{
	if( unit_id < units.max_unit )	return -1;
	units.unit[ unit_id ].Lock();
	if( !(units.unit[ unit_id ].flags & 1) ) {
		units.unit[ unit_id ].UnLock();
		return -1;
		}
	int result = game_data->player_network_id[ units.unit[ unit_id ].owner_id ];
	units.unit[ unit_id ].UnLock();
	return result;
}

