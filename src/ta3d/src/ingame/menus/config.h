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

#ifndef CONFIG_H
#define CONFIG_H

#include "base.h"
#include <languages/i18n.h>

namespace TA3D
{
	class TA3DCONFIG;
namespace Menus
{

	class Config : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of Config
		*/
		static bool Execute();
	public:
		Config();
		//! Destructor
		virtual ~Config();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		TA3DCONFIG *saved_config;
		std::vector< QString > fps_limits;
		int nb_res;
		int res_width[100];
		int res_height[100];
		int res_bpp[100];
		std::vector<I18N::Language> languageList;

		bool save;
		uint32 timer;
		bool time_out;
	};

} // namespace Menus
} // namespace TA3D

#endif // CONFIG_H
