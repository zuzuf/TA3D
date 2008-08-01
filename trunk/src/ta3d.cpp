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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "misc/matrix.h"

#include "intro.h"			// Introduction
#include "cTA3D_Engine.h"	// The Engine
#include "ta3d.h"			// Some core include
#include "menu.h"			// Game menus
#include "restore.h"		// Save/Load mecanisms
#include "network/TA3D_Network.h"	// Network functionnalities such as chat
#include "gfx/fx.h"
#include "misc/paths.h"
#include "misc/files.h"
#include "misc/camera.h"
#include "misc/material.light.h"
#include "ingame/gamedata.h"
#include "languages/i18n.h"
#include <list>
#include <vector>
#include "jpeg/ta3d_jpg.h"
#include "ingame/menus/statistics.h"
#include "misc/math.h"
#include "sounds/manager.h"
#include "logs/logs.h"
#include "console.h"
#include "ingame/weapons/weapons.h"
#include "fbi.h"
#include "UnitEngine.h"
#include "tnt.h"
#include "scripts/script.h"
#include "gfx/shader.h"
#include "ingame/players.h"



#ifndef SCROLL_SPEED
#   define SCROLL_SPEED		400.0f
#endif

#define PICK_TOLERANCE  5

Vector3D cursor_on_map( Camera *cam,MAP *map, bool on_mini_map = false);




/*--------------------------------------------------------------------\
  |                              Game Engine                            |
  \--------------------------------------------------------------------*/
int play(GameData *game_data)
{

    if (network_manager.isConnected())
        network_manager.cleanQueues();

    if (game_data == NULL)
    {
        LOG_ERROR("Cannot start a game !");
        return -1;
    }

    gfx->SCREEN_W_TO_640 = 1.0f;				// To have mouse sensibility undependent from the resolution
    gfx->SCREEN_H_TO_480 = 1.0f;

    expected_players=game_data->nb_players;

    int start_time=msec_timer;		// Pour la mesure du temps de chargement

    LOG_INFO("Loading textures...");
    loading(0.0f,I18N::Translate("Loading textures"));

    /*-----------------------charge les textures-------------------------*/

    texture_manager.all_texture();

    LOG_INFO("Loading 3D Models...");
    loading(100.0f/7.0f,I18N::Translate("Loading 3D Models"));
    /*-----------------------charge les modèles 3D-----------------------*/

    model_manager.init();
    model_manager.load_all(loading);
    model_manager.optimise_all();

    LOG_INFO("Loading graphical features...");
    loading(200.0f/7.0f,I18N::Translate("Loading graphical features"));
    /*-----------------------charge les éléments graphiques--------------*/

    load_features(loading);
    feature_manager.clean();

    model_manager.compute_ids();

    LOG_INFO("Loading weapons...");
    loading(250.0f/7.0f,I18N::Translate("Loading weapons"));
    /*-----------------------charge les armes----------------------------*/

    load_weapons(loading);

    weapons.init();

    LOG_INFO("Loading units...");
    loading(300.0f/7.0f,I18N::Translate("Loading units"));
    /*-----------------------charge les unités---------------------------*/

    load_all_units(loading);

    LOG_DEBUG("Freeing unused memory");
    loading(400.0f/7.0f,I18N::Translate("Free unused memory"));
    /*-----------------------libère la mémoire inutilisée----------------*/

    texture_manager.destroy();

    LOG_INFO("Initializing the engine...");
    loading(500.0f/7.0f,I18N::Translate("Initialising engine"));
    /*-----------------------initialise le moteur------------------------*/

    gfx->SetDefState();
    particle_engine.init();

    set_palette(pal);

    LOG_DEBUG("Adding players...");
    players.init();													// Object containing data about players
    for (uint16 i = 0; i<game_data->nb_players; i++)
        players.add((char*)game_data->player_names[i].c_str(),(char*)game_data->player_sides[i].c_str(),game_data->player_control[i],game_data->energy[i],game_data->metal[i],game_data->ai_level[i]);		// add a player

    if (players.local_human_id >= 0)
    {
        String prefix = "";
        String intgaf = "";

        for (int i = 0; i < ta3dSideData.nb_side; ++i)
            if (ta3dSideData.side_name[ i ] == game_data->player_sides[ players.local_human_id ])
            {
                prefix = ta3dSideData.side_pref[ i ];
                intgaf = ta3dSideData.side_int[ i ];
                break;
            }

        if (prefix != "" && intgaf != "")
            unit_manager.load_panel_texture( prefix, intgaf);
    }

    units.init( true);

    if (!game_data->use_only.empty()) 			// We are told not to use all units !!
    {
        cTAFileParser useonly_parser( game_data->use_only, false, false, true);		// In gadgets mode so we can read the special key :)
        int i = 0;
        for (; i < unit_manager.nb_unit ; i++)
            unit_manager.unit_type[ i ].not_used = true;
        String unit_name = "";
        i = 0;
        while( !(unit_name = useonly_parser.pullAsString( format( "gadget%d", i))).empty())
        {
            int idx = unit_manager.get_unit_index( unit_name.c_str());
            if (idx >= 0)
                unit_manager.unit_type[ idx ].not_used = false;
            ++i;
        }
    }

    LOG_INFO("Loading the GUI...");
    loading(550.0f/7.0f,I18N::Translate("Loading GUI"));
    //-------------------	Code related to the GUI		--------------------------

    AREA	game_area;			// Area object that will handle game GUI

    game_area.load_tdf( "gui/game.area");					// Loads the game area

    try
    {
        game_area.load_window( ta3dSideData.guis_dir + ta3dSideData.side_pref[players.side_view] + "gen.gui");			// Load the order interface
        I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(String( ta3dSideData.side_pref[players.side_view]) + "gen.hide").c_str(), NULL, NULL);	// Hide it
    }
    catch(...)
    {
        LOG_WARNING("`gen.gui` is missing or can not be loaded");
    }

    try
    {
        game_area.load_window( ta3dSideData.guis_dir + ta3dSideData.side_pref[players.side_view] + "dl.gui");			// Load the default build interface
        I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(String( ta3dSideData.side_pref[players.side_view]) + "dl.hide").c_str(), NULL, NULL);	// Hide it
    }
    catch(...)
    {
        LOG_WARNING("`dl.gui` is missing or can not be loaded");
    }

    for (int i = 0 ; i < unit_manager.nb_unit ; i++)  // Loads the GUI
    {
        if (!(i & 0xF))
            loading( (550.0f + 50.0f * i / (unit_manager.nb_unit+1))/7.0f,I18N::Translate("Loading GUI"));
        if (String::ToLower(unit_manager.unit_type[i].side) == String::ToLower(ta3dSideData.side_name[players.side_view]))
        {
            int e = 1;
            while( HPIManager->Exists( ta3dSideData.guis_dir + unit_manager.unit_type[ i ].Unitname + format( "%d.gui", e)))
            {
                game_area.load_window( ta3dSideData.guis_dir + unit_manager.unit_type[ i ].Unitname + format( "%d.gui", e));			// Load the build interface
                I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( unit_manager.unit_type[ i ].Unitname + format( "%d.hide", e)).c_str(), NULL, NULL);	// Hide it
                e++;
            }
        }
    }

    String current_gui;

    //----------------------------------------------------------------------------

    LOG_INFO("Loading the map...");
    loading(600.0f/7.0f,I18N::Translate("Loading the map"));
    /*-----------------------charge la carte-----------------------------*/

    LOG_DEBUG("Extracting `" << game_data->map_filename << "`...");
    byte *map_file=HPIManager->PullFromHPI(game_data->map_filename);

    if (!map_file)
        return -1;
    MAP *map=load_tnt_map(map_file);
    delete[] map_file;

    LOG_INFO("Loading details texture...");
    map->load_details_texture( "gfx/details.jpg");			// Load the details texture

    LOG_INFO("Initialising the Fog Of War...");
    map->clear_FOW( game_data->fog_of_war);

    units.map = map;			// Setup some useful information

    game_data->map_filename = Paths::Files::ReplaceExtension( game_data->map_filename, ".ota");

    LOG_DEBUG("Extracting `" << game_data->map_filename << "`...");
    uint32 ota_size=0;
    map_file=HPIManager->PullFromHPI(game_data->map_filename,&ota_size);
    if (map_file)
    {
        LOG_INFO("Loading map informations...");
        map->ota_data.load((char*)map_file,ota_size);

        if (map->ota_data.lavaworld) // make sure we'll draw lava and not water
        {
            gfx->destroy_texture( map->lava_map);

            BITMAP *tmp = create_bitmap_ex(32, 16, 16);
            clear_to_color( tmp, 0xFFFFFFFF);
            map->lava_map = gfx->make_texture( tmp);
            destroy_bitmap( tmp);
        }

        delete[] map_file;
    }

    game_data->map_filename.resize( game_data->map_filename.size() - 3);		// Remove the ".ota" extension

    SKY_DATA	*sky_data = choose_a_sky( game_data->map_filename, map->ota_data.planet);

    if (sky_data == NULL)
    {
        sky_data = new SKY_DATA;
        sky_data->texture_name = "gfx/sky/sky.jpg";
    }

    bool spherical_sky = sky_data->spherical;

    if (map->ota_data.maxwindspeed!=map->ota_data.minwindspeed)
        map->wind=(TA3D_RAND()%(map->ota_data.maxwindspeed-map->ota_data.minwindspeed))+map->ota_data.minwindspeed;

    uint32	script_timer = 0;

    GLuint	sky;
    sky = gfx->load_texture( sky_data->texture_name, FILTER_TRILINEAR, NULL, NULL, false);
    GLuint	glow = gfx->load_texture("gfx/glow.tga");
    GLuint	freecam_on = gfx->load_texture("gfx/freecam_on.tga");
    GLuint	freecam_off = gfx->load_texture("gfx/freecam_off.tga");
    GLuint	arrow_texture = gfx->load_texture("gfx/arrow.tga");
    GLuint	circle_texture = gfx->load_texture("gfx/circle.tga");
    GLuint	water = 0;

    loading(100.0f,I18N::Translate("Load finished"));

    start_time = msec_timer-start_time;
    LOG_INFO("Loading time: " << (float)start_time * 0.001f << " sec.");

    float Conv=0.001f;
    float dt=0.0f;
    float t=0.0f;
    int count=msec_timer;
    int nbfps=0;
    int fpscount=msec_timer;
    int fps=0;
    int i;

    bool	reflection_drawn_last_time = false;

    float camera_zscroll =  - 0.00001f;		// The position of the camera on the virtual "rail"
    float r1,r2,r3;
    r1=r2=r3=0.0f;
    r1 = -lp_CONFIG->camera_def_angle - 0.00001f;
    Camera cam;
    Vector3D cam_target;
    int cam_target_mx = gfx->SCREEN_W_HALF;
    int cam_target_my = gfx->SCREEN_H_HALF;
    bool cam_has_target=false;
    bool freecam=false;
    int cam_def_timer = msec_timer;		// Just to see if the cam has been long enough at the default angle
    int track_mode = -1;			// Tracking a unit ? negative value => no
    bool last_time_activated_track_mode = false;
    Camera::inGame=&cam;

    cam.rpos.x = cam.rpos.y = cam.rpos.z = 0.0f;
    cam.rpos.z += 150.0f;
    cam.rpos.y = lp_CONFIG->camera_def_h;
    cam.zfar=500.0f;
    cam.setWidthFactor(gfx->width, gfx->height);

    float FogD=0.3f;
    float FogNear=cam.zfar*0.5f;
    float FogColor[] = {0.8f,0.8f,0.8f,1.0f};

    memcpy(FogColor, sky_data->FogColor, sizeof( float) * 4);

    GLuint FogMode=GL_LINEAR;
    glFogi (GL_FOG_MODE, FogMode);
    glFogfv (GL_FOG_COLOR, FogColor);
    glFogf (GL_FOG_DENSITY, FogD);
    glHint (GL_FOG_HINT, GL_NICEST);
    glFogf (GL_FOG_START, FogNear);
    glFogf (GL_FOG_END, cam.zfar);

    glEnable(GL_FOG);

    HWLight sun;
    sun.Att=0.0f;
    sun.Dir.x=-1.0f;
    sun.Dir.y=2.0f;
    sun.Dir.z=1.0f;
    sun.Dir.unit();
    sun.LightAmbient[0]=0.25f;
    sun.LightAmbient[1]=0.25f;
    sun.LightAmbient[2]=0.25f;
    sun.LightAmbient[3]=0.25f;
    sun.LightDiffuse[0]=1.0f;
    sun.LightDiffuse[1]=1.0f;
    sun.LightDiffuse[2]=1.0f;
    sun.LightDiffuse[3]=1.0f;
    sun.LightSpecular[0]=0.0f;
    sun.LightSpecular[1]=0.0f;
    sun.LightSpecular[2]=0.0f;
    sun.LightSpecular[3]=0.0f;
    sun.Directionnal=true;

    lp_CONFIG->pause = false;

    float speed_limit= lp_CONFIG->fps_limit;
    float delay=(speed_limit==0.0f) ? 0.0f : 1.0f/speed_limit;
    int nb_shoot=0;
    bool shoot=false;

    bool ordered_destruct = false;

    bool tilde=false;
    bool done=false;

    int	exit_mode=EXIT_NONE;

    fire=particle_engine.addtex("gfx/fire.tga");
    build_part=particle_engine.addtex("gfx/part.tga");

    fx_manager.loadData();

    int mx,my,omb=mouse_b,omb2=mouse_b,omb3=mouse_b, amx=mouse_x, amy=mouse_y;
    int	sel_x[2],sel_y[2],selecting=false;
    int cur_sel = -1;
    int old_gui_sel = -1;
    bool old_sel = false;
    bool selected = false;
    int	build=-1;				// Indique si l'utilisateur veut construire quelque chose
    bool build_order_given = false;
    int cur_sel_index=-1;
    int omz=mouse_z;
    float cam_h = lp_CONFIG->camera_def_h;

    bool show_script=false;			// Affichage des scripts
    bool show_model=false;			// Affichage les noms des sous objets du modèle 3D de l'unité sélectionnée
    bool rotate_light=false;
    float light_angle=0.0f;
    bool cheat_metal=false;
    bool cheat_energy=false;
    bool internal_name=false;
    bool internal_idx=false;
    bool ia_debug=false;
    bool view_dbg=false;
    bool show_mission_info=false;
    bool speed_changed = false;
    float show_timefactor = 0.0f;

    float show_gamestatus = 0.0f;

    float unit_info=0.0f;
    int	unit_info_id=-1;

    rest(100);						// Juste pour éviter un "Xlib: ..." / To avoid an "Xlib: ..." error

    float wind_t=t;					// To handle wind variations
    bool wind_change=false;

    int video_timer=msec_timer;			// To handle video
    bool video_shoot=false;

    int current_order=SIGNAL_ORDER_NONE;

    WATER	water_obj;
    water_obj.build(map->map_w,map->map_h,1000);

    SKY		sky_obj;
    sky_obj.build(10,400,sky_data->full_sphere);
    float	sky_angle = sky_data->rotation_offset;

    Shader	water_shader, water_shader_reflec, water_pass1, water_pass1_low, water_pass2;
    GLuint transtex,reflectex,first_pass,second_pass,water_color;
    GLuint water_FBO;

    if (g_useProgram && g_useFBO && map->water)
    {
        glGenFramebuffersEXT(1,&water_FBO);

        water_pass1.load("shaders/water_pass1.frag","shaders/water_pass1.vert");
        water_pass1_low.load("shaders/water_pass1_low.frag","shaders/water_pass1.vert");
        water_pass2.load("shaders/water_pass2.frag","shaders/water_pass2.vert");
        water_shader.load("shaders/water.frag","shaders/water.vert");
        water_shader_reflec.load("shaders/water_reflec.frag","shaders/water.vert");

        allegro_gl_use_alpha_channel(true);

        allegro_gl_set_texture_format(GL_RGBA8);
        BITMAP *tmp=create_bitmap_ex(32,512,512);	// On ne peut pas utiliser screen donc on crée un BITMAP temporaire
        transtex = gfx->make_texture( tmp, FILTER_LINEAR);		// pour les effets de transparence de l'eau
        reflectex = gfx->make_texture( tmp, FILTER_LINEAR);	// pour les effets de reflection sur l'eau
        first_pass = gfx->make_texture( tmp, FILTER_LINEAR);	// pour les effets de reflection/transparence sur l'eau
        second_pass = gfx->make_texture( tmp, FILTER_LINEAR);	// pour les effets de reflection/transparence sur l'eau
        water_color = gfx->make_texture( tmp, FILTER_LINEAR);	// pour les effets de reflection/transparence sur l'eau

        for (int z = 0 ; z < 512 ; z++)						// The wave base model
            for (int x = 0 ; x < 512 ; x++)
            {
                // equation : y = ( 1 - sqrt( 1 - (x*z)^2)) * z / 3 + (1-z) * sin( x * PI / 2) ^ 2 where z is a parameter
                // Stores the gradient vector clamped into 0.0 - 1.0 ( 0 - 0xFF)
                float X = (x - 256.0f) / 256.0f;
                float Z = z / 512.0f;
                float DX = -X * Z * Z / ( 3.0f * sqrt( 1.0f - X * Z * X * Z)) + ( 1.0f - Z) * PI * sin( X * PI / 2.0f) * cos( X * PI / 2.0f);
                float L = sqrt( DX * DX + 1.0f);
                DX = DX / L * 127.0f + 127.0f;
                if (DX < 0.0f)	DX = 0.0f;
                else if (DX > 255.0f)	DX = 255.0f;
                tmp->line[z][(x<<2)] = (int)(DX);
                tmp->line[z][(x<<2)+1] = (int)((127.0f / L) + 127.0f);
                tmp->line[z][(x<<2)+2] = 0;
                tmp->line[z][(x<<2)+3] = 0;
            }

        allegro_gl_set_texture_format(GL_RGB8);

        water = gfx->make_texture( tmp, FILTER_TRILINEAR, false);

        destroy_bitmap(tmp);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

        if (g_useTextureCompression)			// Active la compression de texture
            allegro_gl_set_texture_format(GL_COMPRESSED_RGB_ARB);
        else
            allegro_gl_set_texture_format(GL_RGB8);
    }

    delay=(speed_limit==0.0f) ? 0.0f : 1.0f/speed_limit;

    /*---------------------------- network management --------------------------*/

    TA3DNetwork	ta3d_network( &game_area, game_data);

    if (network_manager.isConnected())
    {
        players.set_network( &ta3d_network);
        game_area.msg("esc_menu.b_save.disable");
    }
    g_ta3d_network = &ta3d_network;

    sound_manager->playMusic();

    wait_room(game_data);

    /*----------------------------- script management --------------------------*/

    LUA_PROGRAM	game_script;					// Script that will rule the game
    if (!network_manager.isConnected() || network_manager.isServer())
        game_script.load(game_data->game_script, map);	// Load the script

    if (!game_data->saved_file.empty()) 			// We have something to load
    {
        load_game( game_data);
        done = !game_data->saved_file.empty();		// If loading the game fails, then exit
    }

    //-----------------------   Code related to threads   ------------------------

    unit_engine_thread_sync = 0;
    weapon_engine_thread_sync = 0;
    particle_engine_thread_sync = 0;

    units.Start();			// Start the Unit Engine

    particle_engine.more_memory();				// Pre-allocating memory (no lag with first particles)
    particle_engine.set_data( map->ota_data.gravity, map->wind_vec);
    particle_engine.Start();		// Start the particle engine

    // Start the weapon engine
    weapons.set_data( map);
    features.set_data( map->wind_vec);		// NB: the feature engine runs in the weapon thread to avoid having too much thread to synchronise
    weapons.Start();

    /*---------------------------- players management --------------------------*/

    players.set_map( map);
    players.Start();

    /*------------------------- end of players management ----------------------*/

    do
    {
        /*------------------------ handle GUI events -------------------------------*/

        bool IsOnGUI = false;
        bool IsOnMinimap = mouse_x < 128 && mouse_y < 128;		// Used to handle on mini map commands
        IsOnGUI = (mouse_x < 128 && ( mouse_y >= SCREEN_H - 64 || mouse_y < 128)) || mouse_y < 32 || mouse_y >= SCREEN_H - 32;		// Priority given to game interface
        if (!IsOnGUI)
            IsOnGUI = (game_area.check() != 0);
        else        // We need to do it there because AREA::check does it and we do it nowhere else
        {
            poll_mouse();
            poll_keyboard();
        }

        IsOnGUI |= mouse_x < 128;		// Priority given to game interface

        if (IsOnMinimap) 				// Check if we can project the cursor position on the map
        {
            if (fabs( (mouse_x - 64) * 252.0f / 128.0f) > map->mini_w * 0.5f)	IsOnMinimap = false;
            else if (fabs( (mouse_y - 64) * 252.0f / 128.0f) > map->mini_h * 0.5f)	IsOnMinimap = false;
        }

        if (IsOnMinimap)
            units.pick_minimap();			// Precompute this, we'll need it

        /*------------------------ end of "handle GUI events" ----------------------*/

        bool can_be_there=false;		// To tell if the unit the player wants to build can be built where the cursor is

        if (video_shoot)
            if ((msec_timer-video_timer)*Conv>=1.0f/15.0f) {
                video_timer=msec_timer;
                shoot=true;
            }

        /*------------------------ handle wind speed and dir -----------------------*/

        if (t-wind_t>=10.0f) 		// Make a change every 10 sec. (simulation time)
        {
            wind_t=t;
            map->wind+=(Math::RandFromTable()%2001)-1000;
            if (map->wind<map->ota_data.minwindspeed)
                map->wind=map->ota_data.minwindspeed;
            else if (map->wind>map->ota_data.maxwindspeed)
                map->wind=map->ota_data.maxwindspeed;
            map->wind_dir += (Math::RandFromTable() % 901) * 0.1f - 45.0f;
            if (map->wind_dir<0.0f)	map->wind_dir+=360.0f;
            else if (map->wind_dir>=360.0f)	map->wind_dir-=360.0f;
            map->wind_vec.y=0.0f;
            map->wind_vec.x=0.01f*map->wind*cos(map->wind_dir*DEG2RAD);
            map->wind_vec.z=0.01f*map->wind*sin(map->wind_dir*DEG2RAD);
            units.set_wind_change();
            wind_change=true;
        }
        else
            wind_change=false;

        /*---------------------- end of wind speed and dir code --------------------*/

        /*----------------------------- sound management ---------------------------*/

        if (units.nb_attacked/(units.nb_attacked + units.nb_built + 1) >= 0.75f)
            sound_manager->setMusicMode(true);
        else
        {
            if (units.nb_attacked/(units.nb_attacked + units.nb_built + 1) <= 0.25f)
                sound_manager->setMusicMode(false);
        }

        sound_manager->setListenerPos(cam.rpos);
        sound_manager->update3DSound();

        /*-------------------------- end of sound management -----------------------*/

        Vector3D old_cam_pos = cam.rpos;
        float old_zscroll = camera_zscroll;
        if (!freecam)
        {
            int delta = IsOnGUI ? 0 : mouse_z-omz;
            camera_zscroll += delta * 2.0f * lp_CONFIG->camera_zoom_speed;
            if (camera_zscroll < -25.0f) camera_zscroll = -25.0f;
            else if (camera_zscroll > 20.0f) camera_zscroll = 20.0f;

            if ((msec_timer - cam_def_timer) * Conv >= 1.0f && delta != 0
                && ( ( camera_zscroll > 0.0f && old_zscroll <= 0.0f)
                     || ( camera_zscroll < 0.0f && old_zscroll >= 0.0f)))			// Just to make it lock near def position
            {
                cam_def_timer = msec_timer;
                old_zscroll = 0.0f;
                if (camera_zscroll > -lp_CONFIG->camera_def_angle)
                    old_zscroll += 0.00001f;
                else
                    old_zscroll -= 0.00001f;
            }

            if ((msec_timer - cam_def_timer) * Conv < 0.5f)
                camera_zscroll = old_zscroll;

            float angle_factor = Math::Max(fabs(-lp_CONFIG->camera_def_angle+45.0f) / 20.0f, fabs(-lp_CONFIG->camera_def_angle+90.0f) / 25.0f);

            r1 = -lp_CONFIG->camera_def_angle + camera_zscroll * angle_factor;
            if (r1 > -45.0f) 		r1 = -45.0f;
            else if (r1 < -90.0f)	r1 = -90.0f;

            cam_h = lp_CONFIG->camera_def_h + (exp(-camera_zscroll * 0.15f) - 1.0f) / (exp(3.75f) - 1.0f) * Math::Max(map->map_w,map->map_h);
            if (delta > 0 && !IsOnGUI)
            {
                if (!cam_has_target || abs( mouse_x - cam_target_mx) > 2 || abs( mouse_y - cam_target_my) > 2)
                {
                    cam_target = cursor_on_map(&cam,map);
                    cam_target_mx = mouse_x;
                    cam_target_my = mouse_y;
                    cam_has_target=true;
                }
            }
        }
        else
        {
            int delta = IsOnGUI ? 0 : mouse_z-omz;
            cam.rpos = cam.rpos - 0.5f * delta * cam.dir;
            cam_has_target = false;
        }
        omz = mouse_z;

        cursor_type=CURSOR_DEFAULT;			// Revient au curseur par défaut

        bool rope_selection = selecting && ( abs( sel_x[0] - sel_x[1]) >= PICK_TOLERANCE || abs( sel_y[0] - sel_y[1]) >= PICK_TOLERANCE);
        if ((!IsOnGUI || IsOnMinimap) && !rope_selection)
        {
            if (selected)
                for (i = 0; i < units.index_list_size; ++i)
                {
                    uint32 e = units.idx_list[i];
                    if ((units.unit[e].flags & 1) && units.unit[e].owner_id==players.local_human_id && units.unit[e].sel && unit_manager.unit_type[units.unit[e].type_id].canmove)
                    {
                        cursor_type=CURSOR_MOVE;
                        break;
                    }
                }
        }

        if (build>=0)
            cursor_type=CURSOR_DEFAULT;

        dt = (msec_timer - count) * Conv; // Regulate frame rate
        rest(0); // To play nice with other threads
        while (dt < delay)
        {
            switch(lp_CONFIG->priority_level)
            {
                case 0: rest(1); break;
                case 1: rest(0); break;
            }
            dt = (msec_timer - count) * Conv;
        }
        count = msec_timer;

        if (spherical_sky)
            sky_angle += sky_data->rotation_speed * dt * units.apparent_timefactor; 

        unit_info -= dt;
        if (!lp_CONFIG->pause)
        {
            light_angle+=dt*units.apparent_timefactor;

            t += dt * units.apparent_timefactor;
        }

        ++nbfps;
        if (nbfps >= 10 && (msec_timer - fpscount) * Conv >= 0.05f)
        {
            fps = (int)(nbfps / ((msec_timer - fpscount) * Conv));
            fpscount = msec_timer;
            nbfps = 0;
        }

        /*------------bloc regroupant ce qui est relatif aux commandes----------------*/

        if (players.local_human_id >= 0 && !console.activated() && !game_area.get_state("chat"))
        {
            if (key[KEY_SPACE]) 				// Show gamestatus window
            {
                if (show_gamestatus == 0.0f)
                {
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)"gamestatus.show", NULL, NULL);	// Show it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)"playerstats.show", NULL, NULL);	// Show it
                }

                show_gamestatus += 10.0f * dt;
                if (show_gamestatus > 1.0f)
                    show_gamestatus = 1.0f;
            }
            else
            {									// Hide gamestatus window
                bool pre_visible = show_gamestatus > 0.0f;

                show_gamestatus -= 10.0f * dt;

                if (show_gamestatus < 0.0f && pre_visible)
                {
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)"gamestatus.hide", NULL, NULL);	// Hide it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)"playerstats.hide", NULL, NULL);	// Hide it
                }
                if (show_gamestatus < 0.0f)
                    show_gamestatus = 0.0f;
            }

            if (show_gamestatus > 0.0f)
            {
                WND *statuswnd = game_area.get_wnd( "gamestatus");
                if (statuswnd)
                    statuswnd->y = (int)(SCREEN_H - (statuswnd->height + 32) * show_gamestatus);
                uint32 game_time = units.current_tick / TICKS_PER_SEC;
                game_area.set_caption( "gamestatus.time_label", I18N::Translate("game time") + format(" : %d:%d:%d", game_time / 3600, (game_time / 60) % 60, game_time % 60));
                game_area.set_caption( "gamestatus.units_label", I18N::Translate("units") + format(" : %d/%d", players.nb_unit[ players.local_human_id ], MAX_UNIT_PER_PLAYER));
                game_area.set_caption( "gamestatus.speed_label", I18N::Translate("speed") + format(" : %d (%d)", (int)lp_CONFIG->timefactor,(int)units.apparent_timefactor));

                statuswnd = game_area.get_wnd( "playerstats");
                if (statuswnd)
                    statuswnd->x = (int)(SCREEN_W - (statuswnd->width + 10) * show_gamestatus);
                for (short int i = 0; i < players.nb_player; ++i)
                {
                    GUIOBJ *obj = game_area.get_object( format("playerstats.p%d_box", i));
                    if (obj)
                        obj->Data = gfx->makeintcol( player_color[ 3 * player_color_map[ i ] ], player_color[ 3 * player_color_map[ i ] + 1 ], player_color[ 3 * player_color_map[ i ] + 2 ], 0.5f);
                    game_area.set_caption(format("playerstats.p%d_kills", i), format( "%d", players.kills[i]));
                    game_area.set_caption(format("playerstats.p%d_losses", i), format( "%d", players.losses[i]));
                }
            }
        }

        if (TA3D_CTRL_PRESSED && key[KEY_D])
        {
            if (!ordered_destruct)
            {
                for (uint16 e=0;e<units.index_list_size;e++)
                {
                    i = units.idx_list[e];
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                        units.unit[i].toggle_self_destruct();
                }
                ordered_destruct = true;
            }
        }
        else
            ordered_destruct = false;

        if (key[KEY_PLUS_PAD] && !console.activated())
        {
            if (!speed_changed && lp_CONFIG->timefactor < 10.0f)
            {
                lp_CONFIG->timefactor++;
                show_timefactor = 1.0f;
            }
            speed_changed = true;
        }
        else
        {
            if ((key[KEY_MINUS] || key[KEY_MINUS_PAD]) && !console.activated())
            {
                if (!speed_changed && lp_CONFIG->timefactor > 1.0f)
                {
                    lp_CONFIG->timefactor--;
                    show_timefactor = 1.0f;
                }
                speed_changed = true;
            }
            else
                speed_changed = false;
        }

        if (track_mode >= 0 && track_mode <= units.max_unit)
        {
            if (!(units.unit[ track_mode ].flags & 1)) // Leave tracking mode
            {
                track_mode = -1;
                cam_has_target = false;
            }
            else
            {
                cam_has_target = true;
                cam_target = units.unit[ track_mode ].Pos;
                cam_target_mx = gfx->SCREEN_W_HALF;
                cam_target_my = gfx->SCREEN_H_HALF;
            }
        }
        else
            track_mode = -1;

        if (track_mode >= 0 && ( cur_sel_index < 0 || cur_sel_index >= units.max_unit || track_mode != cur_sel_index))
            track_mode = -1;

        if (key[KEY_T] && !console.activated() && cur_sel_index>=0 && cur_sel_index<units.max_unit)
        {
            if (!last_time_activated_track_mode)
                track_mode = track_mode == cur_sel_index ? -1 : cur_sel_index;
            last_time_activated_track_mode = true;
        }
        else
            if (key[KEY_T] && !console.activated())
            {
                track_mode = -1;
                cam_has_target = false;
            }
            else
                last_time_activated_track_mode = false;

        if (key[KEY_F1] && units.last_on >= 0 && units.unit[ units.last_on ].type_id >= 0)
        {
            unit_info_id = units.unit[ units.last_on ].type_id;
            unit_info = 1.0f;
        }
        else
            if (key[KEY_F1] && cur_sel >= 0)
            {
                unit_info_id = cur_sel;
                unit_info = 1.0f;
            }

        if (mouse_x < 128.0f && mouse_y < 128.0f && mouse_x >= 0.0f && mouse_y >= 0.0f && mouse_b == 2)
        {
            cam.rpos.x = (mouse_x - 64) * map->map_w / 128.0f * 252.0f / map->mini_w;
            cam.rpos.z = (mouse_y - 64) * map->map_h / 128.0f * 252.0f / map->mini_h;
            cam_has_target=false;
        }
        if (mouse_x < 1)
        {
            Vector3D move_dir = cam.side;
            move_dir.y = 0.0f;
            move_dir.unit();
            cam.rpos = cam.rpos - (SCROLL_SPEED*dt*cam_h / 151.0f)*move_dir;
            cam_has_target=false;
        }
        else
            if (mouse_x>=SCREEN_W-1)
            {
                Vector3D move_dir = cam.side;
                move_dir.y = 0.0f;
                move_dir.unit();
                cam.rpos = cam.rpos + (SCROLL_SPEED*dt*cam_h / 151.0f)*move_dir;
                cam_has_target=false;
            }
        if (mouse_y < 1)
        {
            Vector3D move_dir = cam.up;
            if (move_dir.x==0.0f && move_dir.z==0.0f)
            {
                move_dir = cam.dir;
                move_dir.y=0.0f;
            }
            else
                move_dir.y=0.0f;
            move_dir.unit();
            cam.rpos = cam.rpos+ (SCROLL_SPEED * dt * cam_h / 151.0f) * move_dir;
            cam_has_target = false;
        }
        else
            if (mouse_y >= SCREEN_H - 1)
            {
                Vector3D move_dir(cam.up);
                if (move_dir.x == 0.0f && move_dir.z == 0.0f)
                {
                    move_dir = cam.dir;
                    move_dir.y = 0.0f;
                }
                else
                    move_dir.y = 0.0f;
                move_dir.unit();
                cam.rpos = cam.rpos - (SCROLL_SPEED * dt * cam_h / 151.0f) * move_dir;
                cam_has_target = false;
            }

        if (freecam)
        {
            if (mouse_b == 4)
            {
                get_mouse_mickeys(&mx,&my);
                if (omb == mouse_b)
                {
                    r2 -= mx;
                    r1 -= my;
                }
                position_mouse(gfx->SCREEN_W_HALF, gfx->SCREEN_H_HALF);
            }
            else
                if (omb == 4)
                    position_mouse(gfx->SCREEN_W_HALF, gfx->SCREEN_H_HALF);
        }
        else
            if (!TA3D_CTRL_PRESSED)
            {
                if (mouse_b==4)
                {
                    get_mouse_mickeys(&mx,&my);
                    if (omb==mouse_b)
                    {
                        cam.rpos.x+=mx * cam_h / 151.0f;
                        cam.rpos.z+=my * cam_h / 151.0f;
                        cam_has_target=false;
                    }
                    position_mouse(gfx->SCREEN_W_HALF,gfx->SCREEN_H_HALF);
                }
                else
                    if (omb==4)
                    {
                        position_mouse(gfx->SCREEN_W_HALF,gfx->SCREEN_H_HALF);
                        cam_has_target=false;
                    }
            }
        omb=mouse_b;

        if (!freecam)
        {
            if (key[KEY_UP])
            {
                cam.rpos.z-=SCROLL_SPEED*dt*cam_h / 151.0f;
                cam_has_target = false;
            }
            if (key[KEY_DOWN])
            {
                cam.rpos.z += SCROLL_SPEED * dt * cam_h / 151.0f;
                cam_has_target = false;
            }
            if (key[KEY_RIGHT])
            {
                cam.rpos.x+=SCROLL_SPEED*dt*cam_h / 151.0f;
                cam_has_target=false;
            }
            if (key[KEY_LEFT])
            {
                cam.rpos.x-=SCROLL_SPEED*dt*cam_h / 151.0f;
                cam_has_target=false;
            }

            float h=map->get_unit_h(cam.rpos.x,cam.rpos.z);
            if (h<map->sealvl)
                h=map->sealvl;
            for (short int i = 0; i < 20; ++i)// Increase precision
            {
                for (float T = 0.0f; T < dt ; T += 0.1f)
                    cam.rpos.y += (h + cam_h - cam.rpos.y) * Math::Min(dt - T, 0.1f);
            }
        }
        else
        {
            if (key[KEY_UP])
                cam.rpos = cam.rpos + 100.0f * dt * cam_h / 151.0f * cam.dir;
            if (key[KEY_DOWN])
                cam.rpos = cam.rpos - 100.0f * dt * cam_h / 151.0f * cam.dir;
            if (key[KEY_RIGHT])
                cam.rpos = cam.rpos + 100.0f * dt * cam_h / 151.0f * cam.side;
            if (key[KEY_LEFT])
                cam.rpos = cam.rpos - 100.0f * dt * cam_h / 151.0f * cam.side;
        }

        if (cam.rpos.x < -map->map_w_d)
        {
            cam.rpos.x = -map->map_w_d;
            cam_has_target = false;
        }
        if (cam.rpos.x>map->map_w_d)
        {
            cam.rpos.x = map->map_w_d;
            cam_has_target = false;
        }
        if (cam.rpos.z < -map->map_h_d + 200.0f)
        {
            cam.rpos.z = -map->map_h_d + 200.0f;
            cam_has_target = false;
        }
        if (cam.rpos.z>map->map_h_d && !cam_has_target)
            cam.rpos.z=map->map_h_d;
        if (cam.rpos.z > map->map_h_d+200.0f)
            cam.rpos.z = map->map_h_d+200.0f;

        MATRIX_4x4 Rotation;
        if (lp_CONFIG->camera_zoom == ZOOM_NORMAL)
            Rotation = RotateX( r1 * DEG2RAD) * RotateY( r2 * DEG2RAD) * RotateZ( r3 * DEG2RAD);
        else
            Rotation = RotateX( -lp_CONFIG->camera_def_angle * DEG2RAD) * RotateY( r2 * DEG2RAD) * RotateZ( r3 * DEG2RAD);

        cam.setMatrix(Rotation);
        cam.updateShake(dt);

        if (cam_has_target)
        {
            Vector3D cur_dir;
            cur_dir = cam.dir+cam.widthFactor*2.0f*(cam_target_mx-gfx->SCREEN_W_HALF)*gfx->SCREEN_W_INV*cam.side-1.5f*(cam_target_my-gfx->SCREEN_H_HALF)*gfx->SCREEN_H_INV*cam.up;
            cur_dir.unit();		// Direction pointée par le curseur
            Vector3D moving_target = cam_target - cam.rpos;
            moving_target = moving_target - (moving_target % cur_dir) * cur_dir;
            float d = moving_target.sq();
            moving_target.y = 0.0f;
            float D = moving_target.sq();
            if (D == 0.0f)
                cam_has_target = false;
            else
                moving_target = d / D * moving_target;
            cam.rpos = moving_target + cam.rpos;
        }

        if (!selected)
            current_order = SIGNAL_ORDER_NONE;

        if (current_order != SIGNAL_ORDER_NONE && abs( sel_x[0] - sel_x[1]) < PICK_TOLERANCE && abs( sel_y[0] - sel_y[1]) < PICK_TOLERANCE)
            selecting=false;

        rope_selection = selecting && ( abs( sel_x[0] - sel_x[1]) >= PICK_TOLERANCE || abs( sel_y[0] - sel_y[1]) >= PICK_TOLERANCE);

        if (selected && (!IsOnGUI || IsOnMinimap))
        {
            bool builders=false;
            bool canattack=false;
            bool canreclamate=false;
            bool canresurrect=false;
            for (uint16 e = 0; e < units.index_list_size; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                {
                    builders|=unit_manager.unit_type[units.unit[i].type_id].Builder;
                    canattack|=unit_manager.unit_type[units.unit[i].type_id].canattack;
                    canreclamate|=unit_manager.unit_type[units.unit[i].type_id].CanReclamate;
                    canresurrect|=unit_manager.unit_type[units.unit[i].type_id].canresurrect;
                }
            }
            int pointing = 0;
            if (!IsOnGUI)
            {
                pointing = units.pick(&cam);		// Sur quoi le curseur est-il pointé??
                if (pointing == -1) 				// Is the cursor on a rock, tree, ...?
                {
                    Vector3D cur_pos = cursor_on_map(&cam,map,IsOnMinimap);
                    int px = ((int)(cur_pos.x + map->map_w_d)) >> 3;
                    int py = ((int)(cur_pos.z + map->map_h_d)) >> 3;
                    if (px>=0 && px<map->bloc_w_db && py>=0 && py<map->bloc_h_db && (map->view_map->line[py>>1][px>>1] & (1<<players.local_human_id)) )
                    {
                        int idx = -map->map_data[py][px].unit_idx - 2;				// Basic check
                        if (idx<0 || features.feature[idx].type<0)
                        {
                            units.last_on = -1;
                            for (short int dy = -7 ; dy < 8 ; ++dy)	// Look for things like metal patches
                            {
                                if (py + dy >= 0 && py + dy < map->bloc_h_db)
                                {
                                    for (short int dx = -7 ; dx < 8; ++dx)
                                    {
                                        if (px + dx >= 0 && px + dx < map->bloc_w_db)
                                        {
                                            if (map->map_data[py+dy][px+dx].stuff >= 0)
                                            {
                                                idx = map->map_data[py+dy][px+dx].stuff;
                                                if (features.feature[idx].type >=0
                                                    && feature_manager.feature[ features.feature[idx].type ].footprintx + 1 >= (abs(dx) << 1)
                                                    && feature_manager.feature[ features.feature[idx].type ].footprintz + 1 >= (abs(dy) << 1))
                                                {
                                                    units.last_on = -idx - 2;
                                                    dy = 800;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                            units.last_on = -idx - 2;
                    }
                    pointing = units.last_on;
                }
            }
            else
                pointing = units.pick_minimap();


            if (!rope_selection)
            {
                if (pointing < -1 && canreclamate && feature_manager.feature[features.feature[ -pointing - 2 ].type].reclaimable && build == -1)	cursor_type = CURSOR_RECLAIM;
                if (pointing < -1 && canresurrect && feature_manager.feature[features.feature[ -pointing - 2 ].type].reclaimable && build == -1 && CURSOR_REVIVE != CURSOR_RECLAIM)	cursor_type = CURSOR_REVIVE;
            }

            if (pointing >= 0 && !rope_selection) 	// S'il y a quelque chose sous le curseur
            {
                cursor_type=CURSOR_CROSS;
                bool can_be_captured = false;
                if (units.unit[pointing].owner_id!=players.local_human_id)
                {
                    can_be_captured = true;
                    if (canattack)
                        cursor_type=CURSOR_ATTACK;
                    else
                    {
                        if (canreclamate)
                            cursor_type = CURSOR_RECLAIM;
                        else
                            cursor_type = CURSOR_CANT_ATTACK;
                    }
                }
                else
                    if (units.unit[pointing].port[BUILD_PERCENT_LEFT]>0.0f && builders)
                        cursor_type = CURSOR_REPAIR;

                switch(current_order)
                {
                    case SIGNAL_ORDER_CAPTURE:	cursor_type=CURSOR_CAPTURE;	break;
                    case SIGNAL_ORDER_MOVE:		cursor_type=CURSOR_MOVE;	break;
                    case SIGNAL_ORDER_PATROL:	cursor_type=CURSOR_PATROL;	break;
                    case SIGNAL_ORDER_GUARD:	cursor_type=CURSOR_GUARD;	break;
                    case SIGNAL_ORDER_DGUN:		cursor_type=CURSOR_ATTACK;	break;
                    case SIGNAL_ORDER_ATTACK:	cursor_type=CURSOR_ATTACK;	break;
                    case SIGNAL_ORDER_RECLAM:	cursor_type=CURSOR_RECLAIM;	break;
                    case SIGNAL_ORDER_LOAD:		cursor_type=CURSOR_LOAD;	break;
                    case SIGNAL_ORDER_UNLOAD:	cursor_type=CURSOR_UNLOAD;	break;
                    case SIGNAL_ORDER_REPAIR:	cursor_type=CURSOR_REPAIR;	break;
                }

                if (mouse_b!=1 && omb3==1)
                {
                    if (cursor_type==CURSOR_ATTACK)
                    {
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            uint32 commandfire = current_order == SIGNAL_ORDER_DGUN ? MISSION_FLAG_COMMAND_FIRE : 0;
                            i = units.idx_list[e];
                            units.unit[i].lock();
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel && unit_manager.unit_type[units.unit[i].type_id].canattack
                                && ( unit_manager.unit_type[units.unit[i].type_id].BMcode || !unit_manager.unit_type[units.unit[i].type_id].Builder))
                            {
                                if (( unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 0 ] && unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 0 ]->stockpile)
                                    || ( unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 1 ] && unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 1 ]->stockpile)
                                    || ( unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 2 ] && unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 2 ]->stockpile))
                                    commandfire = MISSION_FLAG_COMMAND_FIRE;
                                if (TA3D_SHIFT_PRESSED)
                                    units.unit[i].add_mission(MISSION_ATTACK,&(units.unit[pointing].Pos),false,0,&(units.unit[pointing]),NULL,commandfire);
                                else
                                    units.unit[i].set_mission(MISSION_ATTACK,&(units.unit[pointing].Pos),false,0,true,&(units.unit[pointing]),NULL,commandfire);
                            }
                            units.unit[i].unlock();
                        }
                        if (!TA3D_SHIFT_PRESSED)	current_order=SIGNAL_ORDER_NONE;
                    }
                    else if (cursor_type == CURSOR_CAPTURE && can_be_captured) {
                        for (uint16 e = 0 ; e < units.index_list_size ; e++)
                        {
                            units.lock();
                            i = units.idx_list[e];
                            units.unlock();
                            units.unit[i].lock();
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel && unit_manager.unit_type[units.unit[i].type_id].CanCapture)
                            {
                                if (TA3D_SHIFT_PRESSED)
                                    units.unit[i].add_mission( MISSION_CAPTURE, &(units.unit[pointing].Pos), false, 0, &(units.unit[pointing]), NULL);
                                else
                                    units.unit[i].set_mission( MISSION_CAPTURE, &(units.unit[pointing].Pos), false, 0, true, &(units.unit[pointing]), NULL);
                            }
                            units.unit[i].unlock();
                        }
                        if (!TA3D_SHIFT_PRESSED)
                            current_order = SIGNAL_ORDER_NONE;
                    }
                    else if (cursor_type==CURSOR_REPAIR) {
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            units.lock();
                            i = units.idx_list[e];
                            units.unlock();
                            units.unit[i].lock();
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel
                                && unit_manager.unit_type[units.unit[i].type_id].Builder && unit_manager.unit_type[units.unit[i].type_id].BMcode)
                            {
                                if (!TA3D_SHIFT_PRESSED)
                                    units.unit[ i ].play_sound("repair");
                                if (TA3D_SHIFT_PRESSED)
                                    units.unit[i].add_mission(MISSION_REPAIR,&(units.unit[pointing].Pos),false,0,&(units.unit[pointing]));
                                else
                                    units.unit[i].set_mission(MISSION_REPAIR,&(units.unit[pointing].Pos),false,0,true,&(units.unit[pointing]));
                            }
                            units.unit[i].unlock();
                        }
                        if (!TA3D_SHIFT_PRESSED)
                            current_order=SIGNAL_ORDER_NONE;
                    }
                    else if (cursor_type==CURSOR_RECLAIM) {
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            units.lock();
                            i = units.idx_list[e];
                            units.unlock();
                            units.unit[i].lock();
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel
                                && unit_manager.unit_type[units.unit[i].type_id].CanReclamate && unit_manager.unit_type[units.unit[i].type_id].BMcode)
                            {
                                if (TA3D_SHIFT_PRESSED)
                                    units.unit[i].add_mission(MISSION_RECLAIM,&(units.unit[pointing].Pos),false,0,&(units.unit[pointing]));
                                else
                                    units.unit[i].set_mission(MISSION_RECLAIM,&(units.unit[pointing].Pos),false,0,true,&(units.unit[pointing]));
                            }
                            units.unit[i].unlock();
                        }
                        if (!TA3D_SHIFT_PRESSED)	current_order=SIGNAL_ORDER_NONE;
                    }
                    else if (cursor_type==CURSOR_GUARD) {	// Le curseur donne un ordre
                        units.give_order_guard(players.local_human_id,pointing,!TA3D_SHIFT_PRESSED);
                        if (!TA3D_SHIFT_PRESSED)
                            current_order=SIGNAL_ORDER_NONE;
                    }
                    else if (cursor_type==CURSOR_LOAD) {	// Le curseur donne un ordre
                        units.give_order_load(players.local_human_id,pointing,!TA3D_SHIFT_PRESSED);
                        if (!TA3D_SHIFT_PRESSED)
                            current_order=SIGNAL_ORDER_NONE;
                    }
                }
            }
            else
            {
                if (!rope_selection)
                {
                    switch(current_order)
                    {
                        case SIGNAL_ORDER_CAPTURE:	cursor_type=CURSOR_CAPTURE;	break;
                        case SIGNAL_ORDER_MOVE:		cursor_type=CURSOR_MOVE;	break;
                        case SIGNAL_ORDER_PATROL:	cursor_type=CURSOR_PATROL;	break;
                        case SIGNAL_ORDER_GUARD:	cursor_type=CURSOR_GUARD;	break;
                        case SIGNAL_ORDER_DGUN:		cursor_type=CURSOR_ATTACK;	break;
                        case SIGNAL_ORDER_ATTACK:	cursor_type=CURSOR_ATTACK;	break;
                        case SIGNAL_ORDER_RECLAM:	cursor_type=CURSOR_RECLAIM;	break;
                        case SIGNAL_ORDER_LOAD:		cursor_type=CURSOR_LOAD;	break;
                        case SIGNAL_ORDER_UNLOAD:	cursor_type=CURSOR_UNLOAD;	break;
                        case SIGNAL_ORDER_REPAIR:	cursor_type=CURSOR_REPAIR;	break;
                    }

                    if (mouse_b != 1 && omb3 == 1)
                    {
                        if (cursor_type == CURSOR_ATTACK)
                        {
                            Vector3D cursor_pos(cursor_on_map(&cam, map, IsOnMinimap));
                            for (uint16 e = 0; e < units.index_list_size; ++e)
                            {
                                uint32 commandfire = current_order == SIGNAL_ORDER_DGUN ? MISSION_FLAG_COMMAND_FIRE : 0;
                                i = units.idx_list[e];
                                units.unit[i].lock();
                                if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel && unit_manager.unit_type[units.unit[i].type_id].canattack
                                    && ( unit_manager.unit_type[units.unit[i].type_id].BMcode || !unit_manager.unit_type[units.unit[i].type_id].Builder))
                                {
                                    if (( unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 0 ] && unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 0 ]->stockpile)
                                        || ( unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 1 ] && unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 1 ]->stockpile)
                                        || ( unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 2 ] && unit_manager.unit_type[ units.unit[i].type_id ].weapon[ 2 ]->stockpile))
                                        commandfire = MISSION_FLAG_COMMAND_FIRE;
                                    if (TA3D_SHIFT_PRESSED)
                                        units.unit[i].add_mission(MISSION_ATTACK,&(cursor_pos),false,0,NULL,NULL,commandfire);
                                    else
                                        units.unit[i].set_mission(MISSION_ATTACK,&(cursor_pos),false,0,true,NULL,NULL,commandfire);
                                }
                                units.unit[i].unlock();
                            }
                            if (!TA3D_SHIFT_PRESSED)
                                current_order = SIGNAL_ORDER_NONE;
                        }
                    }
                }
            }
        }

        if (cursor_type==CURSOR_REVIVE && CURSOR_REVIVE != CURSOR_RECLAIM && !rope_selection && mouse_b != 1 && omb3 == 1 && ( !IsOnGUI || IsOnMinimap)) // The cursor orders to resurrect a wreckage
        {
            Vector3D cur_pos=cursor_on_map(&cam,map,IsOnMinimap);
            int idx = -units.last_on - 2;
            if (idx>=0 && features.feature[ idx ].type >= 0 && feature_manager.feature[ features.feature[ idx ].type ].reclaimable)
            {
                for (uint16 e = 0; e < units.index_list_size; ++e)
                {
                    units.lock();
                    i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel
                        && unit_manager.unit_type[units.unit[i].type_id].canresurrect && unit_manager.unit_type[units.unit[i].type_id].BMcode)
                    {
                        if (TA3D_SHIFT_PRESSED)
                            units.unit[i].add_mission(MISSION_REVIVE,&cur_pos,false,idx,NULL);
                        else
                            units.unit[i].set_mission(MISSION_REVIVE,&cur_pos,false,idx,true,NULL);
                    }
                    units.unit[i].unlock();
                }
            }
            if (!TA3D_SHIFT_PRESSED)
                current_order=SIGNAL_ORDER_NONE;
        }

        // The cursor orders to reclaim something
        if (cursor_type == CURSOR_RECLAIM && !rope_selection && mouse_b != 1 && omb3 == 1 && (!IsOnGUI || IsOnMinimap)) 
        {
            Vector3D cur_pos(cursor_on_map(&cam,map,IsOnMinimap));
            int idx = -units.last_on - 2;
            if (idx >= 0 && features.feature[ idx ].type >= 0 && feature_manager.feature[ features.feature[ idx ].type ].reclaimable)
            {
                for (uint16 e = 0; e < units.index_list_size; ++e)
                {
                    units.lock();
                    i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel
                        && unit_manager.unit_type[units.unit[i].type_id].CanReclamate && unit_manager.unit_type[units.unit[i].type_id].BMcode)
                    {
                        if (TA3D_SHIFT_PRESSED)
                            units.unit[i].add_mission(MISSION_RECLAIM,&cur_pos,false,idx,NULL);
                        else
                            units.unit[i].set_mission(MISSION_RECLAIM,&cur_pos,false,idx,true,NULL);
                    }
                    units.unit[i].unlock();
                }
            }
            if (!TA3D_SHIFT_PRESSED)
                current_order=SIGNAL_ORDER_NONE;
        }

        if (cursor_type==CURSOR_UNLOAD && !rope_selection && mouse_b != 1 && omb3 == 1 && ( !IsOnGUI || IsOnMinimap))	// The cursor orders to unload units
        {
            units.give_order_unload(players.local_human_id,cursor_on_map(&cam,map,IsOnMinimap),!TA3D_SHIFT_PRESSED);
            if (!TA3D_SHIFT_PRESSED)
                current_order=SIGNAL_ORDER_NONE;
        }

        if (cursor_type==CURSOR_MOVE && !rope_selection && mouse_b != 1 && omb3 == 1 && ( !IsOnGUI || IsOnMinimap)) 	// The cursor orders to move
        {
            units.give_order_move(players.local_human_id, cursor_on_map(&cam, map, IsOnMinimap), !TA3D_SHIFT_PRESSED);
            if (!TA3D_SHIFT_PRESSED)
                current_order=SIGNAL_ORDER_NONE;
        }

        // The cursor orders to patrol
        if (cursor_type == CURSOR_PATROL && !rope_selection && mouse_b != 1 && omb3 == 1 && ( !IsOnGUI || IsOnMinimap))
        {
            units.give_order_patrol(players.local_human_id,cursor_on_map(&cam,map,IsOnMinimap),!TA3D_SHIFT_PRESSED);
            if (!TA3D_SHIFT_PRESSED)
                current_order = SIGNAL_ORDER_NONE;
        }

        // The cursor orders to build something
        if (build >= 0 && cursor_type == CURSOR_DEFAULT && mouse_b != 1 && omb3 == 1 && !IsOnGUI)
        {
            Vector3D target=cursor_on_map(&cam,map);
            sel_x[1] = ((int)(target.x)+map->map_w_d)>>3;
            sel_y[1] = ((int)(target.z)+map->map_h_d)>>3;

            int d = Math::Max(abs( sel_x[1] - sel_x[0]), abs( sel_y[1] - sel_y[0]));

            int ox = sel_x[0] + 0xFFFF;
            int oy = sel_y[0] + 0xFFFF;

            for (int c = 0; c <= d; ++c)
            {
                target.x = sel_x[0] + (sel_x[1] - sel_x[0]) * c / Math::Max(d, 1);
                target.z = sel_y[0] + (sel_y[1] - sel_y[0]) * c / Math::Max(d, 1);

                if (abs( ox - (int)target.x) < unit_manager.unit_type[ build ].FootprintX
                    && abs( oy - (int)target.z) < unit_manager.unit_type[ build ].FootprintZ)	continue;
                ox = (int)target.x;
                oy = (int)target.z;

                target.y = Math::Max(map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[ build ].FootprintX, unit_manager.unit_type[ build ].FootprintZ),map->sealvl);
                target.x = target.x * 8.0f - map->map_w_d;
                target.z = target.z * 8.0f - map->map_h_d;

                can_be_there = can_be_built( target, map, build, players.local_human_id);

                if (can_be_there)
                {
                    units.give_order_build(players.local_human_id,build,target,!( TA3D_SHIFT_PRESSED || c != 0));
                    build_order_given = true;
                }
            }
            if (build_order_given)
            {
                if (!TA3D_SHIFT_PRESSED)
                    build=-1;
                sound_manager->playTDFSound( "OKTOBUILD", "sound" , NULL);
            }
            else
                sound_manager->playTDFSound( "NOTOKTOBUILD", "sound" , NULL);
        }
        else
        {
            if (build>=0 && cursor_type==CURSOR_DEFAULT && mouse_b == 1 && omb3 != 1 && !IsOnGUI)// Giving the order to build a row
            {
                Vector3D target(cursor_on_map(&cam, map));
                sel_x[0] = ((int)(target.x)+map->map_w_d)>>3;
                sel_y[0] = ((int)(target.z)+map->map_h_d)>>3;
            }
        }

        if (!TA3D_SHIFT_PRESSED && build_order_given)
            build=-1;

        if (build == -1)
            build_order_given = false;

        if (cursor_type!=CURSOR_DEFAULT && mouse_b!=2 && omb3 ==2 && !IsOnGUI && TA3D_SHIFT_PRESSED) // Remove commands from queue
        {
            Vector3D target = cursor_on_map(&cam,map);
            target.x = ((int)(target.x)+map->map_w_d) >> 3;
            target.z = ((int)(target.z)+map->map_h_d) >> 3;
            target.x = target.x*8.0f-map->map_w_d;
            target.z = target.z*8.0f-map->map_h_d;
            target.y = Math::Max(map->get_unit_h(target.x,target.z),map->sealvl);
            units.remove_order(players.local_human_id,target);
        }

        if (mouse_b!=1 && omb3==1 && !TA3D_SHIFT_PRESSED && ( !IsOnGUI || IsOnMinimap))
            current_order=SIGNAL_ORDER_NONE;

        //---------------------------------	Code de sélection d'unités

        if (!IsOnGUI)
        {
            if (mouse_b == 2 && omb3 != 2) // Deselect units
            {
                if (mouse_b == 2 && current_order != SIGNAL_ORDER_NONE && current_order != SIGNAL_ORDER_MOVE)
                    current_order = SIGNAL_ORDER_NONE;
                else
                {
                    selecting = false;
                    if (mouse_b == 1)
                    {
                        selecting=true;
                        sel_x[0]=mouse_x;
                        sel_y[0]=mouse_y;
                    }
                    if (build >= 0)
                    {
                        if ((!TA3D_SHIFT_PRESSED && !(mouse_b==2 && omb3!=2)) || (mouse_b==2 && omb3!=2))
                            build = -1; // leave build mode
                    }
                    else
                    {
                        selected = false;
                        cur_sel = -1;
                        cur_sel_index = -1;
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if (units.unit[i].owner_id == players.local_human_id) // On peut désélectionner les morts, ça ne change rien :-)
                                units.unit[i].sel = false;
                        }
                    }
                }
            }
        }

        if (build == -1 && (!IsOnGUI || (selecting && (mouse_y<32 || mouse_y>SCREEN_H-32)) || IsOnMinimap)) // Si le curseur est dans la zone de jeu
        {
            if ((mouse_b!=1 && selecting) || ( IsOnMinimap && mouse_b == 1 && omb3 != 1))// Récupère les unités présentes dans la sélection
            {
                bool skip = false;
                if ((abs( sel_x[0] - sel_x[1]) < PICK_TOLERANCE && abs(sel_y[0] - sel_y[1]) < PICK_TOLERANCE) || IsOnMinimap)
                {
                    if (cursor_type == CURSOR_DEFAULT || cursor_type == CURSOR_CROSS)
                    {
                        int pointing = IsOnMinimap ? units.pick_minimap() : units.pick(&cam);		// Sélectionne une unité sur clic
                        if (!TA3D_SHIFT_PRESSED)
                            for (uint16 e = 0; e < units.index_list_size; ++e)
                            {
                                i = units.idx_list[e];
                                if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id)
                                    units.unit[i].sel=false;
                            }
                        if (pointing>=0 && units.unit[pointing].port[BUILD_PERCENT_LEFT]==0.0f)		// On ne sélectionne pas les unités en construction
                            units.unit[pointing].sel^=true;			// Sélectionne/Désélectionne si l'unité est déjà sélectionnée en appuyant sur SHIFT
                        selected=false;
                        for (uint16 e=0;e<units.index_list_size;e++) {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id)
                                selected|=units.unit[i].sel;
                        }
                    }
                    else
                        skip = true;
                }
                else
                    selected = units.select(&cam,sel_x,sel_y);		// Séléction au lasso

                if (!skip)
                {
                    if (selected)			// In order to refresh GUI
                        old_sel = false;
                    cur_sel=-1;
                    cur_sel_index=-1;
                    for (uint16 e=0;e<units.index_list_size && cur_sel!=-2;e++)
                    {
                        i = units.idx_list[e];
                        if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                            cur_sel= (cur_sel==-1) ? i : -2;
                    }
                    if (cur_sel>=0)
                    {
                        cur_sel_index=cur_sel;
                        cur_sel=units.unit[cur_sel].type_id;
                        // Let's do some noise
                        units.unit[ cur_sel_index ].play_sound( "select1");
                    }
                }
            }
            selecting = false;
            if (mouse_b==1 && !IsOnMinimap)
            {
                if (omb3 != 1)
                {
                    sel_x[0] = mouse_x;
                    sel_y[0] = mouse_y;
                }
                sel_x[1] = mouse_x;
                sel_y[1] = mouse_y;
                selecting = true;
            }
        }
        else
            selecting = false;


        omb3 = mouse_b;
        amx  = mouse_x;
        amy  = mouse_y;

        if (IsOnGUI && !IsOnMinimap)
            cursor_type=CURSOR_DEFAULT;

        if (!IsOnGUI && ( cursor_type == CURSOR_DEFAULT || units.last_on == -1))
        {
            units.pick(&cam);		// Let's see what's under the cursor

            if (units.last_on == -1) // Is the cursor on a rock, tree, ...?
            {
                Vector3D cur_pos(cursor_on_map(&cam, map, IsOnMinimap));
                int px=((int)(cur_pos.x+map->map_w_d))>>3;
                int py=((int)(cur_pos.z+map->map_h_d))>>3;
                if (px>=0 && px<map->bloc_w_db && py>=0 && py<map->bloc_h_db && (map->view_map->line[py>>1][px>>1] & (1<<players.local_human_id)))
                {
                    int idx = -map->map_data[py][px].unit_idx - 2;				// Basic check
                    if (idx < 0 || features.feature[idx].type < 0)
                    {
                        units.last_on = -1;
                        for (short int dy = -7; dy < 8; ++dy) // Look for things like metal patches
                        {
                            if (py + dy >= 0 && py + dy < map->bloc_h_db)
                            {
                                for (short int dx = -7; dx < 8; ++dx)
                                {
                                    if (px + dx >= 0 && px + dx < map->bloc_w_db)
                                    {
                                        if (map->map_data[py+dy][px+dx].stuff >= 0)
                                        {
                                            idx = map->map_data[py+dy][px+dx].stuff;
                                            if (features.feature[idx].type >= 0
                                                && feature_manager.feature[ features.feature[idx].type ].footprintx + 1 >= (abs(dx) << 1)
                                                && feature_manager.feature[ features.feature[idx].type ].footprintz + 1 >= (abs(dy) << 1))
                                            {
                                                units.last_on = -idx - 2;
                                                dy = 800;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                        units.last_on = -idx - 2;
                }
            }
        }

        // Select CTRL_* category units
        if (TA3D_CTRL_PRESSED && (key[KEY_C] || key[KEY_F] || key[KEY_V] || key[KEY_B]))
        {
            String check_cat = "CTRL_";
            if (key[KEY_C])
                check_cat += "C";
            else
                if (key[KEY_F])
                    check_cat += "F";
                else
                    if (key[KEY_V])
                        check_cat += "V";
                    else
                        if (key[KEY_B])
                            check_cat += "B";
            for (uint16 e = 0 ; e < units.index_list_size; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].build_percent_left == 0.0f) {
                    if (unit_manager.unit_type[units.unit[i].type_id].checkCategory( check_cat.c_str()))
                        units.unit[i].sel=true;
                    else if (!TA3D_SHIFT_PRESSED)
                        units.unit[i].sel=false;
                }
            }
            cur_sel = -1;
            cur_sel_index = -1;
            build = -1;
            for (uint16 e = 0; e < units.index_list_size && cur_sel != -2; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
                    cur_sel= (cur_sel==-1) ? i : -2;
            }
            selected=(cur_sel!=-1);
            if (cur_sel>=0) {
                cur_sel_index=cur_sel;
                cur_sel=units.unit[cur_sel].type_id;
            }
        }
        else if (TA3D_CTRL_PRESSED && key[KEY_Z]) {		// Séletionne toutes les unités dont le type est déjà sélectionné / Select units of the same type
            bool *sel_type = new bool[unit_manager.nb_unit];
            for (i = 0; i < unit_manager.nb_unit; ++i)
                sel_type[i] = false;
            for (uint16 e = 0; e < units.index_list_size; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                    sel_type[units.unit[i].type_id]=true;
            }
            for (uint16 e=0;e<units.index_list_size; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].build_percent_left == 0.0f && units.unit[i].owner_id==players.local_human_id && sel_type[units.unit[i].type_id])
                    units.unit[i].sel=true;
            }
            cur_sel = -1;
            cur_sel_index = -1;
            for (uint16 e = 0; e < units.index_list_size && cur_sel!= -2; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                    cur_sel= (cur_sel==-1) ? i : -2;
            }
            selected=(cur_sel != -1);
            build=-1;
            if (cur_sel >= 0)
            {
                cur_sel_index=cur_sel;
                cur_sel=units.unit[cur_sel].type_id;
            }
            delete[] sel_type;
        }
        else if (TA3D_CTRL_PRESSED && key[KEY_A]) // Select all the player's units
        {
            for (uint16 e = 0 ; e < units.index_list_size; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].port[BUILD_PERCENT_LEFT] == 0.0f && units.unit[i].owner_id == players.local_human_id)
                    units.unit[i].sel=true;
            }
            cur_sel = -1;
            cur_sel_index = -1;
            for (uint16 e = 0; e < units.index_list_size && cur_sel != -2; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
                    cur_sel= (cur_sel==-1) ? i : -2;
            }
            selected = (cur_sel != -1);
            build = -1;
            if (cur_sel >= 0)
            {
                cur_sel_index=cur_sel;
                cur_sel=units.unit[cur_sel].type_id;
            }
        }
        else
            if (TA3D_CTRL_PRESSED) // Formation de groupes d'unités
            {
                int grpe=-1;
                if (key[KEY_0])	grpe=0;
                if (key[KEY_1])	grpe=1;
                if (key[KEY_2])	grpe=2;
                if (key[KEY_3])	grpe=3;
                if (key[KEY_4])	grpe=4;
                if (key[KEY_5])	grpe=5;
                if (key[KEY_6])	grpe=6;
                if (key[KEY_7])	grpe=7;
                if (key[KEY_8])	grpe=8;
                if (key[KEY_9])	grpe=9;

                if (grpe >= 0)
                {
                    grpe = 1 << grpe;
                    for (uint16 e=0;e<units.index_list_size; ++e)
                    {
                        i = units.idx_list[e];
                        if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id)
                        {
                            if (units.unit[i].sel)
                                units.unit[i].groupe|=grpe;
                            else if (!TA3D_SHIFT_PRESSED)
                                units.unit[i].groupe&=~grpe;
                        }
                    }
                }
            }
            else
            {
                if (key[KEY_ALT]) // Restauration de groupes d'unités
                {
                    int grpe=-1;
                    if (key[KEY_0])	grpe=0;
                    if (key[KEY_1])	grpe=1;
                    if (key[KEY_2])	grpe=2;
                    if (key[KEY_3])	grpe=3;
                    if (key[KEY_4])	grpe=4;
                    if (key[KEY_5])	grpe=5;
                    if (key[KEY_6])	grpe=6;
                    if (key[KEY_7])	grpe=7;
                    if (key[KEY_8])	grpe=8;
                    if (key[KEY_9])	grpe=9;

                    if (grpe>=0)
                    {
                        build=-1;
                        grpe = 1 << grpe;
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id)
                            {
                                if (units.unit[i].groupe&grpe)
                                    units.unit[i].sel=true;
                                else if (!TA3D_SHIFT_PRESSED)
                                    units.unit[i].sel=false;
                            }
                        }
                    }

                    cur_sel = -1;
                    cur_sel_index = -1;
                    for (uint16 e = 0; e < units.index_list_size && cur_sel != -2; ++e)
                    {
                        i = units.idx_list[e];
                        if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                            cur_sel= (cur_sel==-1) ? i : -2;
                    }
                    selected = (cur_sel != -1);
                    if (cur_sel >= 0)
                    {
                        cur_sel_index=cur_sel;
                        cur_sel=units.unit[cur_sel].type_id;
                    }
                }
            }

        /*--------------bloc regroupant ce qui est relatif au temps-------------------*/

        // That code was rewritten multithreaded
        if (!lp_CONFIG->pause)
        {
            float timetosimulate = dt * units.apparent_timefactor;// Physics calculations take place here
            wind_change = false; // Don't try to run following code in separate thread
            features.move(timetosimulate, map);					// Animate objects
            fx_manager.move(timetosimulate);
        }

        /*----------------------------------------------------------------------------*/

        cam_h = cam.rpos.y - map->get_unit_h(cam.rpos.x,cam.rpos.z);

        cam.zfar = 600.0f + Math::Max((cam_h-150.0f)*2.0f, 0.0f);

        if (freecam && cam.rpos.y<map->sealvl)
        {
            FogD=0.03f;
            FogNear=0.0f;
            FogMode=GL_EXP;

            FogColor[0]=0.0f;
            FogColor[1]=0.0f;
            FogColor[2]=0.3f;
            FogColor[3]=1.0f;
        }
        else
        {
            FogD=0.3f;
            FogNear=cam.zfar*0.5f;
            FogMode=GL_LINEAR;

            memcpy( FogColor, sky_data->FogColor, sizeof( float) * 4);
        }

        gfx->SetDefState();
        glClearColor(FogColor[0],FogColor[1],FogColor[2],FogColor[3]);

        glFogi (GL_FOG_MODE, FogMode);
        glFogfv (GL_FOG_COLOR, FogColor);
        glFogf (GL_FOG_DENSITY, FogD);
        glHint (GL_FOG_HINT, GL_NICEST);
        glFogf (GL_FOG_START, FogNear);
        glFogf (GL_FOG_END, cam.zfar);

        // Dessine les reflets sur l'eau
        if (g_useProgram && g_useFBO && lp_CONFIG->water_quality>=2 && map->water && !map->ota_data.lavaworld && !reflection_drawn_last_time) 
        {

            reflection_drawn_last_time = true;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

            glViewport(0, 0, 512, 512);

            gfx->ReInitAllTex();
            glColor4f(1.0f,1.0f,1.0f,1.0f);
            glDisable(GL_BLEND);

            double eqn[4]= { 0.0f, 1.0f, 0.0f, -map->sealvl };

            Camera refcam = cam;
            refcam.zfar *= 2.0f;
            refcam.mirror = true;
            refcam.mirrorPos = -2.0f*map->sealvl;

            refcam.setView();
            glClipPlane(GL_CLIP_PLANE1, eqn);
            glEnable(GL_CLIP_PLANE1);

            sun.Set(refcam);
            sun.Enable();

            refcam.zfar*=100.0f;
            refcam.setView();
            glColor4f(1.0f,1.0f,1.0f,1.0f);
            glEnable(GL_TEXTURE_2D);
            if (lp_CONFIG->render_sky)
            {
                glBindTexture(GL_TEXTURE_2D,sky);
                glDisable(GL_LIGHTING);
                glFogi (GL_FOG_MODE, FogMode);
                glFogfv (GL_FOG_COLOR, FogColor);
                glFogf (GL_FOG_DENSITY, FogD);
                glHint (GL_FOG_HINT, GL_NICEST);
                glFogf (GL_FOG_START, FogNear);
                glFogf (GL_FOG_END, refcam.zfar);
                glDepthMask(GL_FALSE);
                if (spherical_sky)
                {
                    glCullFace(GL_FRONT);
                    glTranslatef(cam.rpos.x,-map->sealvl,cam.rpos.z);
                    glRotatef( sky_angle, 0.0f, 1.0f, 0.0f);
                    float scale_factor = 15.0f * ( cam.rpos.y + cam.shakeVector.y + sky_obj.w) / sky_obj.w;
                    glScalef( scale_factor, scale_factor, scale_factor);
                    sky_obj.draw();
                }
                else
                {
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0f,map->map_h*0.002f);				glVertex3f(-2.0f*map->map_w,300.0f,2.0f*map->map_h);
                    glTexCoord2f(map->map_w*0.002f,map->map_h*0.002f);	glVertex3f(2.0f*map->map_w,300.0f,2.0f*map->map_h);
                    glTexCoord2f(map->map_w*0.002f,0.0f);				glVertex3f(2.0f*map->map_w,300.0f,-2.0f*map->map_h);
                    glTexCoord2f(0.0f,0.0f);							glVertex3f(-2.0f*map->map_w,300.0f,-2.0f*map->map_h);
                    glEnd();
                }
            }
            glDepthMask(GL_TRUE);
            glFogi (GL_FOG_MODE, FogMode);
            glFogfv (GL_FOG_COLOR, FogColor);
            glFogf (GL_FOG_DENSITY, FogD);
            glHint (GL_FOG_HINT, GL_NICEST);
            glFogf (GL_FOG_START, FogNear);
            glFogf (GL_FOG_END, refcam.zfar);
            glEnable(GL_CULL_FACE);
            glEnable(GL_LIGHTING);
            glEnable(GL_FOG);
            glCullFace(GL_FRONT);
            refcam.zfar = (500.0f + (cam_h - 150.0f) * 2.0f) * 2.0f;
            refcam.setView();

            if (cam.rpos.y <= gfx->low_def_limit && lp_CONFIG->water_quality == 4)
            {

                if (lp_CONFIG->wireframe)
                    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

                map->draw(&refcam, 1 << players.local_human_id,  false, 0.0f, t,
                          dt * units.apparent_timefactor,
                          false, false, false);

                if (lp_CONFIG->wireframe)
                    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

                // Dessine les éléments "2D" / "sprites"
                features.draw(refcam);		
                // Dessine les unités / draw units
                units.draw(&refcam,map,false,true,false,lp_CONFIG->height_line);			

                glDisable(GL_CULL_FACE);
                // Dessine les objets produits par les armes / draw weapons
                weapons.draw(&refcam,map);
                // Dessine les particules
                particle_engine.draw(&refcam,map->map_w,map->map_h,map->bloc_w,map->bloc_h,map->view);
                // Effets spéciaux en surface / fx above water
                fx_manager.draw(refcam, map, map->sealvl);		
            }

            glDisable(GL_CLIP_PLANE1);

            gfx->ReInitAllTex( true);

            glColor4f(1.0f,1.0f,1.0f,1.0f);
            glDisable(GL_BLEND);

            glBindTexture(GL_TEXTURE_2D,reflectex);								// Store what's on screen for reflection effect
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 512, 512, 0);

            glViewport(0, 0, SCREEN_W, SCREEN_H);

            gfx->SetDefState();

            glFogi (GL_FOG_MODE, FogMode);
            glFogfv (GL_FOG_COLOR, FogColor);
            glFogf (GL_FOG_DENSITY, FogD);
            glHint (GL_FOG_HINT, GL_NICEST);
            glFogf (GL_FOG_START, FogNear);
            glFogf (GL_FOG_END, cam.zfar);
        }
        else
            reflection_drawn_last_time = false;

        gfx->SetDefState();
        glClearColor(FogColor[0],FogColor[1],FogColor[2],FogColor[3]);
        glClear(GL_DEPTH_BUFFER_BIT);		// Clear screen

        cam.setView();

        sun.Set(cam);
        sun.Enable();

        cam.setView();

        glDisable(GL_FOG);
        glColor3f( 0.0f, 0.0f, 0.0f);				// Black background
        glDisable( GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);
        glBegin( GL_QUADS);
        glVertex3i( -map->map_w, 0, map->map_h);
        glVertex3i( map->map_w, 0, map->map_h);
        glVertex3i( map->map_w, 0, -map->map_h);
        glVertex3i( -map->map_w, 0, -map->map_h);
        glEnd();
        glDepthMask(GL_TRUE);
        glEnable(GL_FOG);

        cam.zfar*=100.0f;
        cam.setView();
        glColor4f(1.0f,1.0f,1.0f,1.0f);
        glEnable(GL_TEXTURE_2D);
        if (lp_CONFIG->render_sky)
        {
            glBindTexture(GL_TEXTURE_2D,sky);
            glDisable(GL_LIGHTING);
            glDepthMask(GL_FALSE);
            if (spherical_sky)
            {
                glTranslatef( cam.rpos.x, cam.rpos.y+cam.shakeVector.y, cam.rpos.z);
                glRotatef( sky_angle, 0.0f, 1.0f, 0.0f);
                sky_obj.draw();
                if (!sky_obj.full)
                {
                    glScalef( 1.0f, -1.0f, 1.0f);
                    glCullFace( GL_FRONT);
                    sky_obj.draw();
                    glCullFace( GL_BACK);
                }
            }
            else
            {
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f,0.0f);							glVertex3f(-2.0f*map->map_w,300.0f,-2.0f*map->map_h);
                glTexCoord2f(map->map_w*0.002f,0.0f);				glVertex3f(2.0f*map->map_w,300.0f,-2.0f*map->map_h);
                glTexCoord2f(map->map_w*0.002f,map->map_h*0.002f);	glVertex3f(2.0f*map->map_w,300.0f,2.0f*map->map_h);
                glTexCoord2f(0.0f,map->map_h*0.002f);				glVertex3f(-2.0f*map->map_w,300.0f,2.0f*map->map_h);
                glEnd();
            }
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_LIGHTING);
        glEnable(GL_FOG);
        cam.zfar = 500.0f + (cam_h - 150.0f) * 2.0f;
        cam.setView();

        if (lp_CONFIG->wireframe)
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

        map->draw(&cam,1<<players.local_human_id,false,0.0f,t,dt*units.apparent_timefactor);

        if (lp_CONFIG->wireframe)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

        features.draw(cam);		// Dessine les éléments "2D"

        /*----------------------------------------------------------------------------------------------*/

        // Dessine les unités sous l'eau / Draw units which are under water
        units.draw(&cam, map, true, false, true, lp_CONFIG->height_line);			

        // Dessine les objets produits par les armes sous l'eau / Draw weapons which are under water
        weapons.draw(&cam, map, true);

        if (map->water)
        {
            // Effets spéciaux sous-marins / Draw fx which are under water
            fx_manager.draw(cam, map, map->sealvl, true); 
        }


        if (map->water)
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            gfx->ReInitAllTex(true);

            if (!g_useProgram || !g_useFBO || lp_CONFIG->water_quality < 2)
            {
                gfx->set_alpha_blending();
                if (lp_CONFIG->water_quality==1) // lp_CONFIG->water_quality=1
                {
                    glColor4f(1.0f,1.0f,1.0f,0.5f);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glClientActiveTextureARB(GL_TEXTURE0_ARB);

                    map->draw(&cam,1,true,map->sealvl,t,dt*lp_CONFIG->timefactor);
                }
                else 	// lp_CONFIG->water_quality=0
                {
                    glColor4f(1.0f,1.0f,1.0f,0.5f);
                    glDisable(GL_LIGHTING);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D,map->low_tex);

                    cam.setView();
                    glTranslatef(0.0f, map->sealvl, map->sea_dec);
                    water_obj.draw(t,cam.rpos.x,cam.rpos.z,false);
                    glColor4f(1.0f,1.0f,1.0f,0.75f);

                    glEnable(GL_LIGHTING);
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    gfx->ReInitTexSys();
                    glEnable(GL_TEXTURE_2D);
                }
                gfx->unset_alpha_blending();
            }
            else
            {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glDisable(GL_LIGHTING);

                // First pass of water rendering, store reflection vector
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,water_FBO);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,first_pass,0);

                //			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                glViewport(0,0,512,512);

                glActiveTextureARB(GL_TEXTURE0_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,map->lava_map);
                glClientActiveTextureARB(GL_TEXTURE0_ARB);

                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,water);
                glClientActiveTextureARB(GL_TEXTURE1_ARB);

                if (lp_CONFIG->water_quality == 2)
                {
                    water_pass1_low.on();
                    water_pass1_low.setvar1i("lava",0);
                    water_pass1_low.setvar1i("map",1);
                    water_pass1_low.setvar1f("t",t);
                    water_pass1_low.setvar2f("factor",water_obj.w/map->map_w,water_obj.w/map->map_h);
                }
                else
                {
                    water_pass1.on();
                    water_pass1.setvar1i("lava",0);
                    water_pass1.setvar1i("map",1);
                    water_pass1.setvar1f("t",t);
                    water_pass1.setvar2f("factor",water_obj.w/map->map_w,water_obj.w/map->map_h);
                }

                cam.setView();
                glTranslatef(0.0f,map->sealvl,0.0f);
                water_obj.draw(t,cam.rpos.x,cam.rpos.z,true);

                if (lp_CONFIG->water_quality == 2)
                    water_pass1_low.off();
                else
                    water_pass1.off();

                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,second_pass,0);					// Second pass of water rendering, store viewing vector

                //			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon
                glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                glActiveTextureARB(GL_TEXTURE0_ARB);
                glDisable(GL_TEXTURE_2D);

                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);

                water_pass2.on();

                cam.setView();
                glTranslatef(0.0f,map->sealvl,0.0f);
                water_obj.draw(t,cam.rpos.x,cam.rpos.z,true);

                water_pass2.off();

                if (lp_CONFIG->water_quality > 2)
                {
                    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,water_color,0);					// Third pass of water rendering, store water color

                    glClear(GL_DEPTH_BUFFER_BIT);		// Efface la texture tampon

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D,map->low_tex);

                    cam.setView();
                    glTranslatef( 0.0f, map->sealvl, map->sea_dec);
                    water_obj.draw(t,cam.rpos.x,cam.rpos.z,false);
                }

                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

                glViewport(0, 0, SCREEN_W, SCREEN_H);

                float logw = log((float)SCREEN_W) / log(2.0f);
                float logh = log((float)SCREEN_H) / log(2.0f);
                int wx = logw>(int)logw ? (int)logw+1 : (int)logw;
                int wy = logh>(int)logh ? (int)logh+1 : (int)logh;
                wx = 1 << wx;
                wy = 1 << wy;
                glBindTexture(GL_TEXTURE_2D,transtex);								// Store what's on screen for transparency effect
                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, wx, wy, 0);

                glEnable(GL_STENCIL_TEST);											// Draw basic water in order to have correct texture mapping
                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS,128, 0xffffffff);
                glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

                glActiveTextureARB(GL_TEXTURE0_ARB);
                glDisable(GL_TEXTURE_2D);
                glClientActiveTextureARB(GL_TEXTURE0_ARB);

                cam.setView();
                glTranslatef(0.0f,map->sealvl,0.0f);
                water_obj.draw(t,cam.rpos.x,cam.rpos.z,true);

                glDisable(GL_STENCIL_TEST);

                glMatrixMode(GL_TEXTURE);
                glLoadIdentity();
                glMatrixMode(GL_MODELVIEW);

                glEnable(GL_LIGHTING);

                glActiveTextureARB(GL_TEXTURE0_ARB);
                if (map->ota_data.lavaworld)
                    glBindTexture(GL_TEXTURE_2D,sky);
                else
                    glBindTexture(GL_TEXTURE_2D,reflectex);
                glEnable(GL_TEXTURE_2D);

                glActiveTextureARB(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_2D,transtex);
                glEnable(GL_TEXTURE_2D);

                glActiveTextureARB(GL_TEXTURE2_ARB);
                glBindTexture(GL_TEXTURE_2D,first_pass);
                glEnable(GL_TEXTURE_2D);

                glActiveTextureARB(GL_TEXTURE3_ARB);
                glBindTexture(GL_TEXTURE_2D,second_pass);
                glEnable(GL_TEXTURE_2D);

                if (lp_CONFIG->water_quality == 2)
                {
                    water_shader.on();
                    water_shader.setvar1i("sky",0);
                    water_shader.setvar1i("rtex",1);
                    water_shader.setvar1i("bump",2);
                    water_shader.setvar1i("view",3);
                    water_shader.setvar2f("coef", (float)SCREEN_W / wx, (float)SCREEN_H / wy);
                }
                else
                {
                    glActiveTextureARB(GL_TEXTURE4_ARB);
                    glBindTexture(GL_TEXTURE_2D,water_color);
                    glEnable(GL_TEXTURE_2D);

                    water_shader_reflec.on();
                    water_shader_reflec.setvar1i("sky",0);
                    water_shader_reflec.setvar1i("rtex",1);
                    water_shader_reflec.setvar1i("bump",2);
                    water_shader_reflec.setvar1i("view",3);
                    water_shader_reflec.setvar1i("water_color",4);
                    water_shader_reflec.setvar2f("coef", (float)SCREEN_W / wx, (float)SCREEN_H / wy);
                }

                glColor4f(1.0f,1.0f,1.0f,1.0f);
                glDisable(GL_DEPTH_TEST);
                cam.setView();
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
                glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
                Vector3D cam_pos = cam.rpos + cam.shakeVector;
                glBegin(GL_QUADS);
                Vector3D P = cam_pos + 1.1f * (cam.dir + 0.75f * cam.up-cam.widthFactor * cam.side);
                glTexCoord2f(0.0f,1.0f);	glVertex3f(P.x,P.y,P.z);

                P = cam_pos + 1.1f*(cam.dir+0.75f*cam.up+cam.widthFactor*cam.side);
                glTexCoord2f(1.0f,1.0f);	glVertex3f(P.x,P.y,P.z);

                P = cam_pos + 1.1f*(cam.dir-0.75f*cam.up+cam.widthFactor*cam.side);
                glTexCoord2f(1.0f,0.0f);	glVertex3f(P.x,P.y,P.z);

                P = cam_pos + 1.1f*(cam.dir-0.75f*cam.up-cam.widthFactor*cam.side);
                glTexCoord2f(0.0f,0.0f);	glVertex3f(P.x,P.y,P.z);
                glEnd();
                glDisable(GL_STENCIL_TEST);
                glEnable(GL_DEPTH_TEST);

                if (lp_CONFIG->water_quality==2)
                    water_shader.off();
                else
                    water_shader_reflec.off();
            }
            gfx->ReInitAllTex( true);
        }

        if (build >= 0 && !IsOnGUI)	// Display the building we want to build (with nice selection quads)
        {
            Vector3D target=cursor_on_map(&cam,map);
            sel_x[1] = ((int)(target.x)+map->map_w_d)>>3;
            sel_y[1] = ((int)(target.z)+map->map_h_d)>>3;

            if (mouse_b != 1 && omb3 != 1)
            {
                sel_x[0] = sel_x[1];
                sel_y[0] = sel_y[1];
            }

            int d = Math::Max(abs(sel_x[1] - sel_x[0]), abs( sel_y[1] - sel_y[0]));

            int ox = sel_x[0] + 0xFFFF;
            int oy = sel_y[0] + 0xFFFF;

            for (int c = 0; c <= d; ++c)
            {
                target.x = sel_x[0] + (sel_x[1] - sel_x[0]) * c / Math::Max(d, 1);
                target.z = sel_y[0] + (sel_y[1] - sel_y[0]) * c / Math::Max(d, 1);

                if (abs( ox - (int)target.x) < unit_manager.unit_type[ build ].FootprintX
                    && abs( oy - (int)target.z) < unit_manager.unit_type[ build ].FootprintZ)	continue;
                ox = (int)target.x;
                oy = (int)target.z;

                target.y = Math::Max(map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[ build ].FootprintX, unit_manager.unit_type[ build ].FootprintZ),map->sealvl);
                target.x = target.x * 8.0f - map->map_w_d;
                target.z = target.z * 8.0f - map->map_h_d;

                can_be_there = can_be_built( target, map, build, players.local_human_id);

                cam.setView();
                glTranslatef(target.x,target.y,target.z);
                glScalef(unit_manager.unit_type[build].Scale,unit_manager.unit_type[build].Scale,unit_manager.unit_type[build].Scale);
                if (unit_manager.unit_type[build].model)
                {
                    gfx->ReInitAllTex( true);
                    if (can_be_there)
                        glColor4f(1.0f,1.0f,1.0f,1.0f);
                    else
                        glColor4f(1.0f,0.0f,0.0f,0.0f);
                    unit_manager.unit_type[build].model->draw(0.0f,NULL,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,players.local_human_id,false);
                    glColor4f(1.0f,1.0f,1.0f,1.0f);
                }
                cam.setView();
                glTranslatef(target.x,target.y,target.z);
                float DX = (unit_manager.unit_type[build].FootprintX<<2);
                float DZ = (unit_manager.unit_type[build].FootprintZ<<2);
                float red=1.0f, green=0.0f;
                if (can_be_there)
                {
                    green = 1.0f;
                    red   = 0.0f;
                }
                glDisable(GL_CULL_FACE);
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glBegin(GL_QUADS);
                glColor4f(red,green,0.0f,1.0f);
                glVertex3f(-DX,0.0f,-DZ);			// First quad
                glVertex3f(DX,0.0f,-DZ);
                glColor4f(red,green,0.0f,0.0f);
                glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
                glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

                glColor4f(red,green,0.0f,1.0f);
                glVertex3f(-DX,0.0f,-DZ);			// Second quad
                glVertex3f(-DX,0.0f,DZ);
                glColor4f(red,green,0.0f,0.0f);
                glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
                glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

                glColor4f(red,green,0.0f,1.0f);
                glVertex3f(DX,0.0f,-DZ);			// Third quad
                glVertex3f(DX,0.0f,DZ);
                glColor4f(red,green,0.0f,0.0f);
                glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
                glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);

                glColor4f(red,green,0.0f,1.0f);
                glVertex3f(-DX,0.0f,DZ);			// Fourth quad
                glVertex3f(DX,0.0f,DZ);
                glColor4f(red,green,0.0f,0.0f);
                glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
                glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
                glEnd();
                glDisable(GL_BLEND);
                glEnable(GL_LIGHTING);
                glEnable(GL_CULL_FACE);
            }
        }


        // Dessine les unités non encore dessinées / Draw units which have not been drawn
        units.draw(&cam,map,false,false,true,lp_CONFIG->height_line);

        // Dessine les objets produits par les armes n'ayant pas été dessinés / Draw weapons which have not been drawn
        weapons.draw(&cam,map,false);


        if (selected && TA3D_SHIFT_PRESSED)
        {
            cam.setView();
            bool builders = false;
            for (uint16 e = 0; e < units.index_list_size; ++e)
            {
                i = units.idx_list[e];
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                {
                    builders |= unit_manager.unit_type[units.unit[i].type_id].Builder;
                    units.unit[i].show_orders();					// Dessine les ordres reçus par l'unité
                }
            }

            if (builders)
            {
                for (uint16 e = 0; e < units.index_list_size; ++e)
                {
                    i = units.idx_list[e];
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && !units.unit[i].sel
                        && unit_manager.unit_type[units.unit[i].type_id].Builder && unit_manager.unit_type[units.unit[i].type_id].BMcode)
                    {
                        units.unit[i].show_orders(true);					// Dessine les ordres reçus par l'unité
                    }
                }
            }
        }


        if (lp_CONFIG->shadow && cam.rpos.y<=gfx->low_def_limit)
        {
            if (lp_CONFIG->shadow_quality <= 1)
            {
                if (rotate_light)
                {
                    sun.Dir.x = -1.0f;
                    sun.Dir.y = 1.0f;
                    sun.Dir.z = 1.0f;
                    sun.Dir.unit();
                    Vector3D Dir(-sun.Dir);
                    Dir.x = cos(light_angle);
                    Dir.z = sin(light_angle);
                    Dir.unit();
                    sun.Dir = -Dir;
                    units.draw_shadow(&cam, Dir, map);
                }
                else
                {
                    sun.Dir.x = -1.0f;
                    sun.Dir.y = 1.0f;
                    sun.Dir.z = 1.0f;
                    sun.Dir.unit();
                    units.draw_shadow(&cam, -sun.Dir, map);
                }
            }
            else
            {
                float alpha = 1.0f - exp((1.0f / lp_CONFIG->shadow_quality) * log(0.5f));
                Vector3D Dir;
                if (rotate_light)
                {
                    sun.Dir.x = -1.0f;
                    sun.Dir.y = 1.0f;
                    sun.Dir.z = 1.0f;
                    sun.Dir.unit();
                    Dir = -sun.Dir;
                    Dir.x = cos(light_angle);
                    Dir.z = sin(light_angle);
                    Dir.unit();
                    sun.Dir = -Dir;
                }
                else
                {
                    sun.Dir.x = -1.0f;
                    sun.Dir.y = 1.0f;
                    sun.Dir.z = 1.0f;
                    sun.Dir.unit();
                    Dir = -sun.Dir;
                }
                for (int i = 0; i < lp_CONFIG->shadow_quality; ++i)
                {
                    Vector3D RDir(Dir);
                    RDir.x += cos(i * PI * 2.0f / lp_CONFIG->shadow_quality) * lp_CONFIG->shadow_r;
                    RDir.z += sin(i * PI * 2.0f / lp_CONFIG->shadow_quality) * lp_CONFIG->shadow_r;
                    RDir.unit();
                    units.draw_shadow(&cam, RDir, map, alpha);
                }
            }
        }

        particle_engine.draw(&cam,map->map_w,map->map_h,map->bloc_w,map->bloc_h,map->view);	// Dessine les particules

        if (!map->water)
            fx_manager.draw(cam, map, map->sealvl, true);		// Effets spéciaux en surface
        fx_manager.draw(cam, map, map->sealvl);		// Effets spéciaux en surface


        if (key[KEY_ESC] && !game_area.get_state( "esc_menu")) // Enter pause mode if we have to show the menu
        {
            lp_CONFIG->pause = true;
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (char*)"esc_menu.b_pause.hide", NULL, NULL);
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (char*)"esc_menu.b_resume.show", NULL, NULL);
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (char*)"esc_menu.show", NULL, NULL);
        }

        if (game_area.get_state("esc_menu.b_exit"))
        {
            exit_mode = -1;
            done = true;
        }

        if (game_area.get_state("esc_menu.b_save")) 	// Fill the file list
        {
            game_area.set_state("esc_menu.b_save", false);

            GUIOBJ *obj_file_list = game_area.get_object("save_menu.l_file");

            if (obj_file_list)
            {
                String::List file_list;
                Paths::Glob(file_list, TA3D::Paths::Savegames + "*.sav");
                file_list.sort();
                obj_file_list->Text.clear();
                obj_file_list->Text.reserve(file_list.size());
                for (String::List::const_iterator i = file_list.begin(); i != file_list.end(); ++i)
                    obj_file_list->Text.push_back(Paths::ExtractFileName(*i));
            }
        }

        if (game_area.get_state("save_menu.l_file")) // Click on the list
        {
            GUIOBJ *obj = game_area.get_object("save_menu.l_file");
            if (obj && obj->Pos >= 0 && obj->Pos < obj->Text.size())
                game_area.set_caption("save_menu.t_name", obj->Text[ obj->Pos]);
        }

        if (game_area.get_state("save_menu.b_save")) // Save the game
        {
            game_area.set_state("save_menu.b_save", false);
            String filename = game_area.get_caption("save_menu.t_name");
            if (!filename.empty())
            {
                filename = Paths::Savegames + Paths::Files::ReplaceExtension(filename, ".sav");
                save_game(filename, game_data); // Save the game
            }
            lp_CONFIG->pause = false;
        }

        if (key[KEY_TILDE] && !game_area.get_state("chat"))
        {
            if (!tilde)
                console.toggleShow();
            tilde = true;
        }
        else
            tilde = false;

        gfx->ReInitAllTex(true);

        gfx->set_2D_mode();		// Affiche console, infos,...

        old_cam_pos=cam.rpos;
        int signal = 0;
        if (!network_manager.isConnected() || network_manager.isServer())
        {
            signal = game_script.run(map,((float)(units.current_tick - script_timer)) / TICKS_PER_SEC,players.local_human_id);

            script_timer = units.current_tick;
        }
        else
            game_script.run(NULL, 0.0f, 0); // In client mode we only want to display text, pictures, ... everything drawn by the script on the server

        if (network_manager.isConnected() || signal == 0)
            signal = g_ta3d_network->get_signal();

        switch(signal)
        {
            case 0:				// Rien de spécial
                break;
            case -1:			// Fin du script
                if (!network_manager.isConnected() || network_manager.isServer())
                    game_script.stop();
                break;
            case -2:			// Pause
                break;
            case -3:			// Attente d'un évènement
                break;
            case 1:				// Fin de partie (match nul)
                done = true;
                exit_mode = EXIT_NONE;
                break;
            case 2:				// Fin de partie (victoire)
                done = true;
                exit_mode = EXIT_VICTORY;
                break;
            case 3:				// Fin de partie (défaite)
                done = !network_manager.isServer();			// Server can't leave, otherwise game stops
                exit_mode = EXIT_DEFEAT;
                break;
            case 4:				// Caméra en mode normal
                if (freecam)
                {
                    freecam = false;
                    r2 = 0.0f;
                }
                break;
            case 5:				// Caméra libre
                if (!freecam)
                    freecam = true;
                break;
        }

        if (selecting) // Affiche le rectangle de selection
        {
            glDisable(GL_TEXTURE_2D);
            glColor4f(0.0f,0.0f,0.0f,1.0f);
            gfx->rect(sel_x[0]+1,sel_y[0]+1,sel_x[1]+1,sel_y[1]+1);
            glColor4f(1.0f,1.0f,1.0f,1.0f);
            gfx->rect(sel_x[0],sel_y[0],sel_x[1],sel_y[1]);
        }

        if (cur_sel_index>=0 && cur_sel_index<units.max_unit && !(units.unit[cur_sel_index].flags & 1))
        {
            cur_sel=-1;
            cur_sel_index=-1;
            current_order=SIGNAL_ORDER_NONE;
        }

        int n = cur_sel;
        if (n == -1)
            n = -2;
        if (n>=0 && units.unit[cur_sel_index].port[BUILD_PERCENT_LEFT]>0.0f)		// Unité non terminée
            n = -1;
        int sel = -1;

        /*------------------- Draw GUI components -------------------------------------------------------*/

        if (current_gui != String( ta3dSideData.side_pref[players.side_view]) + "gen")
            unit_manager.unit_build_menu(n,omb2,dt, true);	// Draw GUI background
        else
            unit_manager.unit_build_menu(-1,omb2,dt, true);	// Draw GUI background

        game_area.draw();

        /*------------------- End of GUI drawings -------------------------------------------------------*/

        /*WND *current_wnd =*/ game_area.get_wnd(current_gui);
        if (current_gui != String( ta3dSideData.side_pref[players.side_view]) + "gen")
            sel = unit_manager.unit_build_menu(n,omb2,dt);	// Menu correspondant à l'unité / Unit's menu
        else
            sel = unit_manager.unit_build_menu(-1,omb2,dt);	// Menu correspondant à l'unité / Unit's menu
        if (sel == -2) // Crée des armes / build weapons
        {
            if (mouse_b == 1 && omb2 != 1)
            {
                if (TA3D_SHIFT_PRESSED)
                    units.unit[cur_sel_index].planned_weapons+=5.0f;
                else
                    units.unit[cur_sel_index].planned_weapons+=1.0f;
            }
            else
            {
                if (mouse_b == 2 && omb2 != 2)
                {
                    units.unit[cur_sel_index].planned_weapons -= (TA3D_SHIFT_PRESSED) ? 5.0f : 1.0f;
                    if (units.unit[cur_sel_index].planned_weapons < 0.0f)
                        units.unit[cur_sel_index].planned_weapons = 0.0f;
                }
            }
            sel = -1;
        }

        bool refresh_gui(false);

        if (!selected && !current_gui.empty())
        {
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".hide").c_str(), NULL, NULL);	// Hide it
            current_gui = "";
            old_sel = false;
        }
        if ((old_gui_sel >= 0 && old_gui_sel != n) || (!old_sel && !selected)) // Update GUI
        {
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + ".hide").c_str(), NULL, NULL);	// Hide it
            current_gui = "";
            old_sel = false;
        }
        if (n >= 0 && n != old_gui_sel)
        {
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".hide").c_str(), NULL, NULL);	// Hide it
            current_gui = String( unit_manager.unit_type[ n ].Unitname) + "1";
            if (game_area.get_wnd(current_gui) == NULL)
            {
                if (unit_manager.unit_type[ n ].nb_unit > 0)				// The default build page
                    current_gui = String( ta3dSideData.side_pref[players.side_view]) + "dl";
                else
                    current_gui = String( ta3dSideData.side_pref[players.side_view]) + "gen";
            }
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".show").c_str(), NULL, NULL);	// Show it
            refresh_gui = true;
        }
        if (n < 0 && ( selected && !old_sel))
        {
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".hide").c_str(), NULL, NULL);	// Hide it
            old_sel = true;
            current_gui = String( ta3dSideData.side_pref[players.side_view]) + "gen";
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".show").c_str(), NULL, NULL);	// Show it
            refresh_gui = true;
        }
        old_gui_sel = n;

        if (refresh_gui)
        {
            /*------------------- GUI update ----------------------------------------------------------------*/

            bool onoffable=false;
            bool canstop=false;
            bool canpatrol=false;
            bool canmove=false;
            bool canguard=false;
            bool canattack=false;
            bool canreclam=false;
            bool builders=false;			// For repair purposes only
            bool canload=false;
            bool cancapture=false;
            bool cancloak=false;
            bool candgun=false;
            int onoff_state = 0;
            byte sforder = 0;
            byte smorder = 0;

            for (uint16 e = 0; e < units.index_list_size; ++e)
            {
                units.lock();
                uint32 i = units.idx_list[e];
                units.unlock();
                units.unit[i].lock();
                if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
                {
                    onoffable |= unit_manager.unit_type[ units.unit[i].type_id ].onoffable;
                    canstop |= unit_manager.unit_type[ units.unit[i].type_id ].canstop;
                    canmove |= unit_manager.unit_type[ units.unit[i].type_id ].canmove;
                    canpatrol |= unit_manager.unit_type[ units.unit[i].type_id ].canpatrol;
                    canguard |= unit_manager.unit_type[ units.unit[i].type_id ].canguard;
                    canattack |= unit_manager.unit_type[ units.unit[i].type_id ].canattack;
                    canreclam |= unit_manager.unit_type[ units.unit[i].type_id ].CanReclamate;
                    builders |= unit_manager.unit_type[ units.unit[i].type_id ].Builder;
                    canload |= unit_manager.unit_type[ units.unit[i].type_id ].canload;
                    cancapture |= unit_manager.unit_type[ units.unit[i].type_id ].CanCapture;
                    cancloak |= unit_manager.unit_type[ units.unit[i].type_id ].CloakCost > 0;
                    candgun |= unit_manager.unit_type[ units.unit[i].type_id ].candgun;

                    if (unit_manager.unit_type[ units.unit[i].type_id ].canattack)
                        sforder |= units.unit[i].port[ STANDINGFIREORDERS ];
                    if (unit_manager.unit_type[ units.unit[i].type_id ].canmove)
                        smorder |= units.unit[i].port[ STANDINGMOVEORDERS ];
                    if (unit_manager.unit_type[ units.unit[i].type_id ].onoffable)
                        onoff_state |= units.unit[ i ].port[ ACTIVATION ] ? 2 : 1;
                }
                units.unit[i].unlock();
            }

            if (onoff_state == 0)
                onoff_state = 3;

            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "STOP", canstop);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVE", canmove);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "PATROL", canpatrol);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "DEFEND", canguard);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ATTACK", canattack);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "RECLAIM", canreclam);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "LOAD", canload);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "UNLOAD", canload);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "REPAIR", builders);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ONOFF", onoffable);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVEORD", canmove);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "FIREORD", canattack);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "CAPTURE", cancapture);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "CLOAK", cancloak);
            game_area.set_enable_flag(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "BLAST", candgun);

            game_area.set_enable_flag(current_gui + ".ARMSTOP", canstop);			// Alternate version to support mods
            game_area.set_enable_flag(current_gui + ".ARMMOVE", canmove);
            game_area.set_enable_flag(current_gui + ".ARMPATROL", canpatrol);
            game_area.set_enable_flag(current_gui + ".ARMDEFEND", canguard);
            game_area.set_enable_flag(current_gui + ".ARMATTACK", canattack);
            game_area.set_enable_flag(current_gui + ".ARMRECLAIM", canreclam);
            game_area.set_enable_flag(current_gui + ".ARMLOAD", canload);
            game_area.set_enable_flag(current_gui + ".ARMUNLOAD", canload);
            game_area.set_enable_flag(current_gui + ".ARMREPAIR", builders);
            game_area.set_enable_flag(current_gui + ".ARMONOFF", onoffable);
            game_area.set_enable_flag(current_gui + ".ARMMOVEORD", canmove);
            game_area.set_enable_flag(current_gui + ".ARMFIREORD", canattack);
            game_area.set_enable_flag(current_gui + ".ARMCAPTURE", cancapture);
            game_area.set_enable_flag(current_gui + ".ARMCLOAK", cancloak);
            game_area.set_enable_flag(current_gui + ".ARMBLAST", candgun);

            GUIOBJ *onoff_gui = game_area.get_object( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ONOFF");
            if (onoff_gui == NULL)
                onoff_gui = game_area.get_object( current_gui + ".ARMONOFF");

            if (onoff_gui)
                onoff_gui->current_state = onoff_state - 1;

            GUIOBJ *sorder_gui = game_area.get_object( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "FIREORD");
            if (sorder_gui == NULL)
                sorder_gui = game_area.get_object( current_gui + ".ARMFIREORD");

            if (sorder_gui)
                sorder_gui->current_state = sforder;

            sorder_gui = game_area.get_object( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVEORD");
            if (sorder_gui == NULL)
                sorder_gui = game_area.get_object( current_gui + ".ARMMOVEORD");

            if (sorder_gui)
                sorder_gui->current_state = smorder;

            if (canload)
            {
                I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "LOAD.show").c_str(), NULL, NULL);	// Show it
                I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "BLAST.hide").c_str(), NULL, NULL);	// Hide it
                I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".ARMLOAD.show").c_str(), NULL, NULL);	// Show it
                I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".ARMBLAST.hide").c_str(), NULL, NULL);	// Hide it
            }
            else
            {
                I_Msg(TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + "." + ta3dSideData.side_pref[players.side_view ] + "LOAD.hide").c_str(), NULL, NULL);	// Hide it
                I_Msg(TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + "." + ta3dSideData.side_pref[players.side_view ] + "BLAST.show").c_str(), NULL, NULL);	// Show it
                I_Msg(TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + ".ARMLOAD.hide").c_str(), NULL, NULL);	// Hide it
                I_Msg(TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + ".ARMBLAST.show").c_str(), NULL, NULL);	// Show it
            }

            if (current_gui != String(ta3dSideData.side_pref[ players.side_view]) + "gen")
            {
                String gen_gui;
                gen_gui << ta3dSideData.side_pref[players.side_view] << "gen";

                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "STOP", canstop);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVE", canmove);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "PATROL", canpatrol);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "DEFEND", canguard);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "ATTACK", canattack);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "RECLAIM", canreclam);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "LOAD", canload);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "UNLOAD", canload);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "REPAIR", builders);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "ONOFF", onoffable);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVEORD", canmove);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "FIREORD", canattack);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "CAPTURE", cancapture);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "CLOAK", cancloak);
                game_area.set_enable_flag( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "BLAST", candgun);

                game_area.set_enable_flag( gen_gui + ".ARMSTOP", canstop);
                game_area.set_enable_flag( gen_gui + ".ARMMOVE", canmove);
                game_area.set_enable_flag( gen_gui + ".ARMPATROL", canpatrol);
                game_area.set_enable_flag( gen_gui + ".ARMDEFEND", canguard);
                game_area.set_enable_flag( gen_gui + ".ARMATTACK", canattack);
                game_area.set_enable_flag( gen_gui + ".ARMRECLAIM", canreclam);
                game_area.set_enable_flag( gen_gui + ".ARMLOAD", canload);
                game_area.set_enable_flag( gen_gui + ".ARMUNLOAD", canload);
                game_area.set_enable_flag( gen_gui + ".ARMREPAIR", builders);
                game_area.set_enable_flag( gen_gui + ".ARMONOFF", onoffable);
                game_area.set_enable_flag( gen_gui + ".ARMMOVEORD", canmove);
                game_area.set_enable_flag( gen_gui + ".ARMFIREORD", canattack);
                game_area.set_enable_flag( gen_gui + ".ARMCAPTURE", cancapture);
                game_area.set_enable_flag( gen_gui + ".ARMCLOAK", cancloak);
                game_area.set_enable_flag( gen_gui + ".ARMBLAST", candgun);

                onoff_gui = game_area.get_object( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "ONOFF");
                if (onoff_gui == NULL)
                    onoff_gui = game_area.get_object( gen_gui + ".ARMONOFF");

                if (onoff_gui)
                    onoff_gui->current_state = onoff_state - 1;

                sorder_gui = game_area.get_object( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "FIREORD");
                if (sorder_gui == NULL)
                    sorder_gui = game_area.get_object( gen_gui + ".ARMFIREORD");

                if (sorder_gui)
                    sorder_gui->current_state = sforder;

                sorder_gui = game_area.get_object( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVEORD");
                if (sorder_gui == NULL)
                    sorder_gui = game_area.get_object( gen_gui + ".ARMMOVEORD");

                if (sorder_gui)
                    sorder_gui->current_state = smorder;

                if (canload)
                {
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "LOAD.show").c_str(), NULL, NULL);	// Show it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "BLAST.hide").c_str(), NULL, NULL);	// Hide it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + ".ARMLOAD.show").c_str(), NULL, NULL);	// Show it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + ".ARMBLAST.hide").c_str(), NULL, NULL);	// Hide it
                }
                else
                {
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "LOAD.hide").c_str(), NULL, NULL);	// Hide it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + "." + ta3dSideData.side_pref[players.side_view] + "BLAST.show").c_str(), NULL, NULL);	// Show it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + ".ARMLOAD.hide").c_str(), NULL, NULL);	// Hide it
                    I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( gen_gui + ".ARMBLAST.show").c_str(), NULL, NULL);	// Show it
                }
            }

            /*------------------- End of GUI update ---------------------------------------------------------*/
        }

        if (!current_gui.empty() && current_gui != String(ta3dSideData.side_pref[players.side_view]) + "gen") // Show information about units
            units.complete_menu(cur_sel_index, sel != -1 || units.last_on <= -2, false);
        else
            units.complete_menu(cur_sel_index, sel != -1 || units.last_on <= -2, true);

        int signal_order = 0;
        features.display_info( -units.last_on - 2);
        players.show_resources();

        /*------------------- GUI reacting code ---------------------------------------------------------*/

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ORDERS")
            || game_area.get_state(current_gui + ".ARMORDERS")) // Go to the order menu
        {
            game_area.set_state(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ORDERS", false);
            game_area.set_state(current_gui + ".ARMORDERS", false);				// Because of mod support
            sound_manager->playTDFSound( "ORDERSBUTTON", "sound" , NULL);
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + ".hide").c_str(), NULL, NULL);	// Hide it
            current_gui = String( ta3dSideData.side_pref[players.side_view]) + "gen";
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".show").c_str(), NULL, NULL);	// Show it
        }

        if (( game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "BUILD") ||
              game_area.get_state( current_gui + ".ARMBUILD")) && old_gui_sel >= 0) // Back to the build menu
        {
            game_area.set_state(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "BUILD", false);
            game_area.set_state(current_gui + ".ARMBUILD", false);
            sound_manager->playTDFSound( "BUILDBUTTON", "sound" , NULL);
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)(current_gui + ".hide").c_str(), NULL, NULL);	// Hide it
            current_gui.clear();
            current_gui << unit_manager.unit_type[old_gui_sel].Unitname << "1";
            if (game_area.get_wnd(current_gui) == NULL)
            {
                current_gui.clear();
                if (unit_manager.unit_type[old_gui_sel].nb_unit > 0) // The default build page
                    current_gui << ta3dSideData.side_pref[players.side_view] << "dl";
                else
                    current_gui << ta3dSideData.side_pref[players.side_view] << "gen";
            }
            I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)( current_gui + ".show").c_str(), NULL, NULL);	// Show it
        }

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "PREV")
            || game_area.get_state( current_gui + ".ARMPREV"))
        {
            sound_manager->playTDFSound( "NEXTBUILDMENU", "sound" , NULL);
            if (unit_manager.unit_type[ old_gui_sel ].nb_pages > 0)
                unit_manager.unit_type[ old_gui_sel ].page = (unit_manager.unit_type[ old_gui_sel ].page + unit_manager.unit_type[ old_gui_sel ].nb_pages-1)%unit_manager.unit_type[old_gui_sel].nb_pages;
        }
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "NEXT")
            || game_area.get_state( current_gui + ".ARMNEXT"))
        {
            sound_manager->playTDFSound( "NEXTBUILDMENU", "sound" , NULL);
            if (unit_manager.unit_type[ old_gui_sel ].nb_pages > 0)
                unit_manager.unit_type[ old_gui_sel ].page= (unit_manager.unit_type[ old_gui_sel ].page+1)%unit_manager.unit_type[ old_gui_sel ].nb_pages;
        }

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ONOFF")
            || game_area.get_state( current_gui + ".ARMONOFF")) // Toggle the on/off value
        {
            signal_order = SIGNAL_ORDER_ONOFF;
            GUIOBJ *onoff_obj = game_area.get_object( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ONOFF");
            if (onoff_obj == NULL)
                onoff_obj = game_area.get_object( current_gui + ".ARMONOFF");
            if (onoff_obj != NULL)
            {
                units.lock();
                for (uint16 e = 0 ; e < units.index_list_size ; ++e)
                {
                    uint16 i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel && unit_manager.unit_type[ units.unit[i].type_id ].onoffable)
                    {
                        if (onoff_obj->nb_stages > 1)
                        {
                            onoff_obj->current_state &= 1;
                            if (onoff_obj->current_state == 0)
                                units.unit[i].deactivate();
                            else
                                units.unit[i].activate();
                        }
                        else
                        {
                            if (units.unit[i].port[ACTIVATION])
                                units.unit[i].deactivate();
                            else
                                units.unit[i].activate();
                        }
                    }
                    units.unit[i].unlock();
                    units.lock();
                }
                units.unlock();
            }
        }

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "CLOAK")
            || game_area.get_state( current_gui + ".ARMCLOAK")) // Toggle the cloak value
        {
            GUIOBJ *cloak_obj = game_area.get_object( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "CLOAK");
            if (cloak_obj == NULL)
                cloak_obj = game_area.get_object( current_gui + ".ARMCLOAK");
            if (cloak_obj != NULL)
            {
                units.lock();
                for (uint16 e = 0; e < units.index_list_size; ++e)
                {
                    uint16 i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel && unit_manager.unit_type[ units.unit[i].type_id ].CloakCost > 0)
                    {
                        if (cloak_obj->nb_stages > 1)
                        {
                            cloak_obj->current_state &= 1;
                            units.unit[i].cloaking = cloak_obj->current_state != 0;
                        }
                        else
                            units.unit[i].cloaking ^= true;
                    }
                    units.unit[i].unlock();
                    units.lock();
                }
                units.unlock();
            }
        }

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "FIREORD")
            || game_area.get_state( current_gui + ".ARMFIREORD")) // Toggle the fireorder value
        {
            sound_manager->playTDFSound( "SETFIREORDERS", "sound" , NULL);
            GUIOBJ *sorder_obj = game_area.get_object(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "FIREORD");
            if (sorder_obj == NULL)
                sorder_obj = game_area.get_object(current_gui + ".ARMFIREORD");
            if (sorder_obj != NULL)
            {
                sorder_obj->current_state %= 3;
                units.lock();
                for (uint16 e = 0 ; e < units.index_list_size ; ++e)
                {
                    uint16 i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
                    {
                        units.unit[i].port[STANDINGFIREORDERS] = sorder_obj->current_state;
                        if (SFORDER_FIRE_AT_WILL != units.unit[i].port[STANDINGFIREORDERS])
                        {
                            for (short int f = 0; f < 3; ++f)
                                units.unit[i].weapon[f].state = WEAPON_FLAG_IDLE;
                            if (units.unit[i].mission && units.unit[i].mission->mission == MISSION_ATTACK && !( units.unit[i].mission->flags & MISSION_FLAG_COMMAND_FIRE))
                                units.unit[i].next_mission();
                        }
                    }
                    units.unit[i].unlock();
                    units.lock();
                }
                units.unlock();
            }
        }

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVEORD")
            || game_area.get_state( current_gui + ".ARMMOVEORD")) // Toggle the moveorder value
        {
            sound_manager->playTDFSound("SETMOVEORDERS", "sound" , NULL);
            GUIOBJ *sorder_obj = game_area.get_object(current_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVEORD");
            if (sorder_obj == NULL)
                sorder_obj = game_area.get_object(current_gui + ".ARMMOVEORD");
            if (sorder_obj != NULL)
            {
                sorder_obj->current_state %= 3;
                units.lock();
                for (uint16 e = 0 ; e < units.index_list_size; ++e)
                {
                    uint16 i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel)
                        units.unit[i].port[ STANDINGMOVEORDERS ] = sorder_obj->current_state;
                    units.unit[i].unlock();
                    units.lock();
                }
                units.unlock();
            }
        }

        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "MOVE")
            || game_area.get_state( current_gui + ".ARMMOVE"))														signal_order = SIGNAL_ORDER_MOVE;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "PATROL")
            || game_area.get_state( current_gui + ".ARMPATROL"))														signal_order = SIGNAL_ORDER_PATROL;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "STOP")
            || game_area.get_state( current_gui + ".ARMSTOP"))														signal_order = SIGNAL_ORDER_STOP;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "DEFEND")
            || game_area.get_state( current_gui + ".ARMDEFEND"))														signal_order = SIGNAL_ORDER_GUARD;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "ATTACK")
            || game_area.get_state( current_gui + ".ARMATTACK"))														signal_order = SIGNAL_ORDER_ATTACK;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "RECLAIM")
            || game_area.get_state( current_gui + ".ARMRECLAIM"))														signal_order = SIGNAL_ORDER_RECLAM;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "LOAD")
            || game_area.get_state( current_gui + ".ARMLOAD"))														signal_order = SIGNAL_ORDER_LOAD;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "UNLOAD")
            || game_area.get_state( current_gui + ".ARMUNLOAD"))														signal_order = SIGNAL_ORDER_UNLOAD;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "REPAIR")
            || game_area.get_state( current_gui + ".ARMREPAIR"))														signal_order = SIGNAL_ORDER_REPAIR;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "CAPTURE")
            || game_area.get_state( current_gui + ".ARMCAPTURE"))														signal_order = SIGNAL_ORDER_CAPTURE;
        if (game_area.get_state( current_gui + "." + ta3dSideData.side_pref[players.side_view] + "BLAST")
            || game_area.get_state( current_gui + ".ARMBLAST"))														signal_order = SIGNAL_ORDER_DGUN;

        /*------------------- End of GUI reacting code --------------------------------------------------*/

        switch(signal_order)
        {
            case SIGNAL_ORDER_CAPTURE:
            case SIGNAL_ORDER_DGUN:
            case SIGNAL_ORDER_MOVE:
            case SIGNAL_ORDER_PATROL:
            case SIGNAL_ORDER_GUARD:
            case SIGNAL_ORDER_ATTACK:
            case SIGNAL_ORDER_RECLAM:
            case SIGNAL_ORDER_LOAD:
            case SIGNAL_ORDER_UNLOAD:
            case SIGNAL_ORDER_REPAIR:
                sound_manager->playTDFSound( "IMMEDIATEORDERS", "sound" , NULL);
                current_order = signal_order;
                break;
            case SIGNAL_ORDER_STOP:
                sound_manager->playTDFSound( "IMMEDIATEORDERS", "sound" , NULL);
                units.lock();
                for (uint16 e = 0; e < units.index_list_size; ++e)
                {
                    uint16 i = units.idx_list[e];
                    units.unlock();
                    units.unit[i].lock();
                    if ((units.unit[i].flags & 1) && units.unit[i].owner_id == players.local_human_id && units.unit[i].sel && unit_manager.unit_type[ units.unit[i].type_id ].canstop)
                        units.unit[i].set_mission(MISSION_STOP);
                    units.unit[i].unlock();
                    units.lock();
                }
                units.unlock();
            case SIGNAL_ORDER_ONOFF:
                current_order=SIGNAL_ORDER_NONE;
                break;
        }
        if (sel>=0 || (mouse_b&2))
            current_order=SIGNAL_ORDER_NONE;

        if (sel >= 0 && mouse_b == 2 && omb2 != 2)
        {
            units.unit[cur_sel_index].lock();
            MISSION*  cur = units.unit[cur_sel_index].mission;
            MISSION** old = &(units.unit[cur_sel_index].mission);
            int nb(1);
            if (TA3D_SHIFT_PRESSED)
                nb = 5;
            if (cur)
            {
                old = &(cur->next);
                cur = cur->next;
            }
            while (cur)
            {
                if ((cur->mission==MISSION_BUILD && cur->data == sel) || cur->mission == MISSION_STOP) // Efface un ordre
                {
                    if (cur->mission==MISSION_BUILD)
                        --nb;
                    *old = cur->next;
                    free(cur);
                    cur = *old;
                    if (nb == 0)
                        break;
                    continue;
                }
                old = &(cur->next);
                cur = cur->next;
            }
            cur = units.unit[cur_sel_index].mission;
            if (nb > 0 && cur != NULL && cur->mission == MISSION_BUILD_2 && cur->data==sel)
            {
                sint32 prev = -1;
                for (i = units.nb_unit-1; i>=0; --i)
                {
                    if (units.idx_list[i] == ((UNIT*)(units.unit[cur_sel_index].mission->p))->idx)
                    {
                        prev = i;
                        break;
                    }
                }
                if (prev >= 0)
                    units.kill(((UNIT*)(units.unit[cur_sel_index].mission->p))->idx,map,prev);
                units.unit[cur_sel_index].next_mission();
            }
            units.unit[cur_sel_index].unlock();
        }

        if (sel >= 0 && mouse_b == 1 && omb2 != 1)
        {
            build = sel;
            sound_manager->playTDFSound( "ADDBUILD", "sound" , NULL);
            if (!unit_manager.unit_type[cur_sel].BMcode) // S'il s'agit d'un bâtiment
            {
                if (TA3D_SHIFT_PRESSED)
                    for (short int i = 0; i < 5; ++i)
                        units.give_order_build(players.local_human_id, build, units.unit[cur_sel_index].Pos, false);
                else
                    units.give_order_build(players.local_human_id, build, units.unit[cur_sel_index].Pos, false);
                build = -1;
            }
        }

        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        gfx->drawtexture(freecam ? freecam_on : freecam_off,32,SCREEN_H-64,95,SCREEN_H);
        glDisable(GL_BLEND);

        if (mouse_x >= 32 && mouse_x <= 95 && mouse_y >= SCREEN_H - 64 && omb2 == 0)
        {
            if (mouse_b==1)
            {
                freecam ^= true;
                if (!freecam)
                    r2 = 0.0f;
            }
            else
            {
                if (mouse_b == 2 && !freecam) // Reset default view
                {
                    camera_zscroll = -0.00001f;
                    r1 = -lp_CONFIG->camera_def_angle - 0.00001f;
                }
            }
        }

        omb2=mouse_b;

        int last_on = units.last_on;

        map->draw_mini(0,0,128,128,&cam,1<<players.local_human_id);	// Mini-carte
        units.draw_mini(map->map_w,map->map_h,map->mini_w,map->mini_h,map->map_data);
        weapons.draw_mini(map->map_w,map->map_h,map->mini_w,map->mini_h);

        if (!freecam && mouse_b == 4) // Moving the cam around
        {
            gfx->set_alpha_blending();
            gfx->set_color( 0xFFFFFFFF);
            gfx->drawtexture( circle_texture, gfx->width * 0.45f, gfx->height * 0.45f, gfx->width * 0.55f, gfx->height * 0.55f);
            gfx->drawtexture( arrow_texture, gfx->width * 0.4f, 0.0f, gfx->width * 0.6f, gfx->height * 0.05f);
            gfx->drawtexture( arrow_texture, gfx->width * 0.4f, gfx->height, gfx->width * 0.6f, gfx->height * 0.95f);
            gfx->drawtexture_flip( arrow_texture, 0.0f, gfx->height * 0.4f, gfx->width * 0.05f, gfx->height * 0.6f);
            gfx->drawtexture_flip( arrow_texture, gfx->width, gfx->height * 0.4f, gfx->width * 0.95f, gfx->height * 0.6f);
            gfx->unset_alpha_blending();
        }

        if (view_dbg)
        {
            glDisable(GL_TEXTURE_2D);
            glColor3f(1.0f,1.0f,1.0f);
            glBegin(GL_POINTS);
            float rw = 128.0f * map->mini_w / 252 / map->bloc_w;
            float rh = 128.0f * map->mini_h / 252 / map->bloc_h;
            float dw = 64.0f - 0.5f * map->bloc_w * rw;
            float dh = 64.0f - 0.5f * map->bloc_h * rh;
            for (int y = 0; y < map->bloc_h; ++y)
            {
                for (int x = 0; x < map->bloc_w; ++x)
                {
                    if (map->view[y][x])
                        glVertex2f(x*rw+dw,y*rh+dh);
                }
            }
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }

        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
        glEnable(GL_BLEND);
        if (show_script)
        {
            if (cur_sel>=0 && unit_manager.unit_type[cur_sel].script)
            {
                float Y(32.0f);
                gfx->print(gfx->normal_font,128.0f,Y,0.0f,0xFFFFFFFF,format("%d scripts",unit_manager.unit_type[cur_sel].script->nb_script));
                Y += 9.0f;
                for (i = 0; i < unit_manager.unit_type[cur_sel].script->nb_script; ++i)
                {
                    if (units.unit[cur_sel_index].is_running(i))
                        gfx->print(gfx->normal_font, 128.0f, Y, 0.0f, 0xFFFFFFFF, format("%d %s (on)", i, unit_manager.unit_type[cur_sel].script->names[i].c_str()));
                    else
                        gfx->print(gfx->normal_font, 128.0f, Y, 0.0f, 0xFFFFFFFF, format("%d %s (off)", i, unit_manager.unit_type[cur_sel].script->names[i].c_str()));
                    Y += 9.0f;
                }
            }
        }

        if (show_model && cur_sel>=0 && unit_manager.unit_type[cur_sel].model)
            unit_manager.unit_type[cur_sel].model->print_struct(32.0f,128.0f,gfx->normal_font);

        if (internal_name && last_on >= 0)
        {
            units.unit[ last_on ].lock();
            if (units.unit[ last_on ].type_id >= 0 && unit_manager.unit_type[ units.unit[ last_on ].type_id ].Unitname!=NULL)
                gfx->print(gfx->normal_font,128.0f,32.0f,0.0f,0xFFFFFFFF,format("internal name %s",unit_manager.unit_type[ units.unit[ last_on ].type_id ].Unitname));
            units.unit[ last_on ].unlock();
        }
        else
        {
            if (internal_name && cur_sel >= 0 && unit_manager.unit_type[cur_sel].Unitname!=NULL)
                gfx->print(gfx->normal_font,128.0f,32.0f,0.0f,0xFFFFFFFF,format("internal name %s",unit_manager.unit_type[cur_sel].Unitname));
        }

        if (internal_idx && last_on >= 0)
            gfx->print(gfx->normal_font,128.0f,32.0f,0.0f,0xFFFFFFFF,format("idx = %d", last_on));
        else
        {
            if (internal_idx && cur_sel_index >= 0)
                gfx->print(gfx->normal_font,128.0f,32.0f,0.0f,0xFFFFFFFF,format("idx = %d", cur_sel_index));
        }

        if (unit_info>0.0f && unit_info_id>=0)
            unit_manager.unit_type[unit_info_id].show_info(unit_info,gfx->TA_font);

        if (last_on != -1 && show_mission_info) // Sur les unités sélectionnées
        {
            const char *unit_info[]={"MISSION_STANDBY","MISSION_VTOL_STANDBY","MISSION_GUARD_NOMOVE","MISSION_MOVE","MISSION_BUILD","MISSION_BUILD_2","MISSION_STOP","MISSION_REPAIR","MISSION_ATTACK",
                "MISSION_PATROL","MISSION_GUARD","MISSION_RECLAIM","MISSION_LOAD","MISSION_UNLOAD","MISSION_STANDBY_MINE"};
            float y = 32.0f;
            for (i = 0; i < units.max_unit; ++i)
            {
                units.unit[i].lock();
                if ((units.unit[i].flags & 1) && last_on == i)
                {
                    if (units.unit[i].mission != NULL && units.unit[i].mission->mission<=0x0E)
                    {
                        gfx->print(gfx->normal_font,128.0f,y,0.0f,0xFFFFFFFF,format("MISSION: %s",unit_info[units.unit[i].mission->mission]));
                        String flags = "";
                        if (units.unit[i].mission->flags & MISSION_FLAG_CAN_ATTACK)	flags += "CAN_ATTACK; ";
                        if (units.unit[i].mission->flags & MISSION_FLAG_SEARCH_PATH)	flags += "SEARCH_PATH; ";
                        if (units.unit[i].mission->flags & MISSION_FLAG_TARGET_WEAPON)	flags += "TARGET_WEAPON; ";
                        if (units.unit[i].mission->flags & MISSION_FLAG_COMMAND_FIRE)	flags += "COMMAND_FIRE; ";
                        if (units.unit[i].mission->flags & MISSION_FLAG_MOVE)	flags += "MOVE; ";
                        if (units.unit[i].mission->flags & MISSION_FLAG_REFRESH_PATH)	flags += "REFRESH_PATH; ";
                        y += gfx->normal_font.height();
                        gfx->print(gfx->normal_font,128.0f,y,0.0f,0xFFFFFFFF,format("FLAGS: %s", flags.c_str()));
                    }
                    else
                        gfx->print(gfx->normal_font,128.0f,y,0.0f,0xFFFFFFFF,format("MISSION: NONE"));
                    y += gfx->normal_font.height();
                }
                units.unit[i].unlock();
            }
        }

        glDisable(GL_BLEND);

        if (show_timefactor > 0.0f)
        {
            String value = format("x %f", lp_CONFIG->timefactor);
            if (value.find('.') != String::npos)
                value.resize(value.find('.') + 2);
            if (show_timefactor > 0.5f)
                gfx->print( gfx->TA_font, gfx->width - (int)gfx->TA_font.length( value)>>1, SCREEN_H-80, 0.0f, 0xFFFFFFFF, value);
            else
            {
                uint32 c = (uint32)(511.0f * show_timefactor) * 0x01010101;
                gfx->print( gfx->TA_font, gfx->width - (int)gfx->TA_font.length( value)>>1, SCREEN_H-80, 0.0f, c, value);
            }
            show_timefactor -= dt;
        }

        ta3d_network.draw(); // Draw network related stuffs (ie: chat messages, ...)

        char *cmd = NULL;
        // Draw the console
        if (!shoot || video_shoot)
            cmd = console.draw(gfx->TA_font, dt, gfx->TA_font.height());

        float Y=0.0f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);

        if (lp_CONFIG->showfps)
        {
            gfx->print(gfx->TA_font,0.0f,Y,0.0f,0xFFFFFFFF,format("fps: %d",fps));
            Y+=9.0f;
        }

        glDisable(GL_BLEND);

        if (mouse_b!=4 || TA3D_CTRL_PRESSED)
            draw_cursor();

        if (shoot)
        {
            BITMAP *shoot_bmp = create_bitmap_ex(32,SCREEN_W,SCREEN_H);
            blit(screen,shoot_bmp,0,0,0,0,SCREEN_W,SCREEN_H);
            char nom[100];
            nom[0]=0;
            strcat(nom,"ta3d-shoot000000");
            nom[strlen(nom)-6]+=(nb_shoot/100000)%10;
            nom[strlen(nom)-5]+=(nb_shoot/10000)%10;
            nom[strlen(nom)-4]+=(nb_shoot/1000)%10;
            nom[strlen(nom)-3]+=(nb_shoot/100)%10;
            nom[strlen(nom)-2]+=(nb_shoot/10)%10;
            nom[strlen(nom)-1]+=nb_shoot%10;
            nb_shoot = (nb_shoot+1)%1000000;
            strcat(nom,".jpg");
            save_jpg_ex((TA3D::Paths::Screenshots + nom).c_str(), shoot_bmp, NULL, 75, JPG_SAMPLING_411, NULL);
            destroy_bitmap(shoot_bmp);
            shoot = false;
        }

        gfx->unset_2D_mode();

        gfx->flip();

        if (cmd) // Analyse les commandes tapées dans la console
        {
            String::Vector params;
            ReadVectorString(params,  cmd, " ");
            if (params.size() > 0)
            {
                if (params[0] == "fps_on") lp_CONFIG->showfps=true;				// Affiche le nombre d'images/seconde
                else if (params[0] == "fps_off") lp_CONFIG->showfps=false;				// Cache le nombre d'images/seconde
                else if (params[0] == "zshoot") // Prend une capture d'écran(tampon de profondeur seulement)
                {
                    BITMAP *bmp=create_bitmap_ex(32,SCREEN_W,SCREEN_H);
                    clear(bmp);
                    glReadPixels(0,0,SCREEN_W,SCREEN_H,GL_DEPTH_COMPONENT,GL_INT,bmp->line[0]);
                    save_bitmap( (TA3D::Paths::Screenshots + "z.tga").c_str(),bmp,NULL);
                    destroy_bitmap(bmp);
                }
                else if (params.size() == 2 && params[0] == "video" && params[1] == "shoot") video_shoot ^= true;		// Capture video
                else if (params[0] == "shoot") shoot = true;					// Prend une capture d'écran
                else if (params.size() == 2 && params[0] == "reload" && params[1] == "shaders" && g_useProgram)
                {
                    water_shader.destroy();
                    water_shader.load("shaders/water.frag","shaders/water.vert");
                    water_shader_reflec.destroy();
                    water_shader_reflec.load("shaders/water_reflec.frag","shaders/water.vert");
                    water_pass1.destroy();
                    water_pass1.load("shaders/water_pass1.frag","shaders/water_pass1.vert");
                    water_pass1_low.destroy();
                    water_pass1_low.load("shaders/water_pass1_low.frag","shaders/water_pass1.vert");
                    water_pass2.destroy();
                    water_pass2.load("shaders/water_pass2.frag","shaders/water_pass2.vert");
                    map->detail_shader.destroy();
                    map->detail_shader.load( "shaders/details.frag", "shaders/details.vert");
                }
                else if (params.size() == 3 && params[0] == "show" && params[1] == "mission" && params[2] == "info")	show_mission_info ^= true;
                else if (params.size() == 2 && params[0] == "view" && params[1] == "debug")	view_dbg^=true;
                else if (params.size() == 2 && params[0] == "ia" && params[1] == "debug")	ia_debug^=true;
                else if (params.size() == 2 && params[0] == "internal" && params[1] == "name") internal_name^=true;		// Affiche/Cache le nom interne de l'unité
                else if (params.size() == 2 && params[0] == "internal" && params[1] == "idx") internal_idx^=true;		// Show/Hide unit indexes in unit array
                else if (params[0] == "exit") done=true;					// Quitte le programme
                else if (params[0] == "wireframe") 	lp_CONFIG->wireframe^=true;
                else if (params[0] == "priority" && params.size() == 2) lp_CONFIG->priority_level = atoi( params[1].c_str());
                else if ( params.size() == 3 && params[0] == "water" && params[1] == "quality") lp_CONFIG->water_quality = atoi( params[2].c_str())%5;
                else if (params[0] == "shadow" && params.size() == 3) {
                    if (params[1] == "quality")
                        lp_CONFIG->shadow_quality = atoi( params[2].c_str());
                    else
                        if (params[1] == "ray") lp_CONFIG->shadow_r = atof( params[2].c_str());
                }
                else if (params[0] == "shadow" && params.size() == 1) lp_CONFIG->shadow^=true;
                else if (params[0] == "details")	lp_CONFIG->detail_tex ^= true;
                else if (params[0] == "particle")	lp_CONFIG->particle^=true;
                else if (params.size() == 2 && params[0] == "explosion" && params[1] == "particles")	lp_CONFIG->explosion_particles^=true;
                else if (params[0] == "waves") lp_CONFIG->waves^=true;
                else if (params.size() == 2 && params[0] == "show" && params[1] == "script") show_script^=true;
                else if (params.size() == 2 && params[0] == "show" && params[1] == "model") show_model^=true;
                else if (params.size() == 2 && params[0] == "rotate" && params[1] == "light") rotate_light^=true;
                else if (params[0] == "shake")
                    cam.setShake(1.0f, 32.0f);
                else if (params[0] == "freecam") {
                    freecam ^= true;
                    if (!freecam)
                        r2=0.0f;
                }
                else if (params[0] == "fps_limit" && params.size() == 2) {
                    speed_limit = atoi( params[1].c_str());
                    if (speed_limit == 0.0f)
                        speed_limit = -1.0f;
                    delay = 1.0f / speed_limit;
                }
                else if (params[0] == "spawn" && params.size() >= 2) {
                    int unit_type = -1;
                    int player_id = players.local_human_id;
                    int nb_to_spawn=1;
                    unit_type = unit_manager.get_unit_index( params[1].c_str());
                    if (params.size() >= 3)
                    {
                        player_id = atoi( params[2].c_str());
                        if (params.size() >= 4)
                            nb_to_spawn = atoi( params[3].c_str());
                    }
                    ThreadSynchroniser->lock();		// Make sure we won't destroy something we mustn't
                    units.lock();
                    for (int i = 0; i < nb_to_spawn; ++i)
                    {
                        int id=0;
                        if (unit_type < 0 || unit_type >= unit_manager.nb_unit)
                            id = units.create( abs( TA3D_RAND()) % unit_manager.nb_unit,player_id);
                        else
                            id = units.create(unit_type,player_id);
                        if (id == -1) // Ho ho no more space to store that unit, limit reached
                            break;
                        units.unit[id].lock();
                        int e(0);

                        do
                        {
                            units.unit[id].Pos.x = (TA3D_RAND() % map->map_w) - map->map_w_d;
                            units.unit[id].Pos.z = (TA3D_RAND() % map->map_h) - map->map_h_d;
                            ++e;
                        } while (e < 100 && !can_be_built(units.unit[id].Pos, map, units.unit[id].type_id, player_id));

                        if (!can_be_built(units.unit[id].Pos, map, units.unit[id].type_id, player_id))
                        {
                            units.unit[id].flags = 4;
                            units.kill(id, map, units.index_list_size - 1);
                        }
                        else
                        {								// Compute unit's Y coordinate
                            Vector3D target_pos = units.unit[id].Pos;
                            target_pos.x=((int)(target_pos.x)+map->map_w_d)>>3;
                            target_pos.z=((int)(target_pos.z)+map->map_h_d)>>3;
                            target_pos.y=Math::Max(map->get_max_rect_h((int)target_pos.x,(int)target_pos.z, unit_manager.unit_type[ units.unit[id].type_id ].FootprintX, unit_manager.unit_type[ units.unit[id].type_id ].FootprintZ),map->sealvl);
                            units.unit[id].Pos.y = target_pos.y;
                            units.unit[id].cur_px = (int)target_pos.x;
                            units.unit[id].cur_py = (int)target_pos.z;
                            units.unit[id].draw_on_map();

                            if (unit_manager.unit_type[units.unit[id].type_id].ActivateWhenBuilt)// Start activated
                            {
                                units.unit[id].port[ ACTIVATION ] = 0;
                                units.unit[id].activate();
                            }
                            if (unit_manager.unit_type[units.unit[id].type_id].init_cloaked)				// Start cloaked
                                units.unit[id].cloaking = true;
                        }
                        units.unit[ id ].unlock();
                    }
                    units.unlock();
                    ThreadSynchroniser->unlock();
                }
                else if (params[0] == "timefactor" && params.size() == 2)
                    lp_CONFIG->timefactor = atof( params[1].c_str());
                else if (params[0] == "addhp" && params.size() == 2) {
                    if (selected) // Sur les unités sélectionnées
                    {
                        int value = atoi( params[1].c_str());
                        units.lock();
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                            {
                                units.unit[i].hp+=value;
                                if (units.unit[i].hp < 0)
                                    units.unit[i].hp = 0;
                                else
                                    if (units.unit[i].hp > unit_manager.unit_type[units.unit[i].type_id].MaxDamage)
                                        units.unit[i].hp = unit_manager.unit_type[units.unit[i].type_id].MaxDamage;
                            }
                        }
                        units.unlock();
                    }
                }
                else if (params[0] == "deactivate") {
                    if (selected) // Sur les unités sélectionnées
                    {
                        units.lock();
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                                units.unit[i].deactivate();
                        }
                        units.unlock();
                    }
                }
                else if (params[0] == "activate") {
                    if (selected) // Sur les unités sélectionnées
                    {
                        units.lock();
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                                units.unit[i].activate();
                        }
                        units.unlock();
                    }
                }
                else if (params[0] == "reset_script") {							// Réinitialise les scripts
                    if (selected) // Sur les unités sélectionnées
                    {
                        units.lock();
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            units.unlock();
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                                units.unit[i].reset_script();
                            units.lock();
                        }
                        units.unlock();
                    }
                }
                else if (params[0] == "unitinfo") {
                    if (selected && cur_sel != -1)	// Sur les unités sélectionnées
                    {
                        const char *unit_info[]={"ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS","HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y","UNIT_XZ","UNIT_Y","UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN",
                            "HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN","BUGGER_OFF","ARMORED"};
                        units.lock();
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                            {
                                printf("flags=%d\n", units.unit[i].flags);
                                for (int f=1;f<21;f++)
                                    printf("%s=%d\n",unit_info[f-1],units.unit[i].port[f]);
                            }
                        }
                        units.unlock();
                    }
                }
                else if (params[0] == "kill") {							// Détruit les unités sélectionnées
                    if (selected) // Sur les unités sélectionnées
                    {
                        units.lock();
                        for (uint16 e=0;e<units.index_list_size;e++)
                        {
                            i = units.idx_list[e];
                            units.unlock();
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel) {
                                units.kill(i,map,e);
                                --e;
                            }
                            units.lock();
                        }
                        units.unlock();
                        selected=false;
                        cur_sel=-1;
                    }
                }
                else if (params[0] == "shootall") {							// Destroy enemy units
                    units.lock();
                    for (uint16 e=0;e<units.max_unit;e++)
                    {
                        i = units.idx_list[e];
                        units.unlock();
                        if ((units.unit[i].flags & 1) && units.unit[i].owner_id != players.local_human_id) {
                            units.kill(i,map,e);
                            e--;
                        }
                        units.lock();
                    }
                    units.unlock();
                }
                else if (params[0] == "script" && params.size() == 2) {							// Force l'éxecution d'un script
                    if (selected) // Sur les unités sélectionnées
                    {
                        int id = atoi( params[1].c_str());
                        units.lock();
                        for (uint16 e = 0; e < units.index_list_size; ++e)
                        {
                            i = units.idx_list[e];
                            if ((units.unit[i].flags & 1) && units.unit[i].owner_id==players.local_human_id && units.unit[i].sel)
                                units.unit[i].launch_script(id);
                        }
                        units.unlock();
                    }
                }
                else if (params[0] == "pause")								// Toggle pause mode
                    lp_CONFIG->pause ^= true;
                else if (params[0] == "give") {							// Give resources to player_id
                    bool success = false;
                    if (params.size() == 4)
                    {
                        int player_id = atoi( params[2].c_str());
                        int amount = atoi( params[3].c_str());
                        if (player_id >= 0 && player_id < players.nb_player)
                        {
                            if (params[1] == "metal" || params[1] == "both")
                            {
                                players.metal[ player_id ] = players.c_metal[ player_id ] = players.c_metal[ player_id ] + amount;					// cheat codes
                                success = true;
                            }
                            if (params[1] == "energy" || params[1] == "both")
                            {
                                players.energy[ player_id ] = players.c_energy[ player_id ] = players.c_energy[ player_id ] + amount;					// cheat codes
                                success = true;
                            }
                        }
                    }
                    if (!success)
                        LOG_ERROR("Command error: The correct syntax is: give metal/energy/both player_id amount");

                }
                else if (params[0] == "metal") cheat_metal^=true;				// cheat codes
                else if (params[0] == "energy") cheat_energy^=true;			// cheat codes
            }
            params.clear();
            free(cmd);
        }
        if (cheat_metal)
            players.metal[players.local_human_id] = players.c_metal[players.local_human_id]=players.metal_s[players.local_human_id];					// cheat codes
        if (cheat_energy)
            players.energy[players.local_human_id] = players.c_energy[players.local_human_id]=players.energy_s[players.local_human_id];				// cheat codes
        if (key[KEY_F12])
            shoot = true;

        /*------------ Code de gestion du déroulement de la partie -----------------------------------*/

        if ((!network_manager.isConnected() || network_manager.isServer()) && signal == -1)	// Si le script est terminé, on reprend les règles standard
        {
            bool win = true;
            for (short int i = 0; i < players.nb_player; ++i)
            {
                if (!players.annihilated[i] && i!=players.local_human_id)
                {
                    win = false;
                    break;
                }
            }
            if (win) // Victoire
            {
                done = true;
                exit_mode = EXIT_VICTORY;
            }
            if (players.annihilated[players.local_human_id])	// Défaite
            {
                done = true;
                exit_mode = EXIT_DEFEAT;
            }
        }

    } while(!done);

    reset_mouse();

    delete sky_data;

    players.DestroyThread();				// Shut down the Players thread
    players.stop_threads();
    weapons.DestroyThread();				// Shut down the Weapon Engine
    units.DestroyThread();					// Shut down the Unit Engine
    particle_engine.DestroyThread();		// Shut down the Particle Engine

    water_obj.destroy();
    sky_obj.destroy();

    Camera::inGame = NULL;

    if (g_useProgram && g_useFBO && map->water)
    {
        glDeleteFramebuffersEXT(1, &water_FBO);
        water_pass1.destroy();
        water_pass1_low.destroy();
        water_pass2.destroy();
        water_shader.destroy();
        water_shader_reflec.destroy();
        gfx->destroy_texture(water_color);
        gfx->destroy_texture(first_pass);
        gfx->destroy_texture(second_pass);
        gfx->destroy_texture(reflectex);
        gfx->destroy_texture(transtex);
    }

    gfx->destroy_texture(freecam_on);
    gfx->destroy_texture(freecam_off);
    gfx->destroy_texture(sky);
    gfx->destroy_texture(water);
    gfx->destroy_texture(glow);
    gfx->destroy_texture(arrow_texture);
    gfx->destroy_texture(circle_texture);

    LOG_INFO("Total Models: " << model_manager.nb_models);
    LOG_INFO("Total Units: " << unit_manager.nb_unit);
    LOG_INFO("Total Textures: " << texture_manager.nbtex);

    switch(exit_mode)
    {
        case EXIT_NONE:
            break;
        case EXIT_VICTORY:
            if (game_data->campaign && !map->ota_data.glamour.empty() && HPIManager->Exists( "bitmaps\\glamour\\" + map->ota_data.glamour + ".pcx"))
            {
                uint32 pcx_size = 0;
                byte *data = HPIManager->PullFromHPI( "bitmaps\\glamour\\" + map->ota_data.glamour + ".pcx", &pcx_size);
                if (data)
                {
                    FILE *dst = TA3D_OpenFile(TA3D::Paths::Caches + "glamour.pcx", "wb");
                    fwrite( data, pcx_size, 1, dst);
                    fclose( dst);
                    delete[] data;
                    GLuint	glamour_tex = gfx->load_texture(TA3D::Paths::Caches + "glamour.pcx");
                    gfx->set_2D_mode();
                    gfx->drawtexture( glamour_tex, 0, 0, SCREEN_W, SCREEN_H);
                    gfx->destroy_texture( glamour_tex);
                    gfx->unset_2D_mode();
                    gfx->flip();

                    while( !keypressed() && mouse_b == 0) {	rest(1);	poll_keyboard(); 	poll_mouse();	}
                    while( mouse_b)	poll_mouse();
                    while( keypressed())	readkey();
                }
            }
            break;
        case EXIT_DEFEAT:
            break;
    }

    LOG_DEBUG("Freeing memory used for the map");
    map->destroy();
    delete map;
    LOG_DEBUG("Freeing memory used for 3d models");
    model_manager.destroy();
    LOG_DEBUG("Freeing memory used for particle engine");
    particle_engine.destroy();
    LOG_DEBUG("Freeing memory used for ingame units");
    units.destroy();
    LOG_DEBUG("Freeing memory used for units");
    unit_manager.destroy();
    LOG_DEBUG("Freeing memory used for weapons");
    weapons.destroy();
    LOG_DEBUG("Freeing memory used for sound");
    sound_manager->stopMusic();
    LOG_DEBUG("Freeing memory used for fx");
    fx_manager.destroy();

    LOG_DEBUG("Freeing memory used for weapon manager");
    weapon_manager.destroy();

    LOG_DEBUG("Freeing memory used for features");
    feature_manager.destroy();

    LOG_DEBUG("Freeing memory used for ingame features");
    features.destroy();

    LOG_DEBUG("Freeing memory used for textures");
    texture_manager.destroy();

    if (!game_data->campaign)
        Menus::Statistics::Execute();

    LOG_DEBUG("Rreeing memory used for players...");
    players.destroy();
    delete HPIManager;
    HPIManager = new cHPIHandler("");
    return exit_mode;
}


int anim_cursor(int type)
{
    return (type == -1)
        ? ((msec_timer-start) / 100) % cursor.anm[cursor_type].nb_bmp
        : ((msec_timer-start) / 100) % cursor.anm[type].nb_bmp;
}


void draw_cursor()
{
    int curseur=anim_cursor();
    if (curseur<0 || curseur>=cursor.anm[cursor_type].nb_bmp)
    {
        curseur=0;
        start=msec_timer;
    }
    float dx=cursor.anm[cursor_type].ofs_x[curseur];
    float dy=cursor.anm[cursor_type].ofs_y[curseur];
    float sx=cursor.anm[cursor_type].bmp[curseur]->w;
    float sy=cursor.anm[cursor_type].bmp[curseur]->h;
    gfx->set_color(0xFFFFFFFF);
    gfx->set_alpha_blending();
    gfx->drawtexture( cursor.anm[cursor_type].glbmp[curseur], mouse_x*gfx->SCREEN_W_TO_640-dx, mouse_y*gfx->SCREEN_H_TO_480-dy, mouse_x*gfx->SCREEN_W_TO_640-dx+sx, mouse_y*gfx->SCREEN_H_TO_480-dy+sy);
    gfx->unset_alpha_blending();
}



Vector3D cursor_on_map(Camera* cam,MAP *map, bool on_mini_map)
{
    if (on_mini_map) // If the cursor is on the mini_map;
    {
        Vector3D map_pos;
        map_pos.x = (mouse_x - 64) * 252.0f / 128.0f * map->map_w / map->mini_w;
        map_pos.z = (mouse_y - 64) * 252.0f / 128.0f * map->map_h / map->mini_h;
        map_pos.y = map->get_unit_h( map_pos.x, map_pos.z);
        return map_pos;
    }
    Vector3D cur_dir;
    cur_dir = cam->dir + cam->widthFactor * 2.0f * (mouse_x - gfx->SCREEN_W_HALF) * gfx->SCREEN_W_INV * cam->side - 1.5f * (mouse_y - gfx->SCREEN_H_HALF) * gfx->SCREEN_H_INV * cam->up;
    cur_dir.unit();		// Direction pointée par le curseur
    return map->hit(cam->pos, cur_dir, true, 2000000000.0f, true);
}


