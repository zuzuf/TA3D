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

/*----------------------------------------------------------------------\
  |                               console.cpp                             |
  |      contient les classes nécessaires à la gestion d'une console dans |
  | programme utilisant Allegro avec ou sans AllegroGL. La console        |
  | dispose de sa propre procédure d'entrée et d'affichage mais celle-ci  |
  | nécessite d'être appellée manuellement pour éviter les problèmes      |
  | découlant d'un appel automatique par un timer.                        |
  \----------------------------------------------------------------------*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "console.h"
#include "logs/logs.h"
#include <list>
#include "misc/osinfo.h"




namespace TA3D
{

    Console console;


    Console::Console()
        :pMaxItemsToDisplay(15), pVisible(0.0f), pShow(false)
    {
        pInputText[0] = 0;
    }


    Console::~Console()
    {}



    void Console::addEntry(const String& newEntry)
    {
        pMutex.lock();
        pLastEntries.push_back(newEntry);
        if (pLastEntries.size() >= pMaxItemsToDisplay)
            pLastEntries.pop_front();
        pMutex.unlock();
    }


    void Console::toggleShow()
    {
        pMutex.lock();
        pShow ^= true;
        pMutex.unlock();
    }



    char *Console::draw(TA3D::GfxFont& fnt, const float dt, float fsize, const bool forceShow)
    {
        MutexLocker locker(pMutex);

        float m_vis = pVisible;
        bool  m_sho = pShow;
        if (forceShow)
        {
            // The display of the console has been forced
            pShow = true;
            pVisible = 1.0f;
        }
        else
        {
            if (pShow)
            {
                // slide in
                if (pVisible != 1.0f)
                {
                    pVisible += dt;
                    if (pVisible > 1.0f)
                        pVisible = 1.0f;
                }
            }
            else
            {
                // slide out
                if (pVisible != 0.0f)
                {
                    pVisible -= dt;
                    if (pVisible < 0.0f)
                        pVisible = 0.0f;
                }
                if (pVisible == 0.0f)
                    return NULL;
            }
        }

        set_uformat(U_UTF8);
        char keyb = 0;

        if (keypressed())
            keyb = readkey();

        ++fsize;
        float maxh = fsize * pLastEntries.size() * pVisible + 5.0f;

        char* newline = NULL;

        glEnable(GL_BLEND);		// Dessine le cadre de la console
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.75f, 0.75f, 0.608f, 0.5f);

        gfx->rectfill(0,0,SCREEN_W,maxh);

        glColor4f(0.75f, 0.75f, 0.608f, 0.75f);
        gfx->line(SCREEN_W, maxh, 0, maxh);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

        // Print all lines
        int i = 0;
        for (String::List::const_iterator i_entry = pLastEntries.begin(); i_entry != pLastEntries.end(); ++i_entry) 
        {
            gfx->print(fnt, 0.0f, maxh - fsize * (pLastEntries.size() + 1 - i) - 5.0f, 0.0f,
                       0xAFAFAFAF, format(">%s", (char *)((*i_entry).c_str())));
            ++i;
        }

        gfx->print(fnt, 0.0f, maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, format(">%s_", pInputText));

        if (keyb == 13)
        {
            String x = pInputText;
            addEntry(x);
            pInputText[0] = 0;
            newline = strdup((char *)x.c_str());
        }

        if (keyb == 8 && strlen(pInputText) > 0)
            pInputText[strlen(pInputText) - 1] = 0;

        if ((keyb >= '0' && keyb <= '9') ||  (keyb >= 'a' && keyb <= 'z') || 
            (keyb >= 'A' && keyb <= 'Z') || 
            keyb == 32 || keyb == '_' || keyb == '+' || keyb == '-' || keyb == '.') 
        {
            if (strlen(pInputText) < 199) 
            {
                pInputText[strlen(pInputText) + 1] = 0;
                pInputText[strlen(pInputText)] = keyb;
            }
        }
        glDisable(GL_BLEND);
        set_uformat(U_ASCII);

        if (forceShow)
        {
            pShow = m_sho;
            pVisible = m_vis;
        }

        return newline;
    }



    bool Console::activated()
    {
        MutexLocker locker(pMutex);
        return (pShow || pVisible > 0.0f);
    }


} // namespace TA3D
