
#include "mapselector.h"
#include "../../misc/paths.h"
#include <algorithm>
#include "../../tnt.h"
#include "../../logs/logs.h"

# define TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX  "[Map Selector] "



namespace TA3D
{
namespace Menus
{

    /*!
     * \brief Predicate to sort a vector of string
     */
    static bool sortForListOfMaps(const String& u, const String& v)
    {
           return u < v;
    }

    void MapSelector::SortListOfMaps(ListOfMaps& out)
    {
        std::sort(out.begin(), out.end(), sortForListOfMaps);
    }
    




    

    bool MapSelector::Execute(const String& preSelectedMap, String& mapName)
    {
        MapSelector m(preSelectedMap);
        bool r = m.execute();
        mapName = (r) ? mapName = m.selected() : "";
        return (r && (!mapName.empty()));
    }







    MapSelector::MapSelector()
        :Abstract(),
        pSelectedMap(), pDefaultSelectedMap(""), pCachedSizeOfListOfMaps(0),
        pMiniMapTexture(0),
        pLastMapIndex(-1), pMiniMapObj(NULL), dx(0), dy(0),
        pMiniMapX1(0.0f), pMiniMapY1(0.0f), pMiniMapX2(0.0f), pMiniMapY2(0.0f)
    {}


    MapSelector::MapSelector(const String& preSelectedMap)
        :Abstract(),
        pSelectedMap(), pDefaultSelectedMap(preSelectedMap), pCachedSizeOfListOfMaps(0),
        pMiniMapTexture(0),
        pLastMapIndex(-1), pMiniMapObj(NULL), dx(0), dy(0),
        pMiniMapX1(0.0f), pMiniMapY1(0.0f), pMiniMapX2(0.0f), pMiniMapY2(0.0f)
    {}


    MapSelector::~MapSelector()
    {
        ResetTexture(pMiniMapTexture);
    }
    

    void MapSelector::ResetTexture(GLuint& textVar, const GLuint newValue)
    {
        if (textVar)
        {
            // ensure the texture for the mini map is destroyed
            gfx->destroy_texture(textVar);
        }
        textVar = newValue;
    }


    bool MapSelector::doInitialize()
    {
        LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << "Entering...");

        loadAreaFromTDF("map setup", "gui/mapsetup.area");
        // Load all maps, if any
        if (!preloadAllAvailableMaps()) // should abort if no map is present
            return false;

        // Texture for the mini map
        ResetTexture(pMiniMapTexture);

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
            LOG_ERROR(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << "Can not find the GUI object `mapsetup.minimap`");
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
        if (pMapListObj)
        {
            // Load all maps
            pMapListObj->Text.resize(pCachedSizeOfListOfMaps);
            int indx(0);
            for (ListOfMaps::const_iterator i = pListOfMaps.begin(); i != pListOfMaps.end(); ++i, ++indx)
                pMapListObj->Text[indx] = *i;
        }
    }


    void MapSelector::autoSelectMap(const String& shortName)
    {
        if (!pMapListObj)
            return;
        const String::size_type l = shortName.length();
        if (l < 9)
            return;
        const String s(shortName.substr(5, l - 9));
        int indx(0);
        for (ListOfMaps::const_iterator i = pListOfMaps.begin(); i != pListOfMaps.end(); ++i, ++indx)
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
    }


    void MapSelector::scaleAndRePosTheMiniMap(const float coef /* = 504.0f */)
    {
        LOG_ASSERT(coef != 0.0f); // Division by zero

        if (pMiniMapObj)
        {
            float ldx = dx * (pMiniMapX2 - pMiniMapX1) / coef;
            float ldy = dy * (pMiniMapY2 - pMiniMapY1) / coef;
            pMiniMapObj->x1 = pMiniMapMiddleX - ldx;
            pMiniMapObj->y1 = pMiniMapMiddleY - ldy;
            pMiniMapObj->x2 = pMiniMapMiddleX + ldx;
            pMiniMapObj->y2 = pMiniMapMiddleY + ldy;
            pMiniMapObj->u2 = dx / 252.0f;
            pMiniMapObj->v2 = dy / 252.0f;
        }
    }

    void MapSelector::doFinalize()
    {
        if (!pSelectedMap.empty())
            LOG_INFO(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << "The map `" << pSelectedMap << "` has been selected.");
        LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << "Done.");
    }


    void MapSelector::waitForEvent()
    {
        bool keyIsPressed(false);
        do
        {
            // Get if a key was pressed
            keyIsPressed = keypressed();
            // Grab user events
            pArea->check();
            // Wait to reduce CPU consumption 
            rest(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);

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
        if (pLastMapIndex == mapIndex || mapIndex < 0 || mapIndex > pCachedSizeOfListOfMaps)
            return false;

        // Cached value
        pLastMapIndex = mapIndex;

        // Logs
        LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << "`" << pListOfMaps[mapIndex]
                  << "` has been selected (indx: " << mapIndex << ")");

        // The new map name
        pSelectedMap = "maps";
        pSelectedMap += '\\'; // Paths::Separator;
        pSelectedMap += pListOfMaps[mapIndex];
        pSelectedMap += ".tnt";
        // Reload the mini map
        ResetTexture(pMiniMapTexture, load_tnt_minimap_fast(pSelectedMap, dx, dy));

        // OTA
        String otaMap("maps");
        otaMap += '\\'; // Paths::Separator;
        otaMap += pListOfMaps[mapIndex];
        otaMap += ".ota";
        uint32 otaSize(0);
        MAP_OTA mapOTA;
        if (byte* data = HPIManager->PullFromHPI(otaMap, &otaSize))
        {
            mapOTA.load((char*)data, otaSize);
            free(data);
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
        String title;

        // Name of the mission
        if(mapOTA.missionname)
        {
            title += mapOTA.missionname;
            title += "\n";
        }
        // Maximum allowed players for this map
        if(mapOTA.numplayers)
        {
            title += "\n";
            title += TRANSLATE("players: ");
            title += mapOTA.numplayers;
            title += "\n";
        }
        // Description
        if(mapOTA.missiondescription)
        {
            title += "\n";
            title += mapOTA.missiondescription;
        }

        // Change the caption
        pArea->set_caption("mapsetup.map_info", title);
    }


    bool MapSelector::MapIsForNetworkGame(const String& mapShortName)
    {
        uint32 ota_size=0;
        byte* data = HPIManager->PullFromHPI(String("maps\\") + mapShortName + String(".ota"), &ota_size);
        if(data)
        {
            MAP_OTA	map_data;	// Using MAP_OTA because it's faster than cTAFileParser that fills a hash_table object
            map_data.load((char*)data, ota_size);
            bool isNetworkGame = map_data.network;
            free(data);
            map_data.destroy();
            return isNetworkGame;
        }
        return false;
    }


    void MapSelector::GetMultiPlayerMapList(ListOfMaps& out)
    {
        // Clear the map
        out.clear();

        // Load all available maps, without any distinction
        ListOfMaps allMaps;
        if (HPIManager->getFilelist("maps\\*.tnt", allMaps) > 0)
        {
            for (ListOfMaps::const_iterator it = allMaps.begin(); it != allMaps.end(); ++it)
            {
                const String::size_type l(it->length());
                if (l < 9)
                    continue;
                const String newMapName(it->substr(5, l - 9));
                if (MapSelector::MapIsForNetworkGame(newMapName))
                {
                    LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << " Found `" << newMapName << "`");
                    out.push_back(newMapName);
                }
            }
            SortListOfMaps(out);
        }
    } 


    bool MapSelector::preloadAllAvailableMaps()
    {
        GetMultiPlayerMapList(pListOfMaps);
        pCachedSizeOfListOfMaps = pListOfMaps.size();
        switch (pCachedSizeOfListOfMaps)
        {
            case 0:
                {
                    Popup(TRANSLATE("Error"), TRANSLATE("No map found"));
                    Console->AddEntry("No maps found!!");
                    LOG_ERROR(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << "No maps have been found.");
                    return false;
                }
            case 1: LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << pCachedSizeOfListOfMaps
                             << " map only has been found"); break;
            default: LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAP_SELECTOR_PREFIX << pCachedSizeOfListOfMaps
                              << " maps have been found");
        }
        return true;
    }


} // namespace Menus
} // namespace TA3D


