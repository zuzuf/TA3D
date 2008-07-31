#include "intro.h"
#include "../../misc/paths.h"
#include <vector>
#include "../../misc/resources.h"
#include "../../misc/files.h"
#include "../../gfx/gui/skin.h"



//!
# define TA3D_INTRO_MAX_LINES 15

# define TA3D_INTRO_TOP 580.0f

#define LAST_LINE		14

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
        :Abstract(), pContentSize(0), pBackgroundTexture(0), pPreviousFontSize(1.5f),
        pCurrentFontHeight(1.0f)
    {}

    Intro::~Intro()
    {}

    
    bool Intro::doInitialize()
    {
        LOG_ASSERT(NULL != gfx);
        LOG_DEBUG(LOG_PREFIX_MENU_INTRO << "Entering...");
        reloadContent();
        loadBackgroundTexture();

        // Font size
        pPreviousFontSize = gfx->TA_font.get_size();
        gfx->TA_font.change_size( 1.5f );
        pCurrentFontHeight = gfx->TA_font.height();

        gfx->set_2D_mode();
        glScalef(SCREEN_W / 1280.0f, SCREEN_H / 1024.0f, 1.0f);

        pDelta = 0.0f;
        pStartIndex = 0;

        return true;
    }

    void Intro::doFinalize()
    {
        // Restore the previous font size
        gfx->TA_font.change_size(pPreviousFontSize);
        ResetTexture(pBackgroundTexture);
        LOG_DEBUG(LOG_PREFIX_MENU_INTRO << "Done.");
        pScrollTimer = msec_timer;
    }


    void Intro::waitForEvent()
    {
        // Do nothing
        rest(1);
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

        if (!lp_CONFIG->skin_name.empty() && TA3D::Paths::Exists(lp_CONFIG->skin_name))
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
        gfx->drawtexture(pBackgroundTexture, 0.0f, 0.0f, 1280.0f, 1024.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // The text itself
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        int indx = 0;
        for (unsigned int i = pStartIndex; i < pContentSize && indx < TA3D_INTRO_MAX_LINES; ++i, ++indx)
            gfx->print(gfx->TA_font, 220.0f, TA3D_INTRO_TOP + (indx+2) * pCurrentFontHeight - pDelta, 0.0f, pContent[i]);

        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, pBackgroundTexture);
        glBegin(GL_QUADS);

        glColor4f(1.0f, 1.0f, 1.0f, 0.0f);

        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP + pCurrentFontHeight);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP + pCurrentFontHeight);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+2 * pCurrentFontHeight);
        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP+2 * pCurrentFontHeight);

        glTexCoord2f(0.0f,( TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP+2 * pCurrentFontHeight);
        glTexCoord2f(1.0f,( TA3D_INTRO_TOP + 2 * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+2 * pCurrentFontHeight);
        glColor4f(1.0f,1.0f,1.0f,1.0f);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + 4 * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+4 * pCurrentFontHeight);
        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + 4 * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP + 4 * pCurrentFontHeight);

        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + LAST_LINE * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP+LAST_LINE * pCurrentFontHeight);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + LAST_LINE * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+LAST_LINE * pCurrentFontHeight);
        glColor4f(1.0f,1.0f,1.0f,0.0f);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + (LAST_LINE + 2) * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+(LAST_LINE+2) * pCurrentFontHeight);
        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + (LAST_LINE + 2) * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP+(LAST_LINE+2) * pCurrentFontHeight);

        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + (LAST_LINE + 2) * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP+(LAST_LINE + 2) * pCurrentFontHeight);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + (LAST_LINE + 2) * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+(LAST_LINE + 2) * pCurrentFontHeight);
        glTexCoord2f(1.0f, (TA3D_INTRO_TOP + (LAST_LINE + 3) * pCurrentFontHeight) / 1024.0f);
        glVertex2f(1280.0f, TA3D_INTRO_TOP+(LAST_LINE + 3) * pCurrentFontHeight);
        glTexCoord2f(0.0f, (TA3D_INTRO_TOP + (LAST_LINE + 3) * pCurrentFontHeight) / 1024.0f);
        glVertex2f(0.0f, TA3D_INTRO_TOP+(LAST_LINE+3) * pCurrentFontHeight);

        glEnd();
        glDisable(GL_BLEND);

        // Flip
        gfx->flip();
    }


} // namespace Menus
} // namespace TA3D
