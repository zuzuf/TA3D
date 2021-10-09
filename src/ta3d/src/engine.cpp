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
        : pGFXModeActive(false),
          bStarted(false)
	{
		// How many CPU we've got ?
        LOG_INFO("CPU: " << QThread::idealThreadCount());

		gfx = NULL;

		LOG_INFO(TA3D_ENGINE_VERSION << " initializing started:");
		LOG_INFO("Build info : " << __DATE__ << " , " << __TIME__);

		// Load the VFS
		VFS::Instance()->reload();

        if (!VFS::Instance()->fileExists("gamedata/sidedata.tdf") || !VFS::Instance()->fileExists("gamedata/allsound.tdf")
            || !VFS::Instance()->fileExists("gamedata/sound.tdf"))
		{
			showError("RESOURCES ERROR");
			exit(1);
		}

		// Creating GFX Interface
		// Don't try to start sound before gfx, if we have to display the warning message while in fullscreen
		gfx = new GFX();		// TA3D's main window might lose focus and message may not be shown ...
		pGFXModeActive = true;


		// Display informations about OpenGL
		displayInfosAboutOpenGL();

        // Creating Sound & Music Interface
        sound_manager = new TA3D::Audio::Manager();
    }


	Engine::~Engine(void)
	{
        wait();
		cursor.clear();
		ta3dSideData.destroy();

        sound_manager = nullptr;
        gfx = nullptr;
		pGFXModeActive = false;
        InterfaceManager = nullptr;
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

        // Load sound & music data
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

	void Engine::displayInfosAboutOpenGL() const
	{
		logs.checkpoint() << "OpenGL Informations :";
        logs.info() << "Vendor: "   << (const char*) gfx->glGetString(GL_VENDOR);
        CHECK_GL();
        logs.info() << "Renderer: " << (const char*) gfx->glGetString(GL_RENDERER);
        CHECK_GL();
        logs.info() << "Version: "  << (const char*) gfx->glGetString(GL_VERSION);
        CHECK_GL();
        if (gfx->atiWorkaround())
			LOG_WARNING("ATI or SIS card detected ! Using workarounds for ATI/SIS cards");
		LOG_INFO(LOG_PREFIX_OPENGL << "Texture compression: " << (g_useTextureCompression ? "Yes" : "No"));
		LOG_INFO(LOG_PREFIX_OPENGL << "Multi texturing: " << (MultiTexturing ? "Yes" : "No"));
	}
} // namespace TA3D
