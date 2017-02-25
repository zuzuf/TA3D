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

#include "mainmenu.h"
#include <ingame/sidedata.h>
#include <gfx/gfx.h>
#include <logs/logs.h>
#include <ta3dbase.h>
#include "solo.h"
#include "multimenu.h"
#include "config.h"
#include <logs/logs.h>
#include <misc/settings.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include <gfx/video.h>



namespace TA3D
{

	// TODO Must be removed
	void ReadFileParameter();


namespace Menus
{

	bool MainMenu::Execute()
	{
		MainMenu m;
		return m.execute();
	}

	MainMenu::MainMenu()
		:Abstract()
	{}


	MainMenu::~MainMenu()
	{}



	bool MainMenu::doInitialize()
	{
		// Play intro movie if any
		if (!lp_CONFIG->quickstart)
			Video::play("video/intro.mpg");

		LOG_DEBUG(LOG_PREFIX_MENU_MAIN << "Entering...");

		gfx->SetDefState();
		gfx->set_2D_mode();
		gfx->ReInitTexSys();

		// To have mouse sensibility undependent from the resolution
		gfx->SCREEN_W_TO_640 = 1.0f;
		gfx->SCREEN_H_TO_480 = 1.0f;

		// Loading the area
		loadAreaFromTDF("main", "gui/main.area");

		resetCaptions();

		// If there is a file parameter, read it
		ReadFileParameter();
		// Misc
		pDontWaitForEvent = true;

		if (!lp_CONFIG->fullscreen)
			grab_mouse(false);
		return true;
	}


	void MainMenu::resetCaptions()
	{
		// Current mod
		getInfosAboutTheCurrentMod();

        if (!pArea)
            return;
        // Reset the caption
		pArea->caption("main.t_version", TA3D_ENGINE_VERSION);
		pArea->caption("main.t_mod", pCurrentModCaption);
	}

	bool MainMenu::doExecute()
	{
		while(!doLoop())
			;
		return true;
	}

	void MainMenu::redrawTheScreen()
	{
		Abstract::redrawTheScreen();
	}


	void MainMenu::doFinalize()
	{
		gfx->set_2D_mode();
		LOG_DEBUG(LOG_PREFIX_MENU_MAIN << "Done.");
	}


	void MainMenu::getInfosAboutTheCurrentMod()
	{
		pCurrentMod.clear();
		pCurrentModCaption.clear();

		if (TA3D_CURRENT_MOD.length() > 6)
			pCurrentMod += Substr(TA3D_CURRENT_MOD, 5, TA3D_CURRENT_MOD.length() - 6);
        if (!pCurrentMod.isEmpty())
            pCurrentModCaption += "MOD: " + pCurrentMod;
	}


	void MainMenu::resetScreen()
	{
		pDontWaitForEvent = true;

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		gfx->set_color(0xFFFFFFFF);

		resetCaptions();

		// To have mouse sensibility undependent from the resolution
		gfx->SCREEN_W_TO_640 = 1.0f;
		gfx->SCREEN_H_TO_480 = 1.0f;
	}


	bool MainMenu::maySwitchToAnotherMenu()
	{
		// Exit
		if (key[KEY_ESC] || pArea->get_state("main.b_exit"))
			return true;

		// Options
		if (key[KEY_SPACE] || key[KEY_O] || pArea->get_state("main.b_options"))
			return goToMenuOptions();

		// Solo
		if( key[KEY_ENTER] || key[KEY_S] || pArea->get_state("main.b_solo"))
			return goToMenuSolo();

		// Multi player room
		if(key[KEY_B] || key[KEY_M] || pArea->get_state("main.b_multi"))
			return goToMenuMultiPlayers();

		return false;
	}


	bool MainMenu::goToMenuOptions()
	{
		lp_CONFIG->quickstart = false;
		do
		{
			lp_CONFIG->quickrestart = false;
			glPushMatrix();
			Config::Execute();
			lp_CONFIG->quickstart = false;
			glPopMatrix();

			if (lp_CONFIG->quickrestart)
			{
				changeVideoSettings();
				resetScreen();
				lp_CONFIG->quickstart = true;
			}
		} while (lp_CONFIG->quickrestart);

		loadAreaFromTDF("main", "gui/main.area");
		resetScreen();

		if (!lp_CONFIG->fullscreen)
			grab_mouse(false);
		else
			grab_mouse(lp_CONFIG->grab_inputs);

		return false;
	}

	void MainMenu::changeVideoSettings()
	{
		pArea = NULL;                 // Destroy current GUI area
		cursor.clear();               // Destroy cursor data (it's OpenGL textures so they won't survive)
		ta3dSideData.destroy();       // We're going to reset video settings, so this will become obsolete

        gfx = NULL;					// Destroy the old GFX object (required for proper cleaning)
		gfx = new GFX;					// Create a new one with new settings
		gfx->loadFonts();
		gfx->loadDefaultTextures();  // Initialize GFX object

		gfx->set_2D_mode();         // Back to 2D mode :)

		SDL_WM_SetCaption("Total Annihilation 3D", "TA3D");  // Set the window title

		ta3dSideData.init();
		ta3dSideData.loadData();

		// Reloading and creating cursors
        QIODevice *file = VFS::Instance()->readFile("anims/cursors.gaf");	// Load cursors
		cursor.loadGAFFromRawData(file, true);
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

		delete file;
	}


	bool MainMenu::goToMenuMultiPlayers()
	{
		glPushMatrix();
		Menus::MultiMenu::Execute();
		glPopMatrix();
		resetScreen();
		return false;
	}

	bool MainMenu::goToMenuSolo()
	{

		glPushMatrix();
		Menus::Solo::Execute();
		glPopMatrix();
		resetScreen();
		return false;
	}

	void MainMenu::waitForEvent()
	{
		bool keyIsPressed(false);
		do
		{
			// Grab user events
			pArea->check();
			// Get if a key was pressed
			keyIsPressed = pArea->key_pressed;
			// Wait to reduce CPU consumption
			wait();

		} while (!pDontWaitForEvent
			&& pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
			&& mouse_b == 0
			&& !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_B]
			&& !key[KEY_O] && !key[KEY_M] && !key[KEY_S]
			&& !keyIsPressed && !pArea->scrolling);

		// Should wait the an event the next time
		pDontWaitForEvent = false;
	}




} // namespace Menus
} // namespace TA3D

