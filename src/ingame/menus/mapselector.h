#ifndef __TA3D_INGAME_MENUS_MAP_SELECTOR_H__
# define __TA3D_INGAME_MENUS_MAP_SELECTOR_H__

# include "base.h"
# include <vector>
# include "../../EngineClass.h"


namespace TA3D
{
namespace Menus
{


    /*! \class MapSelector
    **
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
        static bool Execute(const String& preSelectedMap, String& mapName);

    public:
        //! \name Constructor & Destructor
        //{
        //! Default constructor
        MapSelector();
        /*!
        ** \brief Constructor
        ** \param PreSelected map
        */
        MapSelector(const String& preSelectedMap);
        //! Destructor
        virtual ~MapSelector();
        //}


        /*!
        ** \brief Get the selected map
        **
        ** This value is affected by the last call to execute(), and is empty by default
        ** or if the selection has been aborted.
        **
        ** \return Name of the selected map, empty if none has been selected
        */
        const String& selected() const { return pSelectedMap; }


        //! \name Map by default
        //{
        /*!
        ** \brief Get the map by default
        */
        const String& mapByDefault() const { return pDefaultSelectedMap; }
        /*!
        ** \brief Set the map by default
        **
        ** Nothing will be done until the next call to execute().
        */
        void mapByDefault(const String& m) { pDefaultSelectedMap = m; }
        //}

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
        typedef std::vector<String>  ListOfMaps;

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
        static bool MapIsForNetworkGame(const String& mapShortName); 

        /*!
        ** \brief Destroy a texture if needed and reassign it
        ** \param textVar Reference to the texture to destroy (index)
        ** \param newValue The newValue to assign. `0` means NULL
        */
        static void ResetTexture(GLuint& textVar, const GLuint newValue = 0);

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
        void autoSelectMap(const String& shortName);

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
        String pSelectedMap;
        //! The map by default
        String pDefaultSelectedMap;

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
        GUIOBJ* pMiniMapObj;

        //! The list of map (Gui)
        GUIOBJ* pMapListObj;

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
