#include "battle.h"
#include "../network/network.h"
#include "../logs/logs.h"
#include "../gfx/gfx.h"
#include "../TA3D_NameSpace.h"
#include "../languages/i18n.h"
#include "../intro.h"
#include "../3do.h"
#include "../tdf.h"
#include "weapons/weapons.h"
#include "../fbi.h"
#include "players.h"
#include "../UnitEngine.h"
#include "../sounds/manager.h"
#include "../gfx/fx.manager.h"
#include "../tnt.h"
#include "../misc/paths.h"
#include "../misc/files.h"


// Should be removed
#define TA3D_BASIC_ENGINE
#include "../ta3d.h"





namespace TA3D
{

    Battle::Result Battle::Execute(GameData* g)
    {
        Battle battle(g);
        return battle.execute();
    }


    Battle::Battle(GameData* g)
        :pResult(brUnknown), pGameData(g), pNetworkEnabled(false), pNetworkIsServer(false),
        map(NULL)
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Preparing a new battle...");
    }

    Battle::~Battle()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Releasing unused resources...");

        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for 3d models");
        model_manager.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for particle engine");
        particle_engine.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for ingame units");
        units.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for units");
        unit_manager.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for weapons");
        weapons.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for sound");
        sound_manager->stopMusic();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for fx");
        fx_manager.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for weapon manager");
        weapon_manager.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for features");
        feature_manager.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for ingame features");
        features.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for textures");
        texture_manager.destroy();
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for players...");
        players.destroy();

        // Reset the HPI manager
        delete HPIManager;
        HPIManager = new cHPIHandler();

        gfx->set_2D_mode();
        gfx->ReInitTexSys();
        LOG_INFO(LOG_PREFIX_BATTLE << "Done.");
    }



    bool Battle::loadFromGameData(GameData* g)
    {
        pResult = brUnknown;
        if (!g)
            return true;

        // Here we go
        uint64 startTime = msec_timer;

        if (!initPreflight(g))
            return false;
        if (!initTextures())
            return false;
        if (!init3DModels())
            return false;
        if (!initGraphicalFeatures())
            return false;
        if (!initWeapons())
            return false;
        if (!initUnits())
            return false;
        if (!initIntermediateCleanup())
            return false;
        if (!initEngine())
            return false;
        if (!initPlayers())
            return false;
        if (!initRestrictions())
            return false;
        if (!initGUI())
            return false;
        if (!initTheMap())
            return false;

        // The loading has finished
        loading(100.0f, I18N::Translate("Load finished"));
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading time: " << ((float)(msec_timer - startTime) * 0.001f) << " sec.");
        return true;
    }


    bool Battle::initPreflight(GameData* g)
    {
        gfx->unset_2D_mode();
        // The game data
        pGameData = g;
        // Network
        pNetworkEnabled = network_manager.isConnected();
        pNetworkIsServer = network_manager.isServer();

        // To have mouse sensibility undependent from the resolution
        gfx->SCREEN_W_TO_640 = 1.0f;
        gfx->SCREEN_H_TO_480 = 1.0f;
        // How many players ?
        expected_players = pGameData->nb_players;

        // The GUI
        pCurrentGUI.clear();
        updateCurrentGUICacheNames();

        // FPS
        fps.countSinceLastTime = 0;
        fps.average = 0;
        fps.lastTime = msec_timer;
        fps.toStr.clear();

        // Misc
        pMouseRectSelection.reset();
        pMouseSelecting = false;

        return true;
    }

    
    bool Battle::initTextures()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading textures...");
        loading(0.0f, I18N::Translate("Loading textures"));
        texture_manager.all_texture();
        return true;
    }
    
    bool Battle::init3DModels()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading 3D Models...");
        loading(100.0f / 7.0f, I18N::Translate("Loading 3D Models"));
        model_manager.init();
        model_manager.load_all(loading);
        model_manager.optimise_all();
        return true;
    }

    bool Battle::initGraphicalFeatures()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading graphical features...");
        loading(200.0f / 7.0f, I18N::Translate("Loading graphical features"));
        load_features(loading);
        feature_manager.clean();
        model_manager.compute_ids();
        return true;
    }

    bool Battle::initWeapons()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading weapons...");
        loading(250.0f / 7.0f, I18N::Translate("Loading weapons"));
        load_weapons(loading);
        weapons.init();
        return true;
    }

    bool Battle::initUnits()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading units...");
        loading(300.0f / 7.0f, I18N::Translate("Loading units"));
        load_all_units(loading);
        return true;
    }
    
    bool Battle::initIntermediateCleanup()
    {
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing unused memory");
        // loading(400.0f / 7.0f, I18N::Translate("Free unused memory"));
        texture_manager.destroy();
        return true;
    }
        
    bool Battle::initEngine()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Initializing the engine...");
        loading(500.0f / 7.0f, I18N::Translate("Initialising engine"));
        gfx->SetDefState();
        particle_engine.init();
        set_palette(pal);
        return true;
    }

    bool Battle::initPlayers()
    {
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Adding players...");
        players.init(); // Object containing data about players
        for (uint8 i = 0; i < pGameData->nb_players; i++)
        {
            players.add((char*)pGameData->player_names[i].c_str(), (char*)pGameData->player_sides[i].c_str(),
                        pGameData->player_control[i],
                        pGameData->energy[i], pGameData->metal[i],
                        pGameData->ai_level[i], pGameData->team[i]); // add a player
        }

        if (players.local_human_id >= 0)
        {
            String prefix;
            String intgaf;
            for (int i = 0; i < ta3dSideData.nb_side; ++i)
            {
                if (ta3dSideData.side_name[i] == pGameData->player_sides[players.local_human_id])
                {
                    prefix = ta3dSideData.side_pref[i];
                    intgaf = ta3dSideData.side_int[i];
                    break;
                }
            }
            if (!prefix.empty() && !intgaf.empty())
                unit_manager.load_panel_texture(prefix, intgaf);
        }
        units.init(true);
        return true;
    }

    bool Battle::initRestrictions()
    {
        if (!pGameData->use_only.empty()) 			// We are told not to use all units !!
        {
            LOG_DEBUG(LOG_PREFIX_BATTLE << "Loading restrictions...");
            cTAFileParser useonly_parser(pGameData->use_only, false, false, true); // In gadgets mode so we can read the special key :)
            for (int i = 0; i < unit_manager.nb_unit ; i++)
                unit_manager.unit_type[i]->not_used = true;
            String unit_name;
            int i = 0;
            while (!(unit_name = useonly_parser.pullAsString(String::Format("gadget%d", i))).empty())
            {
                int idx = unit_manager.get_unit_index( unit_name.c_str());
                if (idx >= 0)
                    unit_manager.unit_type[idx]->not_used = false;
                ++i;
            }
        }
        return true;
    }

    bool Battle::initGUI()
    { 
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading the GUI...");
        loading(550.0f / 7.0f, I18N::Translate("Loading GUI"));
        pArea.load_tdf("gui/game.area");

        try
        {
            pArea.load_window(ta3dSideData.guis_dir + ta3dSideData.side_pref[players.side_view] + "gen.gui"); // Load the order interface
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(String(ta3dSideData.side_pref[players.side_view]) + "gen.hide").c_str(), NULL, NULL);	// Hide it
        }
        catch(...)
        {
            LOG_WARNING(LOG_PREFIX_BATTLE << "`gen.gui` is missing or can not be loaded");
        }

        try
        {
            pArea.load_window(ta3dSideData.guis_dir + ta3dSideData.side_pref[players.side_view] + "dl.gui");			// Load the default build interface
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(String( ta3dSideData.side_pref[players.side_view]) + "dl.hide").c_str(), NULL, NULL);	// Hide it
        }
        catch(...)
        {
            LOG_WARNING(LOG_PREFIX_BATTLE << "`dl.gui` is missing or can not be loaded");
        }

        for (int i = 0; i < unit_manager.nb_unit; ++i)
        {
            if (!(i & 0xF))
                loading((550.0f + 50.0f * i / (unit_manager.nb_unit + 1)) / 7.0f, I18N::Translate("Loading GUI"));
            if (String::ToLower(unit_manager.unit_type[i]->side) == String::ToLower(ta3dSideData.side_name[players.side_view]))
            {
                int e(1);
                while (HPIManager->Exists(ta3dSideData.guis_dir + unit_manager.unit_type[i]->Unitname + String::Format("%d.gui", e)))
                {
                    pArea.load_window( ta3dSideData.guis_dir + unit_manager.unit_type[i]->Unitname + String::Format("%d.gui", e));			// Load the build interface
                    I_Msg(TA3D::TA3D_IM_GUI_MSG, (void*)( unit_manager.unit_type[i]->Unitname + String::Format("%d.hide", e)).c_str(), NULL, NULL);	// Hide it
                    ++e;
                }
            }
        }
        return true;
    }


    bool Battle::initTheMap()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading the map...");
        loading(600.0f / 7.0f, I18N::Translate("Loading the map"));
        LOG_DEBUG(LOG_PREFIX_BATTLE << "Extracting `" << pGameData->map_filename << "`...");

        byte* map_file = HPIManager->PullFromHPI(pGameData->map_filename);
        if (!map_file)
            return false;
        map.reset(load_tnt_map(map_file));
        delete[] map_file;

        LOG_INFO(LOG_PREFIX_BATTLE << "Loading details texture...");
        map->load_details_texture( "gfx/details.jpg");			// Load the details texture

        LOG_INFO(LOG_PREFIX_BATTLE << "Initialising the Fog Of War...");
        map->clear_FOW(pGameData->fog_of_war);

        units.map = map.get(); // Setup some useful information

        pGameData->map_filename = Paths::Files::ReplaceExtension(pGameData->map_filename, ".ota");

        LOG_DEBUG(LOG_PREFIX_BATTLE << "Extracting `" << pGameData->map_filename << "`...");
        uint32 ota_size(0);
        map_file = HPIManager->PullFromHPI(pGameData->map_filename, &ota_size);
        if (map_file)
        {
            LOG_INFO(LOG_PREFIX_BATTLE << "Loading map informations...");
            map->ota_data.load((char*)map_file,ota_size);

            if (map->ota_data.lavaworld) // make sure we'll draw lava and not water
            {
                gfx->destroy_texture(map->lava_map);

                BITMAP *tmp = create_bitmap_ex(32, 16, 16);
                clear_to_color(tmp, 0xFFFFFFFF);
                map->lava_map = gfx->make_texture(tmp);
                destroy_bitmap(tmp);
            }
            delete[] map_file;
        }
        pGameData->map_filename = Paths::Files::ReplaceExtension(pGameData->map_filename, "");

        // The sky
        pSkyData.reset(choose_a_sky(pGameData->map_filename, map->ota_data.planet));
        if (pSkyData.get() == NULL)
        {
            pSkyData.reset(new SKY_DATA());
            pSkyData->texture_name = "gfx/sky/sky.jpg";
        }
        pSkyIsSpherical = pSkyData->spherical;
        return true;
    }



    void Battle::updateCurrentGUICacheNames()
    {
        // Reset
        for (int i = 0; i < cgcEnd; ++i)
            pCurrentGUICache[i] = pCurrentGUI;
        // Each item
        pCurrentGUICache[cgcDot] << ".";
        pCurrentGUICache[cgcShow] << ".show";
        pCurrentGUICache[cgcHide] << ".hide";
    }


} // namespace TA3D
