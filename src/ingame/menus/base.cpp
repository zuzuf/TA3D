
#include "base.h"
#include <typeinfo>
#include "../../TA3D_Exception.h"
#include "../../ta3dbase.h"


using namespace TA3D::Exceptions;

namespace TA3D
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
        GuardEnter(pTypeName + ".doInitialize()");
        pMouseX = -1;
        pMouseY = -1;
        pMouseZ = -1;
        pMouseB = -1;
        cursor_type = CURSOR_DEFAULT;
        reset_keyboard();
        reset_mouse();
        clear_keybuf();
        bool r = doInitialize();
        GuardLeave();
        return r;
    }

    bool Abstract::doGuardExecute()
    {
        GuardEnter(pTypeName + ".doExecute()");
        bool r = doExecute();
        GuardLeave();
        return r;
    }

    void Abstract::doGuardFinalize()
    {
        GuardEnter(pTypeName + ".doFinalize()");
        if (pArea.get() && pArea->background == gfx->glfond)
            pArea->background = 0;
        doFinalize();
        reset_keyboard();
        clear_keybuf();
        reset_mouse();
        GuardLeave();
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
        glEnable(GL_TEXTURE_2D);
        gfx->set_color(0xFFFFFFFF);
        draw_cursor();
        gfx->flip();
    }


    void Abstract::ResetTexture(GLuint& textVar, const GLuint newValue)
    {
        if (textVar)
        {
            // ensure the texture for the mini map is destroyed
            gfx->destroy_texture(textVar);
        }
        textVar = newValue;
    }



} // namespace Menus
} // namespace TA3D
