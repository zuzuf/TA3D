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

#include "multimenu.h"
#include "netmenu.h"
#include "networkroom.h"
#include <languages/i18n.h>
#include <input/keyboard.h>
#include <input/mouse.h>

using namespace TA3D::VARS;



namespace TA3D
{
namespace Menus
{

	bool MultiMenu::Execute()
	{
		MultiMenu m;
		return m.execute();
	}

	MultiMenu::MultiMenu()
		:Abstract()
	{}


	MultiMenu::~MultiMenu()
	{
	}


	void MultiMenu::doFinalize()
	{
		// Wait for user to release ESC
		while (key[KEY_ESC])
		{
            rest(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		clear_keybuf();
	}


	bool MultiMenu::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MULTIMENU << "Entering...");

		// Loading the area
		loadAreaFromTDF("MultiMenu", "gui/multimenu.area");

		return true;
	}



	void MultiMenu::waitForEvent()
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
				 && !pArea->key_pressed && !pArea->scrolling);
	}


	bool MultiMenu::maySwitchToAnotherMenu()
	{
		if (pArea->get_state("multimenu.b_netserver"))
		{
			Menus::NetMenu::Execute();
			return true;
		}

		if (pArea->get_state("multimenu.b_lan"))
		{
			Menus::NetworkRoom::Execute();
			return true;
		}

		// Exit
		if (key[KEY_ESC] || pArea->get_state("multimenu.b_back"))
			return true;

		return false;
	}
}
}
