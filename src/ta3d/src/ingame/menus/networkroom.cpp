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

#include "networkroom.h"
#include <input/keyboard.h>
#include <input/mouse.h>
#include <ingame/gamedata.h>
#include <languages/i18n.h>
#include <misc/paths.h>
#include <restore.h>
#include "setupgame.h"
#include <TA3D_NameSpace.h>

using namespace TA3D::VARS;

#define SERVER_LIST_REFRESH_DELAY   5000

namespace TA3D
{
	static inline QString FixBlank(const QString& s)
	{
		QString t(s);
		t.replace(' ', char(1));
		return t;
	}

	static inline QString UnfixBlank(const QString& s)
	{
		QString t(s);
		t.replace(char(1), ' ');
		return t;
	}
namespace Menus
{

	bool NetworkRoom::Execute()
	{
		NetworkRoom m;
		return m.execute();
	}

	NetworkRoom::NetworkRoom()
		:Abstract()
	{}


	NetworkRoom::~NetworkRoom()
	{}


	void NetworkRoom::doFinalize()
	{
		// Wait for user to release ESC
		reset_mouse();
		while (key[KEY_ESC])
		{
			SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		clear_keybuf();

		network_manager.Disconnect();

        if (!join_host.isEmpty()) // Join a game
		{
			SetupGame::Execute(true, join_host);
			gfx->set_2D_mode();
			gfx->ReInitTexSys();
		}
	}


	bool NetworkRoom::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MULTIMENU << "Entering...");

		// Loading the area
		loadAreaFromTDF("network game area", "gui/networkgame.area");

		network_manager.InitBroadcast(1234);      // broadcast mode

		server_list_timer = msec_timer - SERVER_LIST_REFRESH_DELAY;

		return true;
	}



	void NetworkRoom::waitForEvent()
	{
		do
		{
			// Grab user events
			pArea->check();

			// Wait to reduce CPU consumption
			wait();

		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
				 && mouse_b == 0
				 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
				 && !pArea->key_pressed && !pArea->scrolling
				 && !network_manager.BroadcastedMessages()
				 && msec_timer - server_list_timer < SERVER_LIST_REFRESH_DELAY);
	}


	bool NetworkRoom::maySwitchToAnotherMenu()
	{
		if (network_manager.BroadcastedMessages())
		{
			QString msg = network_manager.getNextBroadcastedMessage();
            while (!msg.isEmpty())
			{
                const QStringList params = msg.split(' ', QString::SkipEmptyParts);
				if (params.size() == 6 && params[0] == "PONG" && params[1] == "SERVER") // It looks like "PONG SERVER <name> <mod> <version> <nb open player slots>
				{
					QString name = UnfixBlank(params[2]);
					QString mod = UnfixBlank(params[3]);
					if (mod == ".")
						mod.clear();
					QString version = UnfixBlank(params[4]);
					QString host_address = network_manager.getLastMessageAddress();
					int nb_open = params[5].toInt(nullptr, 0);

					if (Substr(version, 0, 3) == Substr(TA3D_ENGINE_VERSION, 0, 3) && version.size() == QString(TA3D_ENGINE_VERSION).size() && mod == TA3D_CURRENT_MOD && nb_open != 0)
					{
						bool updated = false;
                        for (std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ; ++server_i )       // Update the list
						{
							if (server_i->name == name)
							{
								updated = true;
								server_i->timer = msec_timer;
								server_i->nb_open = nb_open;
								server_i->host = host_address;
								if (name == o_sel)
									o_sel += "_";
								break;
							}
						}
						if (!updated)
						{
							SERVER_DATA new_server;
							new_server.internet = false;
							new_server.name = name;
							new_server.timer = msec_timer;
							new_server.nb_open = nb_open;
							new_server.host = host_address;
							servers.push_back( new_server);
						}
					}
				}

				msg = network_manager.getNextBroadcastedMessage();
			}

			for (std::list< SERVER_DATA >::iterator server_i = servers.begin(); server_i != servers.end() ;) // Remove those who timeout
				if (!server_i->internet && msec_timer - server_i->timer >= 30000)
					servers.erase(server_i++);
				else
                    ++server_i;

			Gui::GUIOBJ::Ptr obj = pArea->get_object("networkgame.server_list");
			if (obj)
			{
                obj->Text.clear();
                obj->Text.reserve(servers.size());
				QStringList server_names;
                for (std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ; ++server_i )       // Remove those who timeout
					server_names.push_back( server_i->name);
                std::sort(server_names.begin(), server_names.end());
				// Remove those who timeout
                for (const QString &server_i : server_names)
                    obj->Text.push_back(server_i);
                if (obj->Text.isEmpty())
					obj->Text.push_back(I18N::Translate("No server found"));
			}
		}

		bool refresh_all = false;
		if (pArea->get_state("networkgame.b_refresh"))
		{
			refresh_all = true;
			servers.clear();
		}

		if (msec_timer - server_list_timer >= SERVER_LIST_REFRESH_DELAY || refresh_all) // Refresh server list
		{
			for( std::list< SERVER_DATA >::iterator server_i = servers.begin() ; server_i != servers.end() ;)      // Remove those who timeout
			{
				if (!server_i->internet && msec_timer - server_i->timer >= 30000)
					servers.erase( server_i++);
				else
					server_i++;
			}

			Gui::GUIOBJ::Ptr obj = pArea->get_object("networkgame.server_list");
			if (obj)
			{
                obj->Text.clear();
                obj->Text.reserve(servers.size());
				QStringList server_names;
                for (const SERVER_DATA &server_i : servers)       // Remove those who timeout
                    server_names.push_back( server_i.name);
                std::sort(server_names.begin(), server_names.end());
                for (const QString &server_i : server_names) // Remove those who timeout
                    obj->Text.push_back(server_i);
                if (obj->Text.isEmpty())
					obj->Text.push_back(I18N::Translate("No server found"));
			}

			server_list_timer = msec_timer;
			if (network_manager.broadcastMessage( "PING SERVER LIST" ) )
				printf("error : could not broadcast packet to refresh server list!!\n");
		}

		if (pArea->get_state( "networkgame.b_load"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("load_menu.l_file");
			if (obj)
			{
				QStringList fileList;
                Paths::Glob(fileList, TA3D::Paths::Savegames + "multiplayer/*.sav");
                std::sort(fileList.begin(), fileList.end());
				obj->Text.clear();
				obj->Text.reserve(fileList.size());
                for (const QString &i : fileList)
					// Remove the Savegames path, leaving just the bare file names
                    obj->Text.push_back(Paths::ExtractFileName(i));
			}
			else
				LOG_ERROR("Impossible to get an area object : `load_menu.l_file`");
		}

		if (pArea->get_state( "load_menu.b_load" ) || (key[KEY_ENTER] && pArea->get_state( "load_menu" )))    // Loading a game
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("load_menu.l_file");
			if (obj && obj->Pos < obj->Text.size())
			{
				GameData game_data;
				QString host = obj->Text[obj->Pos];
                bool network = load_game_data(TA3D::Paths::Savegames + "multiplayer/" + obj->Text[obj->Pos], &game_data);

                if (!game_data.saved_file.isEmpty() && network)
				{
					while (key[KEY_ENTER])
					{
						SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
						poll_inputs();
					}
					clear_keybuf();
					network_manager.Disconnect();

					SetupGame::Execute(false, host, game_data.saved_file);   // Host a game
					return true;
				}
			}
		}

		if (pArea->get_state( "hosting.b_ok" ) || ( key[KEY_ENTER] && pArea->get_state( "hosting")))
		{
			while (key[KEY_ENTER])
			{
				SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
			clear_keybuf();
			network_manager.Disconnect();
			QString host = pArea->caption( "hosting.t_hostname");

			SetupGame::Execute(false, host);   // Host a game
			return true;
		}

		if (pArea->get_state( "joinip.b_ok"))
		{
			join_host = pArea->caption( "joinip.t_hostname");
			return true;
		}

		if (pArea->get_state( "networkgame.b_join"))
		{
			while (key[KEY_ENTER])
			{
				SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
			clear_keybuf();
			std::list< SERVER_DATA >::iterator i_server = servers.begin();
			while (i_server != servers.end() && i_server->name != sel_index)
				++i_server;

			if (i_server != servers.end() )     // Server not found !!
				join_host = i_server->host;
			else
				join_host.clear();
			return true;      // If user click "OK" or hit enter then leave the window
		}

		if (pArea->get_state( "networkgame.b_cancel" ) || key[KEY_ESC])
		{
			while (key[KEY_ESC])
			{
				SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
			clear_keybuf();
			sel_index.clear();
			return true;      // If user click "Cancel" or hit ESC then leave the screen returning NULL
		}

		if (pArea->get_object("networkgame.server_list"))
		{
			Gui::GUIOBJ::Ptr obj = pArea->get_object("networkgame.server_list");
			sel_index = (obj->Pos < obj->Text.size()) ? obj->Text[ obj->Pos ] : QString("");

			if (sel_index != o_sel) // Update displayed server info
			{
				o_sel = sel_index;
				std::list< SERVER_DATA >::iterator i_server = servers.begin();
				while( i_server != servers.end() && i_server->name != sel_index )
					++i_server;

				if (i_server != servers.end())
				{
					pArea->caption("networkgame.server_name", i_server->name);
					pArea->caption("networkgame.host", i_server->host);
                    pArea->caption("networkgame.open_slots", QString::number(i_server->nb_open) );
				}
			}
		}

		return false;
	}
}
}
