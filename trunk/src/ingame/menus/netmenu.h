#ifndef NETMENU_H
#define NETMENU_H

#include "base.h"

namespace TA3D
{
    namespace Menus
    {

        class NetMenu : public Abstract
        {
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
            virtual void addChatMessage(const String &message);

        private:
            enum NetMode { NONE,
                           LOGIN,
                           LOGGED,
                           REGISTER,
                           PASSWORD };

            NetMode askMode;
            NetMode netMode;
            String  login;
            String  password;
        };
    }
}

#endif // NETMENU_H
