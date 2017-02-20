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
|                               console.h                               |
|      This module is responsible for the Console object, useful tool   |
| for developpers and testers.                                          |
\----------------------------------------------------------------------*/

#ifndef _TA3D_XX_CLASSE_CONSOLE_H__
# define _TA3D_XX_CLASSE_CONSOLE_H__

# include <misc/string.h>
# include <threads/mutex.h>
# include <gfx/gfx.h>
#ifndef __LUA_INCLUDES__
#define __LUA_INCLUDES__
#ifdef LUA_NOJIT
# include "../lua/lua.hpp"
#else
# include "../luajit/src/lua.hpp"
#endif
#endif
# include <deque>


namespace TA3D
{


    /*! \class Console
    **
    ** \brief The user console
    */
    class Console
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        Console();
        //! Destructor
        ~Console();
        //@}


        /*!
        ** \brief Toggle the state of the visibility of the console
        */
        void toggleShow();

        /*!
        ** \brief Get if the console is activated
        */
        bool activated();

        /*!
        ** \brief Add an entry in the console
        */
        void addEntry(const QString& newEntry);

        /*!
        ** \brief Draw the console
        **
        ** \param fnt Font
        ** \param dt Delta
        ** \param fsize Size of the font
        ** \param forceShow Display the console even it should be displayed
        */
		void draw(TA3D::Font *fnt, const float dt, const bool forceShow = false);

		QString execute(const QString &cmd);
	private:
		void registerConsoleAPI();
		void runInitScript();

    private:
		typedef std::deque<QString> EntryList;
        //! Mutex
        Mutex pMutex;
        //!
		EntryList pLastEntries;
        //!
        QStringList pLastCommands;
        //!
        int pHistoryPos;
        //!
		uint32 pMaxItemsToDisplay;
        //!
        float pVisible;
        //!
        bool pShow;
        //!
        QString pInputText;
        //!
		uint32 cursorPos;

		//! The Lua state used to execute commands
		lua_State *L;

	private:
		static Console *pInstance;
	public:
		static Console *Instance();
    }; //class Console

} // namespace TA3D

#endif // _TA3D_XX_CLASSE_CONSOLE_H__
