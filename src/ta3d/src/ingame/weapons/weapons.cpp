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

/*-----------------------------------------------------------------------------------\
  |                                      weapons.cpp                                   |
  |   Ce module contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers tdf du jeu totalannihilation concernant les armes utilisées par les   |
  | unités du jeu.                                                                     |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "weapons.h"
#include <gfx/fx.h>
#include <ingame/sidedata.h>
#include <languages/i18n.h>

namespace TA3D
{


	void load_weapons(ProgressNotifier *progress)				// Charge toutes les armes
	{
		QStringList file_list;
        VFS::Instance()->getFilelist(ta3dSideData.weapon_dir + "*.tdf", file_list);

		int n = 0;

        for (const QString &cur_file : file_list)
		{
			if (progress != NULL && !(n & 0xF))
				(*progress)((250.0f + float(n) * 50.0f / float(file_list.size() + 1)) / 7.0f, I18N::Translate("Loading weapons"));
			++n;

            QIODevice *file = VFS::Instance()->readFile(cur_file);
			if (file)
			{
				weapon_manager.load_tdf(file);
				delete file;
			}
		}

        fx_manager.fx_data = VFS::Instance()->readFile("anims/fx.gaf");			// Load weapon animation data and stores it into a cache since it's often used
		if (fx_manager.fx_data)
		{
			weapon_manager.cannonshell.loadGAFFromRawData(fx_manager.fx_data, Gaf::RawDataGetEntryIndex(fx_manager.fx_data, "cannonshell"));
			weapon_manager.cannonshell.convert(false,true);
			weapon_manager.cannonshell.clean();
			// fx_data is deleted later in FXManager::destroy()
		}
	}



} // namesapce TA3D

