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
#include "../../stdafx.h"
#include "skin.manager.h"

namespace TA3D
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
        hash_skin.emptyHashTable();
        hash_skin.initTable(__DEFAULT_HASH_TABLE_SIZE);
    }

    void SKIN_MANAGER::destroy()
    {
        for(std::vector<SKIN*>::iterator it = skins.begin() ; it != skins.end() ; ++it)
            delete *it;
        init();
    }

    SKIN *SKIN_MANAGER::load(const String& filename, const float scale)
    {
        String key(filename);
		key += String::Format("-%.2f", scale);
        SKIN *pSkin = hash_skin.find(key);
        if (pSkin)
            return pSkin;
        pSkin = new SKIN();
        pSkin->load_tdf(filename, scale);
        hash_skin.insert(key, pSkin);
        skins.push_back(pSkin);

        return pSkin;
    }
}
