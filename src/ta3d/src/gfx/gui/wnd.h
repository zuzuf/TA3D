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
#ifndef __TA3D_GFX_GUI_WND_H__
# define __TA3D_GFX_GUI_WND_H__

# include <stdafx.h>
# include <misc/string.h>
# include <threads/thread.h>
# include "skin.h"
# include "object.h"
# include <misc/hash_table.h>
# include <misc/interface.h>
# include <gfx/texture.h>
# include <zuzuf/smartptr.h>



namespace TA3D
{
namespace Gui
{


	class GUIOBJ;

	/*! \class WND
	**
	** \brief Window Object
	*/
	class WND : public ObjectSync
	{
	public:
		//! The most suitable smart pointer for a class `WND`
        typedef zuzuf::smartptr<WND> Ptr;

	public:
		//! \name Constructors & Destructor
		//@{
		//! Default constructor
		WND();
		/*!
		** \brief Constructor
		** \param filename
		*/
		WND(const String& filename);
		//! Destructor
		virtual ~WND();
		//@}

		void destroy();

		/*!
		** \brief Draw the window
		**
		** \param[out] helpMsg
		** \param focus
		*/
		void draw(String& helpMsg, const bool focus = true, const bool deg = true, Skin* skin = NULL);

		/*!
		** \brief Handles Window's moves
		**
		** \param AMx
		** \param AMy
		** \param AMb
		** \param Mx
		** \param My
		** \param Mb
		** \param skin
		** \return
		*/
		byte WinMov(const int AMx, const int AMy, const int AMb, const int Mx, const int My, const int Mb, Skin* skin = NULL);						// Handle window's moves

		/*!
		** \brief Handle window's events
		**
		** \param AMx
		** \param AMy
		** \param AMz
		** \param AMb
		** \param timetoscroll
		** \param skin
		** \return
		*/
		int check(int AMx, int AMy, int AMz, int AMb, bool timetoscroll = true, Skin* skin = NULL);


		/*!
		** \brief Load a window from a *.TDF file describing the window
		**
		** \param filename
		** \param skin
		*/
		void load_tdf(const String& filename, Skin* skin = NULL);

		/*!
		** \brief Load a window from a TA *.GUI file describing the interface
		**
		** \param filename
		** \param gui_hashtable
		*/
		void load_gui(const String& filename, TA3D::UTILS::HashMap< std::vector< TA3D::Interfaces::GfxTexture >* >::Dense & gui_hashtable);



		/*!
		** \brief Respond to Interface message
		**
		** \param message
		** \return
		*/
		uint32  msg(const String& message);

		/*!
		** \brief State of specified object
		**
		** \param message
		** \return
		*/
		bool  get_state(const String& message);

		/*!
		** \brief Value of specified object
		** \param message
		** \return
		*/
		sint32  get_value(const String& message);

		/*!
		** \brief caption of specified object
		** \param message
		** \return
		*/
		String  caption(const String& message);

		/*!
		** \brief pointer to the specified object
		** \param message
		** \return
		*/
		GUIOBJ::Ptr get_object(const String &message);


		unsigned int count();
		unsigned int size();

		GUIOBJ::Ptr object(unsigned int indx);

		/*!
		** \brief Set the focus for all objects
		*/
		void focus(bool value);


	public:
		//! X-coordinates
		int	x;
		//! Y-coordinates
		int y;
		//! Size
		int width;
		//! Size
		int height;
		//! Title height as it is displayed
		int	title_h;
		//! Scrolling offset
		int scrolling;
		//! Scrollable
		bool scrollable;
		//! background size when clamped
		int background_width;
		int background_height;
		bool background_clamp;

		//! Title
		String  Title;
		//! Name of the window
		String  Name;

		//! hashtable used to speed up operations on GUIOBJ objects
		TA3D::UTILS::HashMap<int>::Dense  obj_hashtable;

		//! The texture background
		GLuint  background;
		//! Repeat or scale background
		bool  repeat_bkg;
		//!
		uint32  bkg_w;
		//!
		uint32  bkg_h;

		//! Moveable window ?
		bool Lock;
		//! Draw the title ?
		bool show_title;
		//! Draw borders ?
		bool draw_borders;
		//! Is the window visible ?
		bool hidden;
		//! In order to do some cleaning
		bool was_hidden;
		//! In order not to change focus too fast
		bool tab_was_pressed;
		//! Background color of the window (can use alpha channel)
		uint32  color;
		//! Background window -> stay in background
		bool  background_wnd;
		//! Has this window priority over the others ?
		bool  get_focus;


	private:
		/*!
		** \brief Draw the shadow of the window if it has one
		** \see draw()
		*/
		void doDrawWindowShadow(Skin* skin);
		/*!
		** \brief Draw the background of the window
		** \see draw()
		*/
		void doDrawWindowBackground(Skin* skin);
		/*!
		** \brief Draw the skin of the windows
		** \see draw()
		*/
		void doDrawWindowSkin(Skin* skin, const bool focus, const bool deg);
		/*!
		** \brief Draw a background object
		*/
		void doDrawWindowBackgroundObject(String& helpMsg, const int i, const bool focus, Skin* skin);
		/*!
		** \brief Draw a foreground object
		*/
		void doDrawWindowForegroundObject(Skin* skin, const int i);


		/*!
		** \brief
		** \param wasOnFloattingMenu
		** \param indxMenu
		*/
		void doCheckWasOnFLoattingMenu(const int i, bool& wasOnFloattingMenu, int& indxMenu, Skin* skin);

		/*!
		** \brief Same as get_object
		** \see get_object()
		*/
		GUIOBJ::Ptr doGetObject(const String &message);

		void print(std::ostream& out);

	private:
		typedef std::vector<GUIOBJ::Ptr> ObjectList;
		//!
		bool delete_gltex;
		//!
		float size_factor;
		//!
		bool ingame_window;

		//! Objects within the window
		ObjectList pObjects;

		//! Cache : Font height
		float pCacheFontHeight;

	}; // class WND





} // namespace Gui
} // namespace TA3D

#endif // __TA3D_GFX_GUI_WND_H__
