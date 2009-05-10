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
#ifndef __TA3D_GFX_GUI_SKIN_H__
# define __TA3D_GFX_GUI_SKIN_H__

# include "../../stdafx.h"
# include "../../misc/string.h"
# include "skin.object.h"
# include "text.color.h"


namespace TA3D
{

    /*! \class SKIN
    **
    ** \brief manage skins for the GUI
    */
    class SKIN
    {															// Only one object of this class should be created
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default Constructor
        SKIN() : Name(), prefix() { init(); }
        //! Destructor
        ~SKIN();
        //@}

        /*!
        ** \brief
        */
        void init();

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief Load a skin from a TDF file
        **
        ** \param filename The filename to load
        ** \param scale The scale value
        */
        void load_tdf(const String& filename, const float scale = 1.0f);


        //! Following functions are dedicated to widgets rendering
    public:
        void button(float x, float y, float x2, float y2, const String &Title, bool State);
        void FloatMenu(float x, float y, const String::Vector &Entry, int Index, int StartEntry=0);
        void ListBox (float x1, float y1, float x2, float y2, const String::Vector &Entry, int Index, int Scroll , uint32 flags = 0 );
        void OptionButton(float x, float y, const String &Title, bool Etat);
        void OptionCase(float x, float y, const String &Title, bool Etat);
        void TextBar(float x1, float y1, float x2, float y2, const String &Caption, bool State);
        void TextEditor(float x1, float y1, float x2, float y2, const String::Vector &Entry, int row, int col, bool State);
        void ProgressBar(float x1, float y1, float x2, float y2, int Value);
        void PopupMenu(float x1, float y1, const String &msg);
        void ScrollBar(float x1, float y1, float x2, float y2, float Value, bool vertical=true);
        int draw_text_adjust(float x1, float y1, float x2, float y2, String msg, int pos = 0, bool mission_mode = false);
        void ObjectShadow( float x1, float y1, float x2, float y2, float dx, float dy, float alpha, float fuzzy);

    public:
        //! Default background for windows
        GLuint  wnd_background;
        //! Default background width
        uint32  bkg_w;
        //! Default background height
        uint32  bkg_h;
        //! Default background for buttons
        SKIN_OBJECT button_img[2];
        //! Color parameters for buttons
        TEXT_COLOR  button_color;

        //! Default color parameters
        TEXT_COLOR  text_color;

        //! Borders of the windows
        SKIN_OBJECT	wnd_border;
        //! Progress bar images, one for the background, another one for the bar itself
        SKIN_OBJECT progress_bar[2];
        //! default title bar for windows
        SKIN_OBJECT  wnd_title_bar;
        //! The name of the skin (will be used to change skin)
        String Name;
        //! Background for TEXTBAR, LISTBOX, ... everything that uses a background for text
        SKIN_OBJECT text_background;
        //! The background image for floating menus
        SKIN_OBJECT menu_background;
        //! The selection image (drawn when an element is selected)
        SKIN_OBJECT selection_gfx;
        //! Checkbox images
        SKIN_OBJECT checkbox[2];
        //! Open button images
        SKIN_OBJECT option[2];
        //! Scroll bar
        SKIN_OBJECT scroll[3];
        //! Prefix for various files
        String prefix;

        //! offset to display text at the right place
        float text_y_offset;

    }; // class SKIN



} // namespace TA3D

#endif // __TA3D_GFX_GUI_SKIN_H__
