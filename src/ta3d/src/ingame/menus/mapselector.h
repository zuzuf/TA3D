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
#ifndef __TA3D_INGAME_MENUS_MAP_SELECTOR_H__
# define __TA3D_INGAME_MENUS_MAP_SELECTOR_H__

# include <stdafx.h>
# include <misc/string.h>
# include "base.h"
# include <vector>


namespace TA3D
{
	class MAP_OTA;
namespace Menus
{


	/*!
	** \brief
	*/
	class MapSelector : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of MapSelector
		**
		** \param preSelectedMap The name of the preselected map
		** \param[out] mapName Name of the selected map.
		** It is guaranteed this var will be empty if no map has been selected (returned value = false)
		**
		** \return True if a map has been selected by the user, false otherwise
		*/
		static bool Execute(const QString& preSelectedMap, QString& mapName);

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		MapSelector();
		/*!
		** \brief Constructor
		** \param PreSelected map
		*/
		MapSelector(const QString& preSelectedMap);
		//! Destructor
		virtual ~MapSelector();
		//@}


		/*!
		** \brief Get the selected map
		**
		** This value is affected by the last call to execute(), and is empty by default
		** or if the selection has been aborted.
		**
		** \return Name of the selected map, empty if none has been selected
		*/
		const QString& selected() const { return pSelectedMap; }


		/*!
		** \brief Get the map by default
		*/
		const QString& mapByDefault() const { return pDefaultSelectedMap; }
		/*!
		** \brief Set the map by default
		**
		** Nothing will be done until the next call to execute().
		*/
		void mapByDefault(const QString& m) { pDefaultSelectedMap = m; }

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		/*!
		** \brief List of maps
		** \see SortListOfMaps()
		*/
		typedef QStringList  ListOfMaps;

	private:
		/*!
		** \brief Sort a vector of string (ListOfMaps)
		** \param[out] out The sorted list of vectors using our own predicate
		** \see ListOfMaps
		*/
		static void SortListOfMaps(ListOfMaps& out);

		/*!
		** \brief Retrieve the list of maps designed for network gaming
		** \param[out] out The sorted list of available maps. It may be empty
		*/
		static void GetMultiPlayerMapList(ListOfMaps& out);

		/*!
		** \brief Get if a map is designed for network gaming
		** \param mapShortName Short name of hte map
		** \return True if the map has been found and is designed for network gaming, false otherwise
		*/
		static bool MapIsForNetworkGame(const QString& mapShortName);


	private:
		/*!
		** \brief Load all available maps
		** \return True if at least one map is available, false otherwise
		**
		** \see pListOfMaps
		*/
		bool preloadAllAvailableMaps();

		/*!
		** \brief Reload all available maps into the graphical control
		** \see pListOfMaps
		** \warning preloadAllAvailableMaps() must be called before this method
		*/
		void reloadMapsForGUIControl();

		/*!
		** \brief Auto select a map according its short name
		** \param shortName Name of the map. Nothing will be done if empty.
		*/
		void autoSelectMap(const QString& shortName);

		/*!
		** \brief Select a map according its index
		** \return Always equals to false
		** \see pSelectedMap
		*/
		bool doGoSelectSingleMap(const int mapIndex);

		/*!
		** \brief Reset the title of the local area according informations contained into a map OTA
		** \param mapOTA Map OTA which contains usefull information
		*/
		void doResetAreaCaptionFromMapOTA(MAP_OTA& mapOTA);

		/*!
		** \brief Update the mini map using the preloaded texturie (pMiniMapTexture)
		** \see pMiniMapTexture
		*/
		void doUpdateMiniMap();

		/*!
		** \brief Positionate the mini map using a given coefficient to scale it
		*/
		void scaleAndRePosTheMiniMap(const float coef = 504.0f);

	private:
		//! Name of the selected map
		QString pSelectedMap;
		//! The map by default
		QString pDefaultSelectedMap;

		/*!
		** \brief List of ordered maps designed for network gaming
		** \see preloadAllAvailableMaps()
		*/
		ListOfMaps pListOfMaps;
		//! Size of pListOfMaps
		int pCachedSizeOfListOfMaps;

		//! Texture for the mini map
		GLuint pMiniMapTexture;

		//! Last index selected in the list of maps (-1 means no map)
		int pLastMapIndex;

		//! Reference to the mini map object (Gui)
		Gui::GUIOBJ::Ptr pMiniMapObj;

		//! The list of map (Gui)
		Gui::GUIOBJ::Ptr pMapListObj;

		//! Cached value for the width of the mini map
		int dx;
		//! Cached value for the height of the mini map
		int dy;
		//! Previous x1-coordinate of the mini map object
		float pMiniMapX1;
		//! Previous y1-coordinate of the mini map object
		float pMiniMapY1;
		//! Previous x2-coordinate of the mini map object
		float pMiniMapX2;
		//! Previous y2-coordinate of the mini map object
		float pMiniMapY2;
		//! Middle X
		float pMiniMapMiddleX;
		//! Middle Y
		float pMiniMapMiddleY;

	}; // class MapSelector




} // namespace Menus
} // namespace TA3D



#endif // __TA3D_INGAME_MENUS_MAP_SELECTOR_H__
