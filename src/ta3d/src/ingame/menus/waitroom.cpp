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

#include "waitroom.h"
#include <input/keyboard.h>
#include <input/mouse.h>
#include <ingame/players.h>
#include <fbi.h>

namespace TA3D
{
namespace Menus
{

	bool WaitRoom::Execute(GameData *game_data)
	{
		WaitRoom m(game_data);
		return m.execute();
	}

	WaitRoom::WaitRoom(GameData *game_data)
		:Abstract(), game_data(game_data)
	{}


	WaitRoom::~WaitRoom()
	{}


	void WaitRoom::doFinalize()
	{
		// Wait for user to release ESC
		reset_mouse();
		while (key[KEY_ESC])
		{
			SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		clear_keybuf();
	}


	bool WaitRoom::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MULTIMENU << "Entering...");

		// Loading the area
		loadAreaFromTDF("wait", "gui/wait.area");

		gfx->set_2D_mode();
		gfx->ReInitTexSys();

		if (!network_manager.isConnected())
			return false;

		lp_CONFIG->timefactor = 1.0f;		// Start at normal game speed (this forces all players to start at the same speed)

		for (int i = game_data->nb_players; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
		{
			dead_player[i] = true;
			pArea->msg(QString("wait.name") << i << ".hide");
			pArea->msg(QString("wait.progress") << i << ".hide");
			pArea->msg(QString("wait.ready") << i << ".hide");
		}

		for (int i = 0; i < game_data->nb_players; ++i)
		{
			dead_player[i] = false;
			if ((game_data->player_control[i] & PLAYER_CONTROL_FLAG_AI) || game_data->player_control[i] == PLAYER_CONTROL_LOCAL_HUMAN )
			{
				pArea->set_data( QString("wait.progress") << i, 100);
				pArea->set_state( QString("wait.ready") << i, true);
			}
			else
				pArea->set_state( QString("wait.ready") << i, false);
			pArea->caption(QString("wait.name") << i, game_data->player_names[i]);
		}

		if (network_manager.isServer())
		{                                   // We can only use units available on all clients
			for (int i = 0; i < unit_manager.nb_unit; ++i)
			{
				if (!unit_manager.unit_type[i]->not_used)           // Small check to ensure useonly file has an effect :)
					network_manager.sendAll(QString("USING ") << unit_manager.unit_type[i]->Unitname);
			}
			network_manager.sendAll("END USING");           // Ok we've finished sending the available unit list
			network_manager.sendAll("READY");               // Only server can tell he is ready before entering main loop
		}
		else
			for (int i = 0; i < unit_manager.nb_unit; ++i)      // Clients disable all units and wait for server to enable the ones we're going to use
				unit_manager.unit_type[i]->not_used = true;

		ping_timer = msec_timer;                    // Used to send simple PING requests in order to detect when a connection fails

		return true;
	}



	void WaitRoom::waitForEvent()
	{
		special_msg.clear();
		playerDropped = false;
		do
		{
			playerDropped = network_manager.getPlayerDropped();
			if (network_manager.getNextSpecial( &received_special_msg ) == 0)
				special_msg = (char*)received_special_msg.message;
			if (!network_manager.isConnected()) // We're disconnected !!
				break;

			// Grab user events
			pArea->check();

			// Wait to reduce CPU consumption
			wait();

		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
				 && mouse_b == 0
				 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
				 && !pArea->key_pressed && !pArea->scrolling && special_msg.empty() && !playerDropped
				 && ( msec_timer - ping_timer < 2000 || !network_manager.isServer() ));
	}


	bool WaitRoom::maySwitchToAnotherMenu()
	{
		//-------------------------------------------------------------- Network Code : syncing information --------------------------------------------------------------

		if (!network_manager.isConnected()) // We're disconnected !!
			return true;

		bool check_ready = false;

		if (network_manager.isServer() && msec_timer - ping_timer >= 2000) // Send a ping request
		{
			network_manager.sendPing();
			ping_timer = msec_timer;
			check_ready = true;

			if (network_manager.isServer())
			{
				for (int i = 0; i < game_data->nb_players; ++i) // Ping time out
				{
					if (game_data->player_network_id[i] > 0 && network_manager.getPingForPlayer(game_data->player_network_id[i]) > 10000)
						network_manager.dropPlayer(game_data->player_network_id[i]);
				}
			}
		}

		if (playerDropped)
		{
			for (int i = 0 ; i < game_data->nb_players ; i++)
				if (game_data->player_network_id[i] > 0 && !network_manager.pollPlayer(game_data->player_network_id[i]))     // A player is disconnected
				{
					dead_player[i] = true;
					pArea->msg(QString("wait.name") << i << ".hide");
					pArea->msg(QString("wait.progress") << i << ".hide");
					pArea->msg(QString("wait.ready") << i << ".hide");
				}
		}

		while (!special_msg.empty())    // Special receiver (sync config data)
		{
			const int from = received_special_msg.from;
			const int player_id = game_data->net2id(from);
			QStringList params;
			QString((char*)received_special_msg.message).explode(params, ' ');
			if (params.size() == 1)
			{
				if (params[0] == "NOT_READY")
					pArea->set_state(QString("wait.ready") << player_id, false);
				else
				{
					if (params[0] == "READY")
					{
						pArea->set_data(QString("wait.progress") << player_id, 100);
						pArea->set_state(QString("wait.ready") << player_id, true);
						check_ready = true;
					}
					else
					{
						if (params[0] == "START")
							return true;
					}
				}
			}
			else
			{
				if (params.size() == 2)
				{
					if (params[0] == "LOADING")
					{
						int percent = Math::Min(100, Math::Max(0, params[1].toInt()));
						pArea->set_data( QString("wait.progress") << player_id, percent);
					}
					else
					{
						if (params[0] == "USING")
						{                                   // We can only use units available on all clients, so check the list
							int type_id = unit_manager.get_unit_index(params[1]);
							if (type_id == -1)            // Tell it's missing
								network_manager.sendAll( QString("MISSING ") << params[1]);
							else
								unit_manager.unit_type[type_id]->not_used = false;  // Enable this unit
						}
						else
						{
							if (params[0] == "END" && params[1] == "USING")
								network_manager.sendAll("READY");
							else
								if (params[0] == "MISSING")
								{
									int idx = unit_manager.get_unit_index(params[1]);
									if (idx >= 0)
										unit_manager.unit_type[idx]->not_used = true;
								}
						}
					}
				}
			}

			if (network_manager.getNextSpecial(&received_special_msg) == 0)
				special_msg = received_special_msg.message;
			else
				special_msg.clear();
		}

		if (network_manager.isServer() && check_ready)// If server is late the game should begin once he is there
		{
			bool ready = true;
			for (short int i = 0; i < game_data->nb_players && ready; ++i)
			{
				if (!pArea->get_state(QString("wait.ready") << i) && !dead_player[i] && game_data->player_network_id[i] > 0)
					ready = false;
			}

			if (ready)
			{
				network_manager.sendAll("START");           // Tell everyone to start the game!!
				rest(1);
				return true;
			}
		}

		//-------------------------------------------------------------- End of Network Code --------------------------------------------------------------

		return false;
	}
}
}
