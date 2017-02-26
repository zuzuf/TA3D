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

#include "solo.h"
#include <list>
#include <languages/i18n.h>
#include <ingame/gamedata.h>
#include <restore.h>
#include <ta3dbase.h>
#include <misc/paths.h>
#include <ingame/battle.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include "campaignmainmenu.h"
#include "setupgame.h"



namespace TA3D
{
namespace Menus
{


	bool Solo::Execute()
	{
		Solo m;
		return m.execute();
	}



	Solo::Solo()
		:Abstract()
	{}

	Solo::~Solo()
	{}

	bool Solo::doInitialize()
	{
		loadAreaFromTDF("solo", "gui/solo.area");
		return true;
	}

	void Solo::doFinalize()
	{
		// Wait for user to release ESC
		while (key[KEY_ESC])
		{
            QThread::msleep(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
	}


	void Solo::waitForEvent()
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
			&& !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
			&& !keyIsPressed && !pArea->scrolling);
	}


	bool Solo::maySwitchToAnotherMenu()
	{
		// Exit
		if (key[KEY_ESC] || pArea->get_state("solo.b_back"))
			return true;

		// All savegames
		if (pArea->get_state("solo.b_load") && pArea->get_object("load_menu.l_file") )
			return doDisplayAllSavegames();

		// Load the selected savegame
		if (pArea->get_state("load_menu.b_load"))
			return doGoMenuLoadSingleGame();

		// Campaign
		if (pArea->get_state("solo.b_campaign") || key[KEY_C])
			return doGoMenuCompaign();

		// Skirmish
		if (pArea->get_state("solo.b_skirmish") || key[KEY_ENTER])
			return doGoMenuSkirmish();

		return false;
	}

	bool Solo::doGoMenuSkirmish()
	{
		glPushMatrix();
		SetupGame::Execute();
		glPopMatrix();
		return false;
	}

	bool Solo::doGoMenuCompaign()
	{
		glPushMatrix();
		CampaignMainMenu::Execute();
		glPopMatrix();
		return false;
	}

	bool Solo::doDisplayAllSavegames()
	{
		Gui::GUIOBJ::Ptr obj = pArea->get_object("load_menu.l_file");
		if (obj)
		{
			QStringList fileList;
            Paths::Glob(fileList, TA3D::Paths::Savegames + "*.sav");
			fileList.sort();
			obj->Text.clear();
			obj->Text.reserve(fileList.size());
            for (const QString &i : fileList)
				// Remove the Savegames path, leaving just the bare file names
                obj->Text.push_back(Paths::ExtractFileName(i));
		}
		else
			LOG_ERROR("Impossible to get an area object : `load_menu.l_file`");
		return false;
	}

	bool Solo::doGoMenuLoadSingleGame()
	{
		pArea->set_state("load_menu.b_load", false);
		Gui::GUIOBJ::Ptr guiObj = pArea->get_object("load_menu.l_file");
		if (!guiObj)
			LOG_ERROR("Impossible to get an area object : `load_menu.l_file`");
		else
		{
			if (guiObj->Pos < guiObj->Text.size())
			{
				GameData game_data;
                bool network = load_game_data(TA3D::Paths::Savegames + guiObj->Text[guiObj->Pos], &game_data);

                if (!game_data.saved_file.isEmpty() && !network)
				{
					gfx->unset_2D_mode();
					Battle::Execute(&game_data);
					gfx->set_2D_mode();
					gfx->ReInitTexSys();
				}
				else if (network)
				{
					pArea->caption("popup.msg", I18N::Translate("MULTI_SOLO_MISMATCH_ERROR"));
					pArea->title("popup", I18N::Translate("Error"));
					pArea->msg("popup.show");
				}
			}
		}
		return false;
	}





} // namespace Menus
} // namespace TA3D

