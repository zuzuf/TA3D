#ifndef __TA3D_GFX_GUI_OBJECT_H__
# define __TA3D_GFX_GUI_OBJECT_H__

# include "base.h"
# include <vector>
# include "wnd.h"
# include "../texture.h"
# include "wnd.h"



namespace TA3D
{

    class WND;


    /*! \class GUIOBJ
    **
    ** \brief Objects within Windows
    */
    class GUIOBJ
    {
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
        uint32  num_entries() const {return Text.size();}

        /*!
        ** \brief Reacts to a message transfered from the interface 
        */
        uint32  msg(const String& message, WND* wnd = NULL);

        /*!
        ** \brief
        */
        void set_caption(const String& caption);


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
                           const String& caption, void (*F)(int), const float size = 1.0f);

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
        void create_optionc(const float X1, const float Y1, const String& caption, const bool ETAT,
                            void (*F)(int), SKIN *skin = NULL, const float size = 1.0f);

        /*!
        ** \brief Option button
        **
        ** \param X1
        ** \param Y1
        */
        void create_optionb(const float X1, const float Y1, const String& Caption, const bool ETAT, void (*F)(int),
                            SKIN* skin = NULL, const float size = 1.0f);

        /*!
        ** \brief Create a Text edit
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        */
        void create_textbar(const float X1, const float Y1, const float X2, const float Y2, const String& Caption,
                            const unsigned int MaxChar, void(*F)(int) = NULL, const float size = 1.0f);

        /*!
        ** \brief Create a TEXTEDITOR widget, it's a large text editor
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        */
        void create_texteditor(const float X1, const float Y1, const float X2, const float Y2, const String& Caption, const float size = 1.0f);

        /*!
        ** \brief Create a floatting menu
        **
        ** \param X1
        ** \param Y1
        ** \param Entry
        ** \param F
        ** \param size
        */
        void create_menu(const float X1, const float Y1, const String::Vector& Entry, void (*F)(int), const float size=1.0f);

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
        void create_menu(const float X1, const float Y1, const float X2, const float Y2, const String::Vector& Entry,
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
        void create_text(const float X1, const float Y1,const String& Caption, const int Col = Noir, const float size = 1.0f);

        /*!
        ** \brief
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        */
        void create_line(const float X1, const float Y1, const float X2, const float Y2, const int Col = Noir);

        /*!
        ** \brief
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        */
        void create_box(const float X1, const float Y1, const float X2, const float Y2, const int Col = Noir);

        /*!
        ** \brief
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        */

        void create_img(const float X1, const float Y1, const float X2, const float Y2, const GLuint img);

        /*!
        ** \brief
        **
        ** \param X1
        ** \param Y1
        ** \param X2
        ** \param Y2
        */
        void create_list(const float X1, const float Y1, const float X2, const float Y2, const String::Vector& Entry, const float size = 1.0f);

        /*!
        ** \brief
        **
        ** \param X1
        ** \param Y1
        ** \param Caption
        ** \param states
        ** \param nb_st
        */
        void create_ta_button(const float X1, const float Y1,const String::Vector& Caption,
                              const std::vector<GLuint>& states, const int nb_st);


    public:
        //! List of textures
        typedef std::vector< TA3D::Interfaces::GfxTexture >   TexturesVector;

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
        String::Vector Text;			// Text displayed by the object
        //!
        void (*Func)(int);	// Pointer to linked function
        //!
        uint32 Data;			// Additional data
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
        String::Vector  OnClick;		// Send that signal when clicked
        //!
        String::Vector  OnHover;		// Send that signal when mouse is over
        //!
        String::Vector  SendDataTo;		// Send Data to that object on the window
        //!
        String::Vector  SendPosTo;		// Send Pos to that object on the window

        //!
        String  Name;			// name of the object
        //!
        String  help_msg;		// Help message displayed when the mouse cursor is over the object

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




} // namespace TA3D

#endif // __TA3D_GFX_GUI_OBJECT_H__
