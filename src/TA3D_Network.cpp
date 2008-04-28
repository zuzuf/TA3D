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

#define CHAT_MESSAGE_TIMEOUT	10000

TA3DNetwork::TA3DNetwork( AREA *area, GAME_DATA *game_data ) : messages()
{
	enter = false;
	this->area = area;
	this->game_data = game_data;
}

TA3DNetwork::~TA3DNetwork()
{
	foreach( messages, i )	i->text.clear();
	messages.clear();
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
						if( messages.size() > 10 )		// Prevent flooding the screen with chat messages
							messages.pop_front();
						msg = "<" + game_data->player_names[ player_id ] + "> " + msg;
						messages.push_back( NetworkMessage( msg, msec_timer ) );
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

	String chat_msg;
	struct chat received_chat_msg;

	if( network_manager.getNextChat( &received_chat_msg ) == 0 )
		chat_msg = received_chat_msg.message;

	while( !chat_msg.empty() ) {													// Chat receiver
		int player_id = game_data->net2id( received_chat_msg.from );
		if( player_id >= 0 ) {
			if( messages.size() > 10 )		// Prevent flooding the screen with chat messages
				messages.pop_front();
			chat_msg = "<" + game_data->player_names[ player_id ] + "> " + chat_msg;
			messages.push_back( NetworkMessage( chat_msg, msec_timer ) );
			}

		if( network_manager.getNextChat( &received_chat_msg ) == 0 )
			chat_msg = received_chat_msg.message;
		else
			chat_msg = "";
		}

	foreach_( messages, i )
		if( msec_timer - i->timer >= CHAT_MESSAGE_TIMEOUT )
			messages.erase( i++ );
		else
			i++;
}

void TA3DNetwork::draw()
{
	if( !network_manager.isConnected() )	return;		// Only works in network mode
	
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
}
