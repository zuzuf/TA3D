#ifndef __TA3D_INGAME_MENUS_INTRO_H__
# define __TA3D_INGAME_MENUS_INTRO_H__

# include "base.h"


# define TA3D_MENUS_INTRO_DEFAULT_INTRO_FILENAME  "intro.txt"


namespace TA3D
{
namespace Menus
{


    /*! \class Intro
    **
    ** \brief The Intro menu 
    */
    class Intro : public Abstract
    {
    public:
        /*!
        ** \brief Execute an instance of Menus::Intro
        */
        static bool Execute();

    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
        Intro();
        //! Destructor
        virtual ~Intro();
        //@}

        
        /*!
        ** \brief reload the content of the scrolling text
        */
        void reloadContent();

    protected:
        virtual bool doInitialize();
        virtual void doFinalize();
        virtual void waitForEvent();
        virtual bool maySwitchToAnotherMenu();
        virtual void redrawTheScreen();

    private:
        /*!
        ** \brief Reload the texture of the background
        */
        void loadBackgroundTexture();

        /*!
        ** \brief Scroll the content
        */
        void scrollTheText();

    private:
        //! The scrolling text
        String::Vector pContent;
        //! Cached size of the content
        unsigned int pContentSize;
        
        uint32 pScrollTimer;

        //! The texture of the background
        GLuint pBackgroundTexture;
        //! The previous font size
        float pPreviousFontSize;
        //! Current font height
        float pCurrentFontHeight;

        //! Delta position to display the text
        float pDelta;
        //! The first index in the list where to start from
        unsigned int pStartIndex;

    }; // class Intro


} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_INTRO_H__
