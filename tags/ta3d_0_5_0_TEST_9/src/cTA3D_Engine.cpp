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
#include "misc/math.h"





namespace TA3D
{


	cTA3D_Engine::cTA3D_Engine(void)
	{
		InterfaceManager = NULL;
		VARS::sound_manager = NULL;
		VARS::HPIManager = NULL;
		m_AllegroRunning = false;
		VARS::gfx = NULL;
		m_SignaledToStop = false;
		m_GFXModeActive = false;

		InitThread();

		InterfaceManager = new IInterfaceManager();

		String str = format( "%s initializing started:\n\n", TA3D_ENGINE_VERSION );
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

		str = format("build info : %s , %s\n\n",__DATE__,__TIME__);
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL );

		// Setting uformat to U_ASCII
		set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters

		// Initalizing allegro
		if( allegro_init() != 0 )
			throw( "allegro_init() yielded unexpected result." );

		// set allegro running status;
		m_AllegroRunning = true;

		// Installing allegro timer
		if( install_timer() != 0 )
			throw( "install_timer() yielded unexpected result." );

		// Installing allegro mouse handler
		if( install_mouse() == -1 )
			throw ( "install_mouse() yielded unexpected result." );

		// Installing allegro keyboard handler
		if( install_keyboard() == -1 )
			throw ( "install_mouse() yielded unexpected result." );

		// Initalizing allegro JPG support
		if( jpgalleg_init() < 0 )
			throw( "jpgalleg_init() yielded unexpected result." );

		// Creating HPI Manager
		TA3D::VARS::HPIManager = new TA3D::UTILS::HPI::cHPIHandler();

		// Creating translation manager
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

		// Creating Sound & Music Interface
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

		// Creating GFX Interface
        // Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		TA3D::VARS::gfx = new TA3D::GFX();		// TA3D's main window might lose focus and allegro's message not be shown ...
		m_GFXModeActive = true;

		gfx->Init();

		set_window_title("Total Annihilation 3D");

		// Loading and creating cursors
		byte *data = HPIManager->PullFromHPI("anims\\cursors.gaf");	// Load cursors
		cursor.loadGAFFromRawData(data, true);
		cursor.convert();

		CURSOR_MOVE        = cursor.findByName("cursormove"); // Match cursor variables with cursor anims
		CURSOR_GREEN       = cursor.findByName("cursorgrn");
		CURSOR_CROSS       = cursor.findByName("cursorselect");
		CURSOR_RED         = cursor.findByName("cursorred");
		CURSOR_LOAD        = cursor.findByName("cursorload");
		CURSOR_UNLOAD      = cursor.findByName("cursorunload");
		CURSOR_GUARD       = cursor.findByName("cursordefend");
		CURSOR_PATROL      = cursor.findByName("cursorpatrol");
		CURSOR_REPAIR      = cursor.findByName("cursorrepair");
		CURSOR_ATTACK      = cursor.findByName("cursorattack");
		CURSOR_BLUE        = cursor.findByName("cursornormal");
		CURSOR_AIR_LOAD    = cursor.findByName("cursorpickup");
		CURSOR_BOMB_ATTACK = cursor.findByName("cursorairstrike");
		CURSOR_BALANCE     = cursor.findByName("cursorunload");
		CURSOR_RECLAIM     = cursor.findByName("cursorreclamate");
		CURSOR_WAIT        = cursor.findByName("cursorhourglass");
		CURSOR_CANT_ATTACK = cursor.findByName("cursortoofar");
		CURSOR_CROSS_LINK  = cursor.findByName("pathicon");
		CURSOR_CAPTURE     = cursor.findByName("cursorcapture");
		CURSOR_REVIVE      = cursor.findByName("cursorrevive");
		if (CURSOR_REVIVE == -1) // If you don't have the required cursors, then resurrection won't work
			CURSOR_REVIVE = cursor.findByName("cursorreclamate");

		delete[] data;
		ThreadSynchroniser = new ObjectSync;

		// Initializing the ascii to scancode table
		for (int i = 0; i < 256; ++i)
			ascii_to_scancode[i] = 0;

		for (int i = 0; i < KEY_MAX; ++i)
        {
			int ascii_code = scancode_to_ascii(i);
			if (ascii_code >= 0 && ascii_code < 256)
				ascii_to_scancode[ascii_code] = i;
		}
	}


	cTA3D_Engine::~cTA3D_Engine(void)
	{
		DestroyThread();
		delete ThreadSynchroniser;
        cursor.clear();
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
	}

	void cTA3D_Engine::Init()
	{
		model_manager.init();
		unit_manager.init();
		feature_manager.init();
		weapon_manager.init();
		fx_manager.init();
        Math::InitializeRandomTable();

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
