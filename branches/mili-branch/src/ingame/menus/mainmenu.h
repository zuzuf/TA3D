#ifndef __TA3D_INGAME_MENUS_X_MAIN_MENU_H__
# define __TA3D_INGAME_MENUS_X_MAIN_MENU_H__

# include "base.h"
# include "../../gui.h"


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

    private:
        /*!
        ** \brief Wait for an user event (mouse, keyboard...)
        */
        void waitForEvent();

        /*!
        ** \brief Reset the screen and OpenGL settings
        */
        void resetScreen();

        /*!
        ** \brief Redraw the entire screen for the main menu
        */
        void redrawTheScreen();

        /*!
        ** \brief Grab informations about the current mod
        */
        void getInfosAboutTheCurrentMod();

        /*!
        ** \brief Execute another menu according the user inputs
        ** \return True if the execution should be aborted, false otherwise
        */
        bool maySwitchToAnotherMenu();

        /*!
        ** \brief Go to the option menu
        ** \return Always false
        */
        bool goToMenuOptions();

        /*!
        ** \brief Go to the multiplayer menu
        ** \return Always false
        */
        bool goToMenuMultiPlayers();

        /*! 
        ** \brief Go to the solo menu
        ** \return Always false
        */
        bool goToMenuSolo();

    private:
        //! Current mod
        String pCurrentMod;
        //! Caption for the current mod (Cache)
        String pCurrentModCaption;

        //! Our Window handle
        std::auto_ptr<AREA> pMainArea;
    
        /*!
        ** Get if we should not wait for an event (mouse,keyboard...) if enabled
        ** \see waitForEvent()
        */
        bool pDontWaitForEvent;

        //! Last value of `mouse_x`
        int pMouseX;
        //! last value of `mouse_y`
        int pMouseY;
        //! Last value of `mouse_z`
        int pMouseZ;
        //! Last value of `mouse_b`
        int pMouseB;

    }; // class MainMenu



} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_X_MAIN_MENU_H__
