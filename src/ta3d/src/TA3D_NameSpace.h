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
# include "misc/vector.h"
# include "misc/string.h"
# include "vfs/vfs.h"
# include "gfx/gfx.h"
# include "network/network.h"
# include "threads/thread.h"
# include <vector>

# define ZOOM_NORMAL       0x0
# define ZOOM_FIXED_ANGLE  0x1




namespace TA3D
{
	struct TA3DCONFIG
	{
		typedef TA3DCONFIG*	Ptr;

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
		uint8	shadowmap_size;	// 0 -> lower (256x256), 1 -> low (512x512), 2 -> normal (1024x1024), 3 -> high (2048x2048)

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

		String	serializedGameData;			// Informations about last game

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

        String  system7zCommand;            // Command used by system() to run 7zip (used to extract archives when needed by the mods manager)

        String  resourcePaths;              // Alternative resource paths to use

		float	menuTransparency;			// OTA interface transparency in game

		bool	tooltips;					// Enable build menus tool tips ?

		bool	developerMode;				// Special mode for developers (especially mod developers)
											// which disables file caching

		int		unitTextureQuality;			// Maximum resolution of unit textures (0->64x64, 1->128x128, 2->256x256, 3->512x512, 4->1024x1024, 5->no limit)

		// Variables used to communicate with all the code
		bool    quickrestart;				// Should be false, set to true when need to restart to enable options/parameters
		bool    quickstart;					// Tell to speed up the starting process
		bool    restorestart;				// Tell it's a restart to restore previous config file
		volatile bool	pause;				// Tell the engine it's in pause mode, so wait for this to be false again
		volatile bool	paused;				// The engine writes its current state here, used by save/load code
		String  file_param;					// File parameter (command line parameter), used to give complex instructions
		volatile bool    enable_shortcuts;	// Tell the GUI module to react to shortcuts or not (deactivated when in chat mode)
		bool	no_sound;
		bool	first_start;
		bool	bUseWorkingDirectory;		// Use working directory as resource folder

		TA3DCONFIG();

		int getMaxTextureSizeAllowed() const;
	};

	// TODO Must be removed
#define   DEBUG_MODE

	namespace VARS
	{
		extern SDL_Surface							*screen;
		extern TA3D::IInterfaceManager::Ptr			InterfaceManager;
		extern TA3D::GFX::Ptr						gfx;

		extern SDL_Color							*pal;
		extern TA3D::TA3DCONFIG::Ptr				lp_CONFIG;

		extern String								TA3D_CURRENT_MOD;

		// Some constant data needed by the engine ( like number of ticks/sec. to simulate )
		#define TICKS_PER_SEC                30
	}

	template<class T>
	inline void setFlag(T &out, const int flag)	{	out = T(out | flag);	}

	template<class T>
	inline void unsetFlag(T &out, const int flag)	{	out = T(out & ~flag);	}

	template<class T>
	inline bool isFlagSet(const T &in, const int flag)	{	return (in & flag) != 0;	}
} // namespace TA3D


#define SCREEN_W    (TA3D::VARS::gfx->width)
#define SCREEN_H    (TA3D::VARS::gfx->height)


#ifndef TA3D_MSEC_TIMER
#define TA3D_MSEC_TIMER
#define msec_timer  SDL_GetTicks()
#endif

// TODO Must be removed
using namespace TA3D::VARS;
using namespace TA3D::UTILS;


#endif // __TA3D_NAMESPACE_H__
