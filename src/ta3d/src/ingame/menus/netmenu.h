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

#ifndef NETMENU_H
# define NETMENU_H

# include <stdafx.h>
# include <misc/string.h>
# include "base.h"
# include <network/http.h>

namespace TA3D
{
    namespace Menus
    {

        class NetMenu : public Abstract
        {
        private:
            class Download
            {
            public:
                typedef std::list<Download*> List;
            public:
                ~Download();
				void start(const QString &filename, const QString &url, const int mID);
                void stop();
                void update();
                bool downloading();
				QString getFilename() {   return filename;    }
				int getModID()	{	return modID;	}
            private:
                QString filename;
                QString wnd;
                Http http;
				int modID;
                static int wndNumber;
				int lastProgress;
            };

        public:
            /*!
            ** \brief Execute an instance of NetMenu
            */
            static bool Execute();
        public:
            NetMenu();
            //! Destructor
            virtual ~NetMenu();

        protected:
            virtual bool doInitialize();
            virtual void doFinalize();
            virtual void waitForEvent();
            virtual bool maySwitchToAnotherMenu();
            virtual void addChatMessage(const QString &message);
            virtual void parseServerMessages();
            virtual void updateGUI();
			virtual void hostAGame();
			virtual void joinAGame();
			virtual void changeMod(const int ID);

        private:
            enum NetMode { NONE,
                           LOGIN,
                           LOGGED,
                           REGISTER,
                           PASSWORD };

            NetMode askMode;
            NetMode netMode;
            QString  login;
            QString  password;
            Download::List downloadList;
        };
    }
}

#endif // NETMENU_H
