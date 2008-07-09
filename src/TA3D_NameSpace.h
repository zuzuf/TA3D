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


# include "TA3D_Exception.h"
# include "misc/interface.h"
# include "cError.h"
# ifndef TA3D_NO_SOUND // Only for the hpiview program
#   include "TA3D_Audio.h"
# else
#   include "misc/vector.h"
#   include "cTAFileParser.h"
# endif
# include "TA3D_hpi.h"
# include "gfx/gfx.h"
# include "console.h"
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

		String	net_server;					// The server that monitor the game server list

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

	namespace UTILS
	{
		// Some useful math and template functions
		TA3D_API_SI float  Deg2Rad (float Deg)  { return (Deg * 0.017453292f); }
		TA3D_API_SI double Deg2Rad (double Deg) { return (Deg * 0.017453292);  }
		TA3D_API_SI float  Rad2Deg (float Rad)  { return (Rad * 57.29578122f); }
		TA3D_API_SI double Rad2Deg (double Rad) { return (Rad * 57.29578122);  }  

		namespace HPI
		{
		}
	}

	namespace VARS
	{
		TA3D_API_E TA3D::IInterfaceManager			*InterfaceManager; 
		TA3D_API_E TA3D::UTILS::HPI::cHPIHandler	*HPIManager;
		TA3D_API_E TA3D::TA3D_DEBUG::cConsole		*Console;
        # ifndef TA3D_NO_SOUND // Only for the hpiview program
		TA3D_API_E TA3D::Interfaces::cAudio			*sound_manager;
        # endif
		TA3D_API_E TA3D::Interfaces::GFX			*gfx;

		TA3D_API_E RGB								*pal;
		TA3D_API_E TA3D::TA3DCONFIG					*lp_CONFIG;

		TA3D_API_E uint8							unit_engine_thread_sync;
		TA3D_API_E uint8							weapon_engine_thread_sync;
		TA3D_API_E uint8							particle_engine_thread_sync;
		TA3D_API_E uint8							players_thread_sync;

		TA3D_API_E ObjectSync						*ThreadSynchroniser;
		TA3D_API_E String							TA3D_CURRENT_MOD;
		TA3D_API_E int								ascii_to_scancode[ 256 ];

		namespace CONSTANTS
		{
			// Common math related macros
			// Pi, our favorite number
#undef PI
#define PI 3.141592653589793238462643383279502884197169399375105f
#define DB_PI 6.28318530717958647693f

			// Square root of 2
#undef SQRT2
#define SQRT2 1.414213562373095048801688724209698078569671875376948

			// Square root of 2 over 2
#undef SQRT2OVER2
#define SQRT2OVER2 0.707106781186547524400844362104849039284835937688474

			// Square root of 3
#undef SQRT3
#define SQRT3 1.732050807568877293527446341505872366942805253810381

			// Square root of 3 over 2
#undef SQRT3OVER2
#define SQRT3OVER2 0.866025403784438646763723170752936183471402626905190

			// The natural number "e"
#undef NAT_E
#define NAT_E 2.718281828459045235360287471352662497757247093699959


			const uint32 uint32max =  0xffffffff;
			const uint32 uint32min =           0;
			const sint32 sint32max =  0x7fffffff;
			const sint32 sint32min = (-2147483647 - 1);

			const uint16 uint16max =  0xffff;
			const uint16 uint16min =       0;
			const sint16 sint16max =  0x7fff;
			const sint16 sint16min = -0x8000;

			const uint8  uint8max  =  0xff;
			const uint8  uint8min  =     0;
			const sint8  sint8max  =  0x7f;
			const sint8  sint8min  = -0x80;

			const uchar  ucharmax  =  0xff;
			const uchar  ucharmin  =     0;
			const schar  scharmax  =  0x7f;
			const schar  scharmin  = -0x80;

			// Some constant data needed by the engine ( like number of ticks/sec. to simulate )
#define TICKS_PER_SEC				30
#define MAX_CODE_PER_TICK			100
		}
	}

	FILE *TA3D_OpenFile( const String &FileName, const String Mode );

    /*!
    ** \brief Remove final comments (ex: 'somecode(); // here are my comments') and trim the string
    ** \param s The string to parse
    ** \return The string without final comments
    */
	String RemoveComments(const String& s);


    /*!
    ** \brief Split a string based on a list of separators
    **
    ** \param[out] lst All found parts
    ** \param s The string to split
    ** \param seps Sequence of chars considered as a separator
    ** \param emptyBefore rue to clear the vector before fulfill it
    ** \warning Do not take care of string representation (with `'` or `"`)
    */
	void ReadVectorString(String::Vector& lst, String s, const String& seps = ",",
                          const bool emptyBefore = true);


	char *replace_chars(char *str);
	void *GetMem( sint32 size, sint32 zero );


	void TA3D_clear_cache();							// Clear the cache if needed (useful when mod has changed)

} // namespace TA3D



// TODO Must be removed
using namespace TA3D;
using namespace TA3D::UTILS;

template<class T> static void Swap (T &a, T &b)
{
	T temp;
	temp = a;
	a = b;
	b = temp;
	return;
}


// bit field mask operation macros
template<class T> static void SetMask (T &var, T &mask)
{
	var |= mask;
	return;
}


template<class T> static void UnsetMask (T &var, T &mask)
{
	var |= ~mask;
	return;
}


// ordinal bit manipulation macros  (least significant bit = bit "0")
template<class T> static T Bit (T &bit)
{
	return (1 << bit);
}

template<class T> static void SetBit (T &var, T &bit)
{
	return (SetMask (var, Bit(bit)));
}


template<class T> static void UnsetBit (T &var, T &bit)
{
    return (UnsetMask (var, Bit(bit)));
}


// Returns -1/1/0 for sign of a variable
template<class T> static T Sign (T x)
{
	if (x > 0)
		return (1);
	else
	if ( x < 0)
		return (-1);
	//else
	return (0);
}

#ifndef TA3D_MSEC_TIMER
#define TA3D_MSEC_TIMER
extern volatile uint32	msec_timer;
#endif

// TODO Must be removed
using namespace TA3D;
using namespace TA3D::VARS;
using namespace TA3D::TA3D_DEBUG;


#endif // __TA3D_NAMESPACE_H__
