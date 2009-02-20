
#include "base.h"
#include <typeinfo>
#define TA3D_BASIC_ENGINE
//#include "../ta3dbase.h"



namespace Editor
{
namespace Menus
{

    Abstract::Abstract()
    {
        pTypeName = typeid(*this).name();
    }

    bool Abstract::execute()
    {
        // Executing if initializing succeeded
        bool r = (doGuardInitialize() && doGuardExecute());
        // Finalizing
        doGuardFinalize();
        return (r);
    }


    bool Abstract::doGuardInitialize()
    {
        pMouseX = -1;
        pMouseY = -1;
        pMouseZ = -1;
        pMouseB = -1;
        reset_keyboard();
        reset_mouse();
        clear_keybuf();
        bool r = doInitialize();
        return r;
    }

    bool Abstract::doGuardExecute()
    {
        bool r = doExecute();
        return r;
    }

    void Abstract::doGuardFinalize()
    {
        if (pArea.get() && pArea->background == gfx->glfond)
            pArea->background = 0;
        doFinalize();
        reset_keyboard();
        clear_keybuf();
        reset_mouse();
    }

    void Abstract::loadAreaFromTDF(const String& caption, const String& relFilename)
    {
        pArea.reset(new AREA(caption));
        pArea->load_tdf(relFilename);
        if (!pArea->background)
            pArea->background = gfx->glfond;
    }


    bool Abstract::doLoop()
    {
        // Wait for an event (mouse, keyboard...)
        waitForEvent();

        // Reset the last cached values for the mouse
        pMouseX = mouse_x;
        pMouseY = mouse_y;
        pMouseZ = mouse_z;
        pMouseB = mouse_b;

        // Manage event
        bool done = maySwitchToAnotherMenu();

        // Redraw the screen
        redrawTheScreen();

        return done;
    }

    bool Abstract::doExecute()
    {
        while (!doLoop())
            ;
        return true;
    }

    void Abstract::redrawTheScreen()
    {
        pArea->draw();
        draw_cursor();
        gfx->flip();
    }


} // namespace Menus
} // namespace Editor
