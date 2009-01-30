#include "intro.h"
#include "../../misc/paths.h"
#include <vector>
#include "../../misc/resources.h"
#include "../../misc/files.h"
#include "../../gfx/gui/skin.h"



//!
# define TA3D_INTRO_TOP (630.0f * SCREEN_H / 1024.0f)

# define TA3D_INTRO_BOTTOM (950.0f * SCREEN_H / 1024.0f)

# define TA3D_INTRO_MAX_LINES ((int)((TA3D_INTRO_BOTTOM - TA3D_INTRO_TOP) / pCurrentFontHeight) + 2)

# define TA3D_INTRO_SPEED  60.0f


namespace TA3D
{
namespace Menus
{


    bool Intro::Execute()
    {
        Menus::Intro m;
        return m.execute();
    }


    Intro::Intro()
        :Abstract(), pContentSize(0), pBackgroundTexture(0),
        pCurrentFontHeight(1.0f)
    {}

    Intro::~Intro()
    {}


    bool Intro::doInitialize()
    {
        LOG_ASSERT(NULL != gfx);
        LOG_DEBUG(LOG_PREFIX_MENU_INTRO << "Entering...");
        pCurrentFontHeight = gui_font->height();

        reloadContent();
        loadBackgroundTexture();

        gfx->set_2D_mode();

        pDelta = 0.0f;
        pStartIndex = 0;
        pScrollTimer = msec_timer;

        return true;
    }

    void Intro::doFinalize()
    {
        ResetTexture(pBackgroundTexture);
        LOG_DEBUG(LOG_PREFIX_MENU_INTRO << "Done.");
        pScrollTimer = msec_timer;
    }


    void Intro::waitForEvent()
    {
        // Do nothing
        rest(1);
        poll_mouse();
        poll_keyboard();
    }


    bool Intro::maySwitchToAnotherMenu()
    {
        pDelta += TA3D_INTRO_SPEED * (msec_timer - pScrollTimer) * 0.001f;
        pScrollTimer = msec_timer;
        if (pDelta > pCurrentFontHeight)
        {
            pDelta = pCurrentFontHeight - pDelta;
            ++pStartIndex;
            if (pStartIndex >= pContentSize)
                return true;
        }
        // Press any key to continue... :)
        return (mouse_b || keypressed());
    }


    void Intro::reloadContent()
    {
        pContent.clear();
        String::Vector list;
        // A big space before
        for (int i = 1; i < TA3D_INTRO_MAX_LINES - 1; ++i)
            pContent.push_back("");
        // Load all text files
        if (Resources::Glob(list, "intro" + Paths::SeparatorAsString + "*.txt"))
        {
            for (String::Vector::const_iterator i = list.begin(); i != list.end(); ++i)
                Paths::Files::Load(pContent, *i, 5 * 1024 /* Max 5Ko */, false);
        }
        pContentSize = pContent.size();
        if (pContentSize == TA3D_INTRO_MAX_LINES)
        {
            pContent.push_back("Welcome to TA3D !");
            ++pContentSize;
        }
    }

    void Intro::loadBackgroundTexture()
    {
        LOG_ASSERT(NULL != gfx);

        if (!lp_CONFIG->skin_name.empty() && HPIManager->Exists(lp_CONFIG->skin_name))
        {
            SKIN skin;
            skin.load_tdf(lp_CONFIG->skin_name);

            if (!skin.prefix.empty())
                ResetTexture(pBackgroundTexture, gfx->load_texture("gfx" + Paths::SeparatorAsString + skin.prefix + "intro.jpg"));
            else
                ResetTexture(pBackgroundTexture, gfx->load_texture("gfx" + Paths::SeparatorAsString + "intro.jpg"));
        }
        else
        {
            ResetTexture(pBackgroundTexture, gfx->load_texture("gfx" + Paths::SeparatorAsString + "intro.jpg"));
        }
    }


    void Intro::redrawTheScreen()
    {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Background
        gfx->drawtexture(pBackgroundTexture, 0.0f, 0.0f, SCREEN_W, SCREEN_H);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float fw = SCREEN_W / 1280.0f;
        float fh = SCREEN_H / 1024.0f;

        // The text itself
        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        int indx = 0;
        for (unsigned int i = pStartIndex; i < pContentSize && indx < TA3D_INTRO_MAX_LINES; ++i, ++indx)
            gfx->print(gui_font, 220.0f * fw, TA3D_INTRO_TOP + (indx-1) * pCurrentFontHeight - pDelta, 0.0f, pContent[i]);

        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, pBackgroundTexture);
        glBegin(GL_QUADS);

        glColor4ub(0xFF, 0xFF, 0xFF, 0);

        glTexCoord2f(0.0f, (TA3D_INTRO_TOP - 60.0f) / SCREEN_H);                     glVertex2f(0.0f, TA3D_INTRO_TOP - 60.0f);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP - 60.0f) / SCREEN_H);                     glVertex2f(SCREEN_W, TA3D_INTRO_TOP - 60.0f);
        glTexCoord2f(1.0f, TA3D_INTRO_TOP / SCREEN_H);                               glVertex2f(SCREEN_W, TA3D_INTRO_TOP);
        glTexCoord2f(0.0f, TA3D_INTRO_TOP / SCREEN_H);                               glVertex2f(0.0f, TA3D_INTRO_TOP);

        glTexCoord2f(0.0f, TA3D_INTRO_TOP / SCREEN_H);                               glVertex2f(0.0f, TA3D_INTRO_TOP);
        glTexCoord2f(1.0f, TA3D_INTRO_TOP / SCREEN_H);                               glVertex2f(SCREEN_W, TA3D_INTRO_TOP);
        glColor4ub(0xFF,0xFF,0xFF,0xFF);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / SCREEN_H);    glVertex2f(SCREEN_W, TA3D_INTRO_TOP + 2 * pCurrentFontHeight);
        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / SCREEN_H);    glVertex2f(0.0f, TA3D_INTRO_TOP + 2 * pCurrentFontHeight);

        glTexCoord2f(0.0f, (TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight) / SCREEN_H); glVertex2f(0.0f, TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight);
        glTexCoord2f(1.0f, (TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight) / SCREEN_H); glVertex2f(SCREEN_W, TA3D_INTRO_BOTTOM - 2 * pCurrentFontHeight);
        glColor4ub(0xFF, 0xFF, 0xFF, 0);
        glTexCoord2f(1.0f, TA3D_INTRO_BOTTOM / SCREEN_H);                            glVertex2f(SCREEN_W, TA3D_INTRO_BOTTOM);
        glTexCoord2f(0.0f, TA3D_INTRO_BOTTOM / SCREEN_H);                            glVertex2f(0.0f, TA3D_INTRO_BOTTOM);

        glTexCoord2f(0.0f, TA3D_INTRO_BOTTOM / SCREEN_H);                            glVertex2f(0.0f, TA3D_INTRO_BOTTOM);
        glTexCoord2f(1.0f, TA3D_INTRO_BOTTOM / SCREEN_H);                            glVertex2f(SCREEN_W, TA3D_INTRO_BOTTOM);
        glTexCoord2f(1.0f, (TA3D_INTRO_BOTTOM + 60.0f) / SCREEN_H);                  glVertex2f(SCREEN_W, TA3D_INTRO_BOTTOM + 60.0f);
        glTexCoord2f(0.0f, (TA3D_INTRO_BOTTOM + 60.0f) / SCREEN_H);                  glVertex2f(0.0f, TA3D_INTRO_BOTTOM + 60.0f);

        glEnd();
        glDisable(GL_BLEND);

        // Flip
        gfx->flip();
    }


} // namespace Menus
} // namespace TA3D
