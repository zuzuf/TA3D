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

/*
 **  File: main.cpp
 ** Notes: The applications main entry point.
 */

#include "stdafx.h"					// standard pch inheritance.
#include "TA3D_NameSpace.h"			// our namespace, a MUST have.
#include "intro.h"					// intro prototypes,   TODO: phase out.
#include "menu.h"					// menu prototypes      TODO: phase out.
#include "engine.h"		       		// The engine class.
//#include "ta3dbase.h"				// Just for the LANG var
#include "EngineClass.h"
#include "backtrace.h"				// Some debugging tools
#include "misc/paths.h"
#include "misc/resources.h"
#include "logs/logs.h"
#include "misc/settings.h"
#include "ingame/sidedata.h"
#include "ingame/menus/intro.h"
#include "ingame/menus/mainmenu.h"
#include "languages/i18n.h"
#include "misc/application.h"
#include "sounds/manager.h"


/*
** Function: ReadFileParameter
**    Notes: This function will eventually load a file given as command line parameter
**             and run given commands. This is used to start a multiplayer game from
**             an external Lobby client
*/
void ReadFileParameter()
{
	if(!TA3D::VARS::lp_CONFIG || TA3D::VARS::lp_CONFIG->file_param.empty())
		return;

	LOG_DEBUG("Reading file parameter `" << TA3D::VARS::lp_CONFIG->file_param << "`...");

	TDFParser parser(TA3D::VARS::lp_CONFIG->file_param);

	String current_mod = TA3D::VARS::TA3D_CURRENT_MOD;

	TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD = parser.pullAsString("TA3D.MOD", current_mod);
	TA3D::VARS::lp_CONFIG->last_script = ReplaceChar( parser.pullAsString( "TA3D.Script", TA3D::VARS::lp_CONFIG->last_script ), '/', '\\' );
	TA3D::VARS::lp_CONFIG->last_map = ReplaceChar( parser.pullAsString( "TA3D.Map", TA3D::VARS::lp_CONFIG->last_map ), '/', '\\' );
	TA3D::VARS::lp_CONFIG->last_FOW = parser.pullAsInt( "TA3D.FOW", TA3D::VARS::lp_CONFIG->last_FOW );

	if (current_mod != TA3D::VARS::TA3D_CURRENT_MOD) // Refresh file structure
	{
		delete HPIManager;
		TA3D_clear_cache();		// Clear the cache

		HPIManager = new TA3D::UTILS::HPI::cHPIHandler();
		ta3dSideData.loadData();				// Refresh side data so we load the correct values
		delete sound_manager;
		sound_manager = new TA3D::Audio::Manager();
		sound_manager->stopMusic();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);
	}

	if (parser.pullAsBool("TA3D.Network game"))
	{
		if (parser.pullAsBool("TA3D.Server"))// Server code
		{
			String host_name = parser.pullAsString( "TA3D.Server name", TA3D::VARS::lp_CONFIG->player_name );
			setup_game( false, host_name.c_str() );		// Start the game in networking mode as server
		}
		else // Client code
		{
			String host_name = parser.pullAsString( "TA3D.Server name", "" );
			setup_game( true, host_name.c_str() );		// Start the game in networking mode as server
		}
	}

	TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD = current_mod;

	if (current_mod != TA3D::VARS::TA3D_CURRENT_MOD) // Refresh file structure
	{
		delete HPIManager;
		TA3D_clear_cache();		// Clear the cache

		HPIManager = new TA3D::UTILS::HPI::cHPIHandler();
		ta3dSideData.loadData();				// Refresh side data so we load the correct values
		delete sound_manager;
		sound_manager = new TA3D::Audio::Manager();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);
	}
}



int hpiview(int argc,char *argv[]);

/*
** Function: ParseCommandLine
**    Notes: this will eventually break down any command line arguments passed to
**              the application at run time.  It don't do anything yet, but eventually
**              we will be adding lots of command parms.
**           If something goes wrong you can safely throw a string for an error.
**             The call to this function is tried, but it only catches exceptions
**             and strings, ie throw( "LoadConfigFile: some error occured" );
**           Remember if you throw an error, or generate one, you are responsible for
**             cleaning up what you initialized!
*/
int ParseCommandLine(int argc, char *argv[])
{
	if (hpiview(argc, argv))
		return 1;

	if (argc > 1)
	{
		// Argument converted to a TA3D::String
		String arg;

		for (int i = 1 ; i < argc ; ++i)
		{
			arg = argv[i];
			if ("--quick-start" == arg) // Quick restart mecanism (bypass the intro screen)
				lp_CONFIG->quickstart = true;
			else
			{
				if ("--file-param" == arg) // Pass a file as parameter, used for complex things
				{
					if (i + 1 < argc)
					{
						++i;
						lp_CONFIG->file_param = argv[i]; // Copy the file name
					}
				}
				else
				{
					if ("--test" == arg) // Runs some tests (to help find and fix bugs)
					{
						GFX::runTests();
						return 1;
					}
					if ("--opengl-test" == arg) // Runs some tests (to help find and fix bugs)
					{
						GFX::runOpenGLTests();
						return 1;
					}
				}
			}
		}
	}
	return 0;
}



int main(int argc, char *argv[])
{
    // Initialize signals
    init_signals();
    // Constructing config
    TA3D::VARS::lp_CONFIG = new TA3D::TA3DCONFIG;

    // Initialize all modules used by ta3d
	TA3D::Initialize(argc, argv, "ta3d");

	TA3D::Settings::Load(); /* Load Config File */
	TA3D_clear_cache();

	if (ParseCommandLine(argc, argv))
		return 1;

	// Initializing the TA3D Engine
	TA3D::Engine engine;

	// Starting the Engine
	engine.start();

	// ok, if we are here, our thread in engine class is running
	// and doing some work loading up alot of crap so while its doing that
	// we are going to show our intro, but first we need to start our timer.
	start = msec_timer; // Initalize timer.

	// while our engine does some loading and intializing, lets show our intro.
	if( !lp_CONFIG->quickstart && lp_CONFIG->file_param.empty())
		Menus::Intro::Execute();

	// The main menu call will eventually not be here, instead
	// we will turn control over to our engine, but for now we'll call the
	// menu this way.
	TA3D::Menus::MainMenu::Execute();

	// if we get here its time to exit, so delete the engine, the engine should clean itself
	//   up and we are outa here, but first lets save our config file, we
	//   need to try/catch this but no worries for now since its not doing anything.
	TA3D::Settings::Save();

	return 0;
}

