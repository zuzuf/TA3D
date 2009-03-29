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
        if (prim.type == DRAW_TYPE_BITMAP)
            glDeleteTextures(1,&prim.tex);
        prim.text.clear();
        if (next)
        {
            next->destroy();
            delete next;
        }
        init();
    }

    void DrawList::add(DrawObject &obj)
    {
        LuaProgram::inGame->lock();
        if (next == NULL)
        {
            next = new DrawList;
            next->prim = obj;
            next->next = NULL;
        }
        else
            next->add(obj);
        LuaProgram::inGame->unlock();
    }

    void DrawList::draw(Font *fnt)
    {
        glPushMatrix();
        switch(prim.type)
        {
            case DRAW_TYPE_POINT:
                glScalef(SCREEN_W / 640.0f, SCREEN_H / 480.0f,1.0f);
                glBegin(GL_POINTS);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glEnd();
                break;
            case DRAW_TYPE_LINE:
                glScalef(SCREEN_W / 640.0f, SCREEN_H / 480.0f,1.0f);
                glBegin(GL_LINES);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glEnd();
                break;
            case DRAW_TYPE_CIRCLE:
                glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
                glBegin(GL_LINE_STRIP);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                {
                    int max = (int)(sqrtf(prim.r[1])*2.0f)*2;
                    if (max > 0)
                        for(int i = 0 ; i <= prim.r[1] * 10 ; i++)
                            glVertex2f(prim.x[0] + prim.r[1] * cosf(i * 6.2831853072f / max), prim.y[0] + prim.r[1] * sinf(i * 6.2831853072f / max));
                }
                glEnd();
                break;
            case DRAW_TYPE_TRIANGLE:
                glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
                glBegin(GL_TRIANGLES);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glVertex2f(prim.x[2],prim.y[2]);
                glEnd();
                break;
            case DRAW_TYPE_BOX:
                glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
                glBegin(GL_LINE_STRIP);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glVertex2f(prim.x[0],prim.y[1]);
                glVertex2f(prim.x[0],prim.y[0]);
                glEnd();
                break;
            case DRAW_TYPE_FILLBOX:
                glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
                glBegin(GL_QUADS);
                glColor3f(prim.r[0],prim.g[0],prim.b[0]);
                glVertex2f(prim.x[0],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[0]);
                glVertex2f(prim.x[1],prim.y[1]);
                glVertex2f(prim.x[0],prim.y[1]);
                glEnd();
                break;
            case DRAW_TYPE_TEXT:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                gfx->print(fnt,prim.x[0] * SCREEN_W / 640.0f + 1,prim.y[0] * SCREEN_H / 480.0f + 1,0.0f,makeacol32(0,0,0, 0xFF),prim.text);
                gfx->print(fnt,prim.x[0] * SCREEN_W / 640.0f,prim.y[0] * SCREEN_H / 480.0f,0.0f,makeacol32((int)(prim.r[0]*255.0f),(int)(prim.g[0]*255.0f),(int)(prim.b[0]*255.0f), 0xFF),prim.text);
                glDisable(GL_BLEND);
                break;
            case DRAW_TYPE_BITMAP:
                glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
                if (prim.tex == 0 && !prim.text.empty())
                {
                    prim.tex = gfx->load_texture( prim.text );
                    prim.text.clear();
                    prim.text = NULL;
                }
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                gfx->drawtexture(prim.tex,prim.x[0],prim.y[0],prim.x[1],prim.y[1]);
                glDisable(GL_BLEND);
                break;
        }
        glPopMatrix();
        if (next)
            next->draw(fnt);
    }
}
