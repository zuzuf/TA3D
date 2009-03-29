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
|      This module is responsible for the Console object, useful tool   |
| for developpers and testers.                                          |
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
        :pMaxItemsToDisplay(15), pVisible(0.0f), pShow(false), cursorPos(0)
    {
        pInputText.clear();
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



    String Console::draw(TA3D::Font *fnt, const float dt, const bool forceShow)
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
                    return "";
            }
        }

        uint16 keyb = 0;
        uint32 keycode = 0;

        if (keypressed())
        {
            keycode = readkey();
            keyb = keycode & 0xFFFF;
            keycode >>= 16;
        }

        float fsize = fnt->height();
        float maxh = fsize * pLastEntries.size() * pVisible + 5.0f;

        String newline;

        glEnable(GL_BLEND);		// Dessine le cadre de la console
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.75f, 0.75f, 0.608f, 0.5f);

        gfx->rectfill(0,0,SCREEN_W,maxh);

        glColor4f(0.75f, 0.75f, 0.608f, 0.75f);
        gfx->line(SCREEN_W, maxh, 0, maxh);

        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

        // Print all lines
        int i = 0;
        for (String::List::const_iterator i_entry = pLastEntries.begin(); i_entry != pLastEntries.end(); ++i_entry)
        {
            gfx->print(fnt, 1.0f, maxh - fsize * (pLastEntries.size() + 1 - i) - 4.0f, 0.0f,
                       makeacol32(0,0,0,0xFF), ">" + *i_entry);
            gfx->print(fnt, 0.0f, maxh - fsize * (pLastEntries.size() + 1 - i) - 5.0f, 0.0f,
                       0xDFDFDFDF, ">" + *i_entry);
            ++i;
        }

        gfx->print(fnt, 1.0f, maxh - fsize - 4.0f, 0.0f, makeacol32(0,0,0,0xFF), ">" + pInputText );
        gfx->print(fnt, 1.0f + fnt->length(">" + pInputText.substrUTF8(0, cursorPos)), maxh - fsize - 4.0f, 0.0f, makeacol32(0,0,0,0xFF), "_" );

        gfx->print(fnt, 0.0f, maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, ">" + pInputText );
        gfx->print(fnt, fnt->length(">" + pInputText.substrUTF8(0, cursorPos)), maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, "_" );

        if (pHistoryPos < 0)
			pHistoryPos = 0;
        else
		{
			if (pHistoryPos > (int)pLastCommands.size())
				pHistoryPos = pLastCommands.size();
		}

        switch(keycode)
        {
        case KEY_ENTER:
            pLastCommands.push_back(pInputText);
            pHistoryPos = pLastCommands.size();
            addEntry(pInputText);
            newline = pInputText;
            pInputText.clear();
            cursorPos = 0;
            break;
        case KEY_BACKSPACE:
            if (pInputText.size() > 0 && cursorPos > 0)
            {
                pInputText = pInputText.substrUTF8(0, cursorPos - 1) + pInputText.substrUTF8(cursorPos, pInputText.sizeUTF8() - cursorPos);
                cursorPos--;
            }
            break;
        case KEY_DEL:
            if (cursorPos < pInputText.sizeUTF8())
                pInputText = pInputText.substrUTF8(0, cursorPos) + pInputText.substrUTF8(cursorPos + 1, pInputText.sizeUTF8() - cursorPos);
            break;
        case KEY_END:
            cursorPos = pInputText.sizeUTF8();
            break;
        case KEY_HOME:
            cursorPos = 0;
            break;
        case KEY_LEFT:
            if (cursorPos > 0)
                cursorPos--;
            break;
        case KEY_RIGHT:
            if (cursorPos < pInputText.sizeUTF8())
                cursorPos++;
            break;
        case KEY_UP:
            if (pHistoryPos > 0)
            {
                --pHistoryPos;
                pInputText = pLastCommands[pHistoryPos];
                cursorPos = pInputText.sizeUTF8();
            }
            break;
        case KEY_DOWN:
            if (pHistoryPos < (int)pLastCommands.size())
            {
                ++pHistoryPos;
                if (pHistoryPos < (int)pLastCommands.size())
                    pInputText = pLastCommands[pHistoryPos];
                else
                    pInputText.clear();
                cursorPos = pInputText.sizeUTF8();
            }
            break;
        case KEY_TILDE:
        case KEY_ESC:
            break;
        default:
            if (keyb != 0 && pInputText.sizeUTF8() < 199)
            {
                pInputText = pInputText.substrUTF8(0, cursorPos) + InttoUTF8(keyb) + pInputText.substrUTF8(cursorPos, pInputText.sizeUTF8() - cursorPos);
                cursorPos++;
            }
        };

        glDisable(GL_BLEND);

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
