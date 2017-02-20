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

#ifndef BRIEFSCREEN_H
#define BRIEFSCREEN_H

#include "base.h"
#include <ingame/battle.h>

namespace TA3D
{
namespace Menus
{

	class BriefScreen : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of BriefScreen
		*/
		static Battle::Result Execute(const QString &campaign_name, const int mission_id);
	public:
		BriefScreen(const QString &campaign_name, const int mission_id);
		//! Destructor
		virtual ~BriefScreen();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		const QString &campaign_name;
		const int mission_id;

		Battle::Result exit_mode;
		QString map_filename;
		TDFParser ota_parser;
		TDFParser brief_parser;

		bool start_game;
		int schema;

		//! Animation data
		Gaf::AnimationList planet_animation;
		float planet_frame;
		int pan_id;
		int rotate_id;
		float pan_x1;
		float pan_x2;
		int time_ref;
	};

} // namespace Menus
} // namespace TA3D

#endif // BRIEFSCREEN_H
