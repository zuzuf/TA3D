/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"

#define TA3D_BASIC_ENGINE
#include "../ta3dbase.h"		// Moteur
#include "../threads/cThread.h"
#include "../gui.h"			// Interface utilisateur
#include "../TA3D_hpi.h"		// Interface HPI requis pour 3do.h
#include "../gfx/particles/particles.h"
#include "../gaf.h"
#include "../3do.h"			// Gestion des modèles 3D
#include "../3ds.h"			// The 3DS model loader
#include "../3dmeditor.h"
#include "../misc/paths.h"
#include "../misc/osinfo.h"
#include "../languages/i18n.h"
#include "../jpeg/ta3d_jpg.h"
#include "../converters/obj.h"
#include "animator.h"

namespace Editor
{
namespace Menus
{
    bool Animator::Execute()
    {
        Animator m;
        return m.execute();
    }


    Animator::Animator()
        :Abstract()
    {}

    Animator::~Animator()
    {}

    bool Animator::doInitialize()
    {
        loadAreaFromTDF("Animator", "gui/animator.area");
        return true;
    }

    void Animator::doFinalize()
    {
        // Do nothing
    }


    void Animator::waitForEvent()
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

        } while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
                 && mouse_b == 0
                 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
                 && !keyIsPressed && !pArea->scrolling);
    }


    bool Animator::maySwitchToAnotherMenu()
    {
        // Exit
        if (key[KEY_ESC] || pArea->get_state("animator.b_close"))
            return true;

        return false;
    }

    void Animator::getTexture()
    {
        GUIOBJ *render = pArea->get_object("animator.render");
        if (render == NULL)     // Nothing to render to
        {
            texture = 0;
            return;
        }
        int w = (int)(render->x2 - render->x1);
        int h = (int)(render->y2 - render->y1);
        if ((GLuint)render->Data == 0)
        {
            allegro_gl_set_texture_format(GL_RGB8);
            render->Data = (uint32) gfx->create_texture( w, h, FILTER_LINEAR );
        }
        texture = render->Data;
    }

    void Animator::renderModel()
    {
        if (texture == 0)   return;     // No render target

        gfx->unset_2D_mode();

        gfx->renderToTexture(texture, true);        // Initialise rendering

        gfx->SetDefState();
        gfx->clearAll();                // Clear the screen

        if (TA3D::VARS::TheModel)
        {
            TA3D::VARS::TheModel->draw(0.0f);
        }

        gfx->renderToTexture(0,true);               // Back to normal render target

        gfx->set_2D_mode();
    }
};
};
