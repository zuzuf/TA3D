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
#include "battle.h"
#include "menus/waitroom.h"
#include <network/network.h>
#include <logs/logs.h>
#include <gfx/gfx.h>
#include <TA3D_NameSpace.h>
#include <languages/i18n.h>
#include <languages/table.h>
#include <mesh/textures.h>
#include <mesh/instancing.h>
#include <mesh/mesh.h>
#include <tdf.h>
#include "weapons/weapons.h"
#include <fbi.h>
#include "players.h"
#include <UnitEngine.h>
#include <sounds/manager.h>
#include <gfx/fx.manager.h>
#include <tnt.h>
#include <misc/paths.h>
#include <misc/files.h>
#include <misc/timer.h>
#include <input/mouse.h>
#include "menus/loading.h"
#include <cache.h>


namespace TA3D
{

	void Battle::reloadShaders()
	{
		if (!g_useProgram)
			return;

		if (map->ota_data.whitefog)
            water_shader = new Shader("shaders/water_fog.frag","shaders/water.vert");
		else
            water_shader = new Shader("shaders/water.frag","shaders/water.vert");
		if (map->ota_data.whitefog)
            water_shader_reflec = new Shader("shaders/water_fog.frag","shaders/water.vert");
		else
            water_shader_reflec = new Shader("shaders/water_reflec.frag","shaders/water.vert");
        water_pass1 = new Shader("shaders/water_pass1.frag","shaders/water_pass1.vert");
        water_pass1_low = new Shader("shaders/water_pass1_low.frag","shaders/water_pass1.vert");
        water_pass2 = new Shader("shaders/water_pass2.frag","shaders/water_pass2.vert");
        map->detail_shader = new Shader( "shaders/details.frag", "shaders/details.vert");
        map->shadow2_shader = new Shader("shaders/map_shadow.frag", "shaders/map_shadow.vert");
        water_simulator_shader = new Shader("shaders/water_simulator.frag","shaders/water_simulator.vert");
        water_simulator_shader2 = new Shader("shaders/water_simulator2.frag","shaders/water_simulator.vert");
        water_simulator_shader3 = new Shader("shaders/water_simulator3.frag","shaders/water_simulator.vert");
        water_simulator_shader4 = new Shader("shaders/water_simulator4.frag","shaders/water_simulator4.vert");
        water_distortions_shader = new Shader("shaders/water_distortions.frag","shaders/water_distortions.vert");
		if (map->ota_data.whitefog)
            water_simulator_reflec = new Shader("shaders/water_sim_fog.frag","shaders/water.vert");
		else
            water_simulator_reflec = new Shader("shaders/water_sim_reflec.frag","shaders/water.vert");
        gfx->model_shader = new Shader("shaders/3do_shadow.frag", "shaders/3do_shadow.vert");
	}

	Battle::Result Battle::Execute(GameData* g)
	{
		Battle battle(g);
		return battle.execute();
	}


	Battle::Battle(GameData* g)
		:pResult(brUnknown), pGameData(g), pNetworkEnabled(false), pNetworkIsServer(false),
		sky(),
		escMenuWasVisible(false),
		bShowPing(false)
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Preparing a new battle...");
		grab_mouse(lp_CONFIG->grab_inputs);
		pInstance = this;
    }

	Battle::~Battle()
	{
		pInstance = NULL;

		LOG_INFO(LOG_PREFIX_BATTLE << "Releasing unused resources...");

		water_obj = nullptr;

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
		LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing shadow textures");
		gfx->delete_shadow_map();

		// Network
		if (g_ta3d_network)
		{
			LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing memory used for network object...");
			g_ta3d_network = NULL;
		}
		// Reset the VFS manager
		LOG_DEBUG(LOG_PREFIX_BATTLE << "Reloading VFS manager...");
		VFS::Instance()->reload();

		// If we're in developer mode, clear the cache
		if (lp_CONFIG->developerMode)
			Cache::Clear(true);

		LOG_DEBUG(LOG_PREFIX_BATTLE << "Reinitializing 2D menus environment...");
		gfx->set_2D_mode();
		gfx->ReInitTexSys();
		LOG_INFO(LOG_PREFIX_BATTLE << "Done.");

		if (!lp_CONFIG->fullscreen)
			grab_mouse(false);
	}



	bool Battle::loadFromGameData(GameData* g)
	{
		pResult = brUnknown;
		if (!g)
			return true;

		loading = new Menus::Loading;
        zuzuf::smartptr<Menus::Loading> spLoading = loading;

        freecam_on = nullptr;
        freecam_off = nullptr;
        arrow_texture = nullptr;
        circle_texture = nullptr;
        pause_tex = nullptr;
		water = 0;
		water_sim0 = 0;
		water_sim1 = 0;
		water_sim2 = 0;
        height_tex = 0;
        transtex = 0;
        reflectex = 0;
        first_pass = 0;
        second_pass = 0;
        water_color = 0;

        // We don't want to load things we won't be able to use
		gfx->checkConfig();

		// Here we go
		uint64 startTime = msectimer();
		uint64 timer[23];

		timer[0] = msectimer();
		if (!initPreflight(g))
			return false;
		timer[1] = msectimer();
		if (!initTextures())
			return false;
		timer[2] = msectimer();
		if (!init3DModels())
			return false;
		timer[3] = msectimer();
		if (!initGraphicalFeatures())
			return false;
		timer[4] = msectimer();
		if (!initWeapons())
			return false;
		timer[5] = msectimer();
		if (!initUnits())
			return false;
		timer[6] = msectimer();
		if (!initIntermediateCleanup())
			return false;
		timer[7] = msectimer();
		if (!initEngine())
			return false;
		timer[8] = msectimer();
		if (!initPlayers())
			return false;
		timer[9] = msectimer();
		if (!initRestrictions())
			return false;
		timer[10] = msectimer();
		if (!initGUI())
			return false;
		timer[11] = msectimer();
		if (!initTheMap())
			return false;
		timer[12] = msectimer();
		if (!initTheSky())
			return false;
		timer[13] = msectimer();
		if (!initTheSun())
			return false;
		timer[14] = msectimer();
		if (!initAllTextures())
			return false;
		timer[15] = msectimer();
		if (!initTheCamera())
			return false;
		timer[16] = msectimer();
		if (!initTheWind())
			return false;
		timer[17] = msectimer();
		if (!initTheFog())
			return false;
		timer[18] = msectimer();
		if (!initParticules())
			return false;
		timer[19] = msectimer();
		if (!initTheWater())
			return false;
		timer[20] = msectimer();
		if (!initPostFlight())
			return false;
		timer[21] = msectimer();

		unit_manager.waitUntilReady();
		timer[22] = msectimer();

		// The loading has finished
		(*loading)(100.0f, I18N::Translate("Load finished"));
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading time: " << ((float)(msectimer() - startTime) * 0.001f) << " sec.");
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
			"initPostFlight()",
			"waitUntilReady()"};
		for (int i = 0 ; i < 22 ; ++i)
			LOG_INFO(LOG_PREFIX_BATTLE << functionName[i] << " done in " << timer[i+1] - timer[i] << " msec.");
#endif

		TranslationTable::Update();

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
		fps.lastTime = msectimer();
		fps.toStr.clear();

		// Misc
		pMouseRectSelection.reset();
		pMouseSelecting = false;
		pCacheShowGameStatus = false;

		return true;
	}


	bool Battle::initTextures()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading textures...");
		(*loading)(0.0f, I18N::Translate("Loading textures"));
		texture_manager.all_texture();
		return true;
	}

	bool Battle::init3DModels()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading 3D Models...");
		(*loading)(100.0f / 7.0f, I18N::Translate("Loading 3D Models"));
		model_manager.init();
		model_manager.load_all(loading);
		return true;
	}

	bool Battle::initGraphicalFeatures()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading graphical features...");
		(*loading)(200.0f / 7.0f, I18N::Translate("Loading graphical features"));
		load_features(loading);
		feature_manager.clean();
		model_manager.compute_ids();
		return true;
	}

	bool Battle::initWeapons()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading weapons...");
		(*loading)(250.0f / 7.0f, I18N::Translate("Loading weapons"));
		load_weapons(loading);
		weapons.init();
		return true;
	}

	bool Battle::initUnits()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading units...");
		(*loading)(300.0f / 7.0f, I18N::Translate("Loading units"));
		unit_manager.load_all_units(loading);
		return true;
	}

	bool Battle::initIntermediateCleanup()
	{
		LOG_DEBUG(LOG_PREFIX_BATTLE << "Freeing unused memory");
		(*loading)(400.0f / 7.0f, I18N::Translate("Free unused memory"));
		texture_manager.destroy();
		return true;
	}

	bool Battle::initEngine()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Initializing the engine...");
		(*loading)(500.0f / 7.0f, I18N::Translate("Initialising engine"));
		gfx->SetDefState();
		particle_engine.init();
		return true;
	}

	bool Battle::initPlayers()
	{
		LOG_DEBUG(LOG_PREFIX_BATTLE << "Adding players...");
		players.init(); // Object containing data about players
		for (int i = 0; i < pGameData->nb_players; ++i)
		{
			players.add(pGameData->player_names[i], pGameData->player_sides[i],
						pGameData->player_control[i],
						pGameData->energy[i], pGameData->metal[i],
						pGameData->ai_level[i], pGameData->team[i]); // add a player
		}

		if (players.local_human_id >= 0)
		{
			QString intgaf;
			for (int i = 0; i < ta3dSideData.nb_side; ++i)
			{
				if (ta3dSideData.side_name[i] == pGameData->player_sides[players.local_human_id])
				{
					intgaf = ta3dSideData.side_int[i];
					break;
				}
			}
            if (!intgaf.isEmpty())
				unit_manager.load_panel_texture(intgaf);
		}
		TA3D::MAX_UNIT_PER_PLAYER = pGameData->max_unit_per_player;
		units.init(true);
		return true;
	}

	bool Battle::initRestrictions()
	{
        if (!pGameData->use_only.isEmpty()) 			// We are told not to use all units !!
		{
			LOG_DEBUG(LOG_PREFIX_BATTLE << "Loading restrictions...");
			TDFParser useonly_parser(pGameData->use_only, false, false, true); // In gadgets mode so we can read the special key :)
			for (int i = 0; i < unit_manager.nb_unit ; i++)
				unit_manager.unit_type[i]->not_used = true;
			QString unit_name;
			int i = 0;
            while (!(unit_name = useonly_parser.pullAsString(QString("gadget%1").arg(i))).isEmpty())
			{
				int idx = unit_manager.get_unit_index( unit_name );
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
		(*loading)(550.0f / 7.0f, I18N::Translate("Loading GUI"));
		pArea.load_tdf("gui/game.area");

		try
		{
            pArea.load_window(ta3dSideData.guis_dir + ta3dSideData.side_pref[players.side_view] + "gen.gui"); // Load the order interface
            pArea.msg( ta3dSideData.side_pref[players.side_view] + "gen.hide" );	// Hide it
            pArea.msg( ta3dSideData.side_pref[players.side_view] + "gen.enableScrolling" );	// Enable scrolling
		}
		catch(...)
		{
			LOG_WARNING(LOG_PREFIX_BATTLE << "`gen.gui` is missing or can not be loaded");
		}

		try
		{
            pArea.load_window(ta3dSideData.guis_dir + ta3dSideData.side_pref[players.side_view] + "dl.gui");			// Load the default build interface
            pArea.msg( ta3dSideData.side_pref[players.side_view] + "dl.hide" );	// Hide it
            pArea.msg( ta3dSideData.side_pref[players.side_view] + "dl.enableScrolling" );	// Enable scrolling
		}
		catch(...)
		{
			LOG_WARNING(LOG_PREFIX_BATTLE << "`dl.gui` is missing or can not be loaded");
		}

        const QString &sideName = ta3dSideData.side_name[players.side_view].toLower();
		for (int i = 0; i < unit_manager.nb_unit; ++i)
		{
			if (!(i & 0xF))
				(*loading)((550.0f + 50.0f * (float)i / float(unit_manager.nb_unit + 1)) / 7.0f, I18N::Translate("Loading GUI"));
            if (unit_manager.unit_type[i]->side.toLower() == sideName)
			{
				int e(1);
                while (VFS::Instance()->fileExists(ta3dSideData.guis_dir + unit_manager.unit_type[i]->Unitname + QString::number(e) + ".gui"))
				{
                    toBeLoadedMenuSet.insert((ta3dSideData.guis_dir + unit_manager.unit_type[i]->Unitname + QString::number(e) + ".gui").toLower());
					++e;
				}
			}
		}

		for (unsigned int i = 0; i < players.count(); ++i)
		{
            Gui::GUIOBJ::Ptr obj = pArea.get_object(QString("playerstats.team%1").arg(i));
			if (obj)
			{
				obj->current_state = (byte)TA3D::Math::Log2(players.team[i]);
				obj->Flag &= ~FLAG_HIDDEN;
				obj->Flag &= ~FLAG_CAN_BE_CLICKED;
			}
		}
		return true;
	}


	bool Battle::initTheMap()
	{
		LOG_INFO(LOG_PREFIX_BATTLE << "Loading the map...");
		(*loading)(600.0f / 7.0f, I18N::Translate("Loading the map"));
		LOG_DEBUG(LOG_PREFIX_BATTLE << "Extracting `" << pGameData->map_filename << "`...");

        QIODevice* map_file = VFS::Instance()->readFile(pGameData->map_filename);
		if (!map_file)
			return false;
		LOG_DEBUG(LOG_PREFIX_BATTLE << "`" << pGameData->map_filename << "` extracted");
		LOG_DEBUG(LOG_PREFIX_BATTLE << "loading map data ...");
        map = load_tnt_map(map_file);
		LOG_DEBUG(LOG_PREFIX_BATTLE << "map data loaded");
		delete map_file;

		LOG_INFO(LOG_PREFIX_BATTLE << "Loading details texture...");
		map->load_details_texture( "gfx/details.jpg");			// Load the details texture

		LOG_INFO(LOG_PREFIX_BATTLE << "Initialising the Fog Of War...");
		map->clear_FOW(pGameData->fog_of_war);

        units.map = map; // Setup some useful information

		pGameData->map_filename = Paths::Files::ReplaceExtension(pGameData->map_filename, ".ota");

		LOG_DEBUG(LOG_PREFIX_BATTLE << "Extracting `" << pGameData->map_filename << "`...");
		map_file = VFS::Instance()->readFile(pGameData->map_filename);
		if (map_file)
		{
			LOG_INFO(LOG_PREFIX_BATTLE << "Loading map informations...");
			map->ota_data.load(map_file);

			if (map->ota_data.lavaworld) // make sure we'll draw lava and not water
			{
                map->lava_map = nullptr;

				QImage tmp = gfx->create_surface_ex(32, 16, 16);
                tmp.fill(0xFFFFFFFF);
				map->lava_map = gfx->make_texture(tmp);
			}
			delete map_file;
		}
		pGameData->map_filename = Paths::Files::ReplaceExtension(pGameData->map_filename, "");

		LOG_INFO(LOG_PREFIX_BATTLE << "Computing walkable areas...");
		(*loading)(650.0f / 7.0f, I18N::Translate("Computing walkable areas"));
		Pathfinder::instance()->computeWalkableAreas();

		return true;
	}


	bool Battle::initTheSky()
	{
		// The sky
		sky.choose_a_sky(Paths::ExtractFileName(pGameData->map_filename), ToLower(map->ota_data.planet));

		sky_angle = sky.rotationOffset();
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
        freecam_on = gfx->load_texture("gfx/freecam_on.tga");
        freecam_off = gfx->load_texture("gfx/freecam_off.tga");
        arrow_texture = gfx->load_texture("gfx/arrow.tga");
        circle_texture = gfx->load_texture("gfx/circle.tga");
        pause_tex = gfx->load_texture("gfx/pause.png");
		water = 0;
		return true;
	}

	bool Battle::initTheCamera()
	{
		r1 = r2 = r3 = 0.f;

		cam.reset();
		cam_target.reset();
		camera_zscroll =  -0.00001f;

		cam_target_mx = gfx->SCREEN_W_HALF;
		cam_target_my = gfx->SCREEN_H_HALF;
		cam_has_target = false;
		freecam = false;
		cam_def_timer = msectimer();		// Just to see if the cam has been long enough at the default angle
		track_mode = -1;			// Tracking a unit ? negative value => no
		last_time_activated_track_mode = false;
		Camera::inGame = &cam;

		cam.rpos.x = cam.rpos.y = cam.rpos.z = 0.f;
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
			map->wind = float((TA3D_RAND() % (map->ota_data.maxwindspeed - map->ota_data.minwindspeed)) + map->ota_data.minwindspeed);
		return true;
	}


	bool Battle::initTheFog()
	{
		FogD = 0.3f;
		FogFar = cam.zfar;
		FogNear = FogFar * 0.5f;
		FogColor[0] = 0.8f;
		FogColor[1] = 0.8f;
		FogColor[2] = 0.8f;
		FogColor[3] = 1.0f;

		memcpy(FogColor, sky.fogColor(), sizeof(float) * 4);

		FogMode = GL_LINEAR;
		glFogi(GL_FOG_MODE, FogMode);
		glFogfv(GL_FOG_COLOR, FogColor);
		glFogf(GL_FOG_DENSITY, FogD);
		glHint(GL_FOG_HINT, GL_NICEST);
		glFogf(GL_FOG_START, FogNear);
		glFogf(GL_FOG_END, FogFar);
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
		water_obj->build((float)map->map_w, (float)map->map_h, 1000.0f);
		water_sim0 = water_sim1 = water_sim2 = 0;

		if (g_useProgram && g_useFBO && map->water && lp_CONFIG->water_quality >= 2)
		{
            gfx->glGenFramebuffers(1, &water_FBO);

			if (2 == lp_CONFIG->water_quality)
                water_pass1_low = new Shader("shaders/water_pass1_low.frag","shaders/water_pass1.vert");
			else
                water_pass1 = new Shader("shaders/water_pass1.frag","shaders/water_pass1.vert");
            water_pass2 = new Shader("shaders/water_pass2.frag","shaders/water_pass2.vert");
			if (2 == lp_CONFIG->water_quality)
			{
				if (map->ota_data.whitefog)
                    water_shader = new Shader("shaders/water_fog.frag","shaders/water.vert");
				else
                    water_shader = new Shader("shaders/water.frag","shaders/water.vert");
			}
			else
			{
				if (map->ota_data.whitefog)
                    water_shader_reflec = new Shader("shaders/water_fog.frag","shaders/water.vert");
				else
                    water_shader_reflec = new Shader("shaders/water_reflec.frag","shaders/water.vert");
			}

			if (5 == lp_CONFIG->water_quality)
			{
                water_simulator_shader = new Shader("shaders/water_simulator.frag","shaders/water_simulator.vert");     // Compute variation speed
                water_simulator_shader2 = new Shader("shaders/water_simulator2.frag","shaders/water_simulator.vert");   // Compute variation
                water_simulator_shader3 = new Shader("shaders/water_simulator3.frag","shaders/water_simulator.vert");   // Copy to a normal RGB filtered texture (faster than filtering an RGB32F texture)
                water_simulator_shader4 = new Shader("shaders/water_simulator4.frag","shaders/water_simulator4.vert");  // Compute normals on screen
                water_distortions_shader = new Shader("shaders/water_distortions.frag","shaders/water_distortions.vert");	// Distortions renderer
				if (map->ota_data.whitefog)
                    water_simulator_reflec = new Shader("shaders/water_sim_fog.frag","shaders/water.vert");
				else
                    water_simulator_reflec = new Shader("shaders/water_sim_reflec.frag","shaders/water.vert");
			}

			gfx->set_texture_format(gfx->defaultTextureFormat_RGBA());

			QImage tmp = gfx->create_surface_ex(32,512,512);

			// Water transparency
			transtex = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water reflection
			reflectex = gfx->make_texture( tmp, FILTER_LINEAR);
			int ln2w = Math::Log2(SCREEN_W);
			int ln2h = Math::Log2(SCREEN_H);
			if ((1 << ln2w) < SCREEN_W)
				++ln2w;
			if ((1 << ln2h) < SCREEN_H)
				++ln2h;
			const int workwidth = g_useNonPowerOfTwoTextures ? SCREEN_W : 1 << ln2w;
			const int workheight = g_useNonPowerOfTwoTextures ? SCREEN_H : 1 << ln2h;
			// Water transparency/reflection
			if (lp_CONFIG->water_quality >= 5)
				first_pass = gfx->create_texture_RGBA32F(workwidth, workheight, FILTER_NONE, false);
			else
				first_pass = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water transparency/reflection
			if (lp_CONFIG->water_quality >= 5)
				second_pass = gfx->create_texture_RGBA16F(workwidth, workheight, FILTER_LINEAR, false);
			else
				second_pass = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water transparency/reflection
			water_color = gfx->make_texture( tmp, FILTER_LINEAR);
			// Water simulation data
			if (lp_CONFIG->water_quality >= 5)
			{
				last_water_refresh = msectimer();
				const int simulation_w = 256;
				const int simulation_h = 256;
				water_sim2 = gfx->create_texture_RGBA16F(simulation_w, simulation_h, FILTER_LINEAR, false);
				water_distortions = gfx->create_texture_RGB16F(workwidth, workheight, FILTER_NONE, false);
				float *data = new float[ simulation_w * simulation_h * 4 ];
				const int water_map_size = simulation_w * simulation_h;
				const int water_map_size4 = simulation_w * simulation_h * 4;
				const uint32 water_map_size4m = water_map_size4 - 1U;
				const uint32 simulation_w4 = simulation_w * 4;
				memset(data, 0, water_map_size4 * sizeof(float));

                QString water_cache = TA3D::Paths::Caches + "water_cache.sim";

				if (!TA3D::Paths::Exists(water_cache))
				{
					for( int i = 0 ; i < 500 ; i++ )                    // Initialize it with multiscale data
					{
                        float coef = 5.0f * sqrtf(500.0f - float(i));
						for( int e = 0 ; e < water_map_size ; e++ )
                            data[(e << 2) + 1] += (float(rand() % 2000) * 0.000001f - 0.001f) * coef;
						for( int e = 0 ; e < water_map_size ; e++ )
						{
							uint32 offset = (e << 2) | 1u;
							data[offset] = (data[offset] * 4.0f
											+ data[(offset + 4) & water_map_size4m]
											+ data[(offset + simulation_w4) & water_map_size4m]
											+ data[(offset + water_map_size4 - 4) & water_map_size4m]
											+ data[(offset + water_map_size4 - simulation_w * 4) & water_map_size4m]
											+ (data[(offset + 4 + simulation_w4) & water_map_size4m]
											+ data[(offset + 4 + water_map_size4 - simulation_w4) & water_map_size4m]
											+ data[(offset + water_map_size4 - 4 + simulation_w4) & water_map_size4m]
											+ data[(offset + water_map_size4 - 4 - simulation_w4) & water_map_size4m]) * 0.25f) * 0.11111111111111111f;
						}
					}
					float sum = 0.0f;
					for( int e = 0 ; e < water_map_size ; e++ )
						sum += data[(e << 2) | 1];
					sum /= simulation_h * simulation_w;
					for( int e = 0 ; e < water_map_size ; e++ )
						data[(e << 2) | 1] -= sum;
					for (int y = 0 ; y < simulation_h ; ++y)
						for (int x = 0 ; x < simulation_w ; ++x)
							data[(y * simulation_w + x) * 4 + 3] = data[(y * simulation_w + x) * 4 + 1];
                    QFile file(water_cache);
                    file.open(QIODevice::WriteOnly);
                    if (file.isOpen())
					{
						file.write((const char*)data, sizeof(float) * water_map_size4);
						file.close();
					}
				}
				else
				{
                    QFile file(water_cache);
                    file.open(QIODevice::ReadOnly);
                    if (file.isOpen())
					{
						file.read((char*)data, sizeof(float) * water_map_size4);
						file.close();
					}
				}
				water_sim0 = gfx->make_texture_RGBA32F(256,256,data,FILTER_NONE,false);
				water_sim1 = gfx->make_texture_RGBA32F(256,256,data,FILTER_NONE,false);
				DELETE_ARRAY(data);

				//  Let's create the height map texture used to render progressive water effects using water depth
				int h_w = Math::Min( map->bloc_w_db, gfx->max_texture_size() );
				int h_h = Math::Min( map->bloc_h_db, gfx->max_texture_size() );
				data = new float[ h_w * h_h ];
				for(int y = 0 ; y < h_h ; y++)
					for(int x = 0 ; x < h_w ; x++)
						data[y * h_w + x] = (map->sealvl - map->get_h(x * map->bloc_w_db / h_w, y * map->bloc_h_db / h_h)) * 0.00392156862745098f;	// / 255
				height_tex = gfx->make_texture_A16F( h_w, h_h, data, FILTER_LINEAR, true );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
				DELETE_ARRAY(data);
			}

			for (int z = 0 ; z < 512 ; ++z) // The wave base model
			{
				for (int x = 0 ; x < 512 ; ++x)
				{
					// equation : y = ( 1 - sqrtf( 1 - (x*z)^2)) * z / 3 + (1-z) * sinf( x * PI / 2) ^ 2 where z is a parameter
					// Stores the gradient vector clamped into 0.0 - 1.0 ( 0 - 0xFF)
					float X = (float(x) - 256.0f) * 0.00390625f;		// / 256
					float Z = float(z) * 0.001953125f;					// / 512
					float DX = -X * Z * Z / ( 3.0f * sqrtf( 1.0f - X * Z * X * Z)) + ( 1.0f - Z) * PI * sinf( X * PI * 0.5f) * cosf( X * PI * 0.5f);
					float L = sqrtf( DX * DX + 1.0f);

					DX = DX / L * 127.0f + 127.0f;
					if (DX < 0.0f)
						DX = 0.0f;
					else
					{
						if (DX > 255.0f)
							DX = 255.0f;
					}

                    SurfaceByte(tmp,(x<<2),z) = uint8(Math::Clamp(int(DX), 0, 255));
                    SurfaceByte(tmp,(x<<2)+1,z) = uint8(Math::Clamp((int)((127.0f / L) + 127.0f), 0, 255));
					SurfaceByte(tmp,(x<<2)+2,z) = 0;
					SurfaceByte(tmp,(x<<2)+3,z) = 0;
				}
			}

			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
			water = gfx->make_texture( tmp, FILTER_LINEAR, false);
            gfx->glBindFramebuffer(GL_FRAMEBUFFER,0);

			// Enable the texture compression
			if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
				gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
			else
				gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
		}

		// A few things required by (pseudo-)instancing code to render highlighted objects
		INSTANCING::water = map->water;
		INSTANCING::sealvl = map->sealvl;

		return true;
	}


	bool Battle::initPostFlight()
	{
		dt = 0.0f;
		t = 0.0f;
		count = msectimer();
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
		delay = (speed_limit < 1.0f) ? 0.0f : (1.0f / speed_limit);
		ordered_destruct = false;
		tilde = false;
		done = false;

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
		unit_info_id = -1;

		lp_CONFIG->pause = false;
		video_timer = msectimer(); // To handle video
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
			nb_shoot = (nb_shoot + 1) % 1000000;
        }while (TA3D::Paths::Exists(TA3D::Paths::Screenshots + QString::asprintf("ta3d-shoot%.6d.png", nb_shoot)) && nb_shoot != 999999);

		return true;
	}


	void Battle::updateCurrentGUICacheNames()
	{
		// Reset
		for (int i = 0; i < cgcEnd; ++i)
			pCurrentGUICache[i] = pCurrentGUI;
		// Each item
        pCurrentGUICache[cgcDot] += ".";
        pCurrentGUICache[cgcShow] += ".show";
        pCurrentGUICache[cgcHide] += ".hide";
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
		Menus::WaitRoom::Execute(pGameData);
	}

} // namespace TA3D
