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

/*---------------------------------------------------------------------------------\
  |                                        gui.cpp                                   |
  |         Contient les fonctions nécessaires à la gestion de l'interface de ta3D   |
  |  comme les boutons, les fenêtres,...                                             |
  |                                                                                  |
  \---------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "misc/matrix.h"
#include "ta3dbase.h"
#include "misc/paths.h"
#include "languages/i18n.h"
#include "gfx/gui/skin.h"
#include "gfx/gui/skin.object.h"
#include "gfx/gui/wnd.h"

using namespace TA3D::Exceptions;



namespace TA3D
{


bool use_normal_alpha_function = false;
float gui_font_h = 8.0f;




void glbutton(const String &caption,float x1,float y1,float x2,float y2,bool etat)
{
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    if(!etat)
    {
        glColor3f(1.0f,1.0f,1.0f);	glVertex2f(x1,y1);			// Dessine les bords du bouton
        glColor3f(0.5f,0.5f,0.5f);	glVertex2f(x2,y1);
        glColor3f(0.0f,0.0f,0.0f);	glVertex2f(x2,y2);
        glColor3f(0.5f,0.5f,0.5f);	glVertex2f(x1,y2);
    }
    else
    {
        glColor3f(0.0f,0.0f,0.0f);	glVertex2f(x1,y1);			// Dessine les bords du bouton
        glColor3f(0.5f,0.5f,0.5f);	glVertex2f(x2,y1);
        glColor3f(1.0f,1.0f,1.0f);	glVertex2f(x2,y2);
        glColor3f(0.5f,0.5f,0.5f);	glVertex2f(x1,y2);
    }

    glVertex2f(x1+2,y1+2);			// Dessine le fond du bouton
    glVertex2f(x2-2,y1+2);
    glVertex2f(x2-2,y2-2);
    glVertex2f(x1+2,y2-2);
    glEnd();

    glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
    glEnable(GL_BLEND);
    if(etat)
        gfx->print(gfx->normal_font,0.5f*(x1+x2-9*caption.length())+1.0f,0.5f*(y1+y2-9.0f)+1.0f,0.0f,0xFFFFFFFF,caption);
    else
        gfx->print(gfx->normal_font,0.5f*(x1+x2-9*caption.length()),0.5f*(y1+y2-9.0f),0.0f,0xFFFFFFFF,caption);
    glDisable(GL_BLEND);
}

const String msg_box(TA3D::Interfaces::GfxFont fnt,const String &title,const String &msg,bool ask)
{
    gfx->set_2D_mode();
    for(int i=0;i<2;i++)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glColor4f(0.5f,0.5f,0.5f,1.0f);
        glBegin(GL_QUADS);
        glVertex2f(SCREEN_W>>2,SCREEN_H>>2);
        glVertex2f(SCREEN_W*3>>2,SCREEN_H>>2);
        glVertex2f(SCREEN_W*3>>2,SCREEN_H*3>>2);
        glVertex2f(SCREEN_W>>2,SCREEN_H*3>>2);
        glEnd();
        glBegin(GL_LINES);
        glColor4f(1.0f,1.0f,1.0f,1.0f);
        glVertex2f(SCREEN_W>>2,SCREEN_H>>2);		glVertex2f(SCREEN_W*3>>2,SCREEN_H>>2);
        glVertex2f(SCREEN_W>>2,SCREEN_H>>2);		glVertex2f(SCREEN_W>>2,SCREEN_H*3>>2);

        glColor4f(0.25f,0.25f,0.25f,1.0f);
        glVertex2f(SCREEN_W*3>>2,SCREEN_H*3>>2);	glVertex2f(SCREEN_W>>2,SCREEN_H*3>>2);
        glVertex2f(SCREEN_W*3>>2,SCREEN_H*3>>2);	glVertex2f(SCREEN_W*3>>2,SCREEN_H>>2);
        glEnd();
        glBegin(GL_QUADS);
        glColor4f(0.3f,0.3f,1.0f,1.0f);
        glVertex2f((SCREEN_W>>2)+3,(SCREEN_H>>2)+3);
        glColor4f(0.7f,0.7f,1.0f,1.0f);
        glVertex2f((SCREEN_W*3>>2)-3,(SCREEN_H>>2)+3);
        glVertex2f((SCREEN_W*3>>2)-3,(SCREEN_H>>2)+3+fnt.height());
        glColor4f(0.3f,0.3f,1.0f,1.0f);
        glVertex2f((SCREEN_W>>2)+3,(SCREEN_H>>2)+3+fnt.height());
        glEnd();
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        gfx->print(fnt,gfx->SCREEN_W_HALF-fnt.length(title)*0.5f,(SCREEN_H>>2)+3+0.0f*fnt.height(),0.0f,0xFFFFFFFF,title);		// Titre

        gfx->print(fnt,(SCREEN_W>>2)+5,(SCREEN_H>>2)+3+2*fnt.height(),0.0f,0xFFFFFFFF,msg);		// Message
        gfx->flip();
    }

    String buf = "";

    if(ask)
        clear_keybuf();

    bool done=false;

    do {
        poll_keyboard();
        if(key[KEY_ESC] || key[KEY_ENTER]) {
            while(key[KEY_ESC] || key[KEY_ENTER]) {
                rest(1);
                poll_keyboard();
            }
            done=true;
        }
        else
            rest(1);

        if(ask) {
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glColor4f(0.15f,0.15f,0.15f,1.0);
            glBegin(GL_QUADS);
            glVertex2f(gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF-fnt.height()*0.5f);
            glVertex2f(gfx->SCREEN_W_HALF+100,gfx->SCREEN_H_HALF-fnt.height()*0.5f);
            glVertex2f(gfx->SCREEN_W_HALF+100,gfx->SCREEN_H_HALF+fnt.height()*0.5f);
            glVertex2f(gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF+fnt.height()*0.5f);
            glEnd();
            glBegin(GL_LINES);
            glColor4f(0.0f,0.0f,0.0f,1.0);
            glVertex2f(gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF-fnt.height()*0.5f);			glVertex2f(gfx->SCREEN_W_HALF+100,gfx->SCREEN_H_HALF-fnt.height()*0.5f);
            glVertex2f(gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF-fnt.height()*0.5f);			glVertex2f(gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF+fnt.height()*0.5f);

            glColor4f(1.0f,1.0f,1.0f,1.0);
            glVertex2f(gfx->SCREEN_W_HALF+100,gfx->SCREEN_H_HALF+fnt.height()*0.5f);			glVertex2f(gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF+fnt.height()*0.5f);
            glVertex2f(gfx->SCREEN_W_HALF+100,gfx->SCREEN_H_HALF+fnt.height()*0.5f);			glVertex2f(gfx->SCREEN_W_HALF+100,gfx->SCREEN_H_HALF-fnt.height()*0.5f);
            glEnd();
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);

            if(keypressed()) {
                char key_code=readkey();
                switch(key_code)
                {
                    case 27:
                    case 9:
                    case 10:
                    case 13:
                        break;
                    case 8:
                        if(buf[0])
                            buf.resize( buf.length() - 1 );
                        //					buf[strlen(buf)-1]=0;
                        break;
                    default:
                        if(fnt.length(buf)<180) {
                            buf += key_code;
                            //					buf[strlen(buf)+1]=0;
                            //					buf[strlen(buf)]=key_code;
                        }
                };
            }

            gfx->print(fnt,gfx->SCREEN_W_HALF-100,gfx->SCREEN_H_HALF-fnt.height()*0.5f,0.0f,0xFFFFFFFF,format("%s_",buf.c_str()), 1.0f);
        }

        gfx->flip();
    }while(!done);

    gfx->unset_2D_mode();

    return buf;
}

//-------------- These are the GUI functions needed by the editors ----------------------------



/*---------------------------------------------------------------------------\
  |        Draw a window with the parameters from wnd                          |
  \---------------------------------------------------------------------------*/

void draw_Window(wnd& Wnd)
{
    gfx->rectfill(Wnd.x, Wnd.y, Wnd.x + Wnd.width - 1, Wnd.y + Wnd.height - 1, GrisM);
    gfx->rect(Wnd.x, Wnd.y, Wnd.x + Wnd.width - 1, Wnd.y + Wnd.height - 1, Noir);
    gfx->rect(Wnd.x + 1, Wnd.y + 1, Wnd.x + Wnd.width - 2, Wnd.y + Wnd.height - 2, GrisF);
    gfx->line(Wnd.x, Wnd.y, Wnd.x + Wnd.width - 1, Wnd.y, Blanc);
    gfx->line(Wnd.x, Wnd.y, Wnd.x, Wnd.y + Wnd.height - 1, Blanc);
    gfx->line(Wnd.x + 1, Wnd.y + 1, Wnd.x + Wnd.width - 2, Wnd.y + 1, GrisC);
    gfx->line(Wnd.x + 1, Wnd.y + 1, Wnd.x + 1, Wnd.y + Wnd.height - 2, GrisC);
    gfx->rectfill(Wnd.x + 3, Wnd.y + 3, Wnd.x + Wnd.width - 4, Wnd.y + 11, Bleu);
    gfx->print(gui_font, Wnd.x + 4, Wnd.y + 4, 0.0f, Blanc, Wnd.Title);
}

/*---------------------------------------------------------------------------\
  |        Déplace une fenêtre avec les paramètres Wnd de type window          |
  \---------------------------------------------------------------------------*/

unsigned char WinMov(int AMx,int AMy,int AMb,int Mx,int My,int Mb,wnd *Wnd)
{
    if (AMb == 1 && Mb == 1)
    {
        if (AMx >= (*Wnd).x + 3 && AMx <= (*Wnd).x + (*Wnd).width - 4)
        {
            if (AMy >= (*Wnd).y + 3 && AMy <= (*Wnd).y + 11)
            {
                (*Wnd).x += Mx - AMx;
                (*Wnd).y += My - AMy;
            }
        }
    }
    if (Mx >= (*Wnd).x && Mx <= (*Wnd).x + (*Wnd).width && My >= (*Wnd).y && My <= (*Wnd).y + (*Wnd).height)
        return 1;
    return 0;
}


/*---------------------------------------------------------------------------\
  |        Dessine un bouton en (x,y) avec le texte Title,enfoncé si Etat=1    |
  \---------------------------------------------------------------------------*/

void button (float x,float y,float x2,float y2,const String &Title,bool Etat,float size, SKIN *skin )
{
    if( skin && skin->button_img[ Etat ].tex ) {					// If we have gfx to skin the button then do it
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        gfx->set_color( 1.0f, 1.0f, 1.0f, 1.0f );
        glEnable( GL_BLEND );									// Enable alpha blending
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        skin->button_img[ Etat ].draw( x, y, x2, y2 );

        if( !Title.empty() ) {
            if( Etat)
                gfx->print(gui_font,(int)((x+x2)*0.5f-(gui_font.length(Title)*0.5f*size)+1),(int)((y+y2-(int)(gui_font.height()*size))*0.5f+1),0.0f,use_normal_alpha_function ? Blanc : Noir,Title,size);
            else
                gfx->print(gui_font,(int)((x+x2)*0.5f-(gui_font.length(Title)*0.5f*size)),(int)(y+y2-(int)(gui_font.height()*size))*0.5f,0.0f,use_normal_alpha_function ? Blanc : Noir,Title,size);
        }
        glPopAttrib();
    }
    else {
        gfx->rectfill(x,y,x2,y2,GrisM);
        gfx->rect(x,y,x2,y2,Noir);
        gfx->rect(x+1,y+1,x2-1,y2-1,GrisF);
        if(!Etat) {
            gfx->line(x,y,x2,y,Blanc);
            gfx->line(x,y,x,y2,Blanc);
            gfx->line(x+1,y+1,x2-1,y+1,GrisC);
            gfx->line(x+1,y+1,x+1,y2-1,GrisC);
            if( !Title.empty() )
                gfx->print(gui_font,(int)((x+x2)*0.5f-(gui_font.length(Title)*0.5f*size)),(int)(y+y2-(int)(gui_font.height()*size))*0.5f,0.0f,use_normal_alpha_function ? Blanc : Noir,Title,size);
        }
        else {
            gfx->line(x2,y2,x2,y,Blanc);
            gfx->line(x2,y2,x,y2,Blanc);
            gfx->line(x2-1,y2-1,x2-1,y+1,GrisC);
            gfx->line(x2-1,y2-1,x+1,y2-1,GrisC);
            if( !Title.empty() )
                gfx->print(gui_font,(int)((x+x2)*0.5f-(gui_font.length(Title)*0.5f*size)+1),(int)((y+y2-(int)(gui_font.height()*size))*0.5f+1),0.0f,use_normal_alpha_function ? Blanc : Noir,Title,size);
        }
    }
}

/*---------------------------------------------------------------------------\
  |        Draw a list box displaying the content of Entry                     |
  \---------------------------------------------------------------------------*/

void ListBox(float x1,float y1, float x2, float y2,const String::Vector &Entry,int Index, int Scroll, SKIN *skin, float size, uint32 flags )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    if( skin && skin->text_background.tex ) {
        gfx->set_alpha_blending();
        gfx->set_color( 0xFFFFFFFF );

        if( !(flags & FLAG_NO_BORDER) )
            skin->text_background.draw( x1, y1, x2, y2 );

        int i;
        for( i = 0 ; i < Entry.size() ; i++ ) {
            int e = i+Scroll;
            if( e >= Entry.size() || gui_font.height() * (i+1) > y2-y1-skin->text_background.y1+skin->text_background.y2 ) break;		// If we are out break the loop
            if( e == Index ) {
                if( skin->selection_gfx.tex )
                    skin->selection_gfx.draw( x1 + skin->text_background.x1, y1 + skin->text_background.y1 + gui_font.height()*i, x2 + skin->text_background.x2, y1 + skin->text_background.y1+gui_font.height()*(i+1) );
                else
                    gfx->rectfill( x1 + skin->text_background.x1, y1 + skin->text_background.y1 + gui_font.height()*i, x2 + skin->text_background.x2, y1 + skin->text_background.y1+gui_font.height()*(i+1), Bleu );
            }
            String str = Entry[ e ];
            while( gui_font.length( str ) >= x2 - x1 - skin->text_background.x1 + skin->text_background.x2 - skin->scroll[0].sw && str.size() > 0 )
                str.resize( str.size() - 1 );
            gfx->print(gui_font,x1+skin->text_background.x1,y1+skin->text_background.y1+gui_font.height()*i,0.0f,use_normal_alpha_function ? Blanc : Noir,str);
        }

        int TotalScroll = Entry.size() - (int)( (y2 - y1 - skin->text_background.y1 + skin->text_background.y2) / gui_font.height() );
        if( TotalScroll < 0 )	TotalScroll = 0;

        ScrollBar(	x2 + skin->text_background.x2 - skin->scroll[ 0 ].sw, y1 + skin->text_background.y1,
                    x2 + skin->text_background.x2, y2 + skin->text_background.y2,
                    TotalScroll ? ((float)Scroll) / TotalScroll : 0.0f, true, skin, size );

        gfx->unset_alpha_blending();
    }
    else {
        if( !(flags & FLAG_NO_BORDER) ) {
            gfx->rectfill(x1,y1,x2,y2,Blanc);
            gfx->rect(x1,y1,x2,y2,GrisF);
            gfx->rect(x1+1,y1+1,x2-1,y2-1,Noir);
            gfx->line(x1,y2,x2,y2,GrisC);
            gfx->line(x2,y1,x2,y2,GrisC);
            gfx->line(x1+1,y2-1,x2-1,y2-1,Blanc);
            gfx->line(x2-1,y1+1,x2-1,y2-1,Blanc);
        }

        int i;
        for(i=0;i<Entry.size();i++) {
            int e = i+Scroll;
            if( e >= Entry.size() || gui_font.height() * (i+1) > y2-y1-8 ) break;		// If we are out break the loop
            if( e == Index )
                gfx->rectfill( x1+4, y1+4+gui_font.height()*i, x2-4, y1+4+gui_font.height()*(i+1), Bleu );
            gfx->print(gui_font,x1+4,y1+4+gui_font.height()*i,0.0f,use_normal_alpha_function ? Blanc : Noir,Entry[e]);
        }
    }
    gui_font.change_size( old_size );
}

/*---------------------------------------------------------------------------\
  |        Draw a the given text within the given space                        |
  \---------------------------------------------------------------------------*/

int draw_text_adjust( float x1, float y1, float x2, float y2, String msg, float size, int pos, bool mission_mode )
{
    String current = "";
    String current_word = "";
    std::vector< String > Entry;
    int last = 0;
    for( int i = 0 ; i < msg.length() ; i++ )
        if( msg[i] == '\r' )	continue;
        else if( msg[i] == '\n' || gui_font.length( current + ' ' + current_word + msg[i] ) * size >= x2 - x1 ) {
            bool line_too_long = true;
            if( gui_font.length( current + ' ' + current_word + msg[i] ) * size < x2 - x1 ) {
                current += ' ' + current_word;
                current_word.clear();
                line_too_long = false;
            }
            else if( msg[i] != '\n' )
                current_word += msg[i];
            Entry.push_back( current );
            last = i + 1;
            current.clear();
            if( msg[ i ] == '\n' && line_too_long ) {
                Entry.push_back( current_word );
                current_word.clear();
            }
        }
        else {
            if( msg[i] == ' ' ) {
                if( !current.empty() )
                    current += ' ';
                current += current_word;
                current_word.clear();
            }
            else
                current_word += msg[i];
        }

    current += current_word;

    if( last + 1 < msg.length() && !current.empty() )
        Entry.push_back( current );

    gfx->set_alpha_blending();
    gfx->set_color( 1.0f, 1.0f, 1.0f, 1.0f );

    if( mission_mode ) {
        int	old_format = get_uformat();
        set_uformat( U_ASCII );

        uint32	current_color = 0xFFFFFFFF;
        char tmp[2];
        tmp[1] = 0;
        for( int e = pos ; e < Entry.size() ; e++ )
            if( y1 + gui_font.height() * size * (e + 1 - pos) <= y2 ) {
                float x_offset = 0.0f;
                for( int i = 0 ; i < Entry[e].size() ; i++ )
                    if( Entry[e][i] == '&' ) {
                        current_color = 0xFFFFFFFF;									// Default: white
                        if( i + 1 < Entry[e].size() && Entry[e][i+1] == 'R' ) {
                            current_color = 0xFF0000FF;								// Red
                            i++;
                        }
                        else if( i + 1 < Entry[e].size() && Entry[e][i+1] == 'Y' ) {
                            current_color = 0xFF00FFFF;								// Yellow
                            i++;
                        }
                    }
                    else {
                        tmp[0] = Entry[e][i];
                        gfx->print( gui_font, x1 + x_offset, y1 + gui_font.height() * size * (e - pos), 0.0f, current_color, tmp, size );
                        x_offset += gui_font.length( tmp ) * size;
                    }
            }
        set_uformat( old_format );
    }
    else {
        for( int e = pos ; e < Entry.size() ; e++ )
            if( y1 + gui_font.height() * size * (e + 1 - pos) <= y2 )
                gfx->print( gui_font, x1, y1 + gui_font.height() * size * (e - pos), 0.0f, use_normal_alpha_function ? Blanc : Noir, Entry[e], size );
    }

    gfx->unset_alpha_blending();

    return Entry.size();
}

/*---------------------------------------------------------------------------\
  |        Draw a popup menu displaying the text msg using the skin object     |
  \---------------------------------------------------------------------------*/

void PopupMenu( float x1, float y1, const String &msg, SKIN *skin, float size )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    float x2 = x1;
    std::vector< String > Entry;
    int last = 0;
    for( int i = 0 ; i < msg.length() ; i++ )
        if( msg[i] == '\n' ) {
            Entry.push_back( msg.substr( last, i - last ) );
            x2 = max( x2, x1 + gui_font.length( Entry.back() ) );
            last = i+1;
        }
    if( last + 1 < msg.length() ) {
        Entry.push_back( msg.substr( last, msg.length() - last ) );
        x2 = max( x2, x1 + gui_font.length( Entry.back() ) );
    }

    if( skin && skin->menu_background.tex ) {
        x2 += skin->menu_background.x1 - skin->menu_background.x2;
        float y2 = y1+skin->menu_background.y1-skin->menu_background.y2+gui_font.height()*Entry.size();
        if( x2 >= SCREEN_W ) {
            x1 += SCREEN_W - x2 - 1;
            x2 = SCREEN_W - 1;
        }
        if( y2 >= SCREEN_H ) {
            y1 += SCREEN_H - y2 - 1;
            y2 = SCREEN_H - 1;
        }
        gfx->set_alpha_blending();
        gfx->set_color( 1.0f, 1.0f, 1.0f, 1.0f );

        skin->menu_background.draw( x1, y1, x2, y2 );

        for( int e = 0 ; e < Entry.size() ; e++ )
            gfx->print(gui_font,x1+skin->menu_background.x1,y1+skin->menu_background.y1+gui_font.height()*e,0.0f,use_normal_alpha_function ? Blanc : Noir,Entry[e]);

        gfx->unset_alpha_blending();
    }
    else {
        x2 += 8;
        float y2 = y1+8+gui_font.height()*Entry.size();
        if( x2 >= SCREEN_W || y2 >= SCREEN_H ) {
            float x = x1, y = y1;
            x1 = x1 - (x2 - x1);
            y1 = y1 - (y2 - y1);
            x2 = x;
            y2 = y;
        }
        gfx->rectfill(x1,y1,x2,y2,GrisM);
        gfx->rect(x1,y1,x2,y2,Noir);
        gfx->rect(x1+1,y1+1,x2-1,y2-1,GrisF);
        gfx->line(x1,y1,x1,y2,Blanc);
        gfx->line(x1,y1,x2,y1,Blanc);
        gfx->line(x1+1,y1+1,x1+1,y2-1,GrisC);
        gfx->line(x1+1,y1+1,x2-1,y1+1,GrisC);

        for( int e = 0 ; e < Entry.size() ; e++ )
            gfx->print(gui_font,x1+4,y1+4+gui_font.height()*e,0.0f,use_normal_alpha_function ? Blanc : Noir,Entry[e]);
    }
    Entry.clear();

    gui_font.change_size( old_size );
}

/*---------------------------------------------------------------------------\
  |        Dessine un menu flotant avec les paramètres de Entry[]              |
  \---------------------------------------------------------------------------*/

void FloatMenu( float x, float y, const String::Vector &Entry, int Index, int StartEntry, SKIN *skin, float size )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    if( skin && skin->menu_background.tex ) {
        gfx->set_alpha_blending();
        gfx->set_color( 1.0f, 1.0f, 1.0f, 1.0f );

        int i;
        float width = 168.0f * size;
        for( i=0 ; i<Entry.size() - StartEntry ; i++ )
            width = max( width, gui_font.length( Entry[ i ] ) );

        width += skin->menu_background.x1-skin->menu_background.x2;

        skin->menu_background.draw( x, y, x + width, y+skin->menu_background.y1-skin->menu_background.y2+gui_font.height()*(Entry.size() - StartEntry) );

        for( i=0 ; i<Entry.size() - StartEntry ; i++ ) {
            int e = i + StartEntry;
            if( e == Index ) {
                if( skin->selection_gfx.tex )
                    skin->selection_gfx.draw( x+skin->menu_background.x1,y+skin->menu_background.y1+gui_font.height()*i,x + width + skin->menu_background.x2,y+skin->menu_background.y1+gui_font.height()*(i+1) );
                else
                    gfx->rectfill( x+skin->menu_background.x1,y+skin->menu_background.y1+gui_font.height()*i,x + width + skin->menu_background.x2,y+skin->menu_background.y1+gui_font.height()*(i+1), Bleu );
            }
            gfx->print(gui_font,x+skin->menu_background.x1,y+skin->menu_background.y1+gui_font.height()*i,0.0f,use_normal_alpha_function ? Blanc : Noir,Entry[e]);
        }

        gfx->unset_alpha_blending();
    }
    else {
        gfx->rectfill(x,y,x+168,y+8+gui_font.height()*(Entry.size() - StartEntry),GrisM);
        gfx->rect(x,y,x+168,y+8+gui_font.height()*(Entry.size() - StartEntry),Noir);
        gfx->rect(x+1,y+1,x+167,y+7+gui_font.height()*(Entry.size() - StartEntry),GrisF);
        gfx->line(x,y,x,y+8+gui_font.height()*(Entry.size() - StartEntry),Blanc);
        gfx->line(x,y,x+168,y,Blanc);
        gfx->line(x+1,y+1,x+1,y+7+gui_font.height()*(Entry.size() - StartEntry),GrisC);
        gfx->line(x+1,y+1,x+167,y+1,GrisC);

        int i;
        for( i=0 ; i<Entry.size() - StartEntry ; i++ ) {
            int e = i + StartEntry;
            if( e == Index )
                gfx->rectfill( x+4,y+4+gui_font.height()*i,x+164,y+4+gui_font.height()*(i+1), Bleu );
            gfx->print(gui_font,x+4,y+4+gui_font.height()*i,0.0f,use_normal_alpha_function ? Blanc : Noir,Entry[e]);
        }
    }

    gui_font.change_size( old_size );
}

/*---------------------------------------------------------------------------\
  |        Dessine un boutton d'option avec le texte Title                     |
  \---------------------------------------------------------------------------*/

void OptionButton(float x,float y,const String &Title,bool Etat, SKIN *skin, float size )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    if( skin && skin->option[0].tex && skin->option[1].tex ) {
        gfx->set_alpha_blending();
        gfx->set_color( 1.0f, 1.0f, 1.0f, 1.0f );

        skin->option[ Etat ? 1 : 0 ].draw( x, y, x + skin->option[ Etat ? 1 : 0 ].sw, y + skin->option[ Etat ? 1 : 0 ].sh );

        gfx->print(gui_font, x + skin->option[ Etat ? 1 : 0 ].sw + 4.0f * size, y + ( skin->option[ Etat ? 1 : 0 ].sh - gui_font.height() ) * 0.5f,0.0f,use_normal_alpha_function ? Blanc : Noir,Title);

        gfx->unset_alpha_blending();
    }
    else {
        int x1,y1;

        gfx->circlefill(x+6,y+6,6,Blanc);
        gfx->circle(x+6,y+6,6,GrisM);
        gfx->circle(x+6,y+6,5,GrisF);

        // Effet d'ombrage
        for(y1=(int)y ; y1<=y+12 ; y1++)
            for(x1=(int)x ; x1<=x+12 ; x1++)
                if((x1-x-6)*(x1-x-6)+(y1-y-6)*(y1-y-6)<49)
                    if(gfx->getpixel(x1,y1)==GrisF&&x1-x+y1-y-6>6)
                        gfx->putpixel(x1,y1,GrisC);

        if(Etat) {
            gfx->putpixel(x+6,y+6,RougeF);
            gfx->circle(x+6,y+6,1,RougeF);
            gfx->circle(x+6,y+6,2,RougeF);
            gfx->circle(x+6,y+6,3,RougeF);
            gfx->circlefill(x+6,y+6,2,RougeF);
            gfx->putpixel(x+6,y+6,Rouge);
            gfx->circle(x+6,y+6,1,Rouge);
            gfx->circle(x+6,y+6,2,Rouge);
            gfx->circlefill(x+6,y+6,2,Rouge);
        }

        gfx->print(gui_font,x+16,y+3,0.0f,use_normal_alpha_function ? Blanc : Noir,Title);
    }

    gui_font.change_size( old_size );
}

/*---------------------------------------------------------------------------\
  |        Dessine une case à cocher avec le texte Title                       |
  \---------------------------------------------------------------------------*/

void OptionCase(float x,float y,const String &Title,bool Etat, SKIN *skin, float size )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    if( skin && skin->checkbox[0].tex && skin->checkbox[1].tex ) {
        gfx->set_alpha_blending();
        gfx->set_color( 1.0f, 1.0f, 1.0f, 1.0f );

        skin->checkbox[ Etat ? 1 : 0 ].draw( x, y, x + skin->checkbox[ Etat ? 1 : 0 ].sw, y + skin->checkbox[ Etat ? 1 : 0 ].sh );

        gfx->print(gui_font, x + skin->checkbox[ Etat ? 1 : 0 ].sw + 4.0f * size, y + ( skin->checkbox[ Etat ? 1 : 0 ].sh - gui_font.height() ) * 0.5f,0.0f,use_normal_alpha_function ? Blanc : Noir,Title);

        gfx->unset_alpha_blending();
    }
    else {
        gfx->rectfill(x,y,x+12,y+12,Blanc);
        gfx->rect(x,y,x+12,y+12,GrisF);
        gfx->rect(x+1,y+1,x+11,y+11,Noir);
        gfx->line(x,y+12,x+12,y+12,GrisC);
        gfx->line(x+12,y,x+12,y+12,GrisC);
        gfx->line(x+1,y+11,x+11,y+11,Blanc);
        gfx->line(x+11,y+1,x+11,y+11,Blanc);

        if(Etat) {
            gfx->line(x+3,y+6,x+5,y+9,GrisF);
            gfx->line(x+5,y+9,x+8,y+4,GrisF);
            gfx->line(x+3,y+5,x+5,y+8,GrisF);
            gfx->line(x+5,y+8,x+8,y+3,GrisF);
        }

        gfx->print(gui_font,x+16,y+3,0.0f,use_normal_alpha_function ? Blanc : Noir,Title);
    }

    gui_font.change_size( old_size );
}

/*---------------------------------------------------------------------------\
  |        Dessine une barre d'entrée de texte utilisateur                     |
  \---------------------------------------------------------------------------*/

void TextBar(float x1,float y1,float x2,float y2,const String &Caption,bool Etat, SKIN *skin, float size )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    if( skin && skin->text_background.tex ) {
        gfx->set_alpha_blending();
        gfx->set_color( 0xFFFFFFFF );

        skin->text_background.draw( x1, y1, x2, y2 );

        float maxlength = x2 - x1 + skin->text_background.x2 - skin->text_background.x1 - gui_font.length( "_" );
        int dec = 0;
        String strtoprint = Caption.substr( dec, Caption.length() - dec );
        while( gui_font.length( Caption.substr( dec, Caption.length() - dec ) ) >= maxlength && dec < Caption.length() ) {
            dec++;
            strtoprint = Caption.substr( dec, Caption.length() - dec );
        }

        gfx->print(gui_font,x1+skin->text_background.x1,y1+skin->text_background.y1+skin->text_y_offset,0.0f,use_normal_alpha_function ? Blanc : Noir,strtoprint);
		if(Etat) gfx->print(gui_font,x1+skin->text_background.x1+gui_font.length( strtoprint ),y1+skin->text_background.y1+skin->text_y_offset,0.0f,use_normal_alpha_function ? Blanc : Noir,"_");

        gfx->unset_alpha_blending();
    }
    else {
        gfx->rectfill(x1,y1,x2,y2,Blanc);
        gfx->rect(x1,y1,x2,y2,GrisF);
        gfx->rect(x1+1,y1+1,x2-1,y2-1,Noir);
        gfx->line(x1,y2,x2,y2,GrisC);
        gfx->line(x2,y1,x2,y2,GrisC);
        gfx->line(x1+1,y2-1,x2-1,y2-1,Blanc);
        gfx->line(x2-1,y1+1,x2-1,y2-1,Blanc);

        float maxlength = x2 - x1 - 8 - gui_font.length( "_" );
        int dec = 0;
        String strtoprint = Caption.substr( dec, Caption.length() - dec );
        while( gui_font.length( Caption.substr( dec, Caption.length() - dec ) ) >= maxlength && dec < Caption.length() ) {
            dec++;
            strtoprint = Caption.substr( dec, Caption.length() - dec );
        }

        gfx->print(gui_font,x1+4,y1+4,0.0f,use_normal_alpha_function ? Blanc : Noir,Caption);
        if(Etat) gfx->print(gui_font,x1+4+gui_font.length( Caption ),y1+4,0.0f,use_normal_alpha_function ? Blanc : Noir,"_");
    }

    gui_font.change_size( old_size );
}

/*---------------------------------------------------------------------------\
  |                              Draw a scroll bar                             |
  \---------------------------------------------------------------------------*/
void ScrollBar( float x1, float y1, float x2, float y2, float Value, bool vertical, SKIN *skin, float size )
{
    if( skin ) {
        gfx->set_alpha_blending();
        gfx->set_color( 0xFFFFFFFF );

        if( Value < 0.0f )	Value = 0.0f;
        else if( Value > 1.0f )	Value = 1.0f;
        skin->scroll[ vertical ? 0 : 1 ].draw( x1, y1, x2, y2 );

        if( vertical ) {
            float y = y1 + skin->scroll[ 0 ].y1;
            float dx = x2 - x1 - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2;
            y += (y2 - y1 - skin->scroll[ 0 ].y1 + skin->scroll[ 0 ].y2 - dx) * Value;
            skin->scroll[ 2 ].draw( x1 + skin->scroll[ 0 ].x1, y, x2 + skin->scroll[ 0 ].x2, y + dx );
        }
        else {
            float x = x1 + skin->scroll[ 1 ].x1;
            float dy = y2 - y1 - skin->scroll[ 1 ].y1 + skin->scroll[ 1 ].y2;
            x += (x2 - x1 - skin->scroll[ 1 ].x1 + skin->scroll[ 1 ].x2 - dy) * Value;
            skin->scroll[ 2 ].draw( x, y1 + skin->scroll[ 1 ].y1, x + dy, y2 + skin->scroll[ 1 ].y2 );
        }

        gfx->unset_alpha_blending();
    }
}

/*---------------------------------------------------------------------------\
  |                     Dessine une barre de progression                       |
  \---------------------------------------------------------------------------*/

void ProgressBar(float x1,float y1,float x2,float y2,int Value, SKIN *skin, float size )
{
    float old_size = gui_font.get_size();
    gui_font.change_size( size );

    if( skin && skin->progress_bar[0].tex && skin->progress_bar[1].tex ) {			// If we have a skin loaded with gfx for the progress bar
        gfx->set_alpha_blending();
        gfx->set_color( 0xFFFFFFFF );
        skin->progress_bar[0].draw( x1, y1, x2, y2 );
        skin->progress_bar[1].draw( x1 + skin->progress_bar[0].x1, y1 + skin->progress_bar[0].y1, x1 + skin->progress_bar[0].x1 + (skin->progress_bar[0].x2 + x2 - x1 - skin->progress_bar[0].x1) * Value * 0.01f, y2 + skin->progress_bar[0].y2 );			// Draw the bar

        String Buf = format("%d", Value) + "%%";

        gfx->print(gui_font,(x1+x2)*0.5f-gui_font.length( Buf ) * 0.5f,(y1+y2)*0.5f-gui_font.height()*0.5f,0.0f,use_normal_alpha_function ? Blanc : Noir,Buf);

        gfx->unset_alpha_blending();
    }
    else {
        gfx->rectfill(x1,y1,x2,y2,Blanc);
        gfx->rectfill(x1+2,y1,x1+1+(x2-x1-3)*Value/100,y2,Bleu);
        gfx->rect(x1,y1,x2,y2,GrisF);
        gfx->rect(x1+1,y1+1,x2-1,y2-1,Noir);
        gfx->line(x1,y2,x2,y2,GrisC);
        gfx->line(x2,y1,x2,y2,GrisC);
        gfx->line(x1+1,y2-1,x2-1,y2-1,Blanc);
        gfx->line(x2-1,y1+1,x2-1,y2-1,Blanc);

        String Buf = format("%d", Value) + "%%";

        gfx->print(gui_font,(x1+x2)/2-gui_font.length( Buf ) * 0.5f,(y1+y2)*0.5f-gui_font.height()*0.5f,0.0f,use_normal_alpha_function ? Blanc : Noir,Buf);
    }

    gui_font.change_size( old_size );
}

// Renvoie le dossier contenant le fichier fname
String dirname(String fname)
{
    int last=0;
    for(int i=0;i<fname.length();i++) {
        if(fname[i]=='\\')
            fname[i]='/';
        if(fname[i]=='/')
            last=i;
    }
    fname.resize(last);
    if(fname.length()==0)	fname="/";

    return fname;
}

/*---------------------------------------------------------------------------\
  |               Affiche une fenêtre de selection de fichier                  |
  \---------------------------------------------------------------------------*/

const String Dialog(const String &Title, String Filter)
{

    set_uformat(U_UTF8);

    wnd WAsk;               //Cree un objet fenetre

    String Name = "";
    byte key=0;
    int AMouseX,AMouseY,AMouseB,AMouseZ;

    WAsk.Title=Title;       //Defini les proprietes de la fenetre
    WAsk.width=500;
    WAsk.height=400;
    WAsk.x=SCREEN_W-WAsk.width>>1;
    WAsk.y=SCREEN_H-WAsk.height>>1;

    GLuint Img = gfx->make_texture_from_screen();

    //-----  Pour la gestion de l'aborescence des dossiers  ----------------------

    int DecF=0,DecD=0;

    String CurDir = getcwd(NULL,1000);
    // Pour l'arborescence des fichiers
    al_ffblk f;
    String::List Files;
    String::List Dir;
    int NbFiles=0,NbDir=0;
    int i,done,e;

    // Détecte les fichiers/dossiers

detect:

    Files.clear();
    Dir.clear();

    done=al_findfirst((CurDir+"/*.*").c_str(),&f,FA_LABEL|FA_HIDDEN|FA_SYSTEM|FA_DIREC);
    if(done==0) {
        NbDir=0;
        while(done==0) {
            if((f.attrib&FA_DIREC)==FA_DIREC) {
                NbDir++;
                Dir.push_back(f.name);
            }
            done=al_findnext(&f);
        }
    }
    else { NbDir=0; }

    done=al_findfirst((CurDir+"/"+Filter).c_str(),&f,FA_LABEL|FA_HIDDEN|FA_SYSTEM|FA_ARCH);
    if(done==0) {
        NbFiles=0;
        while(done==0) {
            NbFiles++;
            Files.push_back(f.name);
            done=al_findnext(&f);
        }
    }
    else { NbFiles=0; }

    // Tri des fichiers par ordre alphabétique
    Files.sort();

    // Tri des dossiers par ordre alphabétique
    Dir.sort();

    done=0;

    //----------------------------------------------------------------------------

    DecD=DecF=0;

    do
    {

        poll_keyboard();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

        gfx->set_2D_mode();	// On repasse dans le mode dessin 2D pour Allegro

        gfx->drawtexture(Img, 0, SCREEN_H-1, SCREEN_W-1, 0);
        glDisable(GL_TEXTURE_2D);

        //-------  Gestion de la souris  ---------------------------------------------

        AMouseX=mouse_x;
        AMouseY=mouse_y;
        AMouseZ=mouse_z;
        AMouseB=mouse_b;
        poll_mouse();

        //-------  Fin du code de gestion de la souris  ------------------------------

        key=0;
        if(keypressed())
            key=readkey();

        if(key==8 && Name.length()>0) Name.resize(Name.length()-1);
        if(key==8) key=0;
        if(key!=0 && key!=27 && key!=13 && Name.length()*8<WAsk.width-154) Name+=key;
        if(key==27||key==13) done=1;
        if(Name.length()>(WAsk.width-24>>3)) Name.resize(Name.length()-1);

        draw_Window(WAsk);        // Affiche la fenêtre

        // Objets de la fenêtre
        // Barre nom du fichier
        gfx->print(gui_font,WAsk.x+5,WAsk.y+19,0.0f,Noir, I18N::Translate("Nom du fichier:"));
        TextBar(WAsk.x+125,WAsk.y+16,WAsk.x+WAsk.width-5,WAsk.y+30,(char*)Name.c_str(),(retrace_count%60)>30);

        // Liste des dossiers
        gfx->print(gui_font,WAsk.x+21,WAsk.y+40,0.0f,Noir, I18N::Translate("Liste des dossiers:"));
        TextBar(WAsk.x+5,WAsk.y+50,WAsk.x+190,WAsk.y+WAsk.height-10,"",false);
        // Affiche le nom des dossiers
        if (NbDir > 0)
        {
            int f=0;
            for (String::List::const_iterator e = Dir.begin(); e != Dir.end(); ++e)
            {
                i = f - DecD;
                ++f;
                if (i >= 41)
                    break;
                if (i >= 0)
                {
                    gfx->print(gui_font, WAsk.x + 20, WAsk.y + 56 + 8 * i, 0.0f, Noir, *e);
                    gfx->line(WAsk.x + 10, WAsk.y + 53 + 8 * i, WAsk.x + 10, WAsk.y + 60 + i * 8, Noir);
                    gfx->line(WAsk.x + 10, WAsk.y + 60 + 8 * i, WAsk.x + 17, WAsk.y + 60 + i * 8, Noir);
                }
            }
        }
        if (NbDir > 0 && mouse_b == 1 && mouse_x >= WAsk.x + 7 && mouse_x <= WAsk.x + 188
           && mouse_y >= WAsk.y + 56 && mouse_y <= WAsk.y + WAsk.height - 10)
        {
            i = (mouse_y - WAsk.y - 56) / 8;
            e = i + DecD;
            String::List::iterator curdir=Dir.begin();
            for (int f = 0; f < e; ++f)
                ++curdir;
            if (i < 41 && e >= 0 && e < NbDir)
            {
                if (strcmp(curdir->c_str(), ".") != 0 && strcmp(curdir->c_str(), "..") != 0)
                    CurDir << "/" << *curdir;
                else
                    CurDir = dirname(CurDir);
                while (mouse_b != 0)
                    poll_mouse();
                gfx->unset_2D_mode();
                goto detect;
            }
        }

        // Liste des fichiers
        gfx->print(gui_font,WAsk.x+226,WAsk.y+40,0.0f,Noir, I18N::Translate("Liste des fichiers:"));
        TextBar(WAsk.x+210,WAsk.y+50,WAsk.x+480,WAsk.y+WAsk.height-10,"",false);
        // Affiche le nom des fichiers
        if (NbFiles > 0)
        {
            int f=0;
            for (String::List::const_iterator e = Files.begin(); e != Files.end(); ++e)
            {
                i = f - DecF;
                ++f;
                if (i >= 41)
                    break;
                if (i >= 0)
                {
                    gfx->print(gui_font,WAsk.x+225,WAsk.y+56+8*i,0.0f,Noir,*e);
                    gfx->line(WAsk.x+215,WAsk.y+53+8*i,WAsk.x+215,WAsk.y+60+i*8,Noir);
                    gfx->line(WAsk.x+215,WAsk.y+60+8*i,WAsk.x+222,WAsk.y+60+i*8,Noir);
                }
            }
        }
        if(NbFiles>0&&mouse_x>=WAsk.x+212&&mouse_x<=WAsk.x+478
           &&mouse_y>=WAsk.y+56&&mouse_y<=WAsk.y+WAsk.height-10)
        {
            i=(mouse_y-WAsk.y-56)/8;
            e=i+DecF;
            String::List::iterator curfile=Files.begin();
            for (int f = 0; f < e; ++f)
                ++curfile;
            if (mouse_b == 1 && i < 41 && e >= 0 && e < NbFiles)
                Name = *curfile;
        }

        // Barres de défilement
        if(mouse_x>=WAsk.x+210 && mouse_x<=WAsk.x+480 && mouse_y>=WAsk.y+50 && mouse_y<=WAsk.y+WAsk.height-10)
        {
            DecF+=AMouseZ-mouse_z;
            if(DecF>NbFiles-40)	DecF=NbFiles-40;
            if(DecF<0)	DecF=0;
        }
        if(mouse_x>=WAsk.x+5 && mouse_x<=WAsk.x+190 && mouse_y>=WAsk.y+50 && mouse_y<=WAsk.y+WAsk.height-10)
        {
            DecD+=AMouseZ-mouse_z;
            if(DecD>NbDir-40)	DecD=NbDir-40;
            if(DecD<0)	DecD=0;
        }
        // Fichiers
        gfx->rectfill(WAsk.x+481,WAsk.y+50,WAsk.x+493,WAsk.y+WAsk.height-10,0xFF9F9F9F);
        // Haut
        if(mouse_b==1&&mouse_x>=WAsk.x+481&&mouse_x<=WAsk.x+493
           &&mouse_y>=WAsk.y+50&&mouse_y<=WAsk.y+62)
        {
            DecF--;
            if(DecF<0) DecF=0;
            button(WAsk.x+481,WAsk.y+50,WAsk.x+493,WAsk.y+62,"",true);
        }
        else
            button(WAsk.x+481,WAsk.y+50,WAsk.x+493,WAsk.y+62,"",false);
        gfx->line(WAsk.x+484,WAsk.y+58,WAsk.x+487,WAsk.y+55,Noir);
        gfx->line(WAsk.x+484,WAsk.y+57,WAsk.x+487,WAsk.y+54,Noir);
        gfx->line(WAsk.x+490,WAsk.y+58,WAsk.x+487,WAsk.y+55,Noir);
        gfx->line(WAsk.x+490,WAsk.y+57,WAsk.x+487,WAsk.y+54,Noir);
        // Bas
        if(mouse_b==1&&mouse_x>=WAsk.x+481&&mouse_x<=WAsk.x+493
           &&mouse_y>=WAsk.y+WAsk.height-22&&mouse_y<=WAsk.y+WAsk.height-10) {
            DecF++;
            if(DecF>NbFiles-40) DecF=NbFiles-40;
            if(DecF<0) DecF=0;
            button(WAsk.x+481,WAsk.y+WAsk.height-22,WAsk.x+493,WAsk.y+WAsk.height-10,"",true);
        }
        else
            button(WAsk.x+481,WAsk.y+WAsk.height-22,WAsk.x+493,WAsk.y+WAsk.height-10,"",false);
        gfx->line(WAsk.x+484,WAsk.y+WAsk.height-18,WAsk.x+487,WAsk.y+WAsk.height-15,Noir);
        gfx->line(WAsk.x+484,WAsk.y+WAsk.height-17,WAsk.x+487,WAsk.y+WAsk.height-14,Noir);
        gfx->line(WAsk.x+490,WAsk.y+WAsk.height-18,WAsk.x+487,WAsk.y+WAsk.height-15,Noir);
        gfx->line(WAsk.x+490,WAsk.y+WAsk.height-17,WAsk.x+487,WAsk.y+WAsk.height-14,Noir);
        // Carré
        if(NbFiles!=0&&NbFiles>40)
            i=WAsk.y+63+DecF*(WAsk.height-97)/(NbFiles-39);
        else
            i=WAsk.y+63;
        e=i+12;
        button(WAsk.x+481,i,WAsk.x+493,e,"",false);
        if(mouse_b==1&&AMouseB==1&&NbFiles>40
           &&mouse_y!=AMouseY
           &&AMouseX>=WAsk.x+481&&AMouseX<=WAsk.x+493
           &&mouse_x>=WAsk.x+481&&mouse_x<=WAsk.x+493
           &&mouse_y>=WAsk.y+63&&mouse_y<WAsk.y+WAsk.height-22) {
            i=mouse_y-6;
            DecF=(i-WAsk.y-63)*(NbFiles-40)/(WAsk.height-97);
            if(DecF>NbFiles-40) DecF=NbFiles-40;
            if(DecF<0) DecF=0;
        }
        // Dossiers
        gfx->rectfill(WAsk.x+191,WAsk.y+50,WAsk.x+203,WAsk.y+WAsk.height-10,0xFF9F9F9F);
        // Haut
        if(mouse_b==1&&mouse_x>=WAsk.x+191&&mouse_x<=WAsk.x+203
           &&mouse_y>=WAsk.y+50&&mouse_y<=WAsk.y+62) {
            DecD--;
            if(DecD<0) DecD=0;
            button(WAsk.x+191,WAsk.y+50,WAsk.x+203,WAsk.y+62,"",true);
        }
        else
            button(WAsk.x+191,WAsk.y+50,WAsk.x+203,WAsk.y+62,"",false);
        gfx->line(WAsk.x+194,WAsk.y+58,WAsk.x+197,WAsk.y+55,Noir);
        gfx->line(WAsk.x+194,WAsk.y+57,WAsk.x+197,WAsk.y+54,Noir);
        gfx->line(WAsk.x+200,WAsk.y+58,WAsk.x+197,WAsk.y+55,Noir);
        gfx->line(WAsk.x+200,WAsk.y+57,WAsk.x+197,WAsk.y+54,Noir);
        // Bas
        if(mouse_b==1&&mouse_x>=WAsk.x+191&&mouse_x<=WAsk.x+203
           &&mouse_y>=WAsk.y+WAsk.height-22&&mouse_y<=WAsk.y+WAsk.height-10) {
            DecD++;
            if(DecD>NbDir-40) DecD=NbDir-40;
            if(DecD<0) DecD=0;
            button(WAsk.x+191,WAsk.y+WAsk.height-22,WAsk.x+203,WAsk.y+WAsk.height-10,"",true);
        }
        else
            button(WAsk.x+191,WAsk.y+WAsk.height-22,WAsk.x+203,WAsk.y+WAsk.height-10,"",false);
        gfx->line(WAsk.x+194,WAsk.y+WAsk.height-18,WAsk.x+197,WAsk.y+WAsk.height-15,Noir);
        gfx->line(WAsk.x+194,WAsk.y+WAsk.height-17,WAsk.x+197,WAsk.y+WAsk.height-14,Noir);
        gfx->line(WAsk.x+200,WAsk.y+WAsk.height-18,WAsk.x+197,WAsk.y+WAsk.height-15,Noir);
        gfx->line(WAsk.x+200,WAsk.y+WAsk.height-17,WAsk.x+197,WAsk.y+WAsk.height-14,Noir);
        // Carré
        if(NbDir!=0&&NbDir>40)
            i=WAsk.y+63+DecD*(WAsk.height-97)/(NbDir-39);
        else
            i=WAsk.y+63;
        e=i+12;
        button(WAsk.x+191,i,WAsk.x+203,e,"",false);
        if(mouse_b==1&&AMouseB==1&&NbDir>40
           &&mouse_y!=AMouseY
           &&AMouseX>=WAsk.x+191&&AMouseX<=WAsk.x+203
           &&mouse_x>=WAsk.x+191&&mouse_x<=WAsk.x+203
           &&mouse_y>=WAsk.y+63&&mouse_y<WAsk.y+WAsk.height-22) {
            i=mouse_y-6;
            DecD=(i-WAsk.y-63)*(NbDir-40)/(WAsk.height-97);
            if(DecD>NbDir-40) DecD=NbDir-40;
            if(DecD<0) DecD=0;
        }

        show_mouse(screen);
        algl_draw_mouse();
        show_mouse(NULL);

        gfx->unset_2D_mode();	// On repasse dans le mode précédent

        gfx->flip();

    }while(done==0);

    if(key==27) Name = "";
    if(key==13 && strrchr(Name.c_str(),'*')!=NULL) {
        Filter = Name;
        gfx->unset_2D_mode();
        goto detect;
    }

    gfx->destroy_texture(Img);

    while(keypressed())	{
        rest(1);
        poll_keyboard();
        readkey();
    }

    reset_keyboard();

    return strdup((CurDir+"/"+Name).c_str());
}

/*---------------------------------------------------------------------------\
  |               Affiche une fenêtre de demande de type oui/non               |
  \---------------------------------------------------------------------------*/

bool WndAsk(const String &Title,const String &Msg,int ASW_TYPE)
{
    WND Popup;

    Popup.width = Msg.length()*8>300 ? Msg.length()*8+20: 320;
    Popup.height=60;
    Popup.x = SCREEN_W-Popup.width>>1;	Popup.y=SCREEN_H-Popup.height>>1;
    Popup.Lock=false;
    Popup.Title=Title;
    Popup.NbObj=3;
    Popup.Objets = new GUIOBJ[Popup.NbObj];
    // Création des objets de la fenêtre
    // Message
    Popup.Objets[0].create_text(Popup.width-Msg.length()*8>>1,20,Msg);
    // Boutons OK/Oui et Annuler/Non
    if(ASW_TYPE==ASW_OKCANCEL){
        Popup.Objets[1].create_button(Popup.width/3-16,36,Popup.width/3+16,52,"OK",NULL);
        Popup.Objets[2].create_button(Popup.width*2/3-36,36,Popup.width*2/3+36,52,"Annuler",NULL);
    }
    else {
        Popup.Objets[1].create_button(Popup.width/3-20,36,Popup.width/3+20,52,"Oui",NULL);
        Popup.Objets[2].create_button(Popup.width*2/3-20,36,Popup.width*2/3+20,52,"Non",NULL);
    }

    int Popup_Done=0;

    int AMx=mouse_x,AMy=mouse_y,AMz=mouse_z,AMb=mouse_b;

    do
    {
        poll_keyboard();

        if(Popup.Objets[1].Etat) Popup_Done=1;
        if(Popup.Objets[2].Etat) Popup_Done=2;

        AMx=mouse_x;							// Mémorise l'ancien état de la souris
        AMy=mouse_y;
        AMz=mouse_z;
        AMb=mouse_b;

        poll_mouse();								// Obtient l'état de la souris

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

        gfx->set_2D_mode();	// On repasse dans le mode dessin 2D pour Allegro

        Popup.check(AMx,AMy,AMz,AMb);	// Gestion de l'interface utilisateur graphique

        String help_msg = "";
        Popup.draw( help_msg );		// Dessine la boîte de dialogue

        glEnable( GL_TEXTURE_2D );
        show_mouse( screen );
        algl_draw_mouse();			// Dessine le curseur

        gfx->unset_2D_mode();	// On repasse dans le mode précédent

        gfx->flip();

    }while(Popup_Done==0);

    reset_mouse();
    reset_keyboard();

    return Popup_Done==1;
}

/*---------------------------------------------------------------------------\
  |               Affiche une fenêtre Popup d'affichage d'infos                |
  \---------------------------------------------------------------------------*/

void Popup(const String &Title,const String &Msg)
{
    WND Popup;

    Popup.width=Msg.length()*8>300 ? Msg.length()*8+20: 320;
    Popup.height=60;
    Popup.x = SCREEN_W-Popup.width>>1;	Popup.y=SCREEN_H-Popup.height>>1;
    Popup.Lock=false;
    Popup.Title=Title;
    Popup.NbObj=2;
    Popup.Objets = new GUIOBJ[Popup.NbObj];
    // Création des objets de la fenêtre
    // Message
    Popup.Objets[0].create_text(Popup.width-Msg.length()*8>>1,20,Msg);
    // Boutons OK
    Popup.Objets[1].create_button(Popup.width/2-16,36,Popup.width/2+16,52,"OK",NULL);

    bool Popup_Done=false;

    int AMx=mouse_x,AMy=mouse_y,AMz=mouse_z,AMb=mouse_b;

    do
    {
        poll_keyboard();

        if(Popup.Objets[1].Etat) Popup_Done=true;

        AMx=mouse_x;							// Mémorise l'ancien état de la souris
        AMy=mouse_y;
        AMz=mouse_z;
        AMb=mouse_b;

        poll_mouse();								// Obtient l'état de la souris

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

        gfx->set_2D_mode();	// On repasse dans le mode dessin 2D pour Allegro

        Popup.check(AMx,AMy,AMz,AMb);	// Gestion de l'interface utilisateur graphique

        String help_msg = "";
        Popup.draw( help_msg );		// Dessine la boîte de dialogue

        show_mouse(screen);
        algl_draw_mouse();			// Dessine le curseur
        show_mouse(NULL);

        gfx->unset_2D_mode();	// On repasse dans le mode précédent

        gfx->flip();

    }while(!Popup_Done);
    reset_keyboard();
}

/*---------------------------------------------------------------------------\
  |               Affiche une fenêtre de demande d'entrée utilisateur          |
  \---------------------------------------------------------------------------*/

const String GetVal(const String &Title)
{
    String Answ = "";

    WND Popup;

    Popup.width=320;
    Popup.height=60;
    Popup.x=SCREEN_W-Popup.width>>1;	Popup.y=SCREEN_H-Popup.height>>1;
    Popup.Lock=false;
    Popup.Title=Title;
    Popup.NbObj=2;
    Popup.Objets = new GUIOBJ[Popup.NbObj];
    // Création des objets de la fenêtre
    // Message
    Popup.Objets[0].create_textbar(10,20,310,34,Answ,100,NULL);
    // Boutons OK
    Popup.Objets[1].create_button(Popup.width/2-16,36,Popup.width/2+16,52,"OK",NULL);
    Popup.Objets[0].Focus=true;

    bool Popup_Done=false;

    int AMx=mouse_x,AMy=mouse_y,AMz=mouse_z,AMb=mouse_b;

    do
    {
        poll_mouse();
    }while(mouse_b!=0);

    do
    {
        poll_keyboard();

        if(Popup.Objets[1].Etat || Popup.Objets[0].Etat) Popup_Done=true;

        AMx=mouse_x;							// Mémorise l'ancien état de la souris
        AMy=mouse_y;
        AMz=mouse_z;
        AMb=mouse_b;

        poll_mouse();								// Obtient l'état de la souris

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

        gfx->set_2D_mode();	// On repasse dans le mode dessin 2D pour Allegro

        Popup.check(AMx,AMy,AMz,AMb);	// Gestion de l'interface utilisateur graphique

        String help_msg = "";
        Popup.draw( help_msg );		// Dessine la boîte de dialogue

        algl_draw_mouse();			// Dessine le curseur

        gfx->unset_2D_mode();	// On repasse dans le mode précédent

        gfx->flip();

    }while(!Popup_Done);

    reset_keyboard();

    Answ = Popup.Objets[ 0 ].Text[ 0 ];

    return Answ;
}

} // namespace TA3D
