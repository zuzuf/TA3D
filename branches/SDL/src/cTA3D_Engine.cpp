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
#include "sounds/manager.h"
#include "misc/math.h"





namespace TA3D
{

	namespace
	{
		void showError(const String& s, const String& additional = String())
		{
			LOG_ERROR(I18N::Translate(s));
//			set_uformat(U_UTF8);   // fixed size, 8-bit ASCII characters
//			allegro_message((String(I18N::Translate(s)) << additional).c_str());
//			set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters
#warning FIXME: ugly print to console instead of a nice window
            std::cerr << I18N::Translate(s) << additional << std::endl;
		}

	}


	cTA3D_Engine::cTA3D_Engine(void)
	{
		InterfaceManager = NULL;
		VARS::sound_manager = NULL;
		VARS::HPIManager = NULL;
		m_SDLRunning = false;
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
//		set_uformat(U_ASCII);   // fixed size, 8-bit ASCII characters

		// Initalizing SDL video
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
			throw ("SDL_Init(SDL_INIT_VIDEO) yielded unexpected result.");

		// set allegro running status;
		m_SDLRunning = true;

		// Installing SDL timer
		if (SDL_Init(SDL_INIT_TIMER) != 0)
			throw ("SDL_Init(SDL_INIT_TIMER) yielded unexpected result.");

//		// Installing SDL event thread
//		if (SDL_Init(SDL_INIT_EVENTTHREAD) != 0)
//			throw ("SDL_Init(SDL_INIT_EVENTTHREAD) yielded unexpected result.");

		// Creating HPI Manager
		TA3D::VARS::HPIManager = new TA3D::UTILS::HPI::cHPIHandler();

		// Creating translation manager
        I18N::Instance()->loadFromFile("gamedata\\translate.tdf", true, true);
        I18N::Instance()->loadFromFile("ta3d.res", false);   // Loads translation data (TA3D translations in UTF8)

		if (!HPIManager->Exists("gamedata\\sidedata.tdf") || !HPIManager->Exists("gamedata\\allsound.tdf") || !HPIManager->Exists("gamedata\\sound.tdf"))
		{
			showError("RESOURCES ERROR");
			exit(1);
		}

		// Creating Sound & Music Interface
		sound_manager = new TA3D::Audio::Manager(1.0f, 0.0f, 0.0f);
		sound_manager->stopMusic();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);

		if (!sound_manager->isRunning() && !lp_CONFIG->quickstart)
			showError("FMOD WARNING");

		// Creating GFX Interface
        // Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		TA3D::VARS::gfx = new TA3D::GFX();		// TA3D's main window might lose focus and allegro's message not be shown ...
		m_GFXModeActive = true;

		gfx->Init();

		SDL_WM_SetCaption("Total Annihilation 3D","TA3D");

        SDL_ShowCursor(SDL_DISABLE);

		// Loading and creating cursors
		byte *data = HPIManager->PullFromHPI("anims\\cursors.gaf");	// Load cursors
		if (data)
		{
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
		}
		else
		{
			showError("RESOURCES ERROR", " (anims\\cursors.gaf not found)");
			exit(2);
		}


		ThreadSynchroniser = new ObjectSync;

		// Initializing the ascii to scancode table
		for (int i = 0; i < 256; ++i)
			ascii_to_scancode[i] = 0;

        ascii_to_scancode[ 'a' ] = KEY_A;
        ascii_to_scancode[ 'b' ] = KEY_B;
        ascii_to_scancode[ 'c' ] = KEY_C;
        ascii_to_scancode[ 'd' ] = KEY_D;
        ascii_to_scancode[ 'e' ] = KEY_E;
        ascii_to_scancode[ 'f' ] = KEY_F;
        ascii_to_scancode[ 'g' ] = KEY_G;
        ascii_to_scancode[ 'h' ] = KEY_H;
        ascii_to_scancode[ 'i' ] = KEY_I;
        ascii_to_scancode[ 'j' ] = KEY_J;
        ascii_to_scancode[ 'k' ] = KEY_K;
        ascii_to_scancode[ 'l' ] = KEY_L;
        ascii_to_scancode[ 'm' ] = KEY_M;
        ascii_to_scancode[ 'n' ] = KEY_N;
        ascii_to_scancode[ 'o' ] = KEY_O;
        ascii_to_scancode[ 'p' ] = KEY_P;
        ascii_to_scancode[ 'q' ] = KEY_Q;
        ascii_to_scancode[ 'r' ] = KEY_R;
        ascii_to_scancode[ 's' ] = KEY_S;
        ascii_to_scancode[ 't' ] = KEY_T;
        ascii_to_scancode[ 'u' ] = KEY_U;
        ascii_to_scancode[ 'v' ] = KEY_V;
        ascii_to_scancode[ 'w' ] = KEY_W;
        ascii_to_scancode[ 'x' ] = KEY_X;
        ascii_to_scancode[ 'y' ] = KEY_Y;
        ascii_to_scancode[ 'z' ] = KEY_Z;

		for (int i = 0; i < 26; ++i)
            ascii_to_scancode[ 'A' + i ] = ascii_to_scancode[ 'a' + i ];

        ascii_to_scancode[ '0' ] = KEY_0;
        ascii_to_scancode[ '1' ] = KEY_1;
        ascii_to_scancode[ '2' ] = KEY_2;
        ascii_to_scancode[ '3' ] = KEY_3;
        ascii_to_scancode[ '4' ] = KEY_4;
        ascii_to_scancode[ '5' ] = KEY_5;
        ascii_to_scancode[ '6' ] = KEY_6;
        ascii_to_scancode[ '7' ] = KEY_7;
        ascii_to_scancode[ '8' ] = KEY_8;
        ascii_to_scancode[ '9' ] = KEY_9;

        ascii_to_scancode[ ' ' ] = KEY_SPACE;
        ascii_to_scancode[ '\n' ] = KEY_ENTER;
        ascii_to_scancode[ 27 ] = KEY_ESC;
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

		if (m_SDLRunning)
		{
			SDL_Quit();
			m_SDLRunning = false;
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
