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
#ifndef __TA3D_GFX_GUI_OBJECT_H__
# define __TA3D_GFX_GUI_OBJECT_H__

# include <stdafx.h>
# include <vector>
# include <zuzuf/smartptr.h>
# include <misc/string.h>
# include "base.h"
# include <gfx/texture.h>
# include "skin.h"



namespace TA3D
{
namespace Gui
{



	class WND;


	/*! \class GUIOBJ
	**
	** \brief Objects within Windows
	*/
    class GUIOBJ : public zuzuf::ref_count
	{
	public:
		//! The most suitable smart pointer
        typedef zuzuf::smartptr<GUIOBJ>  Ptr;

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		GUIOBJ();
		//! Destructor
		~GUIOBJ();
		//@}

		/*!
		** \brief
		*/
		uint32  num_entries() const {return uint32(Text.size());}

		/*!
		** \brief Reacts to a message transfered from the interface
		*/
		uint32  msg(const QString& message, WND* wnd = NULL);

		/*!
		** \brief
		*/
		void caption(const QString& caption);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		** \param caption
		** \param F
		** \param size
		*/
		void create_button(const float X1, const float Y1, const float X2, const float Y2,
			const QString& caption, void (*F)(int), const float size = 1.0f);

		/*!
		** \brief Option Checkbox
		**
		** \param X1
		** \param Y1
		** \param caption
		** \param ETAT
		** \param F
		** \param skin
		** \param size
		*/
		void create_optionc(const float X1, const float Y1, const QString& caption, const bool ETAT,
							void (*F)(int), Skin *skin = NULL, const float size = 1.0f);

		/*!
		** \brief Option button
		**
		** \param X1
		** \param Y1
		*/
		void create_optionb(const float X1, const float Y1, const QString& Caption, const bool ETAT, void (*F)(int),
							Skin* skin = NULL, const float size = 1.0f);

		/*!
		** \brief Create a Text edit
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		*/
		void create_textbar(const float X1, const float Y1, const float X2, const float Y2, const QString& Caption,
							const unsigned int MaxChar, void(*F)(int) = NULL, const float size = 1.0f);

		/*!
		** \brief Create a TEXTEDITOR widget, it's a large text editor
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		** \param caption
		** \param size
		*/
		void create_texteditor(const float X1, const float Y1, const float X2, const float Y2,
							   const QString& caption, const float size = 1.0f);

		/*!
		** \brief Create a floatting menu
		**
		** \param X1
		** \param Y1
		** \param Entry
		** \param F
		** \param size
		*/
        void create_menu(const float X1, const float Y1, const QStringList& Entry, void (*F)(int),
						 const float size=1.0f);

		/*!
		** \brief Create a Popup menu
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		** \param Entry
		** \param F
		** \param size
		*/
        void create_menu(const float X1, const float Y1, const float X2, const float Y2, const QStringList& Entry,
						 void (*F)(int), const float size = 1.0f);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		*/
		void create_pbar(const float X1, const float Y1, const float X2, const float Y2,
						 const int PCent, const float size = 1.0f);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		*/
		void create_text(const float X1, const float Y1,const QString& Caption, const int Col = Black, const float size = 1.0f);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		*/
		void create_line(const float X1, const float Y1, const float X2, const float Y2, const int Col = Black);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		*/
		void create_box(const float X1, const float Y1, const float X2, const float Y2, const int Col = Black);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		*/

        void create_img(const float X1, const float Y1, const float X2, const float Y2, const GfxTexture::Ptr &img);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		*/
        void create_list(const float X1, const float Y1, const float X2, const float Y2, const QStringList& Entry, const float size = 1.0f);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param Caption
		** \param states
		** \param nb_st
		*/
        void create_ta_button(const float X1, const float Y1,const QStringList& Caption,
                              const std::vector<GfxTexture::Ptr>& states, const int nb_st);

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		** \param min
		** \param max
		** \param value
		*/
		void create_hslider(const float X1, const float Y1, const float X2, const float Y2,
							const int vmin, const int vmax, const int value);
		void create_vslider(const float X1, const float Y1, const float X2, const float Y2,
							const int vmin, const int vmax, const int value);

		void print(std::ostream& out);

	public:
		//! List of textures
        typedef std::vector< TA3D::GfxTexture::Ptr >   TexturesVector;

	public:
		//!
		byte Type;			// Type of objet
		//!
		bool Focus;			// Selected??
		//!
		//!
		bool Etat;			// State of the object
		//!
		float x1;
		//!
		float y1;			// Position(within the window)
		//!
		float x2;
		//!
		float y2;
		//!
        QStringList Text;			// Text displayed by the object
		//!
		void (*Func)(int);	// Pointer to linked function
		//! TODO Must be renammed
        uint32 Data;            	// Additional data
        GfxTexture::Ptr TextureData;	// Additional texture data
        //!
		uint32 Pos;			// Position in a list
		//!
		sint32 Value;			// Used by floatting menus
		//!
		float s;				// Size factor (for text)
		//!
		uint32 Flag;			// Flags
		//!
		bool  MouseOn;		// If the cursor is on it
		//!
		bool  activated;		// For buttons/menus/... indicates that it is pressed (while click isn't finished)
		//!
		bool  destroy_img;	// For img control, tell to destroy the texture

		//!
        QStringList OnClick;		// Send that signal when clicked
		//!
        QStringList OnHover;		// Send that signal when mouse is over
		//!
        QStringList SendDataTo;		// Send Data to that object on the window
		//!
        QStringList SendPosTo;		// Send Pos to that object on the window

		//!
		QString  Name;			// name of the object
		//!
		QString  help_msg;		// Help message displayed when the mouse cursor is over the object

		//!
		float  u1;
		//!
		float  v1;
		//!
		float  u2;
		//!
		float  v2;
		bool  wait_a_turn;	// Used to deal with show/hide msg

		//!
		byte  current_state;
		//!
		TexturesVector gltex_states;
		//!
		byte  nb_stages;
		//!
		sint16  shortcut_key;

	}; // class GUIOBJ




} // namespace Gui
} // namespace TA3D

# include "wnd.h"

#endif // __TA3D_GFX_GUI_OBJECT_H__
