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

#include "base.h"
#include <typeinfo>
#include <ta3dbase.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <misc/timer.h>


namespace TA3D
{
namespace Menus
{

	Abstract::Abstract() : timer(msectimer())
	{
		pTypeName = typeid(*this).name();
	}


	bool Abstract::execute()
	{
		// Executing if initializing succeeded
		bool r = (doGuardInitialize() && doGuardExecute());
		// Finalizing
		doGuardFinalize();
		return (r);
	}


	bool Abstract::doGuardInitialize()
	{
		pMouseX = -1;
		pMouseY = -1;
		pMouseZ = -1;
		pMouseB = -1;
		cursor_type = CURSOR_DEFAULT;
		reset_keyboard();
		reset_mouse();
		clear_keybuf();
		bool r = doInitialize();
		return r;
	}

	bool Abstract::doGuardExecute()
	{
		bool r = doExecute();
		return r;
	}

	void Abstract::doGuardFinalize()
	{
		if (pArea && pArea->background == gfx->glfond)
			pArea->background = 0;
		doFinalize();
		reset_keyboard();
		clear_keybuf();
		reset_mouse();
	}

	void Abstract::loadAreaFromTDF(const QString& caption, const QString& relFilename)
	{
		pArea = new Gui::AREA(caption);
		pArea->load_tdf(relFilename);
		if (!pArea->background)
			pArea->background = gfx->glfond;
	}


	bool Abstract::doLoop()
	{
		// Wait for an event (mouse, keyboard...)
		waitForEvent();

		// Reset the last cached values for the mouse
		pMouseX = mouse_x;
		pMouseY = mouse_y;
		pMouseZ = mouse_z;
		pMouseB = mouse_b;

		// Manage event
		bool done = maySwitchToAnotherMenu();

		// Redraw the screen
		redrawTheScreen();

		return done;
	}

	bool Abstract::doExecute()
	{
		while (!doLoop())
			;
		return true;
	}

	void Abstract::redrawTheScreen()
	{
		pArea->draw();
		draw_cursor();
		gfx->flip();
	}


	void Abstract::ResetTexture(GLuint& textVar, const GLuint newValue)
	{
		if (textVar)
		{
			// ensure the texture for the mini map is destroyed
			gfx->destroy_texture(textVar);
		}
		textVar = newValue;
	}


	void Abstract::wait(int msec)
	{
		if (msec == -1)
			msec = TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING;
		timer += msec;
		uint32 tmp = msectimer();
        QThread::msleep(tmp >= timer ? 0 : timer - tmp);
		timer = msectimer();
	}


} // namespace Menus
} // namespace TA3D
