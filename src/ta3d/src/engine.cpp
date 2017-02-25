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

#include "UnitEngine.h"            // The Unit Engine

#include "gfx/fx.h"	               // Special FX engine
#include "languages/i18n.h"
#include "sounds/manager.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "backtrace.h"
#include <QThread>



namespace TA3D
{
	Synchronizer Engine::synchronizer(4);

	namespace
	{
		void showError(const QString& s, const QString& additional = QString())
		{
			LOG_ERROR(I18N::Translate(s));
            criticalMessage(I18N::Translate(s) + additional);
		}

		void showWarning(const QString& s, const QString& additional = QString())
		{
			LOG_WARNING(I18N::Translate(s));
			Gui::AREA::Ptr pArea = new Gui::AREA();
			pArea->load_tdf("gui/empty.area");
            pArea->popup(I18N::Translate("Warning"), I18N::Translate(s) + additional);
		}
	}



	Engine::Engine()
        : pSDLRunning(false),
          pGFXModeActive(false),
          bStarted(false)
	{
		// How many CPU we've got ?
        LOG_INFO("CPU: " << QThread::idealThreadCount());

		gfx = NULL;

		LOG_INFO(TA3D_ENGINE_VERSION << " initializing started:");
		LOG_INFO("Build info : " << __DATE__ << " , " << __TIME__);

		// Initalizing SDL video
		if (::SDL_Init(SDL_INIT_VIDEO))
            throw std::runtime_error("SDL_Init(SDL_INIT_VIDEO) yielded unexpected result.");
		// Installing SDL timer
		if (::SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
            throw std::runtime_error("SDL_InitSubSystem(SDL_INIT_TIMER) yielded unexpected result.");
		// Initializing SDL Net
		if (::SDLNet_Init() == -1)
            throw std::runtime_error("SDLNet_Init() failed.");

		// Load the VFS
		VFS::Instance()->reload();

        if (!VFS::Instance()->fileExists("gamedata/sidedata.tdf") || !VFS::Instance()->fileExists("gamedata/allsound.tdf")
            || !VFS::Instance()->fileExists("gamedata/sound.tdf"))
		{
			showError("RESOURCES ERROR");
			exit(1);
		}

		// set SDL running status;
		pSDLRunning = true;


		// Outputs SDL_net version numbers
		SDL_version compiled_version;
		const SDL_version *linked_version;
		SDL_NET_VERSION(&compiled_version);
		LOG_DEBUG(LOG_PREFIX_NET << "Compiled with SDL_net version: " << (int)compiled_version.major << "." << (int)compiled_version.minor << "." << (int)compiled_version.patch);
		linked_version = SDLNet_Linked_Version();
		LOG_DEBUG(LOG_PREFIX_NET << "Running with SDL_net version: " << (int)linked_version->major << "." << (int)linked_version->minor << "." << (int)linked_version->patch);


		// Creating GFX Interface
		// Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		gfx = new GFX();		// TA3D's main window might lose focus and message may not be shown ...
		pGFXModeActive = true;


		// Title of the Window / Application
		SDL_WM_SetCaption("Total Annihilation 3D", "TA3D");

		// Display informations about OpenGL
		displayInfosAboutOpenGL();
	}


	Engine::~Engine(void)
	{
        wait();
		cursor.clear();
		ta3dSideData.destroy();

		sound_manager = NULL;
		gfx = NULL;
		pGFXModeActive = false;
		InterfaceManager = NULL;

		if (pSDLRunning)
		{
			pSDLRunning = false;
			SDLNet_Quit();
			SDL_Quit();
		}
	}


	void Engine::initializationFromTheMainThread()
	{
		// Load the default textures
		gfx->loadDefaultTextures();

		// Load fonts (it crashes on some systems if not run from the main thread)
		gfx->loadFonts();

		// Initialize the mouse handler
		LOG_INFO("Initializing the mouse device handler");
		init_mouse();
		// Initialize the keyboard handler
		LOG_INFO("Initializing the keyboard device handler");
		init_keyboard();

		if (!sound_manager->isRunning() && !lp_CONFIG->quickstart)
			showWarning("FMOD WARNING");
	}


    void Engine::run()
	{
		// Creating translation manager
        I18N::Instance()->loadFromFile("gamedata/translate.tdf", true, true);
		I18N::Instance()->loadFromResources();

		// Apply settings for the current language (required since it failed when loading settings because languages were not loaded)
        if (!lp_CONFIG->Lang.isEmpty())
			I18N::Instance()->currentLanguage(lp_CONFIG->Lang);
		else
		{
			LOG_INFO(LOG_PREFIX_I18N << "language not set, guessing from system config");
			if (!I18N::Instance()->tryToDetermineTheLanguage())
			{
				LOG_INFO(LOG_PREFIX_I18N << "language detection failed, language set to 'english'");
				lp_CONFIG->Lang = "english";
				I18N::Instance()->currentLanguage(lp_CONFIG->Lang);
			}
			else
				lp_CONFIG->Lang = I18N::Instance()->currentLanguage()->englishCaption();
		}

		// Creating Sound & Music Interface
		sound_manager = new TA3D::Audio::Manager();
		sound_manager->loadTDFSounds(true);
		sound_manager->loadTDFSounds(false);

		model_manager.init();
		unit_manager.init();
		feature_manager.init();
		weapon_manager.init();
		fx_manager.init();
		Math::RandomTable.reset();

		ta3dSideData.init();
		ta3dSideData.loadData();

		sound_manager->loadTDFSounds(false);

        bStarted = true;
	}


	void rest(uint32 msec)
	{
        QThread::msleep(msec);
	}


	void Engine::displayInfosAboutOpenGL() const
	{
		logs.checkpoint() << "OpenGL Informations :";
		logs.info() << "Vendor: "   << (const char*) glGetString(GL_VENDOR);
		logs.info() << "Renderer: " << (const char*) glGetString(GL_RENDERER);
		logs.info() << "Version: "  << (const char*) glGetString(GL_VERSION);
		if (gfx->atiWorkaround())
			LOG_WARNING("ATI or SIS card detected ! Using workarounds for ATI/SIS cards");
		LOG_INFO(LOG_PREFIX_OPENGL << "Texture compression: " << (g_useTextureCompression ? "Yes" : "No"));
		LOG_INFO(LOG_PREFIX_OPENGL << "Stencil Two Side: " << (g_useStencilTwoSide ? "Yes" : "No"));
		LOG_INFO(LOG_PREFIX_OPENGL << "FBO: " << (g_useFBO ? "Yes" : "No"));
		LOG_INFO(LOG_PREFIX_OPENGL << "Shaders: " << (g_useProgram ? "Yes" : "No"));
		LOG_INFO(LOG_PREFIX_OPENGL << "Multi texturing: " << (MultiTexturing ? "Yes" : "No"));
	}
} // namespace TA3D
