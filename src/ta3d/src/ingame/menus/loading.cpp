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
#include "../../TA3D_NameSpace.h"
#include "../../misc/paths.h"
#include "../../gui.h"
#include "../../languages/i18n.h"
#include "../../gfx/gui/skin.h"



namespace TA3D
{
namespace Menus
{

    Loading::Loading()
        :pNbTasksCompleted(0.0f), pMaxTasksCompleted(100.0f),
        pPercent(0.0f), pLastPercent(-1.0f), pBroadcastInformations(true),
        pCaption("Loading..."),
        pBackgroundTexture(0), pCurrentFontHeight(0.0f)
    {
        LOG_DEBUG(LOG_PREFIX_MENU_LOADING << "Starting...");
        pStartTime = msec_timer;
        doNoticeOtherPlayers();
        loadTheBackgroundTexture();
        initializeDrawing();
    }

    Loading::Loading(const float maxTasks)
        :pNbTasksCompleted(0.0f), pMaxTasksCompleted(maxTasks),
        pPercent(0.0f), pLastPercent(-1.0f), pBroadcastInformations(true),
        pCaption("Loading..."),
        pBackgroundTexture(0), pCurrentFontHeight(0.0f)
    {
        LOG_DEBUG(LOG_PREFIX_MENU_LOADING << "Starting...");
        pStartTime = msec_timer;
        doNoticeOtherPlayers();
        loadTheBackgroundTexture();
        initializeDrawing();
    }

    Loading::~Loading()
    {
        finalizeDrawing();
        gfx->destroy_texture(pBackgroundTexture);

        // Loading time
        LOG_DEBUG(LOG_PREFIX_MENU_LOADING << "Done.");
        LOG_INFO("Time of loading :" << (msec_timer - pStartTime) * 0.001f << "s");
    }

    void Loading::initializeDrawing()
    {
        LOG_ASSERT(NULL != gfx);

        gfx->set_2D_mode();

        pCurrentFontHeight = gui_font->height();
    }

    void Loading::finalizeDrawing()
    {
        // Reset 3D mode
        gfx->unset_2D_mode();
    }


    String Loading::caption()
    {
        MutexLocker locker(pMutex);
        return pCaption;
    }

    void Loading::caption(const String& s)
    {
        if (!s.empty())
        {
            pMutex.lock();
            pCaption = s;
            pMutex.unlock();
        }
    }

    void Loading::loadTheBackgroundTexture()
    {
        LOG_ASSERT(NULL != gfx);

        if (!lp_CONFIG->skin_name.empty() && TA3D::Paths::Exists(lp_CONFIG->skin_name))
        {
            SKIN skin;
            skin.load_tdf(lp_CONFIG->skin_name);
            if (!skin.prefix.empty())
                pBackgroundTexture = gfx->load_texture_mask("gfx" + Paths::SeparatorAsString + "load.jpg", 7);
            else
                pBackgroundTexture = gfx->load_texture_mask("gfx" + Paths::SeparatorAsString + "load.jpg", 7);
        }
        else
            pBackgroundTexture = gfx->load_texture_mask("gfx" + Paths::SeparatorAsString + "load.jpg", 7);
    }


    void Loading::doNoticeOtherPlayers()
    {
        // Broadcast informations
        if (pBroadcastInformations && network_manager.isConnected() && pLastPercent != pPercent)
            network_manager.sendAll(format("LOADING %d", pPercent));
    }


    void Loading::doProgress(const float progression, const bool relative)
    {
        if (relative)
            pNbTasksCompleted += progression;
        else
            pNbTasksCompleted = progression;
        if (pNbTasksCompleted < 0)
            pNbTasksCompleted = 0;
        else
        {
            if (pNbTasksCompleted > pMaxTasksCompleted)
                pNbTasksCompleted = pMaxTasksCompleted;
        }
        pPercent = pNbTasksCompleted / pMaxTasksCompleted * 100.0f;
    }



    void Loading::progress(const float progression, const bool relative)
    {
        pMutex.lock();
        doProgress(progression, relative);
        pMutex.unlock();
    }

    void Loading::progress(const String& info, const float progression, const bool relative)
    {
        pMutex.lock();
        doProgress(progression, relative);
        if (pCaption != info)
        {
            pCaption = info;
            pMessages.push_front(info + " - " + I18N::Translate("done"));
        }
        pMutex.unlock();
    }


    bool Loading::broadcastInfosAboutLoading()
    {
        MutexLocker locker(pMutex);
        return pBroadcastInformations;
    }


    void Loading::broadcastInfosAboutLoading(const bool v)
    {
        pMutex.lock();
        pBroadcastInformations = v;
        pMutex.unlock();
    }

    int Loading::maxTasks()
    {
        MutexLocker locker(pMutex);
        return (int)pMaxTasksCompleted;
    }

    void Loading::maxTasks(const float v)
    {
        pMutex.lock();
        pMaxTasksCompleted = v;
        pMutex.unlock();
    }

    float Loading::percent()
    {
        MutexLocker locker(pMutex);
        return pPercent;
    }


    void Loading::draw()
    {
        LOG_ASSERT(NULL != gfx);
        MutexLocker locker(pMutex);

        if (pLastPercent == pPercent)
            return;
        pLastPercent = pPercent;
        // Notice other players about the progression
        doNoticeOtherPlayers();

        glPushMatrix();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Draw the texture
        gfx->drawtexture(pBackgroundTexture, 0.0f, 0.0f, SCREEN_W, SCREEN_H);

        float fw = SCREEN_W / 1280.0f;
        float fh = SCREEN_H / 1024.0f;

        // Draw all previous messages
        int indx(0);
        for (String::List::const_iterator i = pMessages.begin() ; i != pMessages.end() ; ++i, ++indx)
            gfx->print(gui_font, 105.0f * fw, 175.0f * fh + pCurrentFontHeight * indx, 0.0f, 0xFFFFFFFF, *i);

        // Draw the progress bar
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.5f, 0.8f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(100.0f * fw, 858.0f * fh);
        glVertex2f((100.0f + 10.72f * pPercent) * fw, 858.0f * fh);
        glVertex2f((100.0f + 10.72f * pPercent) * fw, 917.0f * fh);
        glVertex2f(100.0f * fw, 917.0f * fh);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4ub(0xFF,0xFF,0xFF,0xFF);
        gfx->drawtexture(pBackgroundTexture, 100.0f * fw, 856.0f * fh, 1172.0f * fw, 917.0f * fh,
                         100.0f / 1280.0f, 862.0f / 1024.0f, 1172.0f / 1280.0f, 917.0f / 1024.0f);
        glDisable(GL_BLEND);

        // Draw the caption (horizontally centered)
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        gfx->print(gfx->TA_font, 640.0f * fw - 0.5f * gfx->TA_font->length(pCaption),
                   830 * fh - pCurrentFontHeight * 0.5f, 0.0f, 0xFFFFFFFF,
                   pCaption);
        glDisable(GL_BLEND);

        glPopMatrix();

        // Flip the screen
        gfx->flip();
    }



} // namespace Menus
} // namespace TA3D

