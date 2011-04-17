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
		pLastEntries.resize(pMaxItemsToDisplay, String());

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
		if (!pShow || pInputText.empty())       // Need to clear the input text before closing console
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
				if (!Yuni::Math::Equals(pVisible, 1.0f))
				{
					pVisible += dt;
					if (pVisible > 1.0f)
						pVisible = 1.0f;
				}
			}
			else
			{
				// slide out
				if (!Yuni::Math::Zero(pVisible))
				{
					pVisible -= dt;
					if (pVisible < 0.0f)
						pVisible = 0.0f;
				}
				if (Yuni::Math::Zero(pVisible))
					return;
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

		switch (keycode)
		{
		case KEY_TAB:				// TAB-completion code
			if (!pInputText.empty() && pInputText.last() != ' ')
			{
				String tmp = pInputText;
				const String delimiters(" ()'\"");
				for(int i = pInputText.size() - 1 ; i >= 0 ; --i)
					if (delimiters.contains(pInputText[i]))
					{
						tmp = Substr(pInputText, i + 1);
						break;
					}
				if (!tmp.empty())
				{
					String obj("_G.");
					String param(tmp);
					for(int i = tmp.size() - 1 ; i >= 0 ; --i)
						if (tmp[i] == '.')
						{
							obj = Substr(tmp, 0, i + 1);
							param = Substr(tmp, i + 1);
							break;
						}

					String request = String("return ") << obj << "__tab_complete(\"" << param << "\")";

					const String candidates = execute(request);
					if (!candidates.empty())
					{
						String::List candidateList;
						candidates.explode(candidateList,',',true,false,true);

						if (!candidateList.empty())
						{
							String longest = candidateList.front();
							for(String::List::iterator it = candidateList.begin() ; it != candidateList.end() ; ++it)
							{
								while(!longest.empty() && Substr(*it, 0, longest.size()) != longest)
									longest.removeLast();
							}
							if (longest.size() > param.size())
							{
								pInputText << Substr(longest, param.size());
								cursorPos = pInputText.utf8size();
							}
							if (candidateList.size() > 1)
							{
								addEntry(String());
								String buf;
								int n = 0;
								for(String::List::iterator it = candidateList.begin() ; it != candidateList.end() ; ++it)
								{
									if (!buf.empty())
										buf << '|';
									buf << *it;
									++n;
									if (n == 5)
									{
										addEntry('|' + buf);
										n = 0;
										buf.clear();
									}
								}
								if (!buf.empty())
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
			addEntry(String(">") << pInputText);
			execute(pInputText);
			pInputText.clear();
			cursorPos = 0;
			if (Battle::Instance() && Battle::Instance()->shoot && !Battle::Instance()->video_shoot)
				return;
			break;
		case KEY_BACKSPACE:
			if (pInputText.size() > 0 && cursorPos > 0)
			{
				pInputText = SubstrUTF8(pInputText, 0, cursorPos - 1) << SubstrUTF8(pInputText, cursorPos, pInputText.utf8size() - cursorPos);
				cursorPos--;
			}
			break;
		case KEY_DEL:
			if (cursorPos < pInputText.utf8size())
				pInputText = SubstrUTF8(pInputText, 0, cursorPos) << SubstrUTF8(pInputText, cursorPos + 1, pInputText.utf8size() - cursorPos);
			break;
		case KEY_END:
			cursorPos = uint32(pInputText.utf8size());
			break;
		case KEY_HOME:
			cursorPos = 0;
			break;
		case KEY_LEFT:
			if (cursorPos > 0)
				cursorPos--;
			break;
		case KEY_RIGHT:
			if (cursorPos < pInputText.utf8size())
				cursorPos++;
			break;
		case KEY_UP:
			if (pHistoryPos > 0)
			{
				--pHistoryPos;
				pInputText = pLastCommands[pHistoryPos];
				cursorPos = uint32(pInputText.utf8size());
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
				cursorPos = uint32(pInputText.utf8size());
			}
			break;
		case KEY_ESC:
			break;
		case KEY_TILDE:
			if (pInputText.empty())     // If text input is empty, then we're just closing the console
				break;
		default:
			if (keyb != 0 && pInputText.utf8size() < 199)
			{
				pInputText = SubstrUTF8(pInputText, 0, cursorPos) << InttoUTF8(keyb) << SubstrUTF8(pInputText, cursorPos, pInputText.utf8size() - cursorPos);
				cursorPos++;
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
		int i = 0;
		const EntryList::const_iterator end = pLastEntries.end();
		float tableWidth = 80.0f;
		for (EntryList::const_iterator i_entry = pLastEntries.begin(); i_entry != end; ++i_entry)
		{
			if (i_entry->empty())
				continue;
			if (i_entry->first() == '|')
			{
				String::Vector cols;
				i_entry->explode(cols, '|', true, false, true);
				for(uint32 k = 0 ; k < cols.size() ; ++k)
					tableWidth = Math::Max(tableWidth, fnt->length(cols[k]) + 10.0f);
			}
		}
		for (EntryList::const_iterator i_entry = pLastEntries.begin(); i_entry != end; ++i_entry, ++i)
		{
			if (i_entry->empty())
				continue;
			if (i_entry->first() == '|')
			{
				String::Vector cols;
				i_entry->explode(cols, '|', true, false, true);
				for(uint32 k = 0 ; k < cols.size() ; ++k)
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
						   makeacol32(0,0,0,0xFF), *i_entry);
				gfx->print(fnt, 0.0f, maxh - fsize * float(pLastEntries.size() + 1 - i) - 5.0f, 0.0f,
						   0xDFDFDFDF, *i_entry);
			}
		}

		gfx->print(fnt, 1.0f, maxh - fsize - 4.0f, 0.0f, makeacol32(0,0,0,0xFF), String(">") << pInputText );
		gfx->print(fnt, 1.0f + fnt->length(String(">") << SubstrUTF8(pInputText, 0, cursorPos)), maxh - fsize - 4.0f, 0.0f, makeacol32(0,0,0,0xFF), "_" );

		gfx->print(fnt, 0.0f, maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, String(">") << pInputText );
		gfx->print(fnt, fnt->length(String(">") << SubstrUTF8(pInputText, 0, cursorPos)), maxh - fsize - 5.0f, 0.0f, 0xFFFFFFFF, "_" );

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

	String Console::execute(const String &cmd)
	{
		MutexLocker mLocker(pMutex);
		if (L == NULL)
			return String();

		lua_settop(L, 0);
		if (luaL_loadbuffer(L, (const char*)cmd.c_str(), cmd.size(), NULL ))
		{
			if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
				addEntry(lua_tostring(L, -1));
			else
				addEntry("# error running command!");
			return String();
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
					return String();
				}
			}
			catch(...)
			{
				if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
					addEntry(lua_tostring(L, -1));
				else
					addEntry("# error running command!");
				return String();
			}
		}
		String result;
		if (lua_gettop(L) > 0)
			result << lua_tostring(L, -1);
		lua_settop(L, 0);
		return result;
	}

	void Console::runInitScript()
	{
		if (L == NULL)
			return;

		uint32 filesize = 0;
		String initScript = "scripts/console/init.lua";
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
