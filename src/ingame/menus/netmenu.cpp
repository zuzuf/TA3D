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
            while(NetClient::instance()->messageWaiting() && !pArea->get_state("popup"))
            {
                String msg = NetClient::instance()->getNextMessage();
                if (StartsWith(msg, "MESSAGE"))
                {
                    pArea->set_title("popup", I18N::Translate("Server message"));
                    pArea->set_caption("popup.msg", I18N::Translate(msg.substr(8)));
                    pArea->msg("popup.show");
                }
                else if (StartsWith(msg, "ERROR"))
                {
                    pArea->set_title("popup", I18N::Translate("Server error"));
                    pArea->set_caption("popup.msg", I18N::Translate(msg.substr(6)));
                    pArea->msg("popup.show");
                }
            }

            pArea->set_entry("netmenu.peer_list", NetClient::instance()->getPeerList());

            pArea->set_enable_flag("netmenu.b_login", !(netMode == LOGIN || NetClient::instance()->getState() == NetClient::CONNECTED));
            pArea->set_enable_flag("netmenu.b_logout", NetClient::instance()->getState() == NetClient::CONNECTED);
            pArea->set_enable_flag("netmenu.b_register", NetClient::instance()->getState() != NetClient::CONNECTED);

            if (NetClient::instance()->getState() == NetClient::CONNECTED)
            {
                pArea->msg("netmenu.b_login.hide");
                pArea->msg("netmenu.b_logout.show");
            }
            else
            {
                pArea->msg("netmenu.b_login.show");
                pArea->msg("netmenu.b_logout.hide");
            }

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
                        netMode = NONE;
                    }
                    break;
                case LOGIN:
                    if (pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        login = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        askMode = NONE;
                        netMode = NONE;
                    }
                    break;
                case PASSWORD:
                    if (pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        password = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        password.clear();
                        askMode = NONE;
                        netMode = NONE;
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
                        netMode = NONE;
                    }
                    break;
                case LOGIN:
                    if (pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        login = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        askMode = NONE;
                        netMode = NONE;
                    }
                    break;
                case PASSWORD:
                    if (pArea->get_state("ask.b_ok"))       // User clicked ok
                    {
                        password = pArea->get_caption("ask.t_result");
                        askMode = NONE;
                    }
                    else if (pArea->get_state("ask.b_cancel"))       // User clicked cancel
                    {
                        login.clear();
                        password.clear();
                        askMode = NONE;
                        netMode = NONE;
                    }
                    break;
                };
                break;
            case NONE:
            default:
                login.clear();
                password.clear();
                if (pArea->get_state("netmenu.b_register"))
                {
                    netMode = REGISTER;
                    askMode = NONE;
                }
                else if (pArea->get_state("netmenu.b_login"))
                {
                    netMode = LOGIN;
                    askMode = NONE;
                }
                else if (pArea->get_state("netmenu.b_logout"))
                {
                    NetClient::instance()->destroyInstance();
                }
                break;
            };

            // Exit
            if (key[KEY_ESC] || pArea->get_state("netmenu.b_back"))
                return true;

            return false;
        }
    }
}
