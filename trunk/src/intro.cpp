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

/*-----------------------------------------------------------------------------\
  |                                     intro.cpp                                |
  |       Ce module contient les fonctions de l'introduction de TA3D             |
  |                                                                              |
  \-----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "intro.h"
#include "console.h"
#include "misc/paths.h"
#include <list>
#include "languages/i18n.h"
#include "gfx/gui/skin.h"
#include "console.h"


void loading(const float percent, const String& msg)
{
    static int last_percent = 0;
    static String::List messages;
    static GLuint Glfond = 0;

    if( network_manager.isConnected() && last_percent != (int)percent )
    {
        last_percent = (int)percent;
        network_manager.sendAll(format("LOADING %d", last_percent));
    }

    bool init=(Glfond==0);

    if(init)
    {
        messages.clear();
        if( !lp_CONFIG->skin_name.empty() && HPIManager->Exists(lp_CONFIG->skin_name)) // Loads a skin
        {
            SKIN *skin = new SKIN;
            skin->load_tdf( lp_CONFIG->skin_name );

            if( !skin->prefix.empty() )
                Glfond = gfx->load_texture_mask("gfx/" + skin->prefix + "load.jpg", 7);
            else
                Glfond = gfx->load_texture_mask("gfx/load.jpg", 7);

            delete skin;
        }
        else
            Glfond = gfx->load_texture_mask("gfx/load.jpg", 7);
    }

    gfx->set_2D_mode();
    glPushMatrix();

    float h = gui_font->height();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

    gfx->drawtexture(Glfond,0.0f,0.0f,SCREEN_W,SCREEN_H);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(0xFF,0xFF,0xFF,0xFF);

    if (messages.empty() || String::ToLower(messages.front()) != String::ToLower(msg))
    {
        if (!messages.empty())
            messages.front() = messages.front() + " - " + I18N::Translate( "done" );
        messages.push_front( msg );
    }

    float fw = SCREEN_W / 1280.0f;
    float fh = SCREEN_H / 1024.0f;

    int e = 0;
    for (String::List::const_iterator i = messages.begin(); i != messages.end(); ++i, ++e)
        gfx->print(gui_font, 105.0f * fw, 175.0f * fh + h * e, 0.0f, 0xFFFFFFFF, *i);

    glDisable(GL_BLEND);

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f,0.8f,0.3f);
    glBegin(GL_QUADS);
    glVertex2f(100.0f * fw,858.0f * fh);
    glVertex2f((100.0f+10.72f*percent) * fw,858.0f * fh);
    glVertex2f((100.0f+10.72f*percent) * fw,917.0f * fh);
    glVertex2f(100.0f * fw,917.0f * fh);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(0xFF,0xFF,0xFF,0xFF);

    gfx->drawtexture(Glfond,100.0f * fw,856.0f * fh,1172.0f * fw,917.0f * fh,100.0f / 1280.0f,862.0f/1024.0f,1172.0f/1280.0f,917.0f/1024.0f);

    glDisable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    gfx->print(gui_font,640.0f * fw - 0.5f * gui_font->length(msg),830 * fh - h * 0.5f,0.0f,0xFFFFFFFF,msg);
    glDisable(GL_BLEND);

    glPopMatrix();

    if( lp_CONFIG->draw_console_loading ) // If set in config
        String cmd = console.draw(gui_font, 0.0f, true);			// Display something to show what's happening

    gfx->flip();

    gfx->unset_2D_mode();

    if(percent>=100.0f)
    {
        messages.clear();
        gfx->destroy_texture( Glfond );
    }

//    set_uformat(U_ASCII);
}


