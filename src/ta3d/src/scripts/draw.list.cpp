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

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "draw.list.h"
#include "script.h"

namespace TA3D
{
    void DrawList::init()
    {
        prim.type = DRAW_TYPE_NONE;
        prim.text.clear();
        next = NULL;
    }

    void DrawList::destroy()
    {
        prim.tex = nullptr;
        prim.text.clear();
		next = NULL;
        init();
    }

    void DrawList::add(DrawObject &obj)
    {
        LuaProgram::inGame->lock();
		if (!next)
        {
            next = new DrawList;
            next->prim = obj;
        }
        else
            next->add(obj);
        LuaProgram::inGame->unlock();
    }

    void DrawList::draw(Font *fnt)
    {
        glPushMatrix();
        CHECK_GL();
        const float screen_w = float(SCREEN_W);
        const float screen_h = float(SCREEN_H);
        const float sx = screen_w / 640.0f;
        const float sy = screen_h / 480.0f;
        glScalef(sx, sy, 1.0f);
        CHECK_GL();
        switch(prim.type)
        {
        case DRAW_TYPE_POINT:
            glBegin(GL_POINTS);
            glColor3f(prim.r[0],prim.g[0],prim.b[0]);
            glVertex2f(prim.x[0],prim.y[0]);
            glEnd();
            CHECK_GL();
            break;
        case DRAW_TYPE_LINE:
            glBegin(GL_LINES);
            glColor3f(prim.r[0],prim.g[0],prim.b[0]);
            glVertex2f(prim.x[0],prim.y[0]);
            glVertex2f(prim.x[1],prim.y[1]);
            glEnd();
            CHECK_GL();
            break;
        case DRAW_TYPE_CIRCLE:
            glBegin(GL_LINE_STRIP);
            glColor3f(prim.r[0],prim.g[0],prim.b[0]);
            {
                const int max = (int)(sqrtf(prim.r[1])*2.0f)*2;
                if (max > 0)
                    for(int i = 0 ; i <= prim.r[1] * 10 ; i++)
                        glVertex2f(prim.x[0] + prim.r[1] * cosf(float(i) * 6.2831853072f / float(max)), prim.y[0] + prim.r[1] * sinf(float(i) * 6.2831853072f / float(max)));
            }
            glEnd();
            CHECK_GL();
            break;
        case DRAW_TYPE_TRIANGLE:
            glBegin(GL_TRIANGLES);
            glColor3f(prim.r[0],prim.g[0],prim.b[0]);
            glVertex2f(prim.x[0],prim.y[0]);
            glVertex2f(prim.x[1],prim.y[1]);
            glVertex2f(prim.x[2],prim.y[2]);
            glEnd();
            CHECK_GL();
            break;
        case DRAW_TYPE_BOX:
            glBegin(GL_LINE_STRIP);
            glColor3f(prim.r[0],prim.g[0],prim.b[0]);
            glVertex2f(prim.x[0],prim.y[0]);
            glVertex2f(prim.x[1],prim.y[0]);
            glVertex2f(prim.x[1],prim.y[1]);
            glVertex2f(prim.x[0],prim.y[1]);
            glVertex2f(prim.x[0],prim.y[0]);
            glEnd();
            CHECK_GL();
            break;
        case DRAW_TYPE_FILLBOX:
            glBegin(GL_QUADS);
            glColor3f(prim.r[0],prim.g[0],prim.b[0]);
            glVertex2f(prim.x[0],prim.y[0]);
            glVertex2f(prim.x[1],prim.y[0]);
            glVertex2f(prim.x[1],prim.y[1]);
            glVertex2f(prim.x[0],prim.y[1]);
            glEnd();
            CHECK_GL();
            break;
        case DRAW_TYPE_TEXT:
            glPopMatrix();
            CHECK_GL();
            glPushMatrix();
            CHECK_GL();
            gfx->glEnable(GL_BLEND);
            CHECK_GL();
            gfx->glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            CHECK_GL();
            {
                float x = prim.x[0] * sx;
                float y = prim.y[0] * sy;
                if (x < 0.0f)
                    x += screen_w - 0.5f * fnt->length(prim.text);
                if (y < 0.0f)
                    y += screen_h - 0.5f * fnt->height();
                fnt->print(x + 1, y + 1, makeacol32(0,0,0, 0xFF), prim.text);
                fnt->print(x, y, makeacol32((int)(prim.r[0] * 255.0f), (int)(prim.g[0] * 255.0f), (int)(prim.b[0] * 255.0f), 0xFF), prim.text);
            }
            gfx->glDisable(GL_BLEND);
            CHECK_GL();
            break;
        case DRAW_TYPE_BITMAP:
            if (!prim.tex && !prim.text.isEmpty())
            {
                prim.tex = gfx->load_texture( prim.text );
                prim.text.clear();
            }
            gfx->glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            CHECK_GL();
            gfx->glEnable(GL_BLEND);
            CHECK_GL();
            gfx->drawtexture(prim.tex, prim.x[0] * sx, prim.y[0] * sy, prim.x[1] * sx, prim.y[1] * sy, makecol(0xFF,0xFF,0xFF));
            gfx->glDisable(GL_BLEND);
            CHECK_GL();
            break;
        }
        glPopMatrix();
        CHECK_GL();
        if (next)
            next->draw(fnt);
    }
}
