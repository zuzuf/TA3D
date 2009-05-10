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
#include "threads/thread.h"
#include "engine.h"
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
			//			allegro_message((String(I18N::Translate(s)) << additional).c_str());
#warning FIXME: ugly print to console instead of a nice window
			std::cerr << I18N::Translate(s) << additional << std::endl;
		}

	}


	Engine::Engine()
		:pSDLRunning(false), pGFXModeActive(false), pSignaledToStop(false)
	{
		VARS::sound_manager = NULL;
		VARS::gfx = NULL;

		String str(TA3D_ENGINE_VERSION);
		str << " initializing started:\n\n";
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL);

		str.clear();
		str << "Build info : " << __DATE__ << " , " << __TIME__ << "%s\n\n";
		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)str.c_str(), NULL, NULL);

		// Initalizing SDL video
		if (SDL_Init(SDL_INIT_VIDEO))
			throw ("SDL_Init(SDL_INIT_VIDEO) yielded unexpected result.");

		// set SDL running status;
		pSDLRunning = true;

		// Installing SDL timer
		if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
			throw ("SDL_InitSubSystem(SDL_INIT_TIMER) yielded unexpected result.");

		if (SDLNet_Init() == -1)
			throw ("SDLNet_Init() failed.");

		if (!HPIManager->Exists("gamedata\\sidedata.tdf") || !HPIManager->Exists("gamedata\\allsound.tdf") || !HPIManager->Exists("gamedata\\sound.tdf"))
		{
			showError("RESOURCES ERROR");
			exit(1);
		}

		// Outputs SDL_net version numbers
		SDL_version compiled_version;
		const SDL_version *linked_version;
		SDL_NET_VERSION(&compiled_version);
		LOG_DEBUG(LOG_PREFIX_NET << "Compiled with SDL_net version: " << (int)compiled_version.major << "." << (int)compiled_version.minor << "." << (int)compiled_version.patch);
		linked_version = SDLNet_Linked_Version();
		LOG_DEBUG(LOG_PREFIX_NET << "Running with SDL_net version: " << (int)linked_version->major << "." << (int)linked_version->minor << "." << (int)linked_version->patch);

		// Creating Sound & Music Interface
		sound_manager = new TA3D::Audio::Manager();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);

		if (!sound_manager->isRunning() && !lp_CONFIG->quickstart)
			showError("FMOD WARNING");

		// Creating GFX Interface
		// Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		TA3D::VARS::gfx = new TA3D::GFX();		// TA3D's main window might lose focus and message may not be shown ...
		pGFXModeActive = true;

		gfx->Init();

		SDL_WM_SetCaption("Total Annihilation 3D", "TA3D");

		// Initialize the mouse handler
		init_mouse();
		// Initialize the keyboard handler
		init_keyboard();

		ThreadSynchroniser = new ObjectSync;
	}


	Engine::~Engine(void)
	{
		destroyThread();
		delete ThreadSynchroniser;
		cursor.clear();
		ta3dSideData.destroy();

		delete HPIManager;
		delete sound_manager;

		delete gfx;
		pGFXModeActive = false;

		delete InterfaceManager;

		if (pSDLRunning)
		{
			pSDLRunning = false;
			SDLNet_Quit();
			SDL_Quit();
		}
	}



	void Engine::Init()
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


	void Engine::proc(void*)
	{
		Init();
		while (!pSignaledToStop)
			::rest(100);
	}



	void Engine::signalExitThread()
	{
		pSignaledToStop = true;
	}


} // namespace TA3D
