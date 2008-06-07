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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif

#include "stdafx.h"					// standard pch inheritance.
#include "TA3D_NameSpace.h"			// our namespace, a MUST have.
#include "intro.h"					// intro prototypes,   TODO: phase out.
#include "menu.h"					// menu prototypes      TODO: phase out.
#include "cTA3D_Engine.h"			// The engine class.
#include "ta3dbase.h"				// Just for the LANG var
#include "EngineClass.h"
#include "backtrace.h"				// Some debugging tools

// below defination defines accuracy of timer i guess.
#define precision   MSEC_TO_TIMER(1)

// timer variable, incremented by preccission, set by main.
volatile uint32	msec_timer = 0;

using namespace TA3D::EXCEPTION;
// intrupt timer, driven by allgegro.
void Timer()
{
	msec_timer++;				// msec count
}
END_OF_FUNCTION(Timer); /* I guess allegro needs this. */

/*
** Function: ReadFileParameter
**    Notes: This function will eventually load a file given as command line parameter
**             and run given commands. This is used to start a multiplayer game from
**             an external Lobby client
*/
void ReadFileParameter( void )
{
	if( TA3D::VARS::lp_CONFIG == NULL || TA3D::VARS::lp_CONFIG->file_param.empty() )	return;		// Nothing to do
	
	Console->AddEntry("reading file parameter '%s'", TA3D::VARS::lp_CONFIG->file_param.c_str() );
	
	cTAFileParser parser( TA3D::VARS::lp_CONFIG->file_param );
	
	String current_mod = TA3D::VARS::TA3D_CURRENT_MOD;

	TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD = parser.PullAsString("TA3D.MOD", current_mod);
	TA3D::VARS::lp_CONFIG->last_script = ReplaceChar( parser.PullAsString( "TA3D.Script", TA3D::VARS::lp_CONFIG->last_script ), '/', '\\' );
	TA3D::VARS::lp_CONFIG->last_map = ReplaceChar( parser.PullAsString( "TA3D.Map", TA3D::VARS::lp_CONFIG->last_map ), '/', '\\' );
	TA3D::VARS::lp_CONFIG->last_FOW = parser.PullAsInt( "TA3D.FOW", TA3D::VARS::lp_CONFIG->last_FOW );

	if( current_mod != TA3D::VARS::TA3D_CURRENT_MOD ) {		// Refresh file structure
		delete HPIManager;
		
		TA3D_clear_cache();		// Clear the cache
		
		HPIManager = new cHPIHandler("");
		ta3d_sidedata.load_data();				// Refresh side data so we load the correct values

		delete sound_manager;
		sound_manager = new TA3D::INTERFACES::cAudio ( 1.0f, 0.0f, 0.0f );
		sound_manager->StopMusic();
		sound_manager->LoadTDFSounds( true );
		sound_manager->LoadTDFSounds( false );
		}

	if( parser.PullAsBool( "TA3D.Network game" ) ) {
		if( parser.PullAsBool( "TA3D.Server" ) ) {			// Server code
			char *host_name = strdup( parser.PullAsString( "TA3D.Server name", TA3D::VARS::lp_CONFIG->player_name ).c_str() );
			setup_game( false, host_name );		// Start the game in networking mode as server
			free( host_name );
			}
		else {												// Client code
			char *host_name = strdup( parser.PullAsString( "TA3D.Server name", "" ).c_str() );
			setup_game( true, host_name );		// Start the game in networking mode as server
			free( host_name );
			}
		}
	
	TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD = current_mod;

	if( current_mod != TA3D::VARS::TA3D_CURRENT_MOD ) {		// Refresh file structure
		delete HPIManager;
		
		TA3D_clear_cache();		// Clear the cache
		
		HPIManager = new cHPIHandler("");
		ta3d_sidedata.load_data();				// Refresh side data so we load the correct values

		delete sound_manager;
		sound_manager = new TA3D::INTERFACES::cAudio ( 1.0f, 0.0f, 0.0f );
		sound_manager->StopMusic();
		sound_manager->LoadTDFSounds( true );
		sound_manager->LoadTDFSounds( false );
		}
}

/*
** Function: LoadConfigFile
**    Notes: This function will eventually load our config file if it exists.
**             config files will be stored as 'tdf' format and thus loaded as text,
**             using the cTAFileParser class.
**           If something goes wrong you can safely throw a string for an error.
**             The call to this function is tried, but it only catches exceptions
**             and strings, ie throw( "LoadConfigFile: some error occured" );
*/
void LoadConfigFile( void )
{
	GuardEnter( LoadConfigFile );

	cTAFileParser *cfgFile;

	try { // we need to try catch this cause the config file may not exists
		 // and if it don't exists it will throw an error on reading it, which
		 // will be caught in our main function and the application will exit.
		cfgFile = new TA3D::UTILS::cTAFileParser( TA3D_OUTPUT_DIR + "ta3d.cfg" );
	}
	catch( ... )
	{
		GuardLeave();
		return;
	}

	TA3D::VARS::lp_CONFIG->fps_limit = cfgFile->PullAsFloat( "TA3D.FPS Limit" );
	TA3D::VARS::lp_CONFIG->shadow_r  = cfgFile->PullAsFloat( "TA3D.Shadow R" );
	TA3D::VARS::lp_CONFIG->timefactor = cfgFile->PullAsFloat( "TA3D.Time Factor" );

	TA3D::VARS::lp_CONFIG->shadow_quality = cfgFile->PullAsInt( "TA3D.Shadow Quality" );
	TA3D::VARS::lp_CONFIG->priority_level = cfgFile->PullAsInt( "TA3D.Priority Level" );
	TA3D::VARS::lp_CONFIG->fsaa = cfgFile->PullAsInt( "TA3D.FSAA" );
	TA3D::VARS::lp_CONFIG->Lang = cfgFile->PullAsInt( "TA3D.Language" );
	TA3D::VARS::lp_CONFIG->water_quality = cfgFile->PullAsInt( "TA3D.Water Quality" );
	TA3D::VARS::lp_CONFIG->screen_width = cfgFile->PullAsInt( "TA3D.Screen Width" );
	TA3D::VARS::lp_CONFIG->screen_height = cfgFile->PullAsInt( "TA3D.Screen Height" );
	TA3D::VARS::lp_CONFIG->color_depth = cfgFile->PullAsInt( "TA3D.Color Depth", 32 );

	TA3D::VARS::lp_CONFIG->showfps = cfgFile->PullAsBool( "TA3D.Show FPS" );
	TA3D::VARS::lp_CONFIG->wireframe = cfgFile->PullAsBool( "TA3D.Show Wireframe" );
	TA3D::VARS::lp_CONFIG->particle = cfgFile->PullAsBool( "TA3D.Show particles" );
	TA3D::VARS::lp_CONFIG->waves = cfgFile->PullAsBool( "TA3D.Show Waves" );
	TA3D::VARS::lp_CONFIG->shadow = cfgFile->PullAsBool( "TA3D.Show Shadows" );
	TA3D::VARS::lp_CONFIG->height_line = cfgFile->PullAsBool( "TA3D.Show Height Lines" );
	TA3D::VARS::lp_CONFIG->fullscreen = cfgFile->PullAsBool( "TA3D.Show FullScreen", false );
	TA3D::VARS::lp_CONFIG->detail_tex = cfgFile->PullAsBool( "TA3D.Detail Texture" );
	TA3D::VARS::lp_CONFIG->draw_console_loading = cfgFile->PullAsBool( "TA3D.Draw Console Loading" );

	TA3D::VARS::lp_CONFIG->last_script = ReplaceChar( cfgFile->PullAsString( "TA3D.Last Script", "scripts\\default.c" ), '/', '\\' );
	TA3D::VARS::lp_CONFIG->last_map = ReplaceChar( cfgFile->PullAsString( "TA3D.Last Map", "" ), '/', '\\' );
	TA3D::VARS::lp_CONFIG->last_FOW = cfgFile->PullAsInt( "TA3D.Last FOW", 0 );
	TA3D::VARS::lp_CONFIG->last_MOD = cfgFile->PullAsString( "TA3D.Last MOD", "" );

	TA3D::VARS::lp_CONFIG->camera_zoom = cfgFile->PullAsInt( "TA3D.Camera Zoom Mode", ZOOM_NORMAL );
	TA3D::VARS::lp_CONFIG->camera_def_angle = cfgFile->PullAsFloat( "TA3D.Camera Default Angle", 63.44f );
	TA3D::VARS::lp_CONFIG->camera_def_h = cfgFile->PullAsFloat( "TA3D.Camera Default Height", 200.0f );
	TA3D::VARS::lp_CONFIG->camera_zoom_speed = cfgFile->PullAsFloat( "TA3D.Camera Zoom Speed", 1.0f );

	TA3D::VARS::lp_CONFIG->use_texture_cache = cfgFile->PullAsBool( "TA3D.Use Texture Cache", false );

	TA3D::VARS::lp_CONFIG->skin_name = cfgFile->PullAsString( "TA3D.Skin", "" );

	TA3D::VARS::lp_CONFIG->net_server = cfgFile->PullAsString( "TA3D.Net Server", "ta3d.darkstars.co.uk" );

	TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD;

	TA3D::VARS::lp_CONFIG->player_name = cfgFile->PullAsString( "TA3D.Player name", "player" );

	delete cfgFile; 

	LANG = lp_CONFIG->Lang;

	GuardLeave();
}

void makeBackup( const String FileName )
{
	if( TA3D_exists( FileName ) ) {
		FILE *src = TA3D_OpenFile( FileName, "rb" );
		if( src ) {
			FILE *dst = TA3D_OpenFile( FileName + ".bak", "wb" );
			if( dst ) {
				uint32 src_size = FILE_SIZE( FileName.c_str() );
				byte *buf = new byte[ src_size ];
				fread( buf, src_size, 1, src );
				fwrite( buf, src_size, 1, dst );
				delete[] buf;
				fclose( dst );
				}
			fclose( src );
			}
		}
}

void restoreBackup( const String FileName )
{
	if( TA3D_exists( FileName + ".bak" ) ) {
		FILE *src = TA3D_OpenFile( FileName + ".bak", "rb" );
		if( src ) {
			FILE *dst = TA3D_OpenFile( FileName, "wb" );
			if( dst ) {
				uint32 src_size = FILE_SIZE( (FileName + ".bak").c_str() );
				byte *buf = new byte[ src_size ];
				fread( buf, src_size, 1, src );
				fwrite( buf, src_size, 1, dst );
				delete[] buf;
				fclose( dst );
				}
			fclose( src );
			}
		}
}

/*
** Function: SaveConfigFile
**    Notes: Upon application exit this will write out our config file.
**             note that if the application fails to startup then a config file
**             will never be generated. See LoadConfigFile for notes on format.
**           If something goes wrong you can safely throw a string for an error.
**             The call to this function is tried, but it only catches exceptions
**             and strings, ie throw( "LoadConfigFile: some error occured" );
*/
void SaveConfigFile( void )
{
	GuardEnter( SaveConfigFile );
	lp_CONFIG->Lang = LANG;

	if( !TA3D::VARS::lp_CONFIG )
	{
		GuardLeave();
		return;
	}
	String FileName = TA3D_OUTPUT_DIR + "ta3d.cfg";

	makeBackup( FileName );				// Make a copy that can be restored if TA3D doesn't start any more

	std::ofstream   m_File;

	m_File.open( FileName.c_str(), std::ios::out | std::ios::trunc );

	if( !m_File.is_open() )
	{
		GuardLeave();
		return;
	}

	m_File << "[TA3D]\n{\n";
	m_File << "            FPS Limit=" << TA3D::VARS::lp_CONFIG->fps_limit << ";\n";
	m_File << "             Shadow R=" << TA3D::VARS::lp_CONFIG->shadow_r << ";\n";
	m_File << "          Time Factor=" << TA3D::VARS::lp_CONFIG->timefactor << ";\n";
	m_File << "       Shadow Quality=" << TA3D::VARS::lp_CONFIG->shadow_quality << ";// 0->100\n";
	m_File << "       Priority Level=" << TA3D::VARS::lp_CONFIG->priority_level << ";// 0, 1, 2\n";
	m_File << "                 FSAA=" << TA3D::VARS::lp_CONFIG->fsaa << ";\n";
	m_File << "             Language=" << TA3D::VARS::lp_CONFIG->Lang << ";\n";
	m_File << "        Water Quality=" << TA3D::VARS::lp_CONFIG->water_quality << ";//0->4\n";
	m_File << "         Screen Width=" << TA3D::VARS::lp_CONFIG->screen_width << ";\n";
	m_File << "        Screen Height=" << TA3D::VARS::lp_CONFIG->screen_height << ";\n";
	m_File << "          Color Depth=" << (int)TA3D::VARS::lp_CONFIG->color_depth << ";\n";
	m_File << "             Show FPS=" << TA3D::VARS::lp_CONFIG->showfps << ";\n";
	m_File << "       Show Wireframe=" << TA3D::VARS::lp_CONFIG->wireframe << ";\n";
	m_File << "       Show particles=" << TA3D::VARS::lp_CONFIG->particle << ";\n";
	m_File << "           Show Waves=" << TA3D::VARS::lp_CONFIG->waves << ";\n";
	m_File << "         Show Shadows=" << TA3D::VARS::lp_CONFIG->shadow << ";\n";
	m_File << "    Show Height Lines=" << TA3D::VARS::lp_CONFIG->height_line << ";\n";
	m_File << "      Show FullScreen=" << TA3D::VARS::lp_CONFIG->fullscreen << ";\n";
	m_File << "       Detail Texture=" << TA3D::VARS::lp_CONFIG->detail_tex << ";\n";
	m_File << " Draw Console Loading=" << TA3D::VARS::lp_CONFIG->draw_console_loading << ";\n";
	m_File << "          Last Script=" << ReplaceChar( TA3D::VARS::lp_CONFIG->last_script, '\\', '/' ) << ";\n";
	m_File << "             Last Map=" << ReplaceChar( TA3D::VARS::lp_CONFIG->last_map, '\\', '/' ) << ";\n";

	m_File << "             Last FOW=" << (int)TA3D::VARS::lp_CONFIG->last_FOW << ";\n";
	TA3D::VARS::lp_CONFIG->last_MOD = TA3D::VARS::TA3D_CURRENT_MOD;
	m_File << "             Last MOD=" << TA3D::VARS::lp_CONFIG->last_MOD << ";\n";
	m_File << "          Player name=" << TA3D::VARS::lp_CONFIG->player_name << ";\n";
	m_File << "     Camera Zoom Mode=" << (int)TA3D::VARS::lp_CONFIG->camera_zoom << ";\n";
	m_File << " Camera Default Angle=" << TA3D::VARS::lp_CONFIG->camera_def_angle << ";\n";
	m_File << "Camera Default Height=" << TA3D::VARS::lp_CONFIG->camera_def_h << ";\n";
	m_File << "    Camera Zoom Speed=" << TA3D::VARS::lp_CONFIG->camera_zoom_speed << ";\n";
	m_File << "                 Skin=" << TA3D::VARS::lp_CONFIG->skin_name << ";\n";
	m_File << "    Use Texture Cache=" << TA3D::VARS::lp_CONFIG->use_texture_cache << ";\n";
	m_File << "           Net Server=" << TA3D::VARS::lp_CONFIG->net_server << ";\n";
	m_File << "}\n";

	m_File.flush();
	m_File.close();

	GuardLeave();
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
int ParseCommandLine( int argc, char *argv[] )
{
	GuardEnter( ParseCommandLine );

	if( hpiview( argc, argv ) ) {				// Run hpiview
		GuardLeave();
		return 1;							// We're done
		}

	for( int i = 1 ; i < argc ; i++ ) {
		if( !strcmp( argv[ i ], "--quick-restart" ) ) {			// Quick restart mecanism (bypass the intro screen)
			lp_CONFIG->quickstart = true;
			allegro_init();
			restoreBackup( TA3D_OUTPUT_DIR + "ta3d.cfg" );		// In case it refuses to restart
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
int main(int argc,char *argv[])
{
	init_signals();
	
	GuardStart( main ); // start guard.
		GuardInfo( "Preparing output dir\n" ); // what we are doing.
		CheckOutputDir();
	GuardCatch();      // close guard
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
		LoadConfigFile(); /* Load Config File */

		allegro_init();
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
		if( ParseCommandLine( argc, argv ) ) {   /* process any command line args passed */
												// Job done, exit
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
	TA3D::cTA3D_Engine *Engine = NULL;    // class pointer to our engine.
	GuardStart( main );
		Engine = new TA3D::cTA3D_Engine;  // Create and intialize engine.
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
	if( IsExceptionInProgress() )
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
	if( !lp_CONFIG->quickstart && lp_CONFIG->file_param.empty() ) {
		GuardStart( intro );
			play_intro();
		GuardCatch();
		if( IsExceptionInProgress() )
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
	main_menu();

	// if we get here its time to exit, so delete the engine, the engine should clean itself
	//   up and we are outa here, but first lets save our config file, we
	//   need to try/catch this but no worries for now since its not doing anything.
	GuardStart( main );
		SaveConfigFile();
	GuardCatch();

	if( IsExceptionInProgress() )
	{
		GuardDisplayAndLogError();
		GuardStart( main );
			delete Engine;
			delete TA3D::VARS::lp_CONFIG;
		GuardCatch();

		exit(1);
	}

	GuardStart( main )
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

	if( !quickrestart )
		return 0; 		// thats it folks.
	else if( !restorestart )
		exit(2);		// ask the monitoring script to restart the program with --quick-restart parameter
	else
		exit(3);		// ask the monitoring script to restart the program with --quick-restart --restore parameters
}
END_OF_MAIN();
/* Allegro needs END_OF_MAIN(); I guess. */ 

