#include "netmenu.h"
#include "../../languages/i18n.h"
#include "../../network/netclient.h"

namespace TA3D
{
    namespace Menus
    {

        bool NetMenu::Execute()
        {
            NetMenu m;
            return m.execute();
        }

        NetMenu::NetMenu()
            :Abstract()
        {}


        NetMenu::~NetMenu()
        {
            NetClient::instance()->disconnect();
        }


        void NetMenu::doFinalize()
        {
            clear_keybuf();
            NetClient::destroyInstance();
        }


        bool NetMenu::doInitialize()
        {
            LOG_DEBUG(LOG_PREFIX_MENU_NETMENU << "Entering...");

            // Loading the area
            loadAreaFromTDF("netmenu", "gui/netmenu.area");

            askMode = NONE;
            netMode = LOGIN;

            return true;
        }



        void NetMenu::waitForEvent()
        {
            do
            {
                // Grab user events
                pArea->check();

                // Wait to reduce CPU consumption
                rest(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);

                // Listen to server
                NetClient::instance()->receive();

            } while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
                     && mouse_b == 0
                     && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
                     && !pArea->key_pressed && !pArea->scrolling
                     && !NetClient::instance()->messageWaiting());
        }


        bool NetMenu::maySwitchToAnotherMenu()
        {
            // First we have to parse the server messages

            // Now we run all processes to get logged/registered
            switch(netMode)
            {
            case LOGIN:
                switch(askMode)
                {
                case NONE:
                    if (login.empty())
                    {
                        askMode = LOGIN;
                        pArea->set_title("ask", I18N::Translate("Login"));
                        pArea->set_caption("ask.t_result", "");
                        pArea->msg("ask.show");
                    }
                    else if (password.empty())
                    {
                        askMode = PASSWORD;
                        pArea->set_title("ask", I18N::Translate("Password"));
                        pArea->set_caption("ask.t_result", "");
                        pArea->msg("ask.show");
                    }
                    else if (NetClient::instance()->getState() != NetClient::CONNECTED)
                    {
                        NetClient::instance()->connect(lp_CONFIG->net_server, 4240, login, password);
                    }
                    break;
                case LOGIN:
                    if (!pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        login = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (!pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        askMode = NONE;
                    }
                    break;
                case PASSWORD:
                    if (!pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        password = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (!pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        password.clear();
                        askMode = NONE;
                    }
                    break;
                };
                break;
            case REGISTER:
                switch(askMode)
                {
                case NONE:
                    if (login.empty())
                    {
                        askMode = LOGIN;
                        pArea->set_title("ask", I18N::Translate("New account - login"));
                        pArea->set_caption("ask.t_result", "");
                        pArea->msg("ask.show");
                    }
                    else if (password.empty())
                    {
                        askMode = PASSWORD;
                        pArea->set_title("ask", I18N::Translate("New account - password"));
                        pArea->set_caption("ask.t_result", "");
                        pArea->msg("ask.show");
                    }
                    else if (NetClient::instance()->getState() != NetClient::CONNECTED)
                    {
                        NetClient::instance()->connect(lp_CONFIG->net_server, 4240, login, password, true);
                    }
                    break;
                case LOGIN:
                    if (!pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        login = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (!pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        askMode = NONE;
                    }
                    break;
                case PASSWORD:
                    if (!pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        password = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (!pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        password.clear();
                        askMode = NONE;
                    }
                    break;
                };
                break;
            case NONE:
            default:
                break;
            };

            // Exit
            if (key[KEY_ESC] || pArea->get_state("netmenu.b_back"))
                return true;

            return false;
        }
    }
}
