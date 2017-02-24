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
#include <languages/i18n.h>
#include <network/netclient.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <misc/paths.h>
#include <TA3D_NameSpace.h>
#include <mods/mods.h>
#include <sounds/manager.h>
#include <cache.h>
#include <ingame/sidedata.h>
#include "setupgame.h"
#include <QProcess>

namespace TA3D
{
namespace Menus
{
    int NetMenu::Download::wndNumber = 0;

    NetMenu::Download::~Download()
    {
        stop();
    }

	void NetMenu::Download::start(const QString &filename, const QString &url, const int mID)
    {
        if (Gui::AREA::current())
        {
            int idx = Gui::AREA::current()->load_window("gui/progress.tdf", QString("dl%1").arg(wndNumber++));
			wnd = Gui::AREA::current()->get_window_name(idx);
            Gui::AREA::current()->title(wnd, url);
            Gui::AREA::current()->set_data(wnd + ".progress", 0);
            Gui::AREA::current()->msg(wnd + ".show");
			this->lastProgress = 0;
		}
		this->modID = mID;
        this->filename = filename;
        http.get(filename, url);
    }

    bool NetMenu::Download::downloading()
    {
        return http.isDownloading();
    }

    void NetMenu::Download::stop()
    {
        if (Gui::AREA::current())
            Gui::AREA::current()->msg(wnd + ".hide");
        http.stop();
    }

    void NetMenu::Download::update()
    {
        if (Gui::AREA::current())
        {
            if (http.isDownloading())
			{
				int p = (int)http.getProgress();
				if (p != lastProgress)		// Don't update too often, it forces refreshing GUI
				{
                    Gui::AREA::current()->set_data(wnd + ".progress", p);
					this->lastProgress = p;
				}
			}
            else
                Gui::AREA::current()->msg(wnd + ".hide");
        }
    }

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
        for(Download::List::iterator i = downloadList.begin() ; i != downloadList.end() ; ++i)
			delete *i;
        downloadList.clear();
    }


	void NetMenu::doFinalize()
	{
		// Wait for user to release ESC
		while (key[KEY_ESC])
		{
			rest(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
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

		NetClient::instance()->connect(lp_CONFIG->net_server, 4240, QString(), QString());
		Mods::instance()->update();

		return true;
	}



	void NetMenu::waitForEvent()
	{
		do
		{
			// Grab user events
			pArea->check();
			// Wait to reduce CPU consumption
			wait();
			// Listen to server
			NetClient::instance()->receive();

		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
			&& mouse_b == 0
			&& !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
			&& !pArea->key_pressed && !pArea->scrolling
			&& !NetClient::instance()->messageWaiting());
	}

	void NetMenu::parseServerMessages()
	{
		while(NetClient::instance()->messageWaiting() && !pArea->get_state("popup")
			&& NetClient::instance()->getState() == NetClient::CONNECTED)
		{
			QString msg = NetClient::instance()->getNextMessage();
			if (msg.startsWith("MESSAGE"))
			{
				pArea->title("popup", I18N::Translate("Server message"));
				pArea->caption("popup.msg", I18N::Translate(Substr(msg,8)));
				pArea->msg("popup.show");
			}
			else if (msg.startsWith("ERROR"))
			{
				pArea->title("popup", I18N::Translate("Server error"));
				pArea->caption("popup.msg", I18N::Translate(Substr(msg,6)));
				pArea->msg("popup.show");
			}
			else if (msg.startsWith("MSG"))
			{
                const QStringList &args = SplitCommand(msg);
				if (args.size() >= 3)
				{
                    const QString &message = args[1] + " > " + args[2];
					addChatMessage(message);
				}
			}
			else if (msg.startsWith("CLIENT"))
			{
				addChatMessage(msg);
			}
		}
	}


	void NetMenu::updateGUI()
	{
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

		if (pArea->get_state("mods"))
		{
			Gui::GUIOBJ::Ptr modListObj = pArea->get_object("mods.l_mods");
			if (modListObj)
			{
				ModInfo::List modList = Mods::instance()->getModList(Mods::MOD_ALL);
				modListObj->Text.clear();
				for(ModInfo::List::iterator i = modList.begin() ; i != modList.end() ; ++i)
					modListObj->Text.push_back(i->getName());
				int idx = modListObj->Pos;
				if (idx >= 0)
				{
					ModInfo::List::iterator pIdx = modList.begin();
					for( ; pIdx != modList.end() && idx > 0 ; ++pIdx)
						--idx;
					if (pIdx != modList.end())
					{
						pArea->caption("mods.m_name", pIdx->getName());
						pArea->caption("mods.m_version", pIdx->getVersion());
						pArea->caption("mods.m_author", pIdx->getAuthor());
						pArea->caption("mods.m_comment", pIdx->getComment());
                        const QString &dir = pIdx->getPathToMod();
                        const QString &filename = dir + '/' + Paths::ExtractFileName(pIdx->getUrl());

						if (pIdx->isInstalled())
						{
                            if (TA3D_CURRENT_MOD != ("mods/" + pIdx->getName() + '/') && (pIdx->getID() != -1 || !TA3D_CURRENT_MOD.isEmpty()))
								pArea->msg("mods.b_load.enable");
							else
								pArea->msg("mods.b_load.disable");
							pArea->msg("mods.b_install.hide");
							pArea->msg("mods.b_remove.show");
							if (pIdx->isUpdateAvailable())
								pArea->msg("mods.b_update.enable");
							else
								pArea->msg("mods.b_update.disable");
						}
						else
						{
							pArea->msg("mods.b_load.disable");
							pArea->msg("mods.b_install.show");
							pArea->msg("mods.b_remove.hide");
							pArea->msg("mods.b_update.disable");
						}

						if (pArea->get_state("mods.b_install"))     // Start download
						{
							pArea->set_state("mods.b_install", false);
							Paths::MakeDir(dir);
							Download *download = new Download;
							download->start(filename, pIdx->getUrl(), pIdx->getID());
							downloadList.push_back(download);
							pIdx->write();
						}
						if (pArea->get_state("mods.b_remove"))     // Remove mod
						{
							pArea->set_state("mods.b_remove", false);
							pIdx->uninstall();
							NetClient::instance()->sendMessage("GET MOD LIST");
						}
						if (pArea->get_state("mods.b_update"))     // Remove old files and start download
						{
							pArea->set_state("mods.b_update", false);
							pIdx->uninstall();

							Download *download = new Download;
							download->start(filename, pIdx->getUrl(), pIdx->getID());
							downloadList.push_back(download);
						}
						if (pArea->get_state("mods.b_load") && pIdx->isInstalled())		// Load this mod
							changeMod(pIdx->getID());
					}
					else
						idx = -1;
				}
				if (idx < 0)
				{
					pArea->caption("mods.m_name", nullptr);
					pArea->caption("mods.m_version", nullptr);
					pArea->caption("mods.m_author", nullptr);
					pArea->caption("mods.m_comment", nullptr);
					pArea->msg("mods.b_install.hide");
					pArea->msg("mods.b_remove.hide");
					pArea->msg("mods.b_update.hide");
				}
			}
			if (pArea->get_state("mods.b_refresh"))     // Refresh mod list
				NetClient::instance()->sendMessage("GET MOD LIST");
		}		// End of if (pArea->get_state("mods"))
		if (NetClient::instance()->getState() == NetClient::CONNECTED)
		{
			if (pArea->get_state("netgames"))
			{
				Gui::GUIOBJ::Ptr serverListObj = pArea->get_object("netgames.l_games");
				if (serverListObj)
				{
					NetClient::GameServer::List serverList = NetClient::instance()->getServerList();
					serverListObj->Text.clear();
					for(NetClient::GameServer::List::iterator i = serverList.begin() ; i != serverList.end() ; ++i)
						serverListObj->Text.push_back(i->first);
					int idx = serverListObj->Pos;
					if (idx >= 0 && idx < int(serverListObj->Text.size()))
					{
						NetClient::GameServer::List::iterator pIdx = serverList.find(serverListObj->Text[idx]);
						if (pIdx != serverList.end())
						{
							pArea->caption("netgames.server_name", pIdx->second.name);
							pArea->caption("netgames.server_version", pIdx->second.version);
							pArea->caption("netgames.server_map", pIdx->second.map);
							pArea->caption("netgames.server_mod", pIdx->second.mod);
							pArea->caption("netgames.server_open_slots", pIdx->second.nb_open);
						}
						else
							idx = -1;
					}
					else
						idx = -1;
					if (idx < 0)
					{
						pArea->caption("netgames.server_name", nullptr);
						pArea->caption("netgames.server_version", nullptr);
						pArea->caption("netgames.server_map", nullptr);
						pArea->caption("netgames.server_mod", nullptr);
						pArea->caption("netgames.server_open_slots", nullptr);
					}
					else if (pArea->get_state("netgames.b_join"))
                        NetClient::instance()->sendMessage("JOIN \"" + Escape(serverListObj->Text[idx]) + "\"");
				}
				if (pArea->get_state("netgames.b_refresh"))     // Refresh server list
					NetClient::instance()->sendMessage("GET SERVER LIST");
			}		// Enf of if (pArea->get_state("netgames"))
			if (pArea->get_state("hosting.b_ok"))
			{
                if (TA3D_CURRENT_MOD.isEmpty())
                    NetClient::instance()->sendMessage("SERVER NAME \"" + Escape(pArea->caption("hosting.t_hostname")) + "\" MOD \"\"");
				else
                    NetClient::instance()->sendMessage("SERVER NAME \"" + Escape(pArea->caption("hosting.t_hostname")) + "\" MOD \"" + Escape(Substr(TA3D_CURRENT_MOD, 5, TA3D_CURRENT_MOD.size() - 6)) + "\"");
				pArea->set_state("hosting.b_ok", false);
			}
			else if (NetClient::instance()->getHostAck())		// NetServer is ready, let's go!
				hostAGame();
            if (!NetClient::instance()->getServerJoined().isEmpty())		// We're free to call the server :)
				joinAGame();
		}
		else
			pArea->msg("netgames.hide");		// Hide game server list when not in connected mode
		for(Download::List::iterator i = downloadList.begin() ; i != downloadList.end() ; )
			if ((*i)->downloading())
			{
				(*i)->update();
				++i;
			}
			else
			{
				QString filename = (*i)->getFilename();
				if (Paths::Exists(filename))        // Success, check if this is an RAR, TAR, 7Z, ZIP archive
				{
                    const QString ext = Paths::ExtractFileExt(filename).toLower();
					LOG_INFO(LOG_PREFIX_SYSTEM << "archive extension is '" << ext << "'");
					bool success = true;
					if (ext == ".7z" || ext == ".rar" || ext == ".zip" || ext == ".tar"
						|| ext == ".gz" || ext == ".bz2" || ext == ".tar.gz" || ext == ".tar.bz2")
					{
                        const QString &command = lp_CONFIG->system7zCommand + " x \"" + filename + "\" -o\"" + Paths::ExtractFilePath(filename) + '"';
						LOG_INFO(LOG_PREFIX_SYSTEM << "running command : '" << command << "'");
                        if (QProcess::execute(command))
						{
							LOG_ERROR(LOG_PREFIX_SYSTEM << "error running command!");
							success = false;
						}
					}
					if (success)
					{
						ModInfo *pMod = Mods::instance()->getMod((*i)->getModID());
						if (pMod)
						{
							pMod->setInstalled(true);
							LOG_INFO(LOG_PREFIX_RESOURCES << "mod installed");
						}
						else
							LOG_ERROR(LOG_PREFIX_RESOURCES << "mod ID error");
					}
					else
						LOG_INFO(LOG_PREFIX_RESOURCES << "mod was not installed");
				}
				else        // Download has failed, remove the mod folder
					LOG_ERROR(LOG_PREFIX_RESOURCES << "mod could not be installed");
				delete *i;
				downloadList.erase(i++);
			}
	}

	bool NetMenu::maySwitchToAnotherMenu()
	{
		// First we have to parse the server messages
		parseServerMessages();

		// Then update GUI info and buttons state
		updateGUI();

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
                        if (login.isEmpty())
						{
							askMode = LOGIN;
							pArea->title("ask", I18N::Translate("Login"));
							pArea->caption("ask.t_result", "");
							pArea->msg("ask.t_result.focus");
							pArea->msg("ask.show");
						}
                        else if (password.isEmpty())
						{
							askMode = PASSWORD;
							pArea->title("ask", I18N::Translate("Password"));
							pArea->caption("ask.t_result", "");
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
							login = pArea->caption("ask.t_result");
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
							password = pArea->caption("ask.t_result");
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
					case LOGGED:
					case REGISTER:
						LOG_ERROR(LOG_PREFIX_MENU_NETMENU << "we should not get here!");
				};
				break;
			case REGISTER:
				switch(askMode)
				{
					case NONE:
                        if (login.isEmpty())
						{
							askMode = LOGIN;
							pArea->title("ask", I18N::Translate("New account - login"));
							pArea->caption("ask.t_result", "");
							pArea->msg("ask.t_result.focus");
							pArea->msg("ask.show");
						}
                        else if (password.isEmpty())
						{
							askMode = PASSWORD;
							pArea->title("ask", I18N::Translate("New account - password"));
							pArea->caption("ask.t_result", "");
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
							login = pArea->caption("ask.t_result");
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
							password = pArea->caption("ask.t_result");
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
					case LOGGED:
					case REGISTER:
						LOG_ERROR(LOG_PREFIX_MENU_NETMENU << "we should not get here!");
				};
				break;
			case NONE:
			default:
				login.clear();
				password.clear();
				if (pArea->get_object("netmenu.chan_list") && !pArea->get_state("netmenu.chan_list"))
				{
					Gui::GUIOBJ::Ptr obj = pArea->get_object("netmenu.chan_list");
					for (unsigned int i = 0 ; i < obj->Text.size(); i++)
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
					Gui::GUIOBJ::Ptr obj = pArea->get_object("netmenu.chan_list");
					if (obj)
					{
						uint32 chanID = obj->Pos;
						if (chanID < obj->Text.size())
							NetClient::instance()->changeChan(obj->Text[chanID]);
					}
				}
				else if (pArea->get_state("netmenu.input"))     // send a message
				{
					QString msg = pArea->caption("netmenu.input");
					pArea->caption("netmenu.input", "");
                    if (!msg.isEmpty())
					{
                        if (msg.startsWith('/'))      // Command mode (IRC like :p)
						{
							if (msg.startsWith("/CHAN"))      // Change chan (this is done using NetClient interface)
							{
								if (msg.size() > 6)
									NetClient::instance()->changeChan(Substr(msg,6));
								else
									NetClient::instance()->changeChan("*");
							}
							else
							{
								NetClient::instance()->sendMessage(Substr(msg,1));       // send the command to server
								addChatMessage(Substr(msg,1));
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



	void NetMenu::addChatMessage(const QString &message)
	{
		Gui::GUIOBJ::Ptr obj = pArea->get_object("netmenu.chat_history");
		if (obj)
		{
			pArea->append("netmenu.chat_history", message);
			if (obj->Text.size() > 25)
				++(obj->Data);
			obj->Pos = uint32(obj->Text.size() - 1);
		}
	}


	void NetMenu::hostAGame()
	{
		QString host = pArea->caption( "hosting.t_hostname");
		QString cfgPlayerName = lp_CONFIG->player_name;
		lp_CONFIG->player_name = NetClient::instance()->getLogin();

		SetupGame::Execute(false, host, QString(), true);

		lp_CONFIG->player_name = cfgPlayerName;
		NetClient::instance()->sendMessage("UNSERVER");
		NetClient::instance()->getHostAck();
	}

	void NetMenu::joinAGame()
	{
		QString host = NetClient::instance()->getServerJoined();
		QString cfgPlayerName = lp_CONFIG->player_name;
		lp_CONFIG->player_name = NetClient::instance()->getLogin();

		SetupGame::Execute(true, host, QString(), true);

		lp_CONFIG->player_name = cfgPlayerName;
        NetClient::instance()->sendMessage("UNJOIN \"" + Escape(host) + "\"");
		NetClient::instance()->clearServerJoined();
	}

	void NetMenu::changeMod(const int ID)
	{
		ModInfo *inf = Mods::instance()->getMod(ID);
        QString newMod = (ID < 0 || inf == NULL) ? QString() : ("mods/" + ModInfo::cleanStringForPortablePathName(inf->getName()) + "/");
		if (TA3D_CURRENT_MOD != newMod) // Refresh the file structure
		{
			TA3D_CURRENT_MOD = lp_CONFIG->last_MOD = newMod;
			Cache::Clear(true); // Force cache reset

			VFS::Instance()->reload();
			ta3dSideData.loadData();                // Refresh side data so we load the correct values
			sound_manager = NULL;		// Proper cleaning
			sound_manager = new TA3D::Audio::Manager();
			sound_manager->loadTDFSounds(true);
			sound_manager->loadTDFSounds(false);
			pArea->title("popup", I18N::Translate("Success"));
			pArea->caption("popup.msg", I18N::Translate("Mod loaded"));
			pArea->msg("popup.show");
		}
	}

} // namespace Menus
} // namespace TA3D
