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

    set_uformat(U_UTF8);
    bool init=(Glfond==0);

    if(init)
    {
        messages.clear();
        if( !lp_CONFIG->skin_name.empty() && TA3D::Paths::Exists(lp_CONFIG->skin_name)) // Loads a skin
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
    glScalef(SCREEN_W/1280.0f,SCREEN_H/1024.0f,1.0f);

    const float TA_font_size = gfx->TA_font.get_size();
    gfx->TA_font.change_size( 1.75f );
    float h = gfx->TA_font.height();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

    gfx->drawtexture(Glfond,0.0f,0.0f,1280.0f,1024.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f,1.0f,1.0f,1.0f);

    if (messages.empty() || String::ToLower(messages.front()) != String::ToLower(msg))
    {
        if (!messages.empty())
            messages.front() = messages.front() + " - " + I18N::Translate( "done" );
        messages.push_front( msg );
    }

    int e = 0;
    for (String::List::const_iterator i = messages.begin(); i != messages.end(); ++i, ++e)
        gfx->print(gfx->TA_font, 105.0f, 175.0f + h * e, 0.0f, 0xFFFFFFFF, *i);

    glDisable(GL_BLEND);

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f,0.8f,0.3f);
    glBegin(GL_QUADS);
    glVertex2f(100.0f,858.0f);
    glVertex2f(100.0f+10.72f*percent,858.0f);
    glVertex2f(100.0f+10.72f*percent,917.0f);
    glVertex2f(100.0f,917.0f);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f,1.0f,1.0f,1.0f);

    gfx->drawtexture(Glfond,100.0f,856.0f,1172.0f,917.0f,100.0f / 1280.0f,862.0f/1024.0f,1172.0f/1280.0f,917.0f/1024.0f);

    glDisable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    gfx->print(gfx->TA_font,640.0f-0.5f*gfx->TA_font.length(msg),830-h*0.5f,0.0f,0xFFFFFFFF,msg);
    glDisable(GL_BLEND);

    gfx->TA_font.change_size( TA_font_size );

    glPopMatrix();

    if( lp_CONFIG->draw_console_loading ) // If set in config
    {
        char *cmd = Console->draw( gfx->TA_font, 0.0f, gfx->TA_font.height(), true );			// Display something to show what's happening
        if( cmd )	free( cmd );
    }

    gfx->flip();

    gfx->unset_2D_mode();

    if(percent>=100.0f) {
        messages.clear();
        gfx->destroy_texture( Glfond );
    }

    set_uformat(U_ASCII);
}


