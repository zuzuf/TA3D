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
#include <stdafx.h>
#include "skin.manager.h"

namespace TA3D
{
namespace Gui
{



	SKIN_MANAGER skin_manager;



	SKIN_MANAGER::SKIN_MANAGER()
	{
		init();
	}

	SKIN_MANAGER::~SKIN_MANAGER()
	{
		destroy();
	}

	void SKIN_MANAGER::init()
	{
		skins.clear();
		hash_skin.clear();
	}

	void SKIN_MANAGER::destroy()
	{
		for (std::vector<Skin*>::iterator it = skins.begin() ; it != skins.end() ; ++it)
			delete *it;
		init();
	}

	Skin *SKIN_MANAGER::load(const QString& filename, const float scale)
	{
		QString key(filename);
        key += QString::asprintf("-%.2f", scale);
		Skin *pSkin = hash_skin[key];
		if (!pSkin)
		{
			pSkin = new Skin();
			pSkin->loadTDFFromFile(filename, scale);
			hash_skin[key] = pSkin;
			skins.push_back(pSkin);
		}
		return pSkin;
	}




} // namespace Gui
} // namespace TA3D

