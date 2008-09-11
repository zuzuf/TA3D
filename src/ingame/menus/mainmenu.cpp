
#include "mainmenu.h"
#include "../../gfx/gfx.h"
#include "../../logs/logs.h"
#include "../../ta3dbase.h"
#include "solo.h"
#include "../../logs/logs.h"
#include "../../misc/settings.h"


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
        LOG_DEBUG(LOG_PREFIX_MENU_MAIN << "Entering...");

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
        while(!doLoop())
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
        LOG_DEBUG(LOG_PREFIX_MENU_MAIN << "Done.");
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
        if (key[KEY_SPACE] || key[KEY_O] || pArea->get_state("main.b_options"))
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
        lp_CONFIG->quickstart = false;
        do
        {
            lp_CONFIG->quickrestart = false;
            glPushMatrix();
            config_menu();
            lp_CONFIG->quickstart = false;
            glPopMatrix();

            if (lp_CONFIG->quickrestart)
            {
                changeVideoSettings();
                resetScreen();
                lp_CONFIG->quickstart = true;
            }
        } while (lp_CONFIG->quickrestart);

        loadAreaFromTDF("main", "gui/main.area");
        resetScreen();
        return false;
    }

    void MainMenu::changeVideoSettings()
    {
        pArea.reset(NULL);          // Destroy current GUI area
        cursor.clear();             // Destroy cursor data (it's OpenGL textures so they won't survive)

        delete gfx;                 // Delete current GFX object
        gfx = new GFX;              // Create a new one with new settings
        gfx->Init();                // Initialize GFX object

        gfx->set_2D_mode();         // Back to 2D mode :)

		set_window_title("Total Annihilation 3D");  // Set the window title

		// Reloading and creating cursors
		byte *data = HPIManager->PullFromHPI("anims\\cursors.gaf");	// Load cursors
		cursor.loadGAFFromRawData(data, true);
		cursor.convert();

		CURSOR_MOVE        = cursor.findByName("cursormove"); // Match cursor variables with cursor anims
		CURSOR_GREEN       = cursor.findByName("cursorgrn");
		CURSOR_CROSS       = cursor.findByName("cursorselect");
		CURSOR_RED         = cursor.findByName("cursorred");
		CURSOR_LOAD        = cursor.findByName("cursorload");
		CURSOR_UNLOAD      = cursor.findByName("cursorunload");
		CURSOR_GUARD       = cursor.findByName("cursordefend");
		CURSOR_PATROL      = cursor.findByName("cursorpatrol");
		CURSOR_REPAIR      = cursor.findByName("cursorrepair");
		CURSOR_ATTACK      = cursor.findByName("cursorattack");
		CURSOR_BLUE        = cursor.findByName("cursornormal");
		CURSOR_AIR_LOAD    = cursor.findByName("cursorpickup");
		CURSOR_BOMB_ATTACK = cursor.findByName("cursorairstrike");
		CURSOR_BALANCE     = cursor.findByName("cursorunload");
		CURSOR_RECLAIM     = cursor.findByName("cursorreclamate");
		CURSOR_WAIT        = cursor.findByName("cursorhourglass");
		CURSOR_CANT_ATTACK = cursor.findByName("cursortoofar");
		CURSOR_CROSS_LINK  = cursor.findByName("pathicon");
		CURSOR_CAPTURE     = cursor.findByName("cursorcapture");
		CURSOR_REVIVE      = cursor.findByName("cursorrevive");
		if (CURSOR_REVIVE == -1) // If you don't have the required cursors, then resurrection won't work
			CURSOR_REVIVE = cursor.findByName("cursorreclamate");

		delete[] data;
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

