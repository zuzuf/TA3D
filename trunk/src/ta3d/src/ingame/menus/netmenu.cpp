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

#include "netmenu.h"
#include "../../languages/i18n.h"
#include "../../network/netclient.h"
#include "../../input/mouse.h"
#include "../../input/keyboard.h"


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

		addChatMessage("TA3D NetClient");
		addChatMessage("start a line with '/' to send commands to the server");
		addChatMessage("----");
		addChatMessage("");

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
		while(NetClient::instance()->messageWaiting() && !pArea->get_state("popup")
			  && NetClient::instance()->getState() == NetClient::CONNECTED)
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
			else if (StartsWith(msg, "MSG"))
			{
				String::Vector args;
				msg.split(args, " ");
				if (args.size() >= 2)
				{
					String message = args[1] + " > " + msg.substrUTF8(5 + args[1].sizeUTF8());
					addChatMessage(message);
				}
			}
			else if (StartsWith(msg, "CLIENT"))
			{
				addChatMessage(msg);
			}
		}

		pArea->set_entry("netmenu.peer_list", NetClient::instance()->getPeerList());
		pArea->set_entry("netmenu.chan_list", NetClient::instance()->getChanList());

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

		if (pArea->get_state("ask") && pArea->get_state("ask.t_result"))    // Validate on enter
		{
			pArea->msg("ask.hide");
			pArea->set_state("ask.b_ok", true);
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
							pArea->msg("ask.t_result.focus");
							pArea->msg("ask.show");
						}
						else if (password.empty())
						{
							askMode = PASSWORD;
							pArea->set_title("ask", I18N::Translate("Password"));
							pArea->set_caption("ask.t_result", "");
							pArea->msg("ask.t_result.focus");
							pArea->msg("ask.show");
						}
						else if (NetClient::instance()->getState() != NetClient::CONNECTED)
						{

							NetClient::instance()->connect(lp_CONFIG->net_server, 4240, login, password);
							if (NetClient::instance()->getState() == NetClient::CONNECTED)
								addChatMessage("you are logged as " + NetClient::instance()->getLogin());
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
							pArea->msg("ask.t_result.focus");
							pArea->msg("ask.show");
						}
						else if (password.empty())
						{
							askMode = PASSWORD;
							pArea->set_title("ask", I18N::Translate("New account - password"));
							pArea->set_caption("ask.t_result", "");
							pArea->msg("ask.t_result.focus");
							pArea->msg("ask.show");
						}
						else if (NetClient::instance()->getState() != NetClient::CONNECTED)
						{
							NetClient::instance()->connect(lp_CONFIG->net_server, 4240, login, password, true);
							if (NetClient::instance()->getState() == NetClient::CONNECTED)
							{
								addChatMessage("you successfully registered as " + NetClient::instance()->getLogin());
								addChatMessage("you are logged as " + NetClient::instance()->getLogin());
							}
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
				if (pArea->get_object("netmenu.chan_list") && !pArea->get_state("netmenu.chan_list"))
				{
					GUIOBJ *obj = pArea->get_object("netmenu.chan_list");
					for(int i = 0 ; i < obj->Text.size() ; i++)
					{
						if (obj->Text[i] == NetClient::instance()->getChan())
						{
							obj->Pos = i;
							break;
						}
					}
				}
				if (pArea->get_state("netmenu.b_register"))     // register
				{
					netMode = REGISTER;
					askMode = NONE;
				}
				else if (pArea->get_state("netmenu.b_login"))   // login
				{
					netMode = LOGIN;
					askMode = NONE;
				}
				else if (pArea->get_state("netmenu.b_logout"))  // logout
				{
					NetClient::instance()->destroyInstance();
				}
				else if (pArea->get_state("netmenu.chan_list")) // change chan
				{
					GUIOBJ *obj = pArea->get_object("netmenu.chan_list");
					if (obj)
					{
						int chanID = obj->Pos;
						if (chanID >= 0 && chanID < obj->Text.size())
							NetClient::instance()->changeChan(obj->Text[chanID]);
					}
				}
				else if (pArea->get_state("netmenu.input"))     // send a message
				{
					String msg = pArea->get_caption("netmenu.input");
					pArea->set_caption("netmenu.input", "");
					if (!msg.empty())
					{
						if (msg[0] == '/')      // Command mode (IRC like :p)
						{
							if (StartsWith(msg, "/CHAN"))      // Change chan (this is done using NetClient interface)
							{
								if (msg.size() > 6)
									NetClient::instance()->changeChan(msg.substr(6));
								else
									NetClient::instance()->changeChan("*");
							}
							else
							{
								NetClient::instance()->sendMessage(msg.substr(1));       // send the command to server
								addChatMessage(msg.substr(1));
							}
						}
						else                    // Chat mode
						{
							NetClient::instance()->sendChan(msg);       // send the message to all users of this chan except us
							addChatMessage(NetClient::instance()->getLogin() + " > " + msg);    // so we have to add it manually
						}
					}
				}
				break;
		};

		// Exit
		if (key[KEY_ESC] || pArea->get_state("netmenu.b_back"))
			return true;

		return false;
	}

	void NetMenu::addChatMessage(const String &message)
	{
		GUIOBJ *obj = pArea->get_object("netmenu.chat_history");
		if (obj)
		{
			obj->Text.push_back(message);
			if (obj->Text.size() > 25)
				obj->Data++;
			obj->Pos = obj->Text.size() - 1;
		}
	}
}
}
