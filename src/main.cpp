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

	TA3D::VARS::lp_CONFIG->showfps = cfgFile->PullAsBool( "TA3D.Show FPS" );
	TA3D::VARS::lp_CONFIG->wireframe = cfgFile->PullAsBool( "TA3D.Show Wireframe" );
	TA3D::VARS::lp_CONFIG->particle = cfgFile->PullAsBool( "TA3D.Show particles" );
	TA3D::VARS::lp_CONFIG->waves = cfgFile->PullAsBool( "TA3D.Show Waves" );
	TA3D::VARS::lp_CONFIG->shadow = cfgFile->PullAsBool( "TA3D.Show Shadows" );
	TA3D::VARS::lp_CONFIG->height_line = cfgFile->PullAsBool( "TA3D.Show Height Lines" );
	TA3D::VARS::lp_CONFIG->fullscreen = cfgFile->PullAsBool( "TA3D.Show FullScreen" );
	TA3D::VARS::lp_CONFIG->detail_tex = cfgFile->PullAsBool( "TA3D.Detail Texture" );
	TA3D::VARS::lp_CONFIG->draw_console_loading = cfgFile->PullAsBool( "TA3D.Draw Console Loading" );

	TA3D::VARS::lp_CONFIG->last_script = cfgFile->PullAsString( "TA3D.Last Script", "scripts\\default.c" );
	TA3D::VARS::lp_CONFIG->last_map = cfgFile->PullAsString( "TA3D.Last Map", "" );
	TA3D::VARS::lp_CONFIG->last_FOW = cfgFile->PullAsInt( "TA3D.Last FOW", 0 );
	TA3D::VARS::lp_CONFIG->last_MOD = cfgFile->PullAsString( "TA3D.Last MOD", "" );

	TA3D::VARS::lp_CONFIG->camera_zoom = cfgFile->PullAsInt( "TA3D.Camera Zoom Mode", ZOOM_NORMAL );
	TA3D::VARS::lp_CONFIG->camera_def_angle = cfgFile->PullAsFloat( "TA3D.Camera Default Angle", 63.44f );
	TA3D::VARS::lp_CONFIG->camera_def_h = cfgFile->PullAsFloat( "TA3D.Camera Default Height", 200.0f );
	TA3D::VARS::lp_CONFIG->camera_zoom_speed = cfgFile->PullAsFloat( "TA3D.Camera Zoom Speed", 1.0f );

	TA3D::VARS::TA3D_CURRENT_MOD = TA3D::VARS::lp_CONFIG->last_MOD;

	TA3D::VARS::lp_CONFIG->player_name = cfgFile->PullAsString( "TA3D.Player name", "player" );

	delete cfgFile; 

	LANG = lp_CONFIG->Lang;

	GuardLeave();
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
	m_File << "             Show FPS=" << TA3D::VARS::lp_CONFIG->showfps << ";\n";
	m_File << "       Show Wireframe=" << TA3D::VARS::lp_CONFIG->wireframe << ";\n";
	m_File << "       Show particles=" << TA3D::VARS::lp_CONFIG->particle << ";\n";
	m_File << "           Show Waves=" << TA3D::VARS::lp_CONFIG->waves << ";\n";
	m_File << "         Show Shadows=" << TA3D::VARS::lp_CONFIG->shadow << ";\n";
	m_File << "    Show Height Lines=" << TA3D::VARS::lp_CONFIG->height_line << ";\n";
	m_File << "      Show FullScreen=" << TA3D::VARS::lp_CONFIG->fullscreen << ";\n";
	m_File << "       Detail Texture=" << TA3D::VARS::lp_CONFIG->detail_tex << ";\n";
	m_File << " Draw Console Loading=" << TA3D::VARS::lp_CONFIG->draw_console_loading << ";\n";
	m_File << "          Last Script=" << TA3D::VARS::lp_CONFIG->last_script << ";\n";
	m_File << "             Last Map=" << TA3D::VARS::lp_CONFIG->last_map << ";\n";
	m_File << "             Last FOW=" << (int)TA3D::VARS::lp_CONFIG->last_FOW << ";\n";
	TA3D::VARS::lp_CONFIG->last_MOD = TA3D::VARS::TA3D_CURRENT_MOD;
	m_File << "             Last MOD=" << TA3D::VARS::lp_CONFIG->last_MOD << ";\n";
	m_File << "          Player name=" << TA3D::VARS::lp_CONFIG->player_name << ";\n";
	m_File << "     Camera Zoom Mode=" << (int)TA3D::VARS::lp_CONFIG->camera_zoom << ";\n";
	m_File << " Camera Default Angle=" << TA3D::VARS::lp_CONFIG->camera_def_angle << ";\n";
	m_File << "Camera Default Height=" << TA3D::VARS::lp_CONFIG->camera_def_h << ";\n";
	m_File << "    Camera Zoom Speed=" << TA3D::VARS::lp_CONFIG->camera_zoom_speed << ";\n";
	m_File << "}\n";

	m_File.flush();
	m_File.close();

	GuardLeave();
}

void install_TA_files( String def_path = "" );

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
void ParseCommandLine( int argc, char *argv[] )
{
	GuardEnter( ParseCommandLine );

	if( argc >= 2 && strcasecmp( argv[ 1 ], "install" ) == 0 ) {			// Installation of TA's files
		install_TA_files( argc >= 3 ? argv[2] : "" );
		exit(0);
		}

	GuardLeave();
}

/*
** Function: main
**    Notes: Whats this for anyhow? :)  Just kidding, this is where it all begin baby!
*/
int main(int argc,char *argv[])
{
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
	GuardCatch();
	if( IsExceptionInProgress() )
	{
		GuardDisplayAndLogError();

		delete TA3D::VARS::lp_CONFIG;
		exit(1);
	}

	GuardStart( main );
		ParseCommandLine( argc, argv );   /* process any command line args passed */
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

	// while our engine does some loading and intlaizing, lets show our intro.
	GuardStart( main );
		play_intro();
	GuardCatch();
	if( IsExceptionInProgress() )
	{
		GuardDisplayAndLogError();
		// We need to guard deleting the engine in case something bad goes wrong during
		//   the cleanup process, note that this might not exit right away because it
		//   might take a few seconds to kill the thread.
		GuardStart( main );
			delete Engine;
			delete TA3D::VARS::lp_CONFIG;
		GuardCatch();

		exit(1);
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

	GuardStart( main )
		delete TA3D::VARS::lp_CONFIG;
	GuardCatch();

	return 0; // thats it folks.
}
END_OF_MAIN();
/* Allegro needs END_OF_MAIN(); I guess. */ 

#define buf_size		1024*1024

void install_TA_files( String def_path )
{
	allegro_init();

	set_uformat( U_ASCII );

#ifdef TA3D_PLATFORM_WINDOWS					// Possible cd-rom path for windows
	int		nb_possible_path = 23;
	char	*possible_path[] = {	"D:\\", "E:\\", "F:\\", "G:\\", "H:\\", "I:\\", "J:\\", "K:\\", "L:\\", "M:\\", "N:\\", "O:\\", "P:\\", "Q:\\", "R:\\", "S:\\", "T:\\",
									"U:\\", "V:\\", "W:\\", "X:\\", "Y:\\", "Z:\\" };
#else											// Possible cd-rom path for other platforms
	int		nb_possible_path = 42;
	char	*possible_path[] = {	"/media/cdrom/", "/media/cdrom0/", "/media/cdrom1/", "/media/cdrom2/", "/media/cdrom3/", "/media/cdrom4/", "/media/cdrom5/",
									"/mnt/cdrom/", "/mnt/cdrom0/", "/mnt/cdrom1/", "/mnt/cdrom2/", "/mnt/cdrom3/", "/mnt/cdrom4/", "/mnt/cdrom5/",
									"/mnt/dvd/", "/mnt/dvd0/", "/mnt/dvd1/", "/mnt/dvd2/", "/mnt/dvd3/", "/mnt/dvd4/", "/mnt/dvd5/",
									"/mnt/dvdrecorder/", "/mnt/dvdrecorder0/", "/mnt/dvdrecorder1/", "/mnt/dvdrecorder2/", "/mnt/dvdrecorder3/", "/mnt/dvdrecorder4/", "/mnt/dvdrecorder5/",
									"/media/dvd/", "/media/dvd0/", "/media/dvd1/", "/media/dvd2/", "/media/dvd3/", "/media/dvd4/", "/media/dvd5/",
									"/media/dvdrecorder/", "/media/dvdrecorder0/", "/media/dvdrecorder1/", "/media/dvdrecorder2/", "/media/dvdrecorder3/", "/media/dvdrecorder4/", "/media/dvdrecorder5/" };
#endif

	allegro_message( "please mount/insert your TA cdrom now" );

	String path_to_TA_cd = "";

	if( def_path != "" ) {						// Explicit path given
		path_to_TA_cd = def_path;
		for( int i = 0 ; i < path_to_TA_cd.size() ; i++ )			// Use UNIX like path
			if( path_to_TA_cd[ i ] == '\\' )
				path_to_TA_cd[ i ] = '/';
		if( path_to_TA_cd[ path_to_TA_cd.size() ] != '/' )			// Check it ends with a '\'
			path_to_TA_cd += "/";
		}
	else									// Look for a path where we can find totala3.hpi
		for(int i = 0 ; i < nb_possible_path ; i++ ) {
			path_to_TA_cd = possible_path[ i ];
			if( exists( ( path_to_TA_cd + "totala3.hpi" ).c_str() )
			 && exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
			}

	HPIManager=new cHPIHandler( path_to_TA_cd );

	uint32 file_size32 = 0;
	byte *data = HPIManager->PullFromHPI_zone( "install\\totala1.hpi", 0, buf_size, &file_size32);			// Extract the totala1.hpi file from the TA CD
	bool success = true;

	if(data) {
		set_color_depth(32);
		set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0 );
		clear( screen );

		rectfill( screen, 0, 0, 640, 480, 0x7F7F7F );

		textprintf_centre_ex( screen, font, 320, 40, 0x0, -1, "Extracting totala1.hpi ( step 1/2 )" );

		FILE *dst = TA3D_OpenFile("totala1.hpi","wb");

		fwrite(data,buf_size,1,dst);

		for( int pos = buf_size ; pos < file_size32 ; pos += buf_size ) {
			int read_size = min( buf_size, (int)(file_size32 - pos) );
			free(data);
			data = HPIManager->PullFromHPI_zone( "install\\totala1.hpi", pos, read_size, &file_size32);			// Extract the totala1.hpi file from the TA CD
			fwrite(data+pos,read_size,1,dst);

			rectfill( screen, 100, 60, 540, 80, makecol( 255, 0, 0 ) );
			rectfill( screen, 100, 60, 100 + 440 * (pos+read_size>>10) / (file_size32>>10), 80, makecol( 255, 255, 0 ) );
			textprintf_centre_ex( screen, font, 320, 66, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (file_size32>>10) );
			}

		fclose(dst);
		free(data);
		}
	else
		success = false;

	if( exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) && success ) {
		textprintf_centre_ex( screen, font, 320, 140, 0x0, -1, "Copying totala2.hpi ( step 2/2 )" );
		rectfill( screen, 100, 160, 540, 180, makecol( 255, 0, 0 ) );
		textprintf_centre_ex( screen, font, 320, 166, 0x0, -1, "0%%" );

		FILE *src = TA3D_OpenFile( path_to_TA_cd + "totala2.hpi", "rb" );
		FILE *dst = TA3D_OpenFile( "totala2.hpi", "wb" );
		int limit = FILE_SIZE( ( path_to_TA_cd + "totala2.hpi" ).c_str() );
		byte *buf = new byte[ buf_size ];			// a 1Mo buffer

		for( int pos = 0 ; pos < limit ; pos+= buf_size ) {
			int read_size = min( buf_size, limit-pos );
			fread( buf, read_size, 1, src );
			fwrite( buf, read_size, 1, dst );

			rectfill( screen, 100, 160, 540, 180, makecol( 255, 0, 0 ) );
			rectfill( screen, 100, 160, 100 + 440 * (pos+read_size>>10) / (limit>>10), 180, makecol( 255, 255, 0 ) );
			textprintf_centre_ex( screen, font, 320, 166, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (limit>>10) );
			}

		delete buf;

		fclose( dst );
		fclose( src );
		}
	else
		success = false;

	if( ! success )							// Print an error message
		allegro_message( "Installation failed:\n    Unable to find TA's cd path!!\n\nplease use this syntax:\n    hpiview install path_to_TA_cd\n                                         " );
	else
		allegro_message( "Installation Successful!!\nNow just run ta3d from its base directory and have fun ;-)!" );

	delete HPIManager;

	allegro_exit();
}
