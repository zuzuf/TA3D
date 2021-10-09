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
#include "intro.h"
#include <misc/paths.h>
#include <vector>
#include <misc/resources.h>
#include <misc/files.h>
#include <gfx/gui/skin.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <misc/timer.h>
#include <languages/i18n.h>
#include <TA3D_NameSpace.h>


//!
# define TA3D_INTRO_TOP (630.0f * float(SCREEN_H) / 1024.0f)

# define TA3D_INTRO_BOTTOM (950.0f * float(SCREEN_H) / 1024.0f)

# define TA3D_INTRO_MAX_LINES ((unsigned int)((TA3D_INTRO_BOTTOM - TA3D_INTRO_TOP) / pCurrentFontHeight) + 2)

# define TA3D_INTRO_SPEED  60.0f


namespace TA3D
{
namespace Menus
{


	bool Intro::Execute()
	{
		Menus::Intro m;
		return m.execute();
	}


	Intro::Intro()
        :Abstract(), pContentSize(0),
		pCurrentFontHeight(1.0f)
	{}

	Intro::~Intro()
	{}


	bool Intro::doInitialize()
	{
        LOG_ASSERT(gfx);
		LOG_DEBUG(LOG_PREFIX_MENU_INTRO << "Entering...");
		if (Gui::gui_font)
			pCurrentFontHeight = Gui::gui_font->height();

		reloadContent();
		loadBackgroundTexture();

		gfx->set_2D_mode();

		pDelta = 0.0f;
		pStartIndex = 0;
		pScrollTimer = msectimer();

		return true;
	}

	void Intro::doFinalize()
	{
        pBackgroundTexture = nullptr;
		LOG_DEBUG(LOG_PREFIX_MENU_INTRO << "Done.");
		pScrollTimer = msectimer();
	}


	void Intro::waitForEvent()
	{
		// Do nothing
		wait();
		poll_inputs();
	}


	bool Intro::maySwitchToAnotherMenu()
	{
		pDelta += TA3D_INTRO_SPEED * float(msectimer() - pScrollTimer) * 0.001f;
		pScrollTimer = msectimer();
		if (pDelta > pCurrentFontHeight)
		{
			pDelta = pCurrentFontHeight - pDelta;
			++pStartIndex;
			if (pStartIndex >= pContentSize)
				return true;
		}
		// Press any key to continue... :)
		return (mouse_b || keypressed());
	}


	void Intro::reloadContent()
	{
		pContent.clear();
		// A big space before
		for (unsigned int i = 1; i < TA3D_INTRO_MAX_LINES - 1; ++i)
            pContent.push_back(QString());

		// Load all text files
        QIODevice *file = VFS::Instance()->readFile("intro/" + I18N::Translate("en.ta3d.txt"));
		if (file == NULL)
            throw std::runtime_error(("Intro file not found! (intro/" + I18N::Translate("en.ta3d.txt") + ')').toStdString());
		if (file->size() <= 5 * 1024)
        {
            while(file->bytesAvailable())
                pContent.push_back(QString::fromUtf8(file->readLine().trimmed()));
        }
		delete file;
		pContentSize = unsigned(pContent.size());
		if (pContentSize == TA3D_INTRO_MAX_LINES)
		{
			pContent.push_back("Welcome to TA3D !");
			++pContentSize;
		}
	}


	void Intro::loadBackgroundTexture()
	{
        LOG_ASSERT(gfx);

        if (!lp_CONFIG->skin_name.isEmpty() && VFS::Instance()->fileExists(lp_CONFIG->skin_name))
		{
			Gui::Skin skin;
			skin.loadTDFFromFile(lp_CONFIG->skin_name);

            if (!skin.prefix().isEmpty())
                pBackgroundTexture = gfx->load_texture("gfx/" + skin.prefix() + "intro.jpg");
			else
                pBackgroundTexture = gfx->load_texture("gfx/intro.jpg");
		}
		else
            pBackgroundTexture = gfx->load_texture("gfx/intro.jpg");
	}


	void Intro::redrawTheScreen()
	{
		// Clear the screen
        gfx->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECK_GL();
        // Background
        gfx->drawtexture(pBackgroundTexture, 0.0f, 0.0f, float(SCREEN_W), float(SCREEN_H), makecol(0xFF,0xFF,0xFF));

        gfx->glEnable(GL_BLEND);
        CHECK_GL();
        gfx->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL();

		float fw = float(SCREEN_W) / 1280.0f;
		//float fh = float(SCREEN_H) / 1024.0f;

		// The text itself
		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        CHECK_GL();
        if (Gui::gui_font)
		{
			unsigned int indx = 0;
			for (unsigned int i = pStartIndex; i < pContentSize && indx < TA3D_INTRO_MAX_LINES; ++i, ++indx)
                Gui::gui_font->print(220.0f * fw, TA3D_INTRO_TOP + float(indx - 1) * pCurrentFontHeight - pDelta, White, pContent[i]);
		}

        gfx->glBlendFunc(GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA);
        CHECK_GL();
        pBackgroundTexture->bind();
		glBegin(GL_QUADS);

		glColor4ub(0xFF, 0xFF, 0xFF, 0);

		glTexCoord2f(0.0f, (TA3D_INTRO_TOP - 60.0f) / float(SCREEN_H));                     glVertex2f(0.0f, TA3D_INTRO_TOP - 60.0f);
		glTexCoord2f(1.0f, (TA3D_INTRO_TOP - 60.0f) / float(SCREEN_H));                     glVertex2f(float(SCREEN_W), TA3D_INTRO_TOP - 60.0f);
		glTexCoord2f(1.0f, TA3D_INTRO_TOP / float(SCREEN_H));                               glVertex2f(float(SCREEN_W), TA3D_INTRO_TOP);
		glTexCoord2f(0.0f, TA3D_INTRO_TOP / float(SCREEN_H));                               glVertex2f(0.0f, TA3D_INTRO_TOP);

		glTexCoord2f(0.0f, TA3D_INTRO_TOP / float(SCREEN_H));                               glVertex2f(0.0f, TA3D_INTRO_TOP);
		glTexCoord2f(1.0f, TA3D_INTRO_TOP / float(SCREEN_H));                               glVertex2f(float(SCREEN_W), TA3D_INTRO_TOP);
		glColor4ub(0xFF,0xFF,0xFF,0xFF);
		glTexCoord2f(1.0f, (TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / float(SCREEN_H));    glVertex2f(float(SCREEN_W), TA3D_INTRO_TOP + 2 * pCurrentFontHeight);
		glTexCoord2f(0.0f, (TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / float(SCREEN_H));    glVertex2f(0.0f, TA3D_INTRO_TOP + 2 * pCurrentFontHeight);

		glTexCoord2f(0.0f, (TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight) / float(SCREEN_H)); glVertex2f(0.0f, TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight);
		glTexCoord2f(1.0f, (TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight) / float(SCREEN_H)); glVertex2f(float(SCREEN_W), TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight);
		glColor4ub(0xFF, 0xFF, 0xFF, 0);
		glTexCoord2f(1.0f, TA3D_INTRO_BOTTOM / float(SCREEN_H));                            glVertex2f(float(SCREEN_W), TA3D_INTRO_BOTTOM);
		glTexCoord2f(0.0f, TA3D_INTRO_BOTTOM / float(SCREEN_H));                            glVertex2f(0.0f, TA3D_INTRO_BOTTOM);

		glTexCoord2f(0.0f, TA3D_INTRO_BOTTOM / float(SCREEN_H));                            glVertex2f(0.0f, TA3D_INTRO_BOTTOM);
		glTexCoord2f(1.0f, TA3D_INTRO_BOTTOM / float(SCREEN_H));                            glVertex2f(float(SCREEN_W), TA3D_INTRO_BOTTOM);
		glTexCoord2f(1.0f, (TA3D_INTRO_BOTTOM + 60.0f) / float(SCREEN_H));                  glVertex2f(float(SCREEN_W), TA3D_INTRO_BOTTOM + 60.0f);
		glTexCoord2f(0.0f, (TA3D_INTRO_BOTTOM + 60.0f) / float(SCREEN_H));                  glVertex2f(0.0f, TA3D_INTRO_BOTTOM + 60.0f);

		glEnd();
        gfx->glDisable(GL_BLEND);
        CHECK_GL();

		// Flip
		gfx->flip();
	}



} // namespace Menus
} // namespace TA3D
