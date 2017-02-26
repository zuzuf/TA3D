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

#include "loading.h"
#include <TA3D_NameSpace.h>
#include <misc/paths.h>
#include <languages/i18n.h>
#include <gfx/gui/skin.h>
#include <gfx/gui/base.h>
#include <misc/timer.h>


namespace TA3D
{
namespace Menus
{

	Loading::Loading()
		:pLastPercent(-1.0f),
		pLastCaption(),
		pBackgroundTexture(0), pCurrentFontHeight(0.0f),
		pCacheScreenRatioWidth(0.f), pCacheScreenRatioHeight(0.f)
	{
		LOG_DEBUG(LOG_PREFIX_MENU_LOADING << "Starting...");
		pStartTime = msectimer();
		loadTheBackgroundTexture();
		initializeDrawing();
	}

	Loading::~Loading()
	{
		finalizeDrawing();
		gfx->destroy_texture(pBackgroundTexture);

		// Loading time
		LOG_DEBUG(LOG_PREFIX_MENU_LOADING << "Done.");
		LOG_INFO("Time of loading :" << float(msectimer() - pStartTime) * 0.001f << "s");
	}

	void Loading::initializeDrawing()
	{
		LOG_ASSERT(NULL != gfx);

		gfx->set_2D_mode();
		pCurrentFontHeight = Gui::gui_font->height();

		pCacheScreenRatioWidth  = float(SCREEN_W) / 1280.0f;
		pCacheScreenRatioHeight = float(SCREEN_H) / 1024.0f;
	}

	void Loading::finalizeDrawing()
	{
		// Reset 3D mode
		gfx->unset_2D_mode();
	}


	void Loading::loadTheBackgroundTexture()
	{
		LOG_ASSERT(NULL != gfx);

        if (!lp_CONFIG->skin_name.isEmpty() && VFS::Instance()->fileExists(lp_CONFIG->skin_name))
		{
			Gui::Skin skin;
			skin.loadTDFFromFile(lp_CONFIG->skin_name);
            if (!skin.prefix().isEmpty())
                pBackgroundTexture = gfx->load_texture_mask("gfx/" + skin.prefix() + "load.png", 7);
			else
				pBackgroundTexture = gfx->load_texture_mask("gfx/load.png", 7);
		}
		else
			pBackgroundTexture = gfx->load_texture_mask("gfx/load.png", 7);
	}


	void Loading::doNoticeOtherPlayers(const float percent)
	{
		// Broadcast informations
		if (network_manager.isConnected() && !Math::Equals(pLastPercent, percent))
            network_manager.sendAll(QString("LOADING %1").arg(percent));
	}


	void Loading::operator()(const float percent, const QString &message)
	{
		LOG_ASSERT(NULL != gfx);

		if (Math::Equals(pLastPercent, percent) && message == pLastCaption)
			return;

		// Notice other players about the progression
		doNoticeOtherPlayers(percent);

		// Update the message list
		if (message != pLastCaption)
		{
            if (!pLastCaption.isEmpty())
                pMessages.front() += " - " + I18N::Translate("done");
			pMessages.push_front(message);
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		glPushMatrix();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Draw the texture
		gfx->drawtexture(pBackgroundTexture, 0.0f, 0.0f, float(SCREEN_W), float(SCREEN_H));

		// Draw all previous messages
		int indx(0);
		const QStringList::const_iterator end = pMessages.end();
		for (QStringList::const_iterator i = pMessages.begin() ; i != end ; ++i, ++indx)
		{
			gfx->print(Gui::gui_font, 105.0f * pCacheScreenRatioWidth + 1.0f, 175.0f * pCacheScreenRatioHeight + pCurrentFontHeight * float(indx) + 1.0f, 0.0f, makeacol(0, 0, 0, 0xFF), *i);
			gfx->print(Gui::gui_font, 105.0f * pCacheScreenRatioWidth, 175.0f * pCacheScreenRatioHeight + pCurrentFontHeight * float(indx), 0.0f, 0xFFFFFFFF, *i);
		}

		// Draw the progress bar
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.5f, 0.8f, 0.3f);
		glBegin(GL_QUADS);
		glVertex2f(100.0f * pCacheScreenRatioWidth, 858.0f * pCacheScreenRatioHeight);
		glVertex2f((100.0f + 10.72f * percent) * pCacheScreenRatioWidth, 858.0f * pCacheScreenRatioHeight);
		glVertex2f((100.0f + 10.72f * percent) * pCacheScreenRatioWidth, 917.0f * pCacheScreenRatioHeight);
		glVertex2f(100.0f * pCacheScreenRatioWidth, 917.0f * pCacheScreenRatioHeight);
		glEnd();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(0xFF,0xFF,0xFF,0xFF);
		gfx->drawtexture(pBackgroundTexture, 100.0f * pCacheScreenRatioWidth, 856.0f * pCacheScreenRatioHeight,
			1172.0f * pCacheScreenRatioWidth, 917.0f * pCacheScreenRatioHeight,
			100.0f / 1280.0f, 856.0f / 1024.0f, 1172.0f / 1280.0f, 917.0f / 1024.0f);
		glDisable(GL_BLEND);

		// Draw the caption (horizontally centered)
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		gfx->print(Gui::gui_font, 640.0f * pCacheScreenRatioWidth - 0.5f * Gui::gui_font->length(message) + 1.0f,
				   830 * pCacheScreenRatioHeight - pCurrentFontHeight * 0.5f + 1.0f,0.0f, makeacol(0, 0, 0, 0xFF),
				   message);
		gfx->print(Gui::gui_font, 640.0f * pCacheScreenRatioWidth - 0.5f * Gui::gui_font->length(message),
				   830 * pCacheScreenRatioHeight - pCurrentFontHeight * 0.5f,0.0f,0xFFFFFFFF,
				   message);
		glDisable(GL_BLEND);

		glPopMatrix();

		// Flip the screen
		gfx->flip();

		pLastPercent = percent;
		pLastCaption = message;
	}



} // namespace Menus
} // namespace TA3D

