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
#ifndef __TA3D_GFX_GUI_AREA_H__
# define __TA3D_GFX_GUI_AREA_H__

# include <stdafx.h>
# include <misc/string.h>
# include "wnd.h"
# include <vector>
# include <misc/interface.h>
# include <zuzuf/smartptr.h>




namespace TA3D
{
namespace Gui
{

	/*!
	** \brief This class is a window handler, so it will manage windows
	** and signals given to them
	*/
    class AREA:	protected IInterface, public virtual zuzuf::ref_count
	{
	public:
		//! The best suitable smart pointer for an area
        typedef zuzuf::smartptr<AREA>  Ptr;

	public:
		//! \name Constructor && Destructor
		//@{
		/*!
		** \brief Constructor
		** \param area_name Name of the area
		*/
		AREA(const QString& nm = "unnamed_area");
		//! Destructor
		virtual ~AREA();
		//@}

		/*!
		** \brief Reset the object
		*/
		void destroy();

		/*!
		** \brief Returns a pointer to current AREA object if one exists, otherwise returns NULL
		*/
		static AREA *current();

		/*!
		** \brief Load a window from a TDF file
		**
		** \param filename TDF File to load
        ** \param name is not empty, overrides the window name (allow having multiple copies of the same window)
        ** \return
		*/
        uint16 load_window(const QString& filename, const QString &name = QString());

        /*!
        ** \brief Returns the name of the idxth window
        **
        ** \param idx window number
        ** \return
        */
        QString get_window_name(const int idx);

		/*!
		** \brief Check the user interface, manage events
		*/
		uint16 check();

		/*!
		** \brief Draw all windows
		*/
		void draw();

		/*!
		** \brief Load a TDF file, telling which windows to load and which skin to use
		**
		** \param filename TDF File to load
		*/
		void load_tdf(const QString& filename);

		/*!
		** \brief Return the specified window
		**
		** This method is thread-safe
		**
		** \param message
		*/
		WND::Ptr get_wnd(const QString& message);

		/*!
		** \brief Return the state of specified object in the specified window
		**
		** \param message
		** \return
		*/
		bool get_state(const QString& message);

		/*!
		** \brief Return true if the given object has its activated flag set (mouse is on and button pressed)
		**
		** \param message
		** \return
		*/
		bool is_activated(const QString& message);

		/*!
		** \brief Return true if the cursor is over the given object
		**
		** \param message
		** \return
		*/
		bool is_mouse_over(const QString& message);

		/*!
		** \brief Return the value of specified object in the specified window
		**
		** \param message
		** \return
		*/
		sint32 get_value(const QString& message);

		/*!
		** \brief Return a pointer to the specified object
		**
		** \param message
		** \return
		*/
		GUIOBJ::Ptr get_object(const QString& message);

		/*!
		** \brief Set the state of specified object in the specified window
		**
		** \param message
		** \return
		*/
		void set_state(const QString& message, const bool state);

		/*!
		** \brief Set the value of specified object in the specified window
		**
		** \param message
		** \return
		*/
		void set_value(const QString& message, const sint32 value);

		/*!
		** \brief Set the data of specified object in the specified window
		**
		** \param message
		** \return
		*/
		void set_data(const QString& message, const sint32 data);

		/*!
		** \brief Set the enabled/disabled state of specified object in the specified window
		**
		** \param message
		** \return
		*/
		void set_enable_flag(const QString& message, const bool enable);


		/*!
		** \brief Return the caption of specified object in the specified window
		**
		** \param message
		** \return
		*/
		QString caption(const QString& message);

		/*!
		** \brief Set the caption of specified object in the specified window
		**
		** \param message
		** \return
		*/
		void caption(const QString& message, const QString& caption);

		/*!
		** \brief Set the title of specified window to title
		**
		** \param message
		** \return
		*/
		void title(const QString& message, const QString& title);

		/*!
		** \brief Set the entry of specified object in the specified window to entry
		**
		** \param message
		** \return
		*/
        void set_entry(const QString& message, const QStringList &entry);

		/*!
		** \brief Append line to the entry of specified object in the specified window
		**
		** \param message
		** \return
		*/
		void append(const QString& message, const QString& line);

		/*!
		** \brief Set the function pointer of an object, it will be called when the widget is clicked
		**
		** \param message
		** \return
		*/
		void set_action(const QString& message, void (*Func)(int));

		/*!
		** \brief Send that message to the area
		**
		** \param message
		** \return
		*/
		int	msg(QString message);

		/*!
		** \brief Display a popup window, locking all other GUI elements until it's closed
		** \param Title
		** \param Msg
		** \return
		*/
		void popup(const QString &Title, const QString &Msg);

		/*!
		** \brief Returns a pointer to current skin
		*/
		inline const Skin *getSkin() const	{	return skin;	}

	public:
		//!
		bool scrolling;
		//! Of course we need a background, not a single color :-)
        GfxTexture::Ptr background;
		//!
		bool key_pressed;

	private:
		/*!
		** \brief Manage signals sent through the interface to GUI
		**
		** \param msg
		** \return
		*/
		virtual uint32 InterfaceMsg(const uint32 MsgID, const QString &msg);

		/*!
		** \brief Same as get_object, but not thread-safe
		** \see get_object()
		*/
		GUIOBJ::Ptr getObjectWL(const QString& message);

		/*!
		** \brief Same as get_wnd, but not thread-safe
		** \see get_wnd()
		*/
		WND::Ptr getWindowWL(const QString& message);

		/*!
		** \brief Same as load_tdf(), but not thread-safe
		*/
		void doLoadTDF(const QString& filename);

	private:
		//! This list stores the stack of all AREA objects so you can grab the current one at any time
		static std::deque<AREA*> area_stack;
		//! Window list
		typedef std::vector<WND::Ptr> WindowList;

		//! This vector stores all the windows the area object deals with
		WindowList  pWindowList;
		//! This vector stores data about the z order of windows
		std::vector<uint16> vec_z_order;

		//! Name of the ares
		QString name;

		//! The last cursor x-position
		int amx;
		//! The last cursor y-position
		int amy;
		//! The last cursor z-position
		int amz;
		//! The last cursor button status
		int amb;

		//! The skin used by the ares
		Skin* skin;

		//! hashtable used to speed up loading of *.gui files and save memory
        TA3D::UTILS::HashMap< std::vector< TA3D::GfxTexture::Ptr >* >::Dense gui_hashtable;
		//! hashtable used to speed up operations on WND objects
		TA3D::UTILS::HashMap<int>::Dense  wnd_hashtable;

		//!
		QString  cached_key;
		//!
		WND::Ptr cached_wnd;

		//!
		uint32  scroll_timer;

		//! Message to display as popup hint
		QString pCacheHelpMsg;

	}; // class AREA





} // namespace Gui
} // namespace TA3D

#endif // __TA3D_GFX_GUI_AREA_H__
