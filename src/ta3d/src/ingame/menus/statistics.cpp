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

#include "statistics.h"
#include <ta3dbase.h>
#include <EngineClass.h>
#include <ingame/players.h>
#include <input/mouse.h>
#include <input/keyboard.h>



namespace TA3D
{
namespace Menus
{


	bool Statistics::Execute()
	{
		Statistics m;
		return m.execute();
	}



	Statistics::Statistics()
		:Abstract()
	{}

	Statistics::~Statistics()
	{}


	void Statistics::doUpdateObject(const QString& id, const short indx, const uint32 color, const QString& value)
	{
		QString nameID("statistics.");
		nameID << id << indx;
		pArea->caption(nameID, value);
		Gui::GUIOBJ::Ptr o = pArea->get_object(nameID);
		if (o)
			o->Data = color;
	}

	bool Statistics::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_STATS << "Entering...");
		gfx->set_2D_mode();
		gfx->ReInitTexSys();
		loadAreaFromTDF("statistics", "gui/statistics.area");

		// Statistics for All players
		for (short i = 0; i < short(players.count()); ++i)
		{
			uint32 color = gfx->makeintcol(player_color[3 * player_color_map[i]],
										   player_color[3 * player_color_map[i] + 1],
										   player_color[3 * player_color_map[i] + 2]);

			doUpdateObject("player", i, color, players.name[i]);
			doUpdateObject("side",   i, color, players.side[i]);
            doUpdateObject("losses", i, color, QString(players.losses[i]));
            doUpdateObject("kills",  i, color, QString(players.kills[i]));
            doUpdateObject("energy", i, color, QString((int)players.energy_total[i]));
            doUpdateObject("metal",  i, color, QString((int)players.metal_total[i]));
		}
		return true;
	}


	void Statistics::doFinalize()
	{
		// Do nothing
		LOG_DEBUG(LOG_PREFIX_MENU_STATS << "Done.");
	}


	void Statistics::waitForEvent()
	{
		bool keyIsPressed(false);
		do
		{
			// Grab user events
			pArea->check();
			// Get if a key was pressed
			keyIsPressed = pArea->key_pressed;
			// Wait to reduce CPU consumption
			wait();

		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
				 && mouse_b == 0
				 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE]
				 && !keyIsPressed && !pArea->scrolling);
	}


	bool Statistics::maySwitchToAnotherMenu()
	{
		// Exit
		return (key[KEY_SPACE] || key[KEY_ENTER] || key[KEY_ESC] || pArea->get_state("statistics.b_ok"));
	}




} // namespace Menus
} // namespace TA3D


