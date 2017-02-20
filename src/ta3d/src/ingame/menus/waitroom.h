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

#ifndef WAITROOM_H
#define WAITROOM_H

#include "base.h"
#include <network/network.h>

namespace TA3D
{
	class GameData;
namespace Menus
{

	class WaitRoom : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of WaitRoom
		*/
		static bool Execute(GameData *game_data);
	public:
		WaitRoom(GameData *game_data);
		//! Destructor
		virtual ~WaitRoom();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		GameData *game_data;
		bool dead_player[TA3D_PLAYERS_HARD_LIMIT];
		uint32 ping_timer;

		QString special_msg;
		struct chat received_special_msg;
		bool playerDropped;
	};

} // namespace Menus
} // namespace TA3D

#endif // WAITROOM_H
