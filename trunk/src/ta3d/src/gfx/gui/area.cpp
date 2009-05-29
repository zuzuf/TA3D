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

#include "area.h"
#include "../../misc/paths.h"
#include "skin.h"
#include "skin.manager.h"
#include "../../console.h"
#include "../../TA3D_NameSpace.h"
#include "../../gui.h"
#include "../../misc/math.h"
#include "../../console.h"
#include "../../misc/tdf.h"
#include "../../input/keyboard.h"
#include "../../input/mouse.h"



namespace TA3D
{


	std::list<AREA*> AREA::area_stack;		// This list stores the stack of all AREA objects so you can grab the current one at any time



	WND::Ptr AREA::getWindowWL(const String& message)
	{
		String lmsg (message);
		lmsg.toLower();
		if (lmsg == cached_key && cached_wnd)
			return cached_wnd;
		int e = wnd_hashtable.find(lmsg) - 1;
		if (e >= 0)
		{
			cached_key = lmsg;
			cached_wnd = pWindowList[e];
			return cached_wnd;
		}
		return WND::Ptr();
	}



	WND::Ptr AREA::get_wnd(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return getWindowWL(message);
	}


	void AREA::set_enable_flag(const String& message, const bool enable)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
		{
			if (enable)
				guiobj->Flag &= ~FLAG_DISABLED;
			else
				guiobj->Flag |= FLAG_DISABLED;
		}
	}


	void AREA::set_state(const String& message, const bool state)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
			guiobj->Etat = state;
	}

	void AREA::set_value(const String& message, const sint32 value)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
			guiobj->Value = value;
	}


	void AREA::set_data(const String& message, const sint32 data)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
			guiobj->Data = data;
	}


	void AREA::caption(const String& message, const String& caption)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj && !guiobj->Text.empty())
		{
			if (guiobj->Type == OBJ_TEXTEDITOR)
				caption.explode(guiobj->Text, '\n');
			else
				guiobj->caption(caption);
		}
		else
		{
			LOG_WARNING("AREA caption : " << message << " not found");
		}
	}


	void AREA::set_action(const String& message, void (*Func)(int))
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
			guiobj->Func = Func;
	}


	void AREA::set_entry(const String& message, const std::list<String>& entry)	// Set the entry of specified object in the specified window to entry (converts List to Vector)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
		{
			guiobj->Text.clear();
			for (std::list<String>::const_iterator i = entry.begin(); i != entry.end(); ++i)
				guiobj->Text.push_back(*i);
		}
	}


	void AREA::set_entry(const String& message, const std::vector<String>& entry)	// Set the entry of specified object in the specified window to entry
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr guiobj = getObjectWL(message);
		if (guiobj)
			guiobj->Text = entry;
	}


	void AREA::title(const String& message, const String& title)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		WND::Ptr wnd = getWindowWL(message);
		if (wnd)
			wnd->Title = title;
	}


	uint16 AREA::check()
	{
		poll_inputs();
		key_pressed = keypressed();
		bool scroll = ((msec_timer - scroll_timer) >= 250);
		if (scroll)
		{
			while (msec_timer - scroll_timer >= 250)
				scroll_timer += 250;
		}

		unsigned int is_on_gui = 0;

		{
			ThreadingPolicy::MutexLocker locker(*this);

			for (unsigned int i = 0; i < pWindowList.size(); ++i)
			{
				if (!is_on_gui || (pWindowList[vec_z_order[i]]->get_focus && !pWindowList[vec_z_order[i]]->hidden))
				{
					is_on_gui |= pWindowList[vec_z_order[i]]->check(amx, amy, amz, amb, scroll, skin);
					if (((is_on_gui && mouse_b && !pWindowList[vec_z_order[0]]->get_focus)
						 || pWindowList[ vec_z_order[i]]->get_focus) && i > 0 && !pWindowList[ vec_z_order[i]]->background_wnd)
					{
						uint16 old = vec_z_order[i];
						for (unsigned int e = i; e > 0; --e)
							vec_z_order[e] = vec_z_order[e - 1];
						vec_z_order[0] = old; // Get the focus
					}
				}
			}

			scrolling = scroll;
			amx = mouse_x;
			amy = mouse_y;
			amz = mouse_z;
			amb = mouse_b;
		}
		if (!console.activated())
			clear_keybuf();

		return is_on_gui;
	}



	uint16 AREA::load_window(const String& filename)
	{
		ThreadingPolicy::MutexLocker locker(*this);

		WND::Ptr newWindow = new WND();
		uint16 wnd_idx = pWindowList.size();

		pWindowList.push_back(newWindow);				// Adds a window to the vector
		vec_z_order.push_back(wnd_idx);

		if (Paths::ExtractFileExt(filename) == ".gui")
			newWindow->load_gui(filename, gui_hashtable); // Loads the window from a *.gui file
		else
		{
			newWindow->load_tdf(filename, skin);	// Loads the window from a *.tdf file
		}

		for (unsigned int i = wnd_idx; i > 0; --i) // The new window appear on top of the others
			vec_z_order[i] = vec_z_order[i - 1];

		vec_z_order[0] = wnd_idx;
		wnd_hashtable.insert(String::ToLower(newWindow->Name), wnd_idx + 1);	// + 1 because it returns 0 on Find failure
		return wnd_idx;
	}


	void AREA::draw()
	{
		ThreadingPolicy::MutexLocker locker(*this);

		if (background)
		{
			gfx->drawtexture(background, 0, 0, gfx->width, gfx->height);
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		// Draws all the windows in focus reversed order so the focused window is drawn on top of the others
		String help_msg;
		if (!pWindowList.empty())
		{
			unsigned int i = pWindowList.size();
			do
			{
				--i;
				pWindowList[vec_z_order[i]]->draw(help_msg, (i == 0), true, skin);
			} while (i);
		}
		if (!help_msg.empty())
			skin->PopupMenu(mouse_x + 20, mouse_y + 20, help_msg);
	}


	void AREA::load_tdf(const String& filename)
	{
		doLoadTDF(filename);
	}


	void AREA::doLoadTDF(const String& filename)
	{
		destroy();		// In case there is an area loaded so we don't waste memory

		String skin_name = (lp_CONFIG != NULL && !lp_CONFIG->skin_name.empty()) ? lp_CONFIG->skin_name : "";
		if (!skin_name.empty() && HPIManager->Exists(skin_name))
			skin = skin_manager.load(skin_name, 1.0f);

		String real_filename = filename;
		if (skin && !skin->prefix().empty())
		{
            real_filename.clear();
            real_filename << Paths::ExtractFilePath(filename) << skin->prefix() << Paths::ExtractFileName(filename);
            if (!HPIManager->Exists(real_filename))	// If it doesn't exist revert to the default name
				real_filename = filename;
		}
        skin = NULL;

		TDFParser areaFile(real_filename);

		area_stack.push_front(this);     // Just in case we want to grab it from elsewhere

        name = Paths::ExtractFileNameWithoutExtension(filename);		// Grab the area's name

		name = areaFile.pullAsString("area.name", name);					// The TDF may override the area name
		skin_name = (lp_CONFIG != NULL && !lp_CONFIG->skin_name.empty())
			? lp_CONFIG->skin_name
			: areaFile.pullAsString("area.skin");

		if (HPIManager->Exists(skin_name)) // Loads a skin
		{
			const int area_width   = areaFile.pullAsInt("area.width", SCREEN_W);
			const int area_height  = areaFile.pullAsInt("area.height", SCREEN_W);
			const float skin_scale = Math::Min((float)SCREEN_H / area_height, (float)SCREEN_W / area_width);
			skin = skin_manager.load(skin_name, skin_scale);
		}

		String::Vector windows_to_load;
		areaFile.pullAsString("area.windows").explode(windows_to_load, ',');
		for (String::Vector::const_iterator i = windows_to_load.begin(); i != windows_to_load.end(); ++i)
			load_window(*i);

		String background_name = areaFile.pullAsString("area.background", "none");
        if (background_name.toLower() != "none")           // If we have a background set then load it
		{
			if(skin && !skin->prefix().empty())
			{
				int name_len = Paths::ExtractFileName(background_name).size();
				if (name_len > 0)
					background_name = background_name.substr(0, background_name.size() - name_len) << skin->prefix() << Paths::ExtractFileName(background_name);
				else
					background_name += skin->prefix();
			}

			if (HPIManager->Exists(background_name)) // Loads a background image
				background = gfx->load_texture(background_name);
			else
			{
				if (skin && !skin->prefix().empty())
				{
					// No prefixed version, retry with default background
					background_name = areaFile.pullAsString("area.background");
					// Loads a background image
					if (HPIManager->Exists(background_name))
						background = gfx->load_texture(background_name);
				}
			}
		}
		else
			background = 0;
	}



	AREA::AREA(const String& area_name)
		:scrolling(false), background(0), name(area_name), skin(NULL), gui_hashtable(), wnd_hashtable()
	{
		amx = mouse_x;
		amy = mouse_y;
		amz = mouse_z;
		amb = mouse_b;

		scroll_timer = msec_timer;

		InitInterface();		// Initialization of the interface
	}


	void AREA::destroy()
	{
		if (current() == this)  area_stack.pop_front();     // Just in case we want to destroy an empty object
		cached_key.clear();
		cached_wnd = NULL;

		gui_hashtable.emptyHashTable();
		gui_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);

		wnd_hashtable.emptyHashTable();
		wnd_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);

		name.clear();

		pWindowList.clear();			// Empty the window vector
		vec_z_order.clear();		// No more windows at end

		gfx->destroy_texture(background);		// Destroy the texture (using safe destroyer)

		skin = NULL;
	}


	AREA *AREA::current()
	{
		return (!area_stack.empty()) ? area_stack.front() : NULL;
	}


	AREA::~AREA()
	{
		if (current() == this)  area_stack.pop_front();     // Just in case we want to destroy an empty object
		DeleteInterface();			// Shut down the interface

		cached_key.clear();
		cached_wnd = NULL;
		gui_hashtable.emptyHashTable();
		wnd_hashtable.emptyHashTable();
		name.clear();

		pWindowList.clear();			// Empty the window vector
		vec_z_order.clear();		// No more windows at end

		if (background == gfx->glfond)      // Don't remove the background texture
			background = 0;
		else
			gfx->destroy_texture(background); // Destroy the texture
	}


	uint32 AREA::InterfaceMsg(const lpcImsg msg)
	{
		if (msg->MsgID != TA3D_IM_GUI_MSG) // Only GUI messages
			return INTERFACE_RESULT_CONTINUE;

		if (msg->lpParm1 == NULL)
		{
			LOG_ERROR("AREA : bad format for interface message!");
			return INTERFACE_RESULT_HANDLED;		// Oups badly written things
		}

		if (this != current())              // It's not for us
			return INTERFACE_RESULT_CONTINUE;

		return this->msg((char*) msg->lpParm1);
	}


	int	AREA::msg(String message)				// Send that message to the area
	{
		ThreadingPolicy::MutexLocker locker(*this);

		uint32 result = INTERFACE_RESULT_CONTINUE;
		message.toLower(); // Get the string associated with the signal

		String::size_type i = message.find('.');
		if (i != String::npos)
		{
			String key = message.substr(0, i); // Extracts the key
			message = message.substr(i + 1, message.size() - i - 1); // Extracts the end of the message

			WND::Ptr the_wnd = getWindowWL(key);
			if (the_wnd)
				result = the_wnd->msg(message);
		}
		else
		{
			if (message == "clear")
			{
				pWindowList.clear();
				vec_z_order.clear();
				wnd_hashtable.emptyHashTable();
				wnd_hashtable.initTable(__DEFAULT_HASH_TABLE_SIZE);
			}
			else
				if (message == "end_the_game")
				{
					// TODO Code here ?
					LOG_ERROR(" area.cpp: message received `end_the_game not handled`");
				}
		}
		return result;				// Ok we're done with it
	}


	bool AREA::get_state(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);

		String::size_type i = message.find('.');
		if (i != String::npos)
		{
			String key = message.substr(0, i); // Extracts the key
			String obj_name = message.substr(i + 1, message.size() - i - 1);

			if (key == "*")
			{
				const WindowList::iterator end = pWindowList.end();
				for (WindowList::iterator e = pWindowList.begin(); e != end; ++e)
				{
					GUIOBJ::Ptr the_obj = (*e)->get_object(obj_name);
					if (the_obj)
						return the_obj->Etat;
				}
			}
			else
			{
				WND::Ptr the_wnd = getWindowWL(key);
				if (the_wnd)
					return the_wnd->get_state(obj_name);
			}
		}
		else
		{
			WND::Ptr the_wnd = getWindowWL(message);
			if (the_wnd)
				return the_wnd->get_state(String());
		}
		return false;
	}


	bool AREA::is_activated(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		GUIOBJ::Ptr obj = getObjectWL(message);
		return (obj) ? obj->activated : false;
	}


	bool AREA::is_mouse_over(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);

		GUIOBJ::Ptr obj = getObjectWL(message);
		return (!obj) ? false : obj->MouseOn;
	}


	sint32 AREA::get_value(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);

		String::size_type i = message.find('.');
		if (i != String::npos)
		{
			String key = message.substr(0, i);
			String obj_name = message.substr(i + 1, message.size() - i - 1);
			if (key == "*")
			{
				const WindowList::iterator end = pWindowList.end();
				for (WindowList::iterator e = pWindowList.begin(); e != end; ++e)
				{
					GUIOBJ::Ptr the_obj = (*e)->get_object(obj_name);
					if (the_obj)
						return the_obj->Value;
				}
			}
			else
			{
				WND::Ptr the_wnd = getWindowWL(key);
				if (the_wnd)
					return the_wnd->get_value(obj_name);
			}
		}
		return -1;
	}



	String AREA::caption(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);

		String::size_type i = message.find('.');
		if (String::npos != i)
		{
			String key = message.substr(0, i);						// Extracts the key
			String obj_name = message.substr(i + 1, message.size() - i - 1);
			if (!key.empty() && key == "*")
			{
				const WindowList::iterator end = pWindowList.end();
				for (WindowList::iterator e = pWindowList.begin(); e != end; ++e)
				{
					GUIOBJ::Ptr the_obj = (*e)->get_object(obj_name);
					if (the_obj)
					{
						if (the_obj->Text.size() > 0)
						{
							if (the_obj->Type == OBJ_TEXTEDITOR)
							{
								String result = the_obj->Text[0];
								for (unsigned int i = 1; i < the_obj->Text.size(); ++i)
									result << '\n' << the_obj->Text[i];
								return result;
							}
							return the_obj->Text[0];	// Return what we found
						}
						return String();
					}
				}
			}
			else
			{
				WND::Ptr the_wnd = getWindowWL(key);
				if (the_wnd)
					return the_wnd->caption(obj_name);
			}
		}
		return String();
	}



	GUIOBJ::Ptr AREA::getObjectWL(const String& message)
	{
		String::size_type i = message.find('.');
		if (i != String::npos)
		{
			String key = message.substr(0, i);						// Extracts the key
			String obj_name = message.substr(i + 1, message.size() - i -1);
			if (key == "*")
			{
				const WindowList::iterator end = pWindowList.end();
				for (WindowList::iterator e = pWindowList.begin(); e != end; ++e)
				{
					GUIOBJ::Ptr the_obj = (*e)->get_object(obj_name);
					if (the_obj)
						return the_obj;
				}
			}
			else
			{
				WND::Ptr the_wnd = getWindowWL(key);
				if (the_wnd)
					return the_wnd->get_object(obj_name);
			}
		}
		return NULL;
	}



	GUIOBJ::Ptr AREA::get_object(const String& message)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		return getObjectWL(message);
	}



} // namespace TA3D
