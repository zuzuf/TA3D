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

#ifndef CAMPAIGNMAINMENU_H
#define CAMPAIGNMAINMENU_H

#include "base.h"
#include <gaf.h>

namespace TA3D
{
namespace Menus
{

	class CampaignMainMenu : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of CampaignMainMenu
		*/
		static bool Execute();
	public:
		CampaignMainMenu();
		//! Destructor
		virtual ~CampaignMainMenu();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		QStringList campaign_list;
		TDFParser::Ptr campaign_parser;
		Gaf::AnimationList side_logos;

		bool start_game;
		uint32 last_campaign_id;
		QString  campaign_name;
		int mission_id;
		int nb_mission;
	};

} // namespace Menus
} // namespace TA3D

#endif // CAMPAIGNMAINMENU_H
