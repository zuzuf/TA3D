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

#include "mapselector.h"
#include <misc/paths.h>
#include <algorithm>
#include <tnt.h>
#include <logs/logs.h>
#include <languages/i18n.h>
#include <misc/tdf.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <EngineClass.h>



namespace TA3D
{
namespace Menus
{

	//! \brief Predicate to sort a vector of string
	static inline bool sortForListOfMaps(const QString& u, const QString& v)
	{
		return u < v;
	}



	void MapSelector::SortListOfMaps(ListOfMaps& out)
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "sorting list of maps");
		std::sort(out.begin(), out.end(), sortForListOfMaps);
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "done");
	}






	bool MapSelector::Execute(const QString& preSelectedMap, QString& mapName)
	{
		MapSelector m(preSelectedMap);
		mapName.clear();
		if (m.execute())
			mapName = m.selected();
        return !mapName.isEmpty();
	}





	MapSelector::MapSelector()
		:Abstract(),
		pSelectedMap(), pDefaultSelectedMap(""), pCachedSizeOfListOfMaps(0),
		pMiniMapTexture(0),
		pLastMapIndex(-1), pMiniMapObj((Gui::GUIOBJ*)NULL), dx(0), dy(0),
		pMiniMapX1(0.0f), pMiniMapY1(0.0f), pMiniMapX2(0.0f), pMiniMapY2(0.0f)
	{}


	MapSelector::MapSelector(const QString& preSelectedMap)
		:Abstract(),
		pSelectedMap(), pDefaultSelectedMap(preSelectedMap), pCachedSizeOfListOfMaps(0),
		pMiniMapTexture(0),
		pLastMapIndex(-1), pMiniMapObj((Gui::GUIOBJ*)NULL), dx(0), dy(0),
		pMiniMapX1(0.0f), pMiniMapY1(0.0f), pMiniMapX2(0.0f), pMiniMapY2(0.0f)
	{}


	MapSelector::~MapSelector()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "reseting mini map texture");
		ResetTexture(pMiniMapTexture);
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "done");
	}


	bool MapSelector::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "Entering...");

		loadAreaFromTDF("map setup", "gui/mapsetup.area");
		// Load all maps, if any
		if (!preloadAllAvailableMaps()) // should abort if no map is present
			return false;

		// Texture for the mini map
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "reseting mini map texture");
		ResetTexture(pMiniMapTexture);
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "done");

		pSelectedMap = pDefaultSelectedMap;

		pLastMapIndex = -1;
		dx = 0;
		dy = 0;
		pMiniMapX1 = 0.0f;
		pMiniMapY1 = 0.0f;
		pMiniMapX2 = 0.0f;
		pMiniMapY2 = 0.0f;
		pMiniMapMiddleX = 0.0f;
		pMiniMapMiddleY = 0.0f;

		// The mini map object
		pMiniMapObj = pArea->get_object("mapsetup.minimap");
		if (!pMiniMapObj)
			LOG_ERROR(LOG_PREFIX_MENU_MAPSELECTOR << "Can not find the GUI object `mapsetup.minimap`");
		else
		{
			pMiniMapObj->Data = 0;
			// Save previous values for the coordinate of the mini map
			pMiniMapX1 = pMiniMapObj->x1;
			pMiniMapY1 = pMiniMapObj->y1;
			pMiniMapX2 = pMiniMapObj->x2;
			pMiniMapY2 = pMiniMapObj->y2;
			// Scaled size
			pMiniMapMiddleX = (pMiniMapX1 + pMiniMapX2) * 0.5f;
			pMiniMapMiddleY = (pMiniMapY1 + pMiniMapY2) * 0.5f;
			// Re positionate the object
			scaleAndRePosTheMiniMap();
		}

		// The control which contains all available maps
		pMapListObj = pArea->get_object("mapsetup.map_list");
		reloadMapsForGUIControl();
		// Select the default map
		autoSelectMap(pDefaultSelectedMap);

		return true;
	}

	void MapSelector::reloadMapsForGUIControl()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "reloadMapsForGUIControl()");
		if (pMapListObj)
		{
			// Load all maps
            pMapListObj->Text.clear();
            pMapListObj->Text.reserve(pCachedSizeOfListOfMaps);
            for (const QString &i : pListOfMaps)
                pMapListObj->Text.push_back(i);
		}
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "done");
	}


	void MapSelector::autoSelectMap(const QString& shortName)
	{
		if (!pMapListObj)
			return;
		const QString::size_type l = shortName.length();
		if (l < 9)
			return;
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "autoSelectMap()");
		const QString& s = Substr(shortName, 5, l - 9);
		int indx(0);
		const ListOfMaps::const_iterator end = pListOfMaps.end();
		for (ListOfMaps::const_iterator i = pListOfMaps.begin(); i != end; ++i, ++indx)
		{
			if (s == *i)
			{
				pMapListObj->Pos = indx;
				pMapListObj->Data = indx;
				doGoSelectSingleMap(indx);
				return;
			}
		}
		// Default value
		pMapListObj->Pos = 0;
		pMapListObj->Data = 0;
		doGoSelectSingleMap(0);
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "done");
	}


	void MapSelector::scaleAndRePosTheMiniMap(const float coef /* = 504.0f */)
	{
		LOG_ASSERT(!Yuni::Math::Zero(coef)); // Division by zero

		if (pMiniMapObj)
		{
			const float ldx = float(dx) * (pMiniMapX2 - pMiniMapX1) / coef;
			const float ldy = float(dy) * (pMiniMapY2 - pMiniMapY1) / coef;
			pMiniMapObj->x1 = pMiniMapMiddleX - ldx;
			pMiniMapObj->y1 = pMiniMapMiddleY - ldy;
			pMiniMapObj->x2 = pMiniMapMiddleX + ldx;
			pMiniMapObj->y2 = pMiniMapMiddleY + ldy;
			pMiniMapObj->u2 = float(dx) / 252.0f;
			pMiniMapObj->v2 = float(dy) / 252.0f;
		}
	}


	void MapSelector::doFinalize()
	{
        if (!pSelectedMap.isEmpty())
			LOG_INFO(LOG_PREFIX_MENU_MAPSELECTOR << "The map `" << pSelectedMap << "` has been selected.");
		LOG_DEBUG(LOG_PREFIX_MENU_MAPSELECTOR << "Done.");
	}


	void MapSelector::waitForEvent()
	{
		if (pArea->get_object("mapsetup.map_list") && !pArea->get_object("mapsetup.map_list")->Focus)
			pArea->msg("mapsetup.map_list.focus");
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


	bool MapSelector::maySwitchToAnotherMenu()
	{
		// Aborting
		if (pArea->get_state("mapsetup.b_cancel") || key[KEY_ESC])
		{
			pSelectedMap = pDefaultSelectedMap;
			return true;
		}
		// Go ! Select the map !
		if (pArea->get_state("mapsetup.b_ok") || key[KEY_ENTER])
			return true;
		// Selection change
		if (pMapListObj)
			return doGoSelectSingleMap(pMapListObj->Pos);
		return false;
	}


	bool MapSelector::doGoSelectSingleMap(const int mapIndex)
	{
		// Bounds checking
		if (pLastMapIndex == mapIndex || mapIndex < 0 || mapIndex >= pCachedSizeOfListOfMaps)
			return false;

		// Cached value
		pLastMapIndex = mapIndex;

		// The new map name
        pSelectedMap = "maps/" + pListOfMaps[mapIndex] + ".tnt";
		// Reload the mini map
		ResetTexture(pMiniMapTexture, load_tnt_minimap_fast(pSelectedMap, dx, dy));

		// OTA
        const QString &otaMap = "maps/" + pListOfMaps[mapIndex] + ".ota";
		MAP_OTA mapOTA;
        if (QIODevice* file = VFS::Instance()->readFile(otaMap))
		{
			mapOTA.load(file);
			delete file;;
		}

		// Update the mini map
		doUpdateMiniMap();
		// Map Info
		doResetAreaCaptionFromMapOTA(mapOTA);

		return false;
	}


	void MapSelector::doUpdateMiniMap()
	{
		if (pMiniMapObj)
		{
			// Swap textures
			ResetTexture(pMiniMapObj->Data, pMiniMapTexture);
			pMiniMapTexture = 0;

			// Resizing
			scaleAndRePosTheMiniMap();
		}
	}

	void MapSelector::doResetAreaCaptionFromMapOTA(MAP_OTA& mapOTA)
	{
		QString title;

		// Name of the mission
        if(!mapOTA.missionname.isEmpty())
            title = mapOTA.missionname + "\n";
		// Maximum allowed players for this map
        if(!mapOTA.numplayers.isEmpty())
            title += "\n" + I18N::Translate("players: ") + mapOTA.numplayers + "\n";
		// Description
        if(!mapOTA.missiondescription.isEmpty())
            title += "\n" + mapOTA.missiondescription;

		// Change the caption
        pArea->caption("mapsetup.map_info", title);
	}


	bool MapSelector::MapIsForNetworkGame(const QString& mapShortName)
	{
        QIODevice* file = VFS::Instance()->readFileRange("maps/" + mapShortName + ".ota", 0, 10240);
		if (file)
		{
			TDFParser ota_parser;
            const QByteArray &buffer = file->read(10240);
            ota_parser.loadFromMemory(mapShortName, (const char*)buffer.data(), buffer.size(), false, false, false);
            const QString &tmp = ota_parser.pullAsString("GlobalHeader.Schema 0.Type").toLower();
			delete file;
			return tmp.startsWith("network");
		}
		return false;
	}


	void MapSelector::GetMultiPlayerMapList(ListOfMaps& out)
	{
		// Clear the map
		out.clear();

		// Load all available maps, without any distinction
		ListOfMaps allMaps;
        if (VFS::Instance()->getFilelist("maps/*.tnt", allMaps) > 0)
		{
			for (ListOfMaps::const_iterator it = allMaps.begin(); it != allMaps.end(); ++it)
			{
				const QString::size_type l(it->length());
				if (l < 9)
					continue;
				const QString& newMapName = Substr(*it, 5, l - 9);
				if (MapSelector::MapIsForNetworkGame(newMapName))
					out.push_back(newMapName);
			}
			SortListOfMaps(out);
		}
	}


	bool MapSelector::preloadAllAvailableMaps()
	{
		GetMultiPlayerMapList(pListOfMaps);
		pCachedSizeOfListOfMaps = int(pListOfMaps.size());
		if (!pCachedSizeOfListOfMaps)
		{
			pArea->popup(I18N::Translate("Error"), I18N::Translate("No map found"));
			LOG_ERROR(LOG_PREFIX_MENU_MAPSELECTOR << "No maps have been found.");
			return false;
		}
		return true;
	}





} // namespace Menus
} // namespace TA3D

