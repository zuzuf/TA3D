
#include "mainmenu.h"
#include "../../gfx/gfx.h"
#include "../../logs/logs.h"
#include "../../ta3dbase.h"

// TODO Must be removed
#include "../../menu.h"


// TODO Must be removed
void ReadFileParameter();


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
        gfx->SetDefState();
        gfx->set_2D_mode();
        gfx->ReInitTexSys();

        // To have mouse sensibility undependent from the resolution
        gfx->SCREEN_W_TO_640 = 1.0f; 
        gfx->SCREEN_H_TO_480 = 1.0f;

        // Loading the area
        pMainArea.reset(new AREA("main"));
        pMainArea->load_tdf("gui/main.area");
        if (!pMainArea->background)
            pMainArea->background = gfx->glfond;

        // Changing the cursor
        cursor_type = CURSOR_DEFAULT;
        // Current mod
        getInfosAboutTheCurrentMod();
        // If there is a file parameter, read it
        ReadFileParameter(); 
        // Misc
        pDontWaitForEvent = true;
        pMouseX = -1;
        pMouseY = -1;
        pMouseZ = -1;
        pMouseB = -1;

        return true;
    }



    bool MainMenu::doExecute()
    {
        bool done = false;
        do
        {
            waitForEvent();

            pMouseX = mouse_x;
            pMouseY = mouse_y;
            pMouseZ = mouse_z;
            pMouseB = mouse_b;

            // Reset the caption
            pMainArea->set_caption("main.t_version", TA3D_ENGINE_VERSION );
            pMainArea->set_caption("main.t_mod", pCurrentModCaption);


            // If ESC, directly stop
            done = (key[KEY_ESC] || maySwitchToAnotherMenu());

            // Redrw the screen
            redrawTheScreen();

        } while(!done && !lp_CONFIG->quickrestart);

        return true;
    }


    void MainMenu::doFinalize()
    {
        if (pMainArea->background == gfx->glfond)
            pMainArea->background = 0;
        gfx->set_2D_mode();
    }

    void MainMenu::getInfosAboutTheCurrentMod()
    {
        pCurrentMod = TA3D_CURRENT_MOD.length() > 6 
            ? TA3D_CURRENT_MOD.substr(5, TA3D_CURRENT_MOD.length() - 6)
            : "";
        if (!pCurrentMod.empty())
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
        cursor_type = CURSOR_DEFAULT;	
    }


    bool MainMenu::maySwitchToAnotherMenu()
    {
        // Exit
        if (key[KEY_ESC] || pMainArea->get_state( "main.b_exit"))
            return true;

        // Options
        if (key[KEY_SPACE] || pMainArea->get_state("main.b_options") || lp_CONFIG->quickstart)
            return goToMenuOptions();

        // Solo
        if( key[KEY_ENTER] || pMainArea->get_state( "main.b_solo"))
            return goToMenuSolo();

        // Multi player room
        if(key[KEY_B] || pMainArea->get_state("main.b_multi"))
            return goToMenuMultiPlayers();

        return false;
    }


    bool MainMenu::goToMenuOptions()
    {
        glPushMatrix();
        config_menu();
        lp_CONFIG->quickstart = false;
        glPopMatrix();
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
        solo_menu();
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
            pMainArea->check();
            // Wait to reduce CPU consumption 
            rest(30);

        } while (!pDontWaitForEvent
                 && pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
                 && mouse_b == 0
                 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_B]
                 && !keyIsPressed && !pMainArea->scrolling);

        // Should wait the an event the next time
        pDontWaitForEvent = false;
    }

    void MainMenu::redrawTheScreen()
    {
        pMainArea->draw();
        glEnable(GL_TEXTURE_2D);
        gfx->set_color(0xFFFFFFFF);
        draw_cursor();
        gfx->flip();
    }



} // namespace Menus
} // namespace TA3D

