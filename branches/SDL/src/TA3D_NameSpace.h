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


# include "misc/interface.h"
# include "cError.h"
# include "misc/vector.h"
# include "cTAFileParser.h"
# include "TA3D_hpi.h"
# include "gfx/gfx.h"
# include "network/network.h"
# include "threads/thread.h"
# include <vector>

# define ZOOM_NORMAL       0x0
# define ZOOM_FIXED_ANGLE  0x1

//! Default server hostname for TA3D
# define TA3D_DEFAULT_SERVER_HOSTNAME  "ta3d.darkstars.co.uk"




namespace TA3D
{
	#define TA3D_API_SI static inline
	#define TA3D_API_EI extern inline
	#define TA3D_API_S static
	#define TA3D_API_E extern



	typedef struct TA3DCONFIG
	{
		real32	fps_limit;
		real32	shadow_r;      // 0.0 -> 100.0
		real32	timefactor;      // 1.0 -> 10.0

		sint16	shadow_quality; // 0 -> 100
		sint16	priority_level; // 0, 1, 2
		sint16	water_quality;  // 0->4
		sint16	fsaa;  // ?
		sint16	Lang;

		uint16	screen_width;
		uint16	screen_height;
		uint8	color_depth;	// Default : 32, can be 16 or 32

		bool	showfps;
		bool	wireframe;
		bool	particle;
		bool    explosion_particles;
		bool	waves;
		bool	shadow;
		bool	height_line;
		bool	fullscreen;

		bool	detail_tex;
		bool	draw_console_loading;

		uint8	camera_zoom;				// ZOOM_NORMAL, ZOOM_FIXED_ANGLE
		float	camera_def_angle;
		float	camera_def_h;
		float	camera_zoom_speed;

		String	last_script;				// Remember last script used
		String	last_map;					// Remember last map played
		byte	last_FOW;					// Remember last FOW state
		String	last_MOD;
		String	player_name;				// Name of local player

		String	skin_name;					// The skin used ( empty means default )

		bool	use_texture_cache;			// Use the texture cache ? (default : disabled because on some systems it doesn't work)
		bool	use_texture_compression;	// Use texture compression capabilities of GFX hardware ? (default : enabled because it greatly reduces video memory requirements)

		String	net_server;					// The server that monitor the game server list
		bool    render_sky;                 // Render the sky ? (on low-end hardware without accelerated T&L it may help to deactivate this)
		bool    low_definition_map;         // Render map in low definition mode (1 quad instead of 4 for each map bloc)

		bool    underwater_bright;          // Render underwater objects brighter

        bool    disable_GLSL;               // Disable GLSL shaders (they won't even be loaded so if there is a problem with them it won't crash)

        bool    right_click_interface;      // Right click interface ?

        bool    ortho_camera;               // Do we use orthographic camera ?

		// Variables used to communicate with all the code
		bool	quickrestart;				// Should be false, set to true when need to restart to enable options/parameters
		bool	quickstart;					// Tell to speed up the starting process
		bool	restorestart;				// Tell it's a restart to restore previous config file
		bool	pause;						// Tell the engine it's in pause mode, so wait for this to be false again
		bool	paused;						// The engine writes its current state here, used by save/load code
		String	file_param;					// File parameter (command line parameter), used to give complex instructions
		bool    enable_shortcuts;           // Tell the GUI module to react to shortcuts or not (deactivated when in chat mode)

		TA3DCONFIG()
		{
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
			camera_def_angle = 63.44f;		// TA angle
			camera_def_h = 200.0f;
			camera_zoom_speed = 1.0f;

			player_name = "player";
			last_script = "scripts\\default.lua";
			last_map = "";
			last_FOW = 0;

			draw_console_loading = false;	// default set to false since it's a developer feature
			detail_tex = false;				// default set to false because of fragment program dependency ( and it's only an eye candy feature )

			fps_limit = -1.0f;
			shadow_r = 0.02f;
			timefactor = 1.0f;
			shadow_quality = 1;
			priority_level = 0;
			water_quality = 1;      		// For now only because we have shaders issues with ati board
			fsaa = 0;
			Lang = 0;       			   // English
			screen_width = 800;
			screen_height = 600;
			color_depth = 32;
			showfps = false;
			wireframe = false;
			particle = true;
			explosion_particles = true;
			waves = true;
			shadow = true;
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
		TA3D_API_E TA3D::IInterfaceManager			*InterfaceManager;
		TA3D_API_E TA3D::UTILS::HPI::cHPIHandler	*HPIManager;
		TA3D_API_E TA3D::GFX* gfx;

		TA3D_API_E SDL_Color						*pal;
		TA3D_API_E TA3D::TA3DCONFIG					*lp_CONFIG;

		TA3D_API_E uint8							unit_engine_thread_sync;
		TA3D_API_E uint8							weapon_engine_thread_sync;
		TA3D_API_E uint8							particle_engine_thread_sync;
		TA3D_API_E uint8							players_thread_sync;

		TA3D_API_E ObjectSync						*ThreadSynchroniser;
		TA3D_API_E String							TA3D_CURRENT_MOD;
		TA3D_API_E int								ascii_to_scancode[ 256 ];

		TA3D_API_E int                              mouse_x, mouse_y, mouse_z, mouse_b;
		TA3D_API_E int                              key[0x1000];
		TA3D_API_E std::list<uint32>                keybuf;

		// Some constant data needed by the engine ( like number of ticks/sec. to simulate )
#define TICKS_PER_SEC				30
#define MAX_CODE_PER_TICK			100
	}

	FILE *TA3D_OpenFile( const String &FileName, const String Mode );


    /*!
    ** \brief Clear the cache if needed (useful when mod has changed)
    */
	void TA3D_clear_cache(bool force=false);

	/*!
	** \brief return true is there are key codes waiting in the buffer, false otherwise
	*/
	inline bool keypressed()    {   return !VARS::keybuf.empty(); }

	/*!
	** \brief return the next key code in the key buffer
	*/
	uint32 readkey();

	/*!
	** \brief clears the key code buffer
	*/
	void clear_keybuf();

	/*!
	** \brief poll keyboard events
	*/
	void poll_keyboard();

	/*!
	** \brief poll mouse events
	*/
	void poll_mouse();

	/*!
	** \brief set mouse position
	*/
	void position_mouse(int x, int y);

	/*!
	** \brief return mouse move since last call
	*/
	void get_mouse_mickeys(int *mx, int *my);
} // namespace TA3D

#define SCREEN_W    (screen->w)
#define SCREEN_H    (screen->h)

#define KEY_ENTER       SDLK_RETURN
#define KEY_SPACE       SDLK_SPACE
#define KEY_LEFT        SDLK_LEFT
#define KEY_RIGHT       SDLK_RIGHT
#define KEY_UP          SDLK_UP
#define KEY_DOWN        SDLK_DOWN
#define KEY_TAB         SDLK_TAB
#define KEY_LSHIFT      SDLK_LSHIFT
#define KEY_RSHIFT      SDLK_RSHIFT
#define KEY_LCONTROL    SDLK_LCTRL
#define KEY_RCONTROL    SDLK_RCTRL
#define KEY_ESC         SDLK_ESCAPE
#define KEY_BACKSPACE   SDLK_BACKSPACE
#define KEY_DEL         SDLK_DELETE
#define KEY_ALT         SDLK_LALT

#define KEY_F1          SDLK_F1
#define KEY_F2          SDLK_F2
#define KEY_F3          SDLK_F3
#define KEY_F4          SDLK_F4
#define KEY_F5          SDLK_F5
#define KEY_F6          SDLK_F6
#define KEY_F7          SDLK_F7
#define KEY_F8          SDLK_F8
#define KEY_F9          SDLK_F9
#define KEY_F10         SDLK_F10
#define KEY_F11         SDLK_F11
#define KEY_F12         SDLK_F12

#define KEY_0           SDLK_0
#define KEY_1           SDLK_1
#define KEY_2           SDLK_2
#define KEY_3           SDLK_3
#define KEY_4           SDLK_4
#define KEY_5           SDLK_5
#define KEY_6           SDLK_6
#define KEY_7           SDLK_7
#define KEY_8           SDLK_8
#define KEY_9           SDLK_9

#define KEY_PLUS        SDLK_PLUS
#define KEY_MINUS       SDLK_MINUS
#define KEY_PLUS_PAD    SDLK_KP_PLUS
#define KEY_MINUS_PAD   SDLK_KP_MINUS

#define KEY_A           SDLK_a
#define KEY_B           SDLK_b
#define KEY_C           SDLK_c
#define KEY_D           SDLK_d
#define KEY_E           SDLK_e
#define KEY_F           SDLK_f
#define KEY_G           SDLK_g
#define KEY_H           SDLK_h
#define KEY_I           SDLK_i
#define KEY_J           SDLK_j
#define KEY_K           SDLK_k
#define KEY_L           SDLK_l
#define KEY_M           SDLK_m
#define KEY_N           SDLK_n
#define KEY_O           SDLK_o
#define KEY_P           SDLK_p
#define KEY_Q           SDLK_q
#define KEY_R           SDLK_r
#define KEY_S           SDLK_s
#define KEY_T           SDLK_t
#define KEY_U           SDLK_u
#define KEY_V           SDLK_v
#define KEY_W           SDLK_w
#define KEY_X           SDLK_x
#define KEY_Y           SDLK_y
#define KEY_Z           SDLK_z

#define KEY_TILDE       SDLK_RIGHTPAREN

#define KEY_CAPSLOCK    SDLK_CAPSLOCK

#ifndef TA3D_MSEC_TIMER
#define TA3D_MSEC_TIMER
#define msec_timer  (SDL_GetTicks())
#endif

// TODO Must be removed
using namespace TA3D::VARS;
using namespace TA3D::UTILS;


#endif // __TA3D_NAMESPACE_H__
