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
#include "../menu.h"


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
        map(NULL), water_obj(NULL)
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Preparing a new battle...");
    }

    Battle::~Battle()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Releasing unused resources...");

		DELETEANDNIL(water_obj);

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

		// Network
		if (g_ta3d_network)
		{
			delete g_ta3d_network;
			g_ta3d_network = NULL;
		}
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
        uint64 timer[22];

        timer[0] = msec_timer;
        if (!initPreflight(g))
            return false;
        timer[1] = msec_timer;
        if (!initTextures())
            return false;
        timer[2] = msec_timer;
        if (!init3DModels())
            return false;
        timer[3] = msec_timer;
        if (!initGraphicalFeatures())
            return false;
        timer[4] = msec_timer;
        if (!initWeapons())
            return false;
        timer[5] = msec_timer;
        if (!initUnits())
            return false;
        timer[6] = msec_timer;
        if (!initIntermediateCleanup())
            return false;
        timer[7] = msec_timer;
        if (!initEngine())
            return false;
        timer[8] = msec_timer;
        if (!initPlayers())
            return false;
        timer[9] = msec_timer;
        if (!initRestrictions())
            return false;
        timer[10] = msec_timer;
        if (!initGUI())
            return false;
        timer[11] = msec_timer;
        if (!initTheMap())
            return false;
        timer[12] = msec_timer;
		if (!initTheSky())
			return false;
        timer[13] = msec_timer;
		if (!initTheSun())
			return false;
        timer[14] = msec_timer;
		if (!initAllTextures())
			return false;
        timer[15] = msec_timer;
		if (!initTheCamera())
			return false;
        timer[16] = msec_timer;
		if (!initTheWind())
			return false;
        timer[17] = msec_timer;
		if (!initTheFog())
			return false;
        timer[18] = msec_timer;
		if (!initParticules())
			return false;
        timer[19] = msec_timer;
		if (!initTheWater())
			return false;
        timer[20] = msec_timer;
		if (!initPostFlight())
			return false;
        timer[21] = msec_timer;

        // The loading has finished
        loading(100.0f, I18N::Translate("Load finished"));
        LOG_INFO(LOG_PREFIX_BATTLE << "Loading time: " << ((float)(msec_timer - startTime) * 0.001f) << " sec.");
#define TA3D_LOADING_STATS
#ifdef TA3D_LOADING_STATS
        LOG_INFO(LOG_PREFIX_BATTLE << "statistics:");
        const char *functionName[] = {  "initPreflight(g)",
                                        "initTextures()",
                                        "init3DModels()",
                                        "initGraphicalFeatures()",
                                        "initWeapons()",
                                        "initUnits()",
                                        "initIntermediateCleanup()",
                                        "initEngine()",
                                        "initPlayers()",
                                        "initRestrictions()",
                                        "initGUI()",
                                        "initTheMap()",
                                        "initTheSky()",
                                        "initTheSun()",
                                        "initAllTextures()",
                                        "initTheCamera()",
                                        "initTheWind()",
                                        "initTheFog()",
                                        "initParticules()",
                                        "initTheWater()",
                                        "initPostFlight()" };
        for(int i = 0 ; i < 20 ; i++)
            LOG_INFO(LOG_PREFIX_BATTLE << functionName[i] << " done in " << timer[i+1] - timer[i] << " msec.");
#endif
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
        loading(400.0f / 7.0f, I18N::Translate("Free unused memory"));
        texture_manager.destroy();
        return true;
    }

    bool Battle::initEngine()
    {
        LOG_INFO(LOG_PREFIX_BATTLE << "Initializing the engine...");
        loading(500.0f / 7.0f, I18N::Translate("Initialising engine"));
        gfx->SetDefState();
        particle_engine.init();
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
        TA3D::MAX_UNIT_PER_PLAYER = pGameData->max_unit_per_player;
        units.init(true);
        return true;
    }

    bool Battle::initRestrictions()
    {
        if (!pGameData->use_only.empty()) 			// We are told not to use all units !!
        {
            LOG_DEBUG(LOG_PREFIX_BATTLE << "Loading restrictions...");
            TDFParser useonly_parser(pGameData->use_only, false, false, true); // In gadgets mode so we can read the special key :)
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

        for(int i = 0 ; i < players.nb_player ; ++i)
        {
            GUIOBJ *obj = pArea.get_object( format("playerstats.team%d",i) );
            if (obj)
            {
                obj->current_state = TA3D::Math::Log2( players.team[i] );
                obj->Flag &= ~FLAG_HIDDEN;
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
        LOG_DEBUG(LOG_PREFIX_BATTLE << "`" << pGameData->map_filename << "` extracted");
        LOG_DEBUG(LOG_PREFIX_BATTLE << "loading map data ...");
        map.reset(load_tnt_map(map_file));
        LOG_DEBUG(LOG_PREFIX_BATTLE << "map data loaded");
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

                SDL_Surface *tmp = gfx->create_surface_ex(32, 16, 16);
                SDL_FillRect(tmp, NULL, 0xFFFFFFFF);
                map->lava_map = gfx->make_texture(tmp);
                SDL_FreeSurface(tmp);
            }
            delete[] map_file;
        }
        pGameData->map_filename = Paths::Files::ReplaceExtension(pGameData->map_filename, "");

        return true;
    }


	bool Battle::initTheSky()
	{
        // The sky
        pSkyData.reset(choose_a_sky(pGameData->map_filename, map->ota_data.planet));
        if (pSkyData.get() == NULL)
        {
            pSkyData.reset(new SKY_DATA());
            pSkyData->texture_name = "gfx/sky/sky.jpg";
        }
        pSkyIsSpherical = pSkyData->spherical;

		sky_obj.build(10, 400, pSkyData->full_sphere);
		sky_angle = pSkyData->rotation_offset;
		return true;
	}


	bool Battle::initTheSun()
	{
		pSun.Att = 0.0f;
		// Direction
		pSun.Dir.x = -1.0f;
		pSun.Dir.y = 2.0f;
		pSun.Dir.z = 1.0f;
		pSun.Dir.unit();
		// Lights
		pSun.LightAmbient[0]  = 0.25f;
		pSun.LightAmbient[1]  = 0.25f;
		pSun.LightAmbient[2]  = 0.25f;
		pSun.LightAmbient[3]  = 0.25f;
		pSun.LightDiffuse[0]  = 1.0f;
		pSun.LightDiffuse[1]  = 1.0f;
		pSun.LightDiffuse[2]  = 1.0f;
		pSun.LightDiffuse[3]  = 1.0f;
		pSun.LightSpecular[0] = 0.0f;
		pSun.LightSpecular[1] = 0.0f;
		pSun.LightSpecular[2] = 0.0f;
		pSun.LightSpecular[3] = 0.0f;
		// Direction
		pSun.Directionnal = true;
		return true;
	}


	bool Battle::initAllTextures()
	{
		sky = gfx->load_texture(pSkyData->texture_name, FILTER_TRILINEAR, NULL, NULL, false);
		glow = gfx->load_texture("gfx/glow.tga");
		freecam_on = gfx->load_texture("gfx/freecam_on.tga");
		freecam_off = gfx->load_texture("gfx/freecam_off.tga");
		arrow_texture = gfx->load_texture("gfx/arrow.tga");
		circle_texture = gfx->load_texture("gfx/circle.tga");
		water = 0;
		return true;
	}

	bool Battle::initTheCamera()
	{
		r1 = r2 = r3 = 0.0f;

		cam.reset();
		cam_target.reset();
        camera_zscroll =  -0.00001f;

		cam_target_mx = gfx->SCREEN_W_HALF;
        cam_target_my = gfx->SCREEN_H_HALF;
        cam_has_target = false;
        freecam = false;
        cam_def_timer = msec_timer;		// Just to see if the cam has been long enough at the default angle
        track_mode = -1;			// Tracking a unit ? negative value => no
        last_time_activated_track_mode = false;
        Camera::inGame = &cam;

        cam.rpos.x = cam.rpos.y = cam.rpos.z = 0.0f;
        cam.rpos.z += 150.0f;
        cam.rpos.y = lp_CONFIG->camera_def_h;
        cam.zfar = 500.0f;
        cam.setWidthFactor(gfx->width, gfx->height);

        r1 = -lp_CONFIG->camera_def_angle - 0.00001f;

		return true;
	}


	bool Battle::initTheWind()
	{
        wind_t = 0.0f; // To handle wind variations
        wind_change = false;
        if (map->ota_data.maxwindspeed != map->ota_data.minwindspeed)
            map->wind = (TA3D_RAND() % (map->ota_data.maxwindspeed - map->ota_data.minwindspeed)) + map->ota_data.minwindspeed;
		return true;
	}


	bool Battle::initTheFog()
	{
		FogD = 0.3f;
        FogNear = cam.zfar * 0.5f;
        FogColor[0] = 0.8f;
		FogColor[1] = 0.8f;
		FogColor[2] = 0.8f;
		FogColor[3] = 1.0f;

        memcpy(FogColor, pSkyData->FogColor, sizeof(float) * 4);

        FogMode = GL_LINEAR;
        glFogi(GL_FOG_MODE, FogMode);
        glFogfv(GL_FOG_COLOR, FogColor);
        glFogf(GL_FOG_DENSITY, FogD);
        glHint(GL_FOG_HINT, GL_NICEST);
        glFogf(GL_FOG_START, FogNear);
        glFogf(GL_FOG_END, cam.zfar);
		// Enable the OpenGL fog
        glEnable(GL_FOG);

		return true;
	}


	bool Battle::initParticules()
	{
        fire = particle_engine.addtex("gfx/fire.tga");
        build_part = particle_engine.addtex("gfx/part.tga");
        fx_manager.loadData();
		return true;
	}


	bool Battle::initTheWater()
	{
		water_obj = new WATER();
		water_obj->build(map->map_w, map->map_h, 1000);

		if (g_useProgram && g_useFBO && map->water && lp_CONFIG->water_quality >= 2)
		{
			glGenFramebuffersEXT(1, &water_FBO);

			if (2 == lp_CONFIG->water_quality)
				water_pass1_low.load("shaders/water_pass1_low.frag","shaders/water_pass1.vert");
			else
				water_pass1.load("shaders/water_pass1.frag","shaders/water_pass1.vert");
			water_pass2.load("shaders/water_pass2.frag","shaders/water_pass2.vert");
			if (2 == lp_CONFIG->water_quality)
				water_shader.load("shaders/water.frag","shaders/water.vert");
			else
				water_shader_reflec.load("shaders/water_reflec.frag","shaders/water.vert");

			if (5 == lp_CONFIG->water_quality)
			{
				water_simulator_shader.load("shaders/water_simulator.frag","shaders/water_simulator.vert");     // Compute variation speed
				water_simulator_shader2.load("shaders/water_simulator2.frag","shaders/water_simulator.vert");   // Compute variation
				water_simulator_shader3.load("shaders/water_simulator3.frag","shaders/water_simulator.vert");   // Copy to a normal RGB filtered texture (faster than filtering an RGB32F texture)
				water_simulator_shader4.load("shaders/water_simulator4.frag","shaders/water_simulator4.vert");  // Compute normals on screen
                water_simulator_reflec.load("shaders/water_sim_reflec.frag","shaders/water.vert");
			}

			gfx->set_texture_format(GL_RGBA8);

			SDL_Surface* tmp = gfx->create_surface_ex(32,512,512);

			// Water transparency
			transtex = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water reflection
			reflectex = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water transparency/reflection
			if (lp_CONFIG->water_quality >= 5)
                first_pass = gfx->create_texture_RGBA32F(512, 512, FILTER_NONE, false);
			else
                first_pass = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water transparency/reflection
			if (lp_CONFIG->water_quality >= 5)
                second_pass = gfx->create_texture_RGBA16F(512, 512, FILTER_LINEAR, false);
			else
                second_pass = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water transparency/reflection
			water_color = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water simulation data
			if (lp_CONFIG->water_quality >= 5)
			{
			    last_water_refresh = msec_timer;
                const int simulation_w = 256;
                const int simulation_h = 256;
			    water_sim2 = gfx->create_texture_RGBA16F(simulation_w, simulation_h, FILTER_LINEAR, false);
                float *data = new float[ simulation_w * simulation_h * 4 ];
                for (int y = 0 ; y < simulation_h ; ++y)
                    for (int x = 0 ; x < simulation_w ; ++x)
                    {
                        data[(y * simulation_w + x) * 4] = 0.0f;
                        data[(y * simulation_w + x) * 4 + 1] = 0.0f;
                        data[(y * simulation_w + x) * 4 + 2] = 0.0f;
                        data[(y * simulation_w + x) * 4 + 3] = 0.0f;
                    }
                for( int i = 0 ; i < 500 ; i++ )                    // Initialize it with multiscale data
                {
                    for( int y = 0 ; y < simulation_h ; y++ )
                        for( int x = 0 ; x < simulation_w ; x++ )
                            data[(y * simulation_w + x) * 4 + 1] += ((rand() % 2000) * 0.000001f - 0.001f) * 5.0f * sqrtf(500 - i);
                    for( int y = 0 ; y < simulation_h ; y++ )
                        for( int x = 0 ; x < simulation_w ; x++ )
                            data[(y * simulation_w + x) * 4 + 1] = (data[(y * simulation_w + x) * 4 + 1]
                                                                  + data[(y * simulation_w + ((x+1) % simulation_w)) * 4 + 1]
                                                                  + data[(((y+1) % simulation_h) * simulation_w + x) * 4 + 1]
                                                                  + data[(y * simulation_w + ((x + simulation_w - 1) % simulation_w)) * 4 + 1]
                                                                  + data[(((y + simulation_w - 1) % simulation_h) * simulation_w + x) * 4 + 1]) * 0.2f;
                }
                float sum = 0.0f;
                for( int y = 0 ; y < simulation_h ; y++ )
                    for( int x = 0 ; x < simulation_w ; x++ )
                        sum += data[(y * simulation_w + x) * 4 + 1];
                sum /= simulation_h * simulation_w;
                for( int y = 0 ; y < simulation_h ; y++ )
                    for( int x = 0 ; x < simulation_w ; x++ )
                        data[(y * simulation_w + x) * 4 + 1] -= sum;
                for (int y = 0 ; y < simulation_h ; ++y)
                    for (int x = 0 ; x < simulation_w ; ++x)
                        data[(y * simulation_w + x) * 4 + 3] = data[(y * simulation_w + x) * 4 + 1];
                water_sim = gfx->make_texture_RGBA32F(256,256,data,FILTER_NONE,false);
                delete[] data;

                //  Let's create the height map texture used to render progressive water effects using water depth
                int h_w = Math::Min( map->bloc_w_db, gfx->max_texture_size() );
                int h_h = Math::Min( map->bloc_h_db, gfx->max_texture_size() );
                data = new float[ h_w * h_h ];
                for(int y = 0 ; y < h_h ; y++)
                    for(int x = 0 ; x < h_w ; x++)
                        data[y * h_w + x] = (map->sealvl - map->get_h(x * map->bloc_w_db / h_w, y * map->bloc_h_db / h_h)) / 255.0f;
                height_tex = gfx->make_texture_A16F( h_w, h_h, data, FILTER_LINEAR, true );
                delete[] data;
			}

			for (int z = 0 ; z < 512 ; ++z) // The wave base model
			{
				for (int x = 0 ; x < 512 ; ++x)
				{
					// equation : y = ( 1 - sqrtf( 1 - (x*z)^2)) * z / 3 + (1-z) * sinf( x * PI / 2) ^ 2 where z is a parameter
					// Stores the gradient vector clamped into 0.0 - 1.0 ( 0 - 0xFF)
					float X = (x - 256.0f) / 256.0f;
					float Z = z / 512.0f;
					float DX = -X * Z * Z / ( 3.0f * sqrtf( 1.0f - X * Z * X * Z)) + ( 1.0f - Z) * PI * sinf( X * PI / 2.0f) * cosf( X * PI / 2.0f);
					float L = sqrtf( DX * DX + 1.0f);

					DX = DX / L * 127.0f + 127.0f;
					if (DX < 0.0f)
						DX = 0.0f;
					else
					{
						if (DX > 255.0f)
							DX = 255.0f;
					}

					SurfaceByte(tmp,(x<<2),z) = (int)(DX);
					SurfaceByte(tmp,(x<<2)+1,z) = (int)((127.0f / L) + 127.0f);
					SurfaceByte(tmp,(x<<2)+2,z) = 0;
					SurfaceByte(tmp,(x<<2)+3,z) = 0;
				}
			}

			gfx->set_texture_format(GL_RGB8);
			water = gfx->make_texture( tmp, FILTER_LINEAR, false);
			SDL_FreeSurface(tmp);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

			// Enable the texture compression
			if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
				gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
			else
				gfx->set_texture_format(GL_RGB8);
		}

        // A few things required by (pseudo-)instancing code to render highlighted objects
        INSTANCING::water = map->water;
        INSTANCING::sealvl = map->sealvl;

		return true;
	}


	bool Battle::initPostFlight()
	{
		script_timer = 0;
		Conv = 0.001f;
		dt = 0.0f;
		t = 0.0f;
		count = msec_timer;
		reflection_drawn_last_time = false;

		mx = my = 0;
		omb = mouse_b;
		omb2 = mouse_b;
		omb3 = mouse_b;
		amx = mouse_x;
		amy = mouse_y;
		cur_sel = -1;
		old_gui_sel = -1;
		old_sel = false;
		selected = false;
		build = -1;				// Indique si l'utilisateur veut construire quelque chose
		build_order_given = false;
		cur_sel_index = -1;
		omz = mouse_z;
		cam_h = lp_CONFIG->camera_def_h;

		speed_limit = lp_CONFIG->fps_limit;
		delay = (speed_limit == 0.0f) ? 0.0f : (1.0f / speed_limit);
		ordered_destruct = false;
		tilde = false;
		done = false;

		show_script = false; // Affichage des scripts
		show_model = false;	// Affichage les noms des sous objets du modèle 3D de l'unité sélectionnée
		rotate_light = false;
		light_angle = 0.0f;
		cheat_metal = false;
		cheat_energy = false;
		internal_name = false;
		internal_idx = false;
		ia_debug = false;
		view_dbg = false;
		show_mission_info = false;
		speed_changed = false;
		show_timefactor = 0.0f;
		show_gamestatus = 0.0f;
		unit_info = 0.0f;
		unit_info_id = -1;

		lp_CONFIG->pause = false;
		video_timer = msec_timer; // To handle video
		video_shoot = false;
		current_order = SIGNAL_ORDER_NONE;

		// Interface
		IsOnGUI = false;
		IsOnMinimap = false;
		can_be_there = false;

        // Detect current screen shot number
        nb_shoot = -1;
		shoot = false;
        do
        {
            nb_shoot = (nb_shoot+1)%1000000;
        }while(TA3D::Paths::Exists(TA3D::Paths::Screenshots + format("ta3d-shoot%.6d.tga", nb_shoot)) && nb_shoot != 999999);

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


	void Battle::waitForNetworkPlayers()
	{
		g_ta3d_network = new TA3DNetwork(&pArea, pGameData);
		if (pNetworkEnabled)
		{
			players.set_network(g_ta3d_network);
			if (!network_manager.isServer())                // Only server is able to save a game
				pArea.msg("esc_menu.b_save.disable");
		}
		sound_manager->playMusic();
		wait_room(pGameData);
	}



} // namespace TA3D
