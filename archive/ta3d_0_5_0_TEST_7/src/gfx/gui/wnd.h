#ifndef __TA3D_GFX_GUI_WND_H__
# define __TA3D_GFX_GUI_WND_H__

# include "../../stdafx.h"
# include "../../misc/string.h"
# include "../../threads/thread.h"
# include "skin.h"
# include "object.h"
# include "../../misc/hash_table.h"
# include "../../misc/interface.h"
# include "../texture.h"



namespace TA3D
{

    class GUIOBJ;

    /*! \class WND
    **
    ** \brief Window Object
    */
    class WND : public ObjectSync
    {
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
        ~WND();
        //@}

        void destroy();																	// Every life has an end...

        /*!
        ** \brief Draw the window
        **
        ** \param[out] helpMsg
        ** \param focus
        */
        void draw(String& helpMsg, const bool focus = true, const bool deg = true, SKIN* skin = NULL);

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
        byte WinMov(const int AMx, const int AMy, const int AMb, const int Mx, const int My, const int Mb, SKIN* skin = NULL);						// Handle window's moves

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
        int check(int AMx, int AMy, int AMz, int AMb, bool timetoscroll = true, SKIN* skin = NULL);


        /*!
        ** \brief Load a window from a *.TDF file describing the window
        **
        ** \param filename
        ** \param skin
        */
        void load_tdf(const String& filename, SKIN* skin = NULL);

        /*!
        ** \brief Load a window from a TA *.GUI file describing the interface
        **
        ** \param filename
        ** \param gui_hashtable
        */
        void load_gui(const String& filename, TA3D::UTILS::cHashTable< std::vector< TA3D::Interfaces::GfxTexture >* > & gui_hashtable);



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
        String  get_caption(const String& message);

        /*!
        ** \brief pointer to the specified object
        ** \param message
        ** \return
        */
        GUIOBJ* get_object(String message);


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

        //! Title
        String  Title;
        //! Name of the window
        String  Name;

        //! Objects within the window
        GUIOBJ* Objets;
        //! Number of objects
        int  NbObj;
        //! hashtable used to speed up operations on GUIOBJ objects
        TA3D::UTILS::cHashTable<int>  obj_hashtable;

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
        //! Format used by the window
        int	 u_format;
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
        ** \brief Draw the background of the window
        ** \see draw()
        */
        void doDrawWindowBackground(SKIN* skin);
        /*!
        ** \brief Draw the skin of the windows
        ** \see draw()
        */
        void doDrawWindowSkin(SKIN* skin, const bool focus, const bool deg);
        /*!
        ** \brief Draw a background object
        */
        void doDrawWindowBackgroundObject(String& helpMsg, const int i, const bool focus, SKIN* skin);
        /*!
        ** \brief Draw a foreground object
        */
        void doDrawWindowForegroundObject(SKIN* skin, const int i);
        
        
        /*!
        ** \brief
        ** \param wasOnFloattingMenu
        ** \param indxMenu
        */
        void doCheckWasOnFLoattingMenu(const int i, bool& wasOnFloattingMenu, int& indxMenu, SKIN* skin);
    
        /*!
        ** \brief Same as get_object
        ** \see get_object()
        */
        GUIOBJ* doGetObject(String message);

    private:
        //!
        bool delete_gltex;
        //!
        float size_factor;

    }; // class WND



} // namespace TA3D

#endif // __TA3D_GFX_GUI_WND_H__
