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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "threads/cThread.h"
#include "threads/thread.h"
#include "cTA3D_Engine.h"
#include "ta3dbase.h"

#include "3do.h"               // For 3DO/3DM management
#include "scripts/cob.h"               // For unit scripts management
#include "tdf.h"               // For 2D features
#include "EngineClass.h"         // The Core Engine

#include "UnitEngine.h"            // The Unit Engine

#include "tnt.h"               // The TNT loader
#include "scripts/script.h"               // The game script manager
#include "ai/ai.h"                  // AI Engine
#include "gfx/fx.h"					// Special FX engine
#include "misc/paths.h"
#include "languages/i18n.h"
#include "jpeg/ta3d_jpg.h"
#include "sounds/manager.h"


using namespace TA3D::Exceptions;



namespace TA3D
{


	cTA3D_Engine::cTA3D_Engine(void)
	{
		GuardEnter( cTA3D_Engine Constructor );
		GuardInfo( "Zeroing Variables." );

		InterfaceManager = NULL;
		VARS::sound_manager = NULL;
		VARS::HPIManager = NULL;
		m_AllegroRunning = false;
		VARS::gfx = NULL;
		m_SignaledToStop = false;
		m_GFXModeActive = false;

		GuardInfo( "Initializing Thead data" );
		InitThread();

		GuardInfo( "Creating Interface Manager." );
		InterfaceManager = new IInterfaceManager();

		GuardInfo( "Creating logging Interface, and attaching it to Interface Manager" );

		GuardInfo( "Logging some stuff to ta3d.log" );

		String str = format( "%s initializing started:\n\n", TA3D_ENGINE_VERSION );
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

		str = format("build info : %s , %s\n\n",__DATE__,__TIME__);
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

		GuardInfo( "Setting uformat to U_ASCII." );
		set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters

		GuardInfo( "Initalizing allegro." );
		if( allegro_init() != 0 )
			throw( "allegro_init() yielded unexpected result." );

		// set allegro running status;
		m_AllegroRunning = true;

		GuardInfo( "Installing allegro timer." );
		if( install_timer() != 0 )
			throw( "install_timer() yielded unexpected result." );

		GuardInfo( "Installing allegro mouse handler." );
		if( install_mouse() == -1 )
			throw ( "install_mouse() yielded unexpected result." );

		GuardInfo( "Installing allegro keyboard handler." );
		if( install_keyboard() == -1 )
			throw ( "install_mouse() yielded unexpected result." );

		GuardInfo( "Initalizing allegro JPG support." );
		if( jpgalleg_init() < 0 )
			throw( "jpgalleg_init() yielded unexpected result." );

		GuardInfo( "Creating HPI Manager." );
		TA3D::VARS::HPIManager = new TA3D::UTILS::HPI::cHPIHandler( GetClientPath() );

		GuardInfo( "Creating translation manager." );

        // Loads translation data (TA translations in ASCII -> UTF8)
        I18N::Instance()->loadFromFile("gamedata\\translate.tdf", true, true);   
        I18N::Instance()->loadFromFile("ta3d.res", false);   // Loads translation data (TA3D translations in UTF8)

		if( !HPIManager->Exists( "gamedata\\sidedata.tdf" ) || !HPIManager->Exists( "gamedata\\allsound.tdf" ) || !HPIManager->Exists( "gamedata\\sound.tdf" ) )
        {
            LOG_ERROR(I18N::Translate("RESOURCES ERROR"));
			set_uformat(U_UTF8);   // fixed size, 8-bit ASCII characters
			allegro_message( I18N::Translate("RESOURCES ERROR").c_str() );
			set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters
			throw ("resources missing!!");
		}

		GuardInfo( "Creating Sound & Music Interface." );
		sound_manager = new TA3D::Audio::Manager(1.0f, 0.0f, 0.0f);
		sound_manager->stopMusic();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);

		if (!sound_manager->isRunning() && !lp_CONFIG->quickstart)
        {
            LOG_ERROR(I18N::Translate("FMOD WARNING"));
			set_uformat(U_UTF8);   // fixed size, 8-bit ASCII characters
			allegro_message( I18N::Translate("FMOD WARNING").c_str() );
			set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters
		}

		GuardInfo( "Creating GFX Interface." );				// Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		TA3D::VARS::gfx = new TA3D::Interfaces::GFX;		// TA3D's main window might lose focus and allegro's message not be shown ...
		m_GFXModeActive = true;

		gfx->Init();

		set_window_title("Total Annihilation 3D");

		GuardInfo( "Loading and creating cursors." );
		byte *data=HPIManager->PullFromHPI("anims\\cursors.gaf");	// Load cursors
		cursor.init();
		cursor.load_gaf(data ,true);
		cursor.convert();

		CURSOR_MOVE=cursor.find_entry("cursormove");		// Match cursor variables with cursor anims
		CURSOR_GREEN=cursor.find_entry("cursorgrn");
		CURSOR_CROSS=cursor.find_entry("cursorselect");
		CURSOR_RED=cursor.find_entry("cursorred");
		CURSOR_LOAD=cursor.find_entry("cursorload");
		CURSOR_UNLOAD=cursor.find_entry("cursorunload");
		CURSOR_GUARD=cursor.find_entry("cursordefend");
		CURSOR_PATROL=cursor.find_entry("cursorpatrol");
		CURSOR_REPAIR=cursor.find_entry("cursorrepair");
		CURSOR_ATTACK=cursor.find_entry("cursorattack");
		CURSOR_BLUE=cursor.find_entry("cursornormal");
		CURSOR_AIR_LOAD=cursor.find_entry("cursorpickup");
		CURSOR_BOMB_ATTACK=cursor.find_entry("cursorairstrike");
		CURSOR_BALANCE=cursor.find_entry("cursorunload");
		CURSOR_RECLAIM=cursor.find_entry("cursorreclamate");
		CURSOR_WAIT=cursor.find_entry("cursorhourglass");
		CURSOR_CANT_ATTACK=cursor.find_entry("cursortoofar");
		CURSOR_CROSS_LINK=cursor.find_entry("pathicon");
		CURSOR_CAPTURE=cursor.find_entry("cursorcapture");
		CURSOR_REVIVE=cursor.find_entry("cursorrevive");
		if( CURSOR_REVIVE == -1 )									// If you don't have the required cursors, then resurrection won't work
			CURSOR_REVIVE=cursor.find_entry("cursorreclamate");

		delete[] data;


		GuardInfo( "Initalizing Critical section data." );

		GuardInfo( "Initializing Thread Synchroniser object" );

		ThreadSynchroniser = new ObjectSync;

		GuardInfo( "Initializing the ascii to scancode table" );

		for( int i = 0 ; i < 256 ; i++ )
			ascii_to_scancode[ i ] = 0;

		for( int i = 0 ; i < KEY_MAX ; ++i)
        {
			int ascii_code = scancode_to_ascii( i );
			if( ascii_code >= 0 && ascii_code < 256 )
				ascii_to_scancode[ ascii_code ] = i;
		}

		GuardLeave();
	}


	cTA3D_Engine::~cTA3D_Engine(void)
	{
		GuardEnter( cTA3D_Engine deconstructor );
		DestroyThread();
		delete ThreadSynchroniser;
		cursor.destroy();
		ta3dSideData.destroy();

		delete HPIManager;
		delete sound_manager;

		delete gfx;
		m_GFXModeActive = false;

		delete InterfaceManager;

		if (m_AllegroRunning)
		{
			allegro_exit();
			m_AllegroRunning = false;
		}

		GuardLeave();
	}

	void cTA3D_Engine::Init()
	{
		model_manager.init();
		unit_manager.init();
		feature_manager.init();
		weapon_manager.init();
		fx_manager.init();
		init_rand_table();

		ta3dSideData.init();
		ta3dSideData.loadData();

		sound_manager->loadTDFSounds(false);
	}


	int cTA3D_Engine::Run()
	{
		Init();
		while( !m_SignaledToStop )
			rest( 100 );
		return 1;
	}
	void cTA3D_Engine::SignalExitThread()
	{
		m_SignaledToStop = true;
		return;
	}


} // namespace TA3D 
