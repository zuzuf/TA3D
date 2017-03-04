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

#ifndef SETUPGAME_H
#define SETUPGAME_H

#include "base.h"
#include <ingame/gamedata.h>
#include <network/network.h>

namespace TA3D
{
	class MAP_OTA;
namespace Menus
{

	class SetupGame : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of SetupGame
		*/
		static bool Execute(const bool client = false, const QString& host = QString(), const QString &saved_game = QString(), const bool bNetServer = false, const bool instantStart = false);
	public:
		SetupGame(const bool client, const QString& host, const QString &saved_game, const bool bNetServer, const bool instantStart);
		//! Destructor
		virtual ~SetupGame();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();
		bool checkNetworkMessages();

	private:
		const bool client;
		const QString& host;
		QString saved_game;
		const bool bNetServer;
		const bool instantStart;

		int my_player_id;
		QString status;
		uint16  player_str_n;
		QString  player_str[4];
		byte    player_control[4];
		QStringList  side_str;
		QStringList  AI_list;
		GameData game_data;

		int net_id_table[10];           // Table used to identify players joining a multiplayer saved game
		float ldx;
		float ldy;
        GfxTexture::Ptr glimg;
		MAP_OTA *map_data;


		//! minimap stuffs
		Gui::GUIOBJ::Ptr minimap_obj;
		float mini_map_x1;
		float mini_map_y1;
		float mini_map_x2;
		float mini_map_y2;
		float mini_map_x;
		float mini_map_y;


		bool start_game;

		QString set_map;
		QString previous_tnt_port;
		QString previous_ota_port;
		QString previous_lua_port;

		int progress_timer;
		int ping_timer;                    // Used to send simple PING requests in order to detect when a connection fails

		bool statusUpdateRequired;

		//! Network stuffs
		QString broadcast_msg;
		QString chat_msg;
		QString special_msg;
		struct chat received_chat_msg;
		struct chat received_special_msg;
		bool playerDropped;
	};
} // namespace Menus
} // namespace TA3D

#endif // SETUPGAME_H
