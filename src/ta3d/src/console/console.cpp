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

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "console.h"
#include <logs/logs.h>
#include <input/keyboard.h>
#include <scripts/lua.env.h>
#include <scripts/lua.thread.h>
#include <ingame/battle.h>


namespace TA3D
{

	Console *Console::pInstance = NULL;

	Console *Console::Instance()
	{
		if (!pInstance)
			return new Console;
		return pInstance;
	}

	Console::Console()
			:pHistoryPos(0), pMaxItemsToDisplay(15), pVisible(0.0f), pShow(false), cursorPos(0)
	{
		pInstance = this;

		pInputText.clear();
		pLastEntries.resize(pMaxItemsToDisplay, QString());

		L = lua_open();
		lua_atpanic(L, lua_panic);	// Just to avoid having Lua exiting TA3D

		registerConsoleAPI();
		runInitScript();
	}


	Console::~Console()
	{
		lua_close(L);
		pInstance = NULL;
	}



	void Console::addEntry(const QString& newEntry)
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
        if (!pShow || pInputText.isEmpty())       // Need to clear the input text before closing console
			pShow ^= true;
		pMutex.unlock();
	}



	void Console::draw(TA3D::Font *fnt, const float dt, const bool forceShow)
	{
		MutexLocker locker(pMutex);

		const float m_vis = pVisible;
		const bool  m_sho = pShow;
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
                if (!Math::Equals(pVisible, 1.0f))
				{
					pVisible += dt;
					if (pVisible > 1.0f)
						pVisible = 1.0f;
				}
			}
			else
			{
				// slide out
                if (!Math::Zero(pVisible))
				{
					pVisible -= dt;
					if (pVisible < 0.0f)
						pVisible = 0.0f;
				}
                if (Math::Zero(pVisible))
					return;
			}
		}

		uint32 keycode = 0;

		if (keypressed())
			keycode = readkey();

		switch (keycode)
		{
        case KEY_NONE:
            break;
		case KEY_TAB:				// TAB-completion code
            if (!pInputText.isEmpty() && !pInputText.endsWith(' '))
			{
				QString tmp = pInputText;
				const QString delimiters(" ()'\"");
				for(int i = pInputText.size() - 1 ; i >= 0 ; --i)
					if (delimiters.contains(pInputText[i]))
					{
						tmp = Substr(pInputText, i + 1);
						break;
					}
                if (!tmp.isEmpty())
				{
					QString obj("_G.");
					QString param(tmp);
					for(int i = tmp.size() - 1 ; i >= 0 ; --i)
						if (tmp[i] == '.')
						{
							obj = Substr(tmp, 0, i + 1);
							param = Substr(tmp, i + 1);
							break;
						}

                    QString request = "return " + obj + "__tab_complete(\"" + param + "\")";

					const QString candidates = execute(request);
                    if (!candidates.isEmpty())
					{
                        QStringList candidateList = candidates.split(',',QString::SkipEmptyParts);

						if (!candidateList.empty())
						{
							QString longest = candidateList.front();
                            for(const QString &it : candidateList)
							{
                                while(!longest.isEmpty() && Substr(it, 0, longest.size()) != longest)
                                    longest.chop(1);
							}
							if (longest.size() > param.size())
							{
                                pInputText += Substr(longest, param.size());
								cursorPos = pInputText.size();
							}
							if (candidateList.size() > 1)
							{
								addEntry(QString());
								QString buf;
								int n = 0;
                                for(const QString &it : candidateList)
								{
                                    if (!buf.isEmpty())
                                        buf += '|';
                                    buf += it;
									++n;
									if (n == 5)
									{
										addEntry('|' + buf);
										n = 0;
										buf.clear();
									}
								}
                                if (!buf.isEmpty())
									addEntry('|' + buf);
							}
						}
					}
				}
			}
			break;
		case KEY_ENTER:
			pLastCommands.push_back(pInputText);
			pHistoryPos = static_cast<int>(pLastCommands.size());
            addEntry(">" + pInputText);
			execute(pInputText);
			pInputText.clear();
			cursorPos = 0;
			if (Battle::Instance() && Battle::Instance()->shoot && !Battle::Instance()->video_shoot)
				return;
			break;
		case KEY_BACKSPACE:
			if (pInputText.size() > 0 && cursorPos > 0)
			{
                pInputText = Substr(pInputText, 0, cursorPos - 1) + Substr(pInputText, cursorPos, pInputText.size() - cursorPos);
				cursorPos--;
			}
			break;
		case KEY_DEL:
			if (cursorPos < pInputText.size())
                pInputText = Substr(pInputText, 0, cursorPos) + Substr(pInputText, cursorPos + 1, pInputText.size() - cursorPos);
			break;
		case KEY_END:
			cursorPos = uint32(pInputText.size());
			break;
		case KEY_HOME:
			cursorPos = 0;
			break;
		case KEY_LEFT:
			if (cursorPos > 0)
				cursorPos--;
			break;
		case KEY_RIGHT:
			if (cursorPos < pInputText.size())
				cursorPos++;
			break;
		case KEY_UP:
			if (pHistoryPos > 0)
			{
				--pHistoryPos;
				pInputText = pLastCommands[pHistoryPos];
				cursorPos = uint32(pInputText.size());
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
				cursorPos = uint32(pInputText.size());
			}
			break;
		case KEY_ESC:
			break;
		case KEY_TILDE:
            if (pInputText.isEmpty())     // If text input is empty, then we're just closing the console
				break;
		default:
            if (pInputText.size() < 199)
			{
                const char c = keycode2char(keycode);
                if (c)
                {
                    pInputText = Substr(pInputText, 0, cursorPos) + c + Substr(pInputText, cursorPos, pInputText.size() - cursorPos);
                    cursorPos++;
                }
			}
		};

		const float fsize = fnt->height();
		const float maxh = fsize * static_cast<float>(pLastEntries.size()) * pVisible + 5.0f;

		glEnable(GL_BLEND);		// Dessine le cadre de la console
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.75f, 0.75f, 0.608f, 0.5f);

		gfx->rectfill(0.0f, 0.0f, static_cast<float>(SCREEN_W), maxh);

		glColor4f(0.75f, 0.75f, 0.608f, 0.75f);
		gfx->line(static_cast<float>(SCREEN_W), maxh, 0.0f, maxh);

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		// Print all lines
		float tableWidth = 80.0f;
        for (const QString &i_entry : pLastEntries)
		{
            if (i_entry.isEmpty())
				continue;
            if (i_entry[0] == '|')
			{
                const QStringList &cols = i_entry.split('|', QString::SkipEmptyParts);
                for(const QString &k : cols)
                    tableWidth = Math::Max(tableWidth, fnt->length(k) + 10.0f);
            }
		}
        int i = -1;
        for (const QString &i_entry : pLastEntries)
		{
            ++i;
            if (i_entry.isEmpty())
				continue;
            if (i_entry[0] == '|')
			{
                const QStringList &cols = i_entry.split('|', QString::SkipEmptyParts);
                for(int k = 0 ; k < cols.size() ; ++k)
				{
					gfx->print(fnt, tableWidth * float(k) + 1.0f, maxh - fsize * float(pLastEntries.size() + 1 - i) - 4.0f, 0.0f,
							   makeacol32(0,0,0,0xFF), cols[k]);
					gfx->print(fnt, tableWidth * float(k) + 0.0f, maxh - fsize * float(pLastEntries.size() + 1 - i) - 5.0f, 0.0f,
							   makeacol32(0xFF, 0xFF, 0, 0xFF), cols[k]);
				}
			}
			else
			{
				gfx->print(fnt, 1.0f, maxh - fsize * float(pLastEntries.size() + 1 - i) - 4.0f, 0.0f,
                           makeacol32(0,0,0,0xFF), i_entry);
				gfx->print(fnt, 0.0f, maxh - fsize * float(pLastEntries.size() + 1 - i) - 5.0f, 0.0f,
                           0xDFDFDFDF, i_entry);
			}
		}

        gfx->print(fnt, 1.0f, maxh - fsize - 4.0f, 0.0f, makeacol32(0,0,0,0xFF), ">" + pInputText );
        gfx->print(fnt, 1.0f + fnt->length(">" + Substr(pInputText, 0, cursorPos)), maxh - fsize - 4.0f, 0.0f, makeacol32(0,0,0,0xFF), "_" );

        gfx->print(fnt, 0.0f, maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, ">" + pInputText );
        gfx->print(fnt, fnt->length(">" + Substr(pInputText, 0, cursorPos)), maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, "_" );

		if (pHistoryPos < 0)
			pHistoryPos = 0;
		else
		{
			if (pHistoryPos > (int)pLastCommands.size())
				pHistoryPos = int(pLastCommands.size());
		}

		glDisable(GL_BLEND);

		if (forceShow)
		{
			pShow = m_sho;
			pVisible = m_vis;
		}
	}



	bool Console::activated()
	{
		MutexLocker locker(pMutex);
		return (pShow || pVisible > 0.0f);
	}

	QString Console::execute(const QString &cmd)
	{
		MutexLocker mLocker(pMutex);
		if (L == NULL)
			return QString();

		lua_settop(L, 0);
        if (luaL_loadbuffer(L, (const char*)cmd.toStdString().c_str(), cmd.size(), NULL ))
		{
			if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
				addEntry(lua_tostring(L, -1));
			else
				addEntry("# error running command!");
			return QString();
		}
		else
		{
			try
			{
				if (lua_pcall(L, 0, LUA_MULTRET, 0))
				{
					if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
						addEntry(lua_tostring(L, -1));
					else
						addEntry("# error running command!");
					return QString();
				}
			}
			catch(...)
			{
				if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
					addEntry(lua_tostring(L, -1));
				else
					addEntry("# error running command!");
				return QString();
			}
		}
		QString result;
		if (lua_gettop(L) > 0)
            result = lua_tostring(L, -1);
		lua_settop(L, 0);
		return result;
	}

	void Console::runInitScript()
	{
		if (L == NULL)
			return;

		uint32 filesize = 0;
		QString initScript = "scripts/console/init.lua";
		byte *buffer = loadLuaFile(initScript , filesize);
		if (buffer)
		{

			if (luaL_loadbuffer(L, (const char*)buffer, filesize, "init" ))
			{
				if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
				{
					LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
					LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));
					LOG_ERROR(LOG_PREFIX_LUA << filesize << " -> " << (int)buffer[filesize-1]);
					LOG_ERROR((const char*) buffer);
				}

				DELETE_ARRAY(buffer);
			}
			else
			{
				// This may not help debugging
				DELETE_ARRAY(buffer);

				try
				{
					if (lua_pcall(L, 0, 0, 0))
					{
						if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
						{
							LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
							LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
						}
						return;
					}
				}
				catch(...)
				{
					if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
					{
						LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
						LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
					}
					return;
				}
			}
		}
		else
		{
			LOG_ERROR(LOG_PREFIX_LUA << "Failed opening `" << initScript << "`");
		}
	}
} // namespace TA3D
