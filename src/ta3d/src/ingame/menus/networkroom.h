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

#ifndef NETWORKROOM_H
#define NETWORKROOM_H

#include "base.h"
#include <network/network.h>

namespace TA3D
{
namespace Menus
{

	class NetworkRoom : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of NetworkRoom
		*/
		static bool Execute();
	public:
		NetworkRoom();
		//! Destructor
		virtual ~NetworkRoom();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		int server_list_timer;

		std::list< SERVER_DATA >    servers;                    // the server list

		QString sel_index;
		QString o_sel;
		QString join_host;
	};

} // namespace Menus
} // namespace TA3D

#endif // NETWORKROOM_H
