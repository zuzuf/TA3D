#ifndef __TA3D_INGAME_MENUS_X_MAIN_MENU_H__
# define __TA3D_INGAME_MENUS_X_MAIN_MENU_H__

# include "base.h"


namespace TA3D
{
namespace Menus
{

    /*! \class MainMenu
    **
    ** \brief The main menu right after the short introduction sequence
    */
    class MainMenu : public Abstract
    {
    public:
        /*!
        ** \brief Execute an instance of MainMenu
        */
        static bool Execute();

    public:
        //! \name Constructor & Destructor
        //{
        //! Default constructor
        MainMenu();
        //! Destructor
        virtual ~MainMenu();
        //}

    protected:
        virtual bool doInitialize();
        virtual bool doExecute();
        virtual void doFinalize();
        virtual void waitForEvent();
        virtual bool maySwitchToAnotherMenu();


    private:
        /*!
        ** \brief Reset the screen and OpenGL settings
        */
        void resetScreen();

        /*!
        ** \brief Redraw the entire screen for the main menu
        */
        virtual void redrawTheScreen();

        /*!
        ** \brief Grab informations about the current mod
        */
        void getInfosAboutTheCurrentMod();

        /*!
        ** \brief Go to the option menu
        ** \return Always equals to false
        */
        bool goToMenuOptions();

        /*!
        ** \brief Go to the multiplayer menu
        ** \return Always equals to false
        */
        bool goToMenuMultiPlayers();

        /*! 
        ** \brief Go to the solo menu
        ** \return Always equals to false
        */
        bool goToMenuSolo();

    private:
        //! Current mod
        String pCurrentMod;
        //! Caption for the current mod (Cache)
        String pCurrentModCaption;

        /*!
        ** Get if we should not wait for an event (mouse,keyboard...) if enabled
        ** \see waitForEvent()
        */
        bool pDontWaitForEvent;

    }; // class MainMenu



} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_X_MAIN_MENU_H__
