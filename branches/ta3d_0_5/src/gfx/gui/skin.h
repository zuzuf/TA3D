#ifndef __TA3D_GFX_GUI_SKIN_H__
# define __TA3D_GFX_GUI_SKIN_H__

# include "../../stdafx.h"
# include "skin.object.h"


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


    public:
        //! Default background for windows
        GLuint  wnd_background;
        //! Default background for buttons
        SKIN_OBJECT  button_img[2];

        //! Borders of the windows
        SKIN_OBJECT	 wnd_border;
        //! Progress bar images, one for the background, another one for the bar itself
        SKIN_OBJECT  progress_bar[2];
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
