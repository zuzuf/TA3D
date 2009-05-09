/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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

/*
**  File: TA3D_NameSpace.h
** Notes:
**   Cire: The goal of this file is mainly to introduce our namespace
**           Some constants are also defined in our namespace, function
**           prototypes ect.
**         This file should be included 'second' in all other cpp files, it will
**           give cpp access to our namespace members.
*/

#ifndef __TA3D_NAMESPACE_H__
# define __TA3D_NAMESPACE_H__

# include "gfx/gfx.toolkit.h"
# include "misc/interface.h"
# include "cError.h"
# include "misc/vector.h"
# include "TA3D_hpi.h"
# include "gfx/gfx.h"
# include "network/network.h"
# include "threads/thread.h"
# include <vector>

# define ZOOM_NORMAL       0x0
# define ZOOM_FIXED_ANGLE  0x1

//! Default server hostname for TA3D
# define TA3D_DEFAULT_SERVER_HOSTNAME  "netserver.ta3d.org"




namespace TA3D
{
    typedef struct TA3DCONFIG
    {
        float  fps_limit;
        float  timefactor;      // 1.0 -> 10.0

        sint16  shadow_quality; // 0 -> none, 1 -> low (shadow volumes), 2 -> normal (shadow maps)
        sint16  priority_level; // 0, 1, 2
        sint16  water_quality;  // 0->4
        sint16  fsaa;   // ?
        String  Lang;   // english, french, german, italian, spanish, japanese, chinese, ...
        sint16  anisotropy;     // Level of anisotropy for textures (1->16)

        uint16  screen_width;
        uint16  screen_height;
        uint8   color_depth;    // Default : 32, can be 16 or 32

        bool    showfps;
        bool    wireframe;
        bool    particle;
        bool    explosion_particles;
        bool    waves;
        bool    height_line;
        bool    fullscreen;
        bool    far_sight;      // Enable far sight renderer (you can see all the map)

        bool    detail_tex;
        bool    draw_console_loading;

        uint8   camera_zoom;                // ZOOM_NORMAL, ZOOM_FIXED_ANGLE
        float   camera_def_angle;
        float   camera_def_h;
        float   camera_zoom_speed;

        String  last_script;                // Remember last script used
        String  last_map;                    // Remember last map played
        byte    last_FOW;                    // Remember last FOW state
        String  last_MOD;
        String  player_name;                // Name of local player

        String  skin_name;                    // The skin used ( empty means default )

        bool    use_texture_cache;            // Use the texture cache ? (default : disabled because on some systems it doesn't work)
        bool    use_texture_compression;    // Use texture compression capabilities of GFX hardware ? (default : enabled because it greatly reduces video memory requirements)

        String  net_server;                    // The server that monitor the game server list
        bool    render_sky;                 // Render the sky ? (on low-end hardware without accelerated T&L it may help to deactivate this)
        bool    low_definition_map;         // Render map in low definition mode (1 quad instead of 4 for each map bloc)

        bool    underwater_bright;          // Render underwater objects brighter

        bool    disable_GLSL;               // Disable GLSL shaders (they won't even be loaded so if there is a problem with them it won't crash)

        bool    right_click_interface;      // Right click interface ?

        bool    ortho_camera;               // Do we use orthographic camera ?

        float   mouse_sensivity;            // Mouse sensivity

        int     sound_volume;               // Self explanatory
        int     music_volume;               // 0 - 128

        bool    grab_inputs;                // Capture inputs ?

        // Variables used to communicate with all the code
        bool    quickrestart;                // Should be false, set to true when need to restart to enable options/parameters
        bool    quickstart;                    // Tell to speed up the starting process
        bool    restorestart;                // Tell it's a restart to restore previous config file
        bool    pause;                        // Tell the engine it's in pause mode, so wait for this to be false again
        bool    paused;                        // The engine writes its current state here, used by save/load code
        String  file_param;                    // File parameter (command line parameter), used to give complex instructions
        bool    enable_shortcuts;           // Tell the GUI module to react to shortcuts or not (deactivated when in chat mode)

        TA3DCONFIG()
        {
            grab_inputs = false;

            sound_volume = music_volume = 128;

            far_sight = true;

            anisotropy = 1;

            mouse_sensivity = 1.0f;

            ortho_camera = false;

            right_click_interface = false;

            disable_GLSL = false;           // By default we want shaders

            underwater_bright = false;

            use_texture_compression = true;

            low_definition_map = false;

            render_sky = true;

            enable_shortcuts = true;

            net_server = TA3D_DEFAULT_SERVER_HOSTNAME;

            file_param.clear();

            use_texture_cache = false;

            pause = false;

            skin_name = "";

            camera_zoom = ZOOM_NORMAL;
            camera_def_angle = 63.44f;        // TA angle
            camera_def_h = 200.0f;
            camera_zoom_speed = 1.0f;

            player_name = "player";
            last_script = "scripts\\default.lua";
            last_map = "";
            last_FOW = 0;

            draw_console_loading = false;    // default set to false since it's a developer feature
            detail_tex = false;                // default set to false because of fragment program dependency ( and it's only an eye candy feature )

            fps_limit = -1.0f;
            timefactor = 1.0f;
            shadow_quality = 2;
            priority_level = 0;
            water_quality = 1;              // For now only because we have shaders issues with ati board
            fsaa = 0;
            Lang = "english";               // English
            screen_width = 800;
            screen_height = 600;
            color_depth = 32;
            showfps = false;
            wireframe = false;
            particle = true;
            explosion_particles = true;
            waves = true;
            height_line = false;
            fullscreen = false;            // For now, later will be true when we will reach a beta release

            last_MOD = "";

            quickrestart = false;
            quickstart = false;
            restorestart = false;
        }

    } sTA3DCONFIG, *LPTA3DCONFIG;

    // TODO Must be removed
#define   DEBUG_MODE

    namespace VARS
    {
        TA3D_API_E SDL_Surface                      *screen;
        TA3D_API_E TA3D::IInterfaceManager          *InterfaceManager;
        TA3D_API_E TA3D::UTILS::HPI::cHPIHandler    *HPIManager;
        TA3D_API_E TA3D::GFX* gfx;

        TA3D_API_E SDL_Color                        *pal;
        TA3D_API_E TA3D::TA3DCONFIG                 *lp_CONFIG;

        TA3D_API_E uint8                            unit_engine_thread_sync;
        TA3D_API_E uint8                            weapon_engine_thread_sync;
        TA3D_API_E uint8                            particle_engine_thread_sync;
        TA3D_API_E uint8                            players_thread_sync;

        TA3D_API_E ObjectSync                       *ThreadSynchroniser;
        TA3D_API_E String                           TA3D_CURRENT_MOD;

        // Some constant data needed by the engine ( like number of ticks/sec. to simulate )
#define TICKS_PER_SEC                30
    }

    FILE *TA3D_OpenFile( const String &FileName, const String Mode );


    /*!
    ** \brief Clear the cache if needed (useful when mod has changed)
    */
    void TA3D_clear_cache(bool force=false);
} // namespace TA3D

#include "input/mouse.h"
#include "input/keyboard.h"

#define SCREEN_W    (screen->w)
#define SCREEN_H    (screen->h)


#ifndef TA3D_MSEC_TIMER
#define TA3D_MSEC_TIMER
#define msec_timer  (SDL_GetTicks())
#endif

// TODO Must be removed
using namespace TA3D::VARS;
using namespace TA3D::UTILS;


#endif // __TA3D_NAMESPACE_H__
