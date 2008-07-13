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
#include "cTA3D_Engine.h"			// The engine class.
#include "ta3dbase.h"				// Just for the LANG var
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
#include "misc/osinfo.h"



// below defination defines accuracy of timer i guess.
#define precision   MSEC_TO_TIMER(1)

// timer variable, incremented by preccission, set by main.
volatile uint32	msec_timer = 0;

using namespace TA3D::Exceptions;



// intrupt timer, driven by allgegro.
void Timer()
{
    msec_timer++;				// msec count
}
END_OF_FUNCTION(Timer) /* I guess allegro needs this. */

/*
 ** Function: ReadFileParameter
 **    Notes: This function will eventually load a file given as command line parameter
 **             and run given commands. This is used to start a multiplayer game from
 **             an external Lobby client
 */
void ReadFileParameter( void )
{
    if(!TA3D::VARS::lp_CONFIG || TA3D::VARS::lp_CONFIG->file_param.empty())
        return;

    Console->AddEntry("reading file parameter '%s'", TA3D::VARS::lp_CONFIG->file_param.c_str() );

    cTAFileParser parser( TA3D::VARS::lp_CONFIG->file_param );

    String current_mod = TA3D::VARS::TA3D_CURRENT_MOD;

    TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD = parser.PullAsString("TA3D.MOD", current_mod);
    TA3D::VARS::lp_CONFIG->last_script = ReplaceChar( parser.PullAsString( "TA3D.Script", TA3D::VARS::lp_CONFIG->last_script ), '/', '\\' );
    TA3D::VARS::lp_CONFIG->last_map = ReplaceChar( parser.PullAsString( "TA3D.Map", TA3D::VARS::lp_CONFIG->last_map ), '/', '\\' );
    TA3D::VARS::lp_CONFIG->last_FOW = parser.PullAsInt( "TA3D.FOW", TA3D::VARS::lp_CONFIG->last_FOW );

    if( current_mod != TA3D::VARS::TA3D_CURRENT_MOD ) // Refresh file structure
    {
        delete HPIManager;

        TA3D_clear_cache();		// Clear the cache

        HPIManager = new cHPIHandler("");
        ta3dSideData.loadData();				// Refresh side data so we load the correct values

        delete sound_manager;
        sound_manager = new TA3D::Interfaces::cAudio ( 1.0f, 0.0f, 0.0f );
        sound_manager->StopMusic();
        sound_manager->LoadTDFSounds( true );
        sound_manager->LoadTDFSounds( false );
    }

    if( parser.PullAsBool( "TA3D.Network game" ) )
    {
        if( parser.PullAsBool( "TA3D.Server" ) ) {			// Server code
            char *host_name = strdup( parser.PullAsString( "TA3D.Server name", TA3D::VARS::lp_CONFIG->player_name ).c_str() );
            setup_game( false, host_name );		// Start the game in networking mode as server
            free( host_name );
        }
        else // Client code
        {
            char *host_name = strdup( parser.PullAsString( "TA3D.Server name", "" ).c_str() );
            setup_game( true, host_name );		// Start the game in networking mode as server
            free( host_name );
        }
    }

    TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD = current_mod;

    if( current_mod != TA3D::VARS::TA3D_CURRENT_MOD ) // Refresh file structure
    {
        delete HPIManager;

        TA3D_clear_cache();		// Clear the cache

        HPIManager = new cHPIHandler("");
        ta3dSideData.loadData();				// Refresh side data so we load the correct values

        delete sound_manager;
        sound_manager = new TA3D::Interfaces::cAudio ( 1.0f, 0.0f, 0.0f );
        sound_manager->StopMusic();
        sound_manager->LoadTDFSounds( true );
        sound_manager->LoadTDFSounds( false );
    }
}



void install_TA_files( String def_path = "" );
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
    GuardEnter(ParseCommandLine);

    if(hpiview(argc, argv))
    {
        GuardLeave();
        return 1;
    }

    for( int i = 1 ; i < argc ; ++i)
    {
        if( !strcmp( argv[ i ], "--quick-restart")) // Quick restart mecanism (bypass the intro screen)
        {
            lp_CONFIG->quickstart = true;
            TA3D::Settings::Restore(TA3D::Paths::ConfigFile);		// In case it refuses to restart
            allegro_exit();
        }
        else if( !strcmp( argv[ i ], "--restore" ) )			// Tell TA3D not to display the quickstart confirm dialog
            lp_CONFIG->restorestart = true;
        else if( !strcmp( argv[ i ], "--file-param" ) ) {		// Pass a file as parameter, used for complex things
            if( i + 1 < argc ) {
                i++;
                lp_CONFIG->file_param = argv[ i ];		// Copy the file name
            }
        }
    }

    GuardLeave();

    return 0;
}

/*
 ** Function: main
 **    Notes: Whats this for anyhow? :)  Just kidding, this is where it all begin baby!
 */
int main(int argc, char *argv[])
{
    Logs::level = LOG_LEVEL_DEBUG;
    // Load and prepare output directories
    if (!TA3D::Paths::Initialize(argc, argv, "ta3d"))
        return 1;
    TA3D::Resources::Initialize();
    allegro_init();
    TA3D::System::DisplayInformations();
    TA3D::System::DisplayInformationsAboutAllegro();

    // Initialize signals
    init_signals();

    if( IsExceptionInProgress() ) // if guard threw an error this will be true.
    {
        GuardDisplayAndLogError();   // record and display the error.
        exit(1);                      // we outa here.
    }

    GuardStart( main ); // start guard.
    GuardInfo( "Constructing config.\n" ); // what we are doing.
    TA3D::VARS::lp_CONFIG = new TA3D::TA3DCONFIG;
    GuardCatch();      // close guard
    if( IsExceptionInProgress() ) // if guard threw an error this will be true.
    {
        GuardDisplayAndLogError();   // record and display the error.
        exit(1);                      // we outa here.
    }

    GuardStart( main );
    TA3D::Settings::Load(); /* Load Config File */
    TA3D_clear_cache();
    allegro_exit();
    GuardCatch();

    if( IsExceptionInProgress() )
    {
        GuardDisplayAndLogError();

        delete TA3D::VARS::lp_CONFIG;
        exit(1);
    }

    GuardStart( main );
    if( ParseCommandLine(argc, argv)) 
    {
        delete TA3D::VARS::lp_CONFIG;
        exit(1);
    }
    GuardCatch();

    if( IsExceptionInProgress() )
    {
        GuardDisplayAndLogError();

        delete TA3D::VARS::lp_CONFIG;
        exit(1);
    }

    /*
     ** Allright we have now created our config structure, loaded our config file
     **   and processed our command line arguments, its time to create the main engine
     **   which in turn will create nearly everything it will need.
     */
    TA3D::cTA3D_Engine* Engine = NULL;
    GuardStart( main );
    Engine = new TA3D::cTA3D_Engine;
    GuardCatch();
    if( IsExceptionInProgress() )
    {
        GuardDisplayAndLogError();
        delete TA3D::VARS::lp_CONFIG;
        exit(1);
    }

    /* we can use guard here cause the entine will start in a 'paused' state
     **     it will continue to init code and set itself as ready when all data it needs
     **     to continue is loaded.  at which point we can poll the engine.
     **     Once the engine is unpaused and in a go state we can no longer use GuardStart
     **     as the engine will use it, until the engine exits.
     */
    GuardStart( main );
    Engine->Start();
    GuardCatch();
    if(IsExceptionInProgress())
    {
        GuardDisplayAndLogError();
        // We need to guard deleting the engine in case something bad goes wrong during
        //   the cleanup process.
        GuardStart( main );
        delete Engine;
        delete TA3D::VARS::lp_CONFIG;
        GuardCatch();
        exit(1);
    }

    // ok, if we are here, our thread in engine class is running
    //   and doing some work loading up alot of crap so while its doing that
    //   we are going to show our intro, but first we need to start our timer.
    install_int_ex( Timer, precision);
    start = msec_timer;      // Initalize timer.

    // while our engine does some loading and intializing, lets show our intro.
    if( !lp_CONFIG->quickstart && lp_CONFIG->file_param.empty())
    {
        GuardStart( intro );
        Menus::Intro::Execute();
        GuardCatch();
        if( IsExceptionInProgress())
        {
            GuardDisplayAndLogError();
            // We need to guard deleting the engine in case something bad goes wrong during
            //   the cleanup process, note that this might not exit right away because it
            //   might take a few seconds to kill the thread.
            GuardStart( intro );
            delete Engine;
            delete TA3D::VARS::lp_CONFIG;
            GuardCatch();
            exit(1);
        }
    }

    // The main menu call will eventually not be here, instead
    //   we will turn control over to our engine, but for now we'll call the
    //   menu this way.
    // Engine->UnPause(); // unpause the engine, the engine will finish initing
    // While( !Engine->Ready() ) { rest( 100 ); } // while its not ready rest a bit.
    //  engines thread should now be running and managing everything for us.
    //  we will continue to keep checking if its running and if so call a helper function
    //  in the engine to do some non-cricticle stuff, every 100 ms or so.
    // While( Engine-> IsRunning() ) { Engine->DoSomeNonImportantShit(); rest( 100 ); }
    // main_menu();
    TA3D::Menus::MainMenu::Execute();

    // if we get here its time to exit, so delete the engine, the engine should clean itself
    //   up and we are outa here, but first lets save our config file, we
    //   need to try/catch this but no worries for now since its not doing anything.
    GuardStart( main );
    TA3D::Settings::Save();
    GuardCatch();

    if(IsExceptionInProgress())
    {
        GuardDisplayAndLogError();
        GuardStart( main );
        delete Engine;
        delete TA3D::VARS::lp_CONFIG;
        GuardCatch();

        exit(1);
    }

    GuardStart(main)
        delete Engine;
    GuardCatch();

    // if something bad happens log and show the error but don't exit
    //    so that we can delete the config var next.
    if( IsExceptionInProgress() )
        GuardDisplayAndLogError();

    bool quickrestart = lp_CONFIG->quickrestart;
    bool restorestart = lp_CONFIG->restorestart;

    GuardStart( main )
        delete TA3D::VARS::lp_CONFIG;
    GuardCatch();

    I18N::Destroy();

    LOG_INFO("Exiting...");
    if( !quickrestart )
        return 0; 		// thats it folks.
    else if( !restorestart )
        exit(2);		// ask the monitoring script to restart the program with --quick-restart parameter
    else
        exit(3);		// ask the monitoring script to restart the program with --quick-restart --restore parameters
}
END_OF_MAIN()
/* Allegro needs END_OF_MAIN() I guess. */ 

