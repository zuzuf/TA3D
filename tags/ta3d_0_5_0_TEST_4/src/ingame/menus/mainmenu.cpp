
#include "mainmenu.h"
#include "../../gfx/gfx.h"
#include "../../logs/logs.h"
#include "../../ta3dbase.h"
#include "solo.h"
#include "../../logs/logs.h"


// TODO Must be removed
#include "../../menu.h"


// TODO Must be removed
void ReadFileParameter();


# define TA3D_LOG_SECTION_MENU_MAIN_PREFIX "[Main Menu] "


namespace TA3D
{
namespace Menus
{

    bool MainMenu::Execute()
    {
        MainMenu m;
        return m.execute();
    }

    MainMenu::MainMenu()
        :Abstract()
    {}


    MainMenu::~MainMenu()
    {}



    bool MainMenu::doInitialize()
    {
        LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAIN_PREFIX << "Entering...");

        gfx->SetDefState();
        gfx->set_2D_mode();
        gfx->ReInitTexSys();

        // To have mouse sensibility undependent from the resolution
        gfx->SCREEN_W_TO_640 = 1.0f; 
        gfx->SCREEN_H_TO_480 = 1.0f;

        // Loading the area
        loadAreaFromTDF("main", "gui/main.area");

        // Current mod
        getInfosAboutTheCurrentMod();
        // If there is a file parameter, read it
        ReadFileParameter(); 
        // Misc
        pDontWaitForEvent = true;
        return true;
    }



    bool MainMenu::doExecute()
    {
        while(!doLoop() && !lp_CONFIG->quickrestart)
            ;
        return true;
    }

    void MainMenu::redrawTheScreen()
    {
        // Reset the caption
        pArea->set_caption("main.t_version", TA3D_ENGINE_VERSION );
        pArea->set_caption("main.t_mod", pCurrentModCaption);

        Abstract::redrawTheScreen();
    }


    void MainMenu::doFinalize()
    {
        gfx->set_2D_mode();
        LOG_DEBUG(TA3D_LOG_SECTION_MENU_MAIN_PREFIX << "Done.");
    }

    void MainMenu::getInfosAboutTheCurrentMod()
    {
        pCurrentMod = TA3D_CURRENT_MOD.length() > 6 
            ? TA3D_CURRENT_MOD.substr(5, TA3D_CURRENT_MOD.length() - 6)
            : "";
        if (pCurrentMod.empty())
            pCurrentModCaption = "";
        else
            pCurrentModCaption = "MOD: " + pCurrentMod;
    }

    void MainMenu::resetScreen()
    {
        pDontWaitForEvent = true;

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        gfx->set_color(0xFFFFFFFF);

        getInfosAboutTheCurrentMod();

        // To have mouse sensibility undependent from the resolution
        gfx->SCREEN_W_TO_640 = 1.0f; 
        gfx->SCREEN_H_TO_480 = 1.0f;
    }


    bool MainMenu::maySwitchToAnotherMenu()
    {
        // Exit
        if (key[KEY_ESC] || pArea->get_state( "main.b_exit"))
            return true;

        // Options
        if (key[KEY_SPACE] || key[KEY_O] || pArea->get_state("main.b_options") || lp_CONFIG->quickstart)
            return goToMenuOptions();

        // Solo
        if( key[KEY_ENTER] || key[KEY_S] || pArea->get_state( "main.b_solo"))
            return goToMenuSolo();

        // Multi player room
        if(key[KEY_B] || key[KEY_M] || pArea->get_state("main.b_multi"))
            return goToMenuMultiPlayers();

        return false;
    }


    bool MainMenu::goToMenuOptions()
    {
        glPushMatrix();
        config_menu();
        lp_CONFIG->quickstart = false;
        glPopMatrix();
        loadAreaFromTDF("main", "gui/main.area");
        resetScreen();
        return false;
    }

    bool MainMenu::goToMenuMultiPlayers()
    {
        glPushMatrix();
        network_room();
        glPopMatrix();
        resetScreen();
        return false;
    }

    bool MainMenu::goToMenuSolo()
    {

        glPushMatrix();
        Menus::Solo::Execute();
        glPopMatrix();
        resetScreen();
        return false;
    }

    void MainMenu::waitForEvent()
    {
        bool keyIsPressed(false);
        do
        {
            // Get if a key was pressed
            keyIsPressed = keypressed();
            // Grab user events
            pArea->check();
            // Wait to reduce CPU consumption 
            rest(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);

        } while (!pDontWaitForEvent
                 && pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
                 && mouse_b == 0
                 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_B]
                 && !key[KEY_O] && !key[KEY_M] && !key[KEY_S]
                 && !keyIsPressed && !pArea->scrolling);

        // Should wait the an event the next time
        pDontWaitForEvent = false;
    }



} // namespace Menus
} // namespace TA3D

