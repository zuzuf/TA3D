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
#include "../misc/math.h"
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
        gfx->set_2D_mode();

        anim_data.load(nb_obj());

            // Initialize renderer
        getTexture();

        cam.pos.z = 10.0f;
        cam.pos.x = cam.pos.y = 0.0f;
        cam.dir.z = -1.0f;
        cam.dir.x = cam.dir.y = 0.0f;
        cam.setMatrix(Scale(1.0f));

        r1 = 0.0f, r2 = 0.0f, r3 = 0.0f;
        zoom = 0.1f;
        amx = mouse_x;
        amy = mouse_y;
        amz = mouse_z;
        sel_idx = -1;
        cursor_idx = -1;

        renderModel();
        return true;
    }

    void Animator::doFinalize()
    {
        if (texture)
        {
            pArea->get_object("animator.render")->Data = 0;
            gfx->destroy_texture( texture );
        }
        gfx->unset_2D_mode();
    }


    void Animator::waitForEvent()
    {
        amx = mouse_x;
        amy = mouse_y;
        amz = mouse_z;

        uint32 selection_timer = msec_timer;
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
                 && !keyIsPressed && !pArea->scrolling
                 && (sel_idx < 0 || msec_timer - selection_timer < 50));
    }


    bool Animator::maySwitchToAnotherMenu()
    {
        // Exit
        if (key[KEY_ESC] || pArea->get_state("animator.b_close"))
            return true;

        bool need_refresh = sel_idx >= 0;
        if (amz != mouse_z)
        {
            zoom *= exp( (amz - mouse_z) * 0.1f );
            need_refresh = true;
        }

        if (key[KEY_R])
        {
            zoom = 0.1f;
            r1 = 0.0f, r2 = 0.0f, r3 = 0.0f;
            need_refresh = true;
        }

        if (key[KEY_X])
        {
            r1 = 0.0f, r2 = 90.0f, r3 = 0.0f;
            need_refresh = true;
        }

        if (key[KEY_Y])
        {
            r1 = 90.0f, r2 = 0.0f, r3 = 0.0f;
            need_refresh = true;
        }

        if (key[KEY_Z])
        {
            r1 = 0.0f, r2 = 0.0f, r3 = 0.0f;
            need_refresh = true;
        }

        if (pArea->is_mouse_over("animator.render"))
        {
            if ((amx != mouse_x || amy != mouse_y) && mouse_b == 2)
            {
                r2 += mouse_x - amx;
                r1 += mouse_y - amy;
                need_refresh = true;
            }
            need_refresh |= userInteraction();
        }

        if (need_refresh)
            renderModel();

        return false;
    }

    Vector3D Animator::getCursor()
    {
        GUIOBJ *render = pArea->get_object("animator.render");
        if (render == NULL) return Vector3D();

        int mx = mouse_x - render->x1;
        int my = mouse_y - render->y1;
        int w = render->x2 - render->x1;
        int h = render->y2 - render->y1;

		Vector3D cur_dir;
		cur_dir = cam.dir + cam.widthFactor * 2.0f * (mx - 0.5f * w) / w * cam.side
			- 1.5f * (my - 0.5f * h) / h * cam.up;

        cur_dir = cur_dir * RotateXYZ( -r1 * DEG2RAD, -r2 * DEG2RAD, -r3 * DEG2RAD );   // Return to object space coordinates

		cur_dir.unit();		// Direction pointée par le curseur
		return cur_dir;
    }

    bool Animator::userInteraction()
    {
        if (TA3D::VARS::TheModel == NULL)   return false;

        Vector3D pointer = getCursor();
        Vector3D pos = 1.0f / zoom * cam.pos * RotateXYZ( -r1 * DEG2RAD, -r2 * DEG2RAD, -r3 * DEG2RAD );

        Vector3D I;
        MATRIX_4x4 M = Scale(zoom);
        int nidx = TA3D::VARS::TheModel->hit(pos,pointer,NULL,&I,M);
        bool result = false;

        if (cursor_idx != nidx)
        {
            cursor_idx = TA3D::Math::Max(nidx,-1);
            result = true;
        }
        if (cursor_idx >= 0 && mouse_b == 1)
        {
            sel_idx = cursor_idx;
            result = true;
        }
        return result;
    }

    void Animator::getTexture()
    {
        GUIOBJ *render = pArea->get_object("animator.render");
        if (render == NULL)     // Nothing to render to
        {
            texture = 0;
            return;
        }
        render->u1 = 0.0f;  render->v1 = 1.0f;
        render->u2 = 1.0f;  render->v2 = 0.0f;
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
        glClearColor( 0.75f, 0.75f, 0.75f, 0.0f );
        gfx->clearAll();                // Clear the screen

        cam.setView();
        glScalef(zoom,zoom,zoom);
        glRotatef(r1,1.0f,0.0f,0.0f);		// Rotation of the objet
        glRotatef(r2,0.0f,1.0f,0.0f);
        glRotatef(r3,0.0f,0.0f,1.0f);

        glDisable(GL_LIGHTING);					// Draw the grid
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINES);
            glColor4f(0.5f,0.5f,0.5f,1.0f);
            for(float y = -100.0f ; y <= 100.0f ; y += 10.0f)
            {
                if (y == 0.0f)
                {
                    glVertex3f(-100.0f,0.0f,y);
                    glVertex3f(0.0f,0.0f,y);
                    glVertex3f(y,0.0f,-100.0f);
                    glVertex3f(y,0.0f,0.0f);
                    continue;
                }
                glVertex3f(-100.0f,0.0f,y);
                glVertex3f(100.0f,0.0f,y);
                glVertex3f(y,0.0f,-100.0f);
                glVertex3f(y,0.0f,100.0f);
            }

            glColor4f(1.0f,0.0f,0.0f,1.0f);
            glVertex3f(0.0f,0.0f,0.0f);         glVertex3f(1000.0f, 0.0f, 0.0f);

            glColor4f(0.0f,1.0f,0.0f,1.0f);
            glVertex3f(0.0f,0.0f,0.0f);         glVertex3f( 0.0f,1000.0f, 0.0f);

            glColor4f(0.0f,0.0f,1.0f,1.0f);
            glVertex3f(0.0f,0.0f,0.0f);         glVertex3f( 0.0f, 0.0f,1000.0f);
        glEnd();

        if (TA3D::VARS::TheModel)
        {
            if (sel_idx >= 0)
                glColor4f(0.5f,0.5f,0.5f,1.0f);
            else
                glColor4f(1.0f,1.0f,1.0f,1.0f);
            for(int i = 0 ; i < anim_data.nb_piece ; i++)
                anim_data.flag[i] = (i != sel_idx ? 0 : FLAG_HIDE);
            TA3D::VARS::TheModel->draw(msec_timer * 0.001f,&anim_data,false,false,false,0,NULL,NULL,NULL,0.0f,NULL,false,0,false);

            if (cursor_idx >= 0 && sel_idx != cursor_idx)
            {
                gfx->ReInitAllTex( true );
                glDisable(GL_TEXTURE_2D);
                for(int i = 0 ; i < anim_data.nb_piece ; i++)
                    anim_data.flag[i] = (i == cursor_idx ? 0 : FLAG_HIDE);
                float tmp[4];
                memcpy(tmp,TA3D::VARS::obj_table[cursor_idx]->surface.Color, 4 * sizeof(float));
                TA3D::VARS::obj_table[cursor_idx]->surface.Color[0] = 1.0;
                TA3D::VARS::obj_table[cursor_idx]->surface.Color[1] = 1.0;
                TA3D::VARS::obj_table[cursor_idx]->surface.Color[2] = 1.0;
                TA3D::VARS::obj_table[cursor_idx]->surface.Color[3] = 0.5f;
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                TA3D::VARS::TheModel->obj.draw(0.0f,&anim_data,false,true,true,0,false,false);
                glDisable(GL_BLEND);
                memcpy(TA3D::VARS::obj_table[cursor_idx]->surface.Color, tmp, 4 * sizeof(float));
            }

            if (sel_idx >= 0)
            {
                gfx->ReInitAllTex( true );
                glEnable(GL_TEXTURE_2D);
                for(int i = 0 ; i < anim_data.nb_piece ; i++)
                    anim_data.flag[i] = (i == sel_idx ? 0 : FLAG_HIDE);
                float col = cos(msec_timer*0.002f)*0.375f+0.625f;
                float tmp[4];
                memcpy(tmp,TA3D::VARS::obj_table[sel_idx]->surface.Color, 4 * sizeof(float));
                TA3D::VARS::obj_table[sel_idx]->surface.Color[0] = col;
                TA3D::VARS::obj_table[sel_idx]->surface.Color[1] = col;
                TA3D::VARS::obj_table[sel_idx]->surface.Color[2] = col;
                TA3D::VARS::obj_table[sel_idx]->surface.Color[3] = 1.0f;
                TA3D::VARS::TheModel->obj.draw(0.0f,&anim_data,false,true,true,0,false,false);
                memcpy(TA3D::VARS::obj_table[sel_idx]->surface.Color, tmp, 4 * sizeof(float));
            }

            gfx->ReInitAllTex( true );
            glEnable(GL_TEXTURE_2D);
        }

        gfx->renderToTexture(0,true);               // Back to normal render target

        gfx->set_2D_mode();
    }
};
};
