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

#include "campaignmainmenu.h"
#include <input/keyboard.h>
#include <input/mouse.h>
#include <vfs/vfs.h>
#include <ingame/battle.h>
#include <ingame/sidedata.h>
#include "briefscreen.h"

namespace TA3D
{
namespace Menus
{

	bool CampaignMainMenu::Execute()
	{
		CampaignMainMenu m;
		return m.execute();
	}

	CampaignMainMenu::CampaignMainMenu()
		:Abstract()
	{}


	CampaignMainMenu::~CampaignMainMenu()
	{}


	void CampaignMainMenu::doFinalize()
	{
		// Wait for user to release ESC
		reset_mouse();
		while (key[KEY_ESC])
		{
			SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		clear_keybuf();

		if (pArea->get_object("campaign.logo"))
			pArea->get_object("campaign.logo")->Data = 0;

		pArea->destroy();

		campaign_parser = NULL;

		if (start_game) // Open the briefing screen and start playing the campaign
		{
			// The result of the last battle
			Battle::Result exitStatus = Battle::brUnknown;

			while (mission_id < nb_mission && (exitStatus = BriefScreen::Execute(campaign_name, mission_id)))
			{
				if (Battle::brVictory == exitStatus)
					++mission_id;
			}

			reset_mouse();
			while (key[KEY_ESC])
			{
				SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
				poll_inputs();
			}
			clear_keybuf();
		}
	}


	bool CampaignMainMenu::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MULTIMENU << "Entering...");

		// Loading the area
		loadAreaFromTDF("campaign", "gui/campaign.area");

		VFS::Instance()->getFilelist("camps\\*.tdf", campaign_list);
		campaign_list.sort();
		for (QStringList::iterator i = campaign_list.begin(); i != campaign_list.end(); ) // Removes sub directories entries
		{
			if (SearchQString(Substr(*i, 6, i->size() - 6), "/", true) != -1 || SearchQString(Substr(*i, 6, i->size() - 6), "\\", true ) != -1)
				campaign_list.erase(i++);
			else
				++i;
		}

		if (pArea->get_object("campaign.campaign_list") && campaign_list.size() > 0)
		{
			Gui::GUIOBJ::Ptr guiobj = pArea->get_object("campaign.campaign_list");
			guiobj->Text.clear();
			guiobj->Text.resize(campaign_list.size());
			int n = 0;
			for (QStringList::const_iterator i = campaign_list.begin(); i != campaign_list.end(); ++i, ++n)
				guiobj->Text[n] = Substr(*i, 6, i->size() - 10);
		}

		side_logos.loadGAFFromDirectory("anims\\newgame", true);
		if (side_logos.size() == 0)
		{
			File *file = VFS::Instance()->readFile( "anims\\newgame.gaf");
			side_logos.loadGAFFromRawData(file, true);
			delete file;
		}

		campaign_parser = NULL;

		start_game = false;

		last_campaign_id = uint32(-1);
		campaign_name.clear();
		mission_id = -1;
		nb_mission = 0;

		return true;
	}



	void CampaignMainMenu::waitForEvent()
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


	bool CampaignMainMenu::maySwitchToAnotherMenu()
	{
		if (pArea->get_object("campaign.campaign_list") && campaign_list.size() > 0) // If we don't have campaign data, then load them
		{
			Gui::GUIOBJ::Ptr guiobj = pArea->get_object("campaign.campaign_list");
			if (guiobj->Pos < guiobj->Text.size() && last_campaign_id != guiobj->Pos )
			{
				last_campaign_id = guiobj->Pos;
				mission_id = -1;
				campaign_name = QString("camps\\") << guiobj->Text[ guiobj->Pos ] << ".tdf";
				campaign_parser = new TDFParser( campaign_name);

				guiobj = pArea->get_object("campaign.mission_list");
				nb_mission = 0;
				if (guiobj)
				{
					guiobj->Text.clear();
					int i = 0;
					QString current_name;
					while (!(current_name = campaign_parser->pullAsString(QString("MISSION") << i << ".missionname")).empty())
					{
						guiobj->Text.push_back(current_name);
						++nb_mission;
						++i;
					}
				}

				guiobj = pArea->get_object("campaign.logo");
				if (guiobj)
				{
					guiobj->Data = 0;
					for (int i = 0 ; i < ta3dSideData.nb_side ; ++i)
					{
						if (ToLower(ta3dSideData.side_name[i] ) == ToLower(campaign_parser->pullAsString("HEADER.campaignside")))
						{
							if (side_logos.size() > i)
								guiobj->Data = side_logos[i].glbmp[0];
							break;
						}
					}
				}
			}
		}

		if (pArea->get_object("campaign.mission_list"))
		{
			Gui::GUIOBJ::Ptr guiobj = pArea->get_object("campaign.mission_list");
			if (guiobj->Pos < guiobj->Text.size())
				mission_id = guiobj->Pos;
		}

		if ((pArea->get_state( "campaign.b_ok") || key[KEY_ENTER]) && campaign_list.size())
		{
			start_game = true;
			return true;      // If user click "OK" or hit enter then leave the window
		}
		if (pArea->get_state( "campaign.b_cancel")) // Leave on Cancel
			return true;

		if (key[KEY_ESC]) // Leave menu on ESC
			return true;

		return false;
	}
}
}
