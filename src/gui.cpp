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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "matrix.h"
#include "ta3dbase.h"

using namespace TA3D::EXCEPTION;

bool use_normal_alpha_function = false;
float gui_font_h = 8.0f;

void glbutton(const String &caption,float x1,float y1,float x2,float y2,bool etat)
{
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	if(!etat) {
		glColor3f(1.0f,1.0f,1.0f);	glVertex2f(x1,y1);			// Dessine les bords du bouton
		glColor3f(0.5f,0.5f,0.5f);	glVertex2f(x2,y1);
		glColor3f(0.0f,0.0f,0.0f);	glVertex2f(x2,y2);
		glColor3f(0.5f,0.5f,0.5f);	glVertex2f(x1,y2);
		}
	else {
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

const String msg_box(TA3D::INTERFACES::GFX_FONT fnt,const String &title,const String &msg,bool ask)
{
	gfx->set_2D_mode();
	for(int i=0;i<2;i++) {
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

TA3D::INTERFACES::GFX_FONT gui_font;

void GUIOBJ::create_ta_button( float X1, float Y1, const Vector< String > &Caption, const Vector< GLuint > &states, int nb_st)
{
	gltex_states.clear();
	gltex_states.resize( states.size() );
	for( int i = 0 ; i < states.size() ; i++ )			// Create the texture Vector
		gltex_states[ i ].set( states[ i ] );

	Type = OBJ_TA_BUTTON;
	x1 = X1;								y1 = Y1;
	if( gltex_states.size() > 0 ) {
		x2 = X1 + gltex_states[0].width;		y2 = Y1 + gltex_states[0].height;	}
	else {
		x2 = X1;	y2 = Y1;	}

	nb_stages = nb_st;
	Etat = false;	Focus = false;
	current_state = 0;
	Text = Caption;
	Func = NULL;
	s = 1.0f;
	Flag = FLAG_CAN_BE_CLICKED | FLAG_MULTI_STATE;
}
			// Dessine un rectangle en pointillés
void GUIOBJ::create_button( float X1,float Y1,float X2,float Y2,const String &Caption,void (*F)(int), float size)
{
	Type=OBJ_BUTTON;
	x1=X1;		y1=Y1;
	x2=X2;		y2=Y2;
	Etat=false;	Focus=false;
	Text.resize(1);
	Text[0] = Caption;
	Func=F;
	s=size;
	Flag = FLAG_CAN_BE_CLICKED;
}
			// Crée une case à cocher
void GUIOBJ::create_optionc(float X1,float Y1,const String &Caption,bool ETAT,void (*F)(int), SKIN *skin, float size)
{
	Type=OBJ_OPTIONC;
	x1=X1;													y1=Y1;
	x2=X1+(int)(gui_font.length( Caption )*size)+4;			y2=Y1;
	if( skin && skin->checkbox[0].tex && skin->checkbox[1].tex ) {
		x2 += max( skin->checkbox[0].sw, skin->checkbox[1].sw );
		y2 += max( skin->checkbox[0].sh, skin->checkbox[1].sh );
		}
	else {
		x2+=8;
		y2+=12;
		}
	Etat=ETAT;	
	Focus=false;
	Text.resize(1);
	Text[0] = Caption;
	Func=F;
	Flag = FLAG_SWITCH | FLAG_CAN_BE_CLICKED;
	s=size;
}
			// Crée un bouton d'option
void GUIOBJ::create_optionb(float X1,float Y1,const String &Caption,bool ETAT,void (*F)(int), SKIN *skin, float size)
{
	Type=OBJ_OPTIONB;
	x1=X1;												y1=Y1;
	x2=X1+(int)(gui_font.length( Caption )*size)+4;		y2=Y1;
	if( skin && skin->option[0].tex && skin->option[1].tex ) {
		x2 += max( skin->option[0].sw, skin->option[1].sw );
		y2 += max( skin->option[0].sh, skin->option[1].sh );
		}
	else {
		x2+=8;
		y2+=12;
		}
	Etat=ETAT;	
	Focus=false;
	Text.resize(1);
	Text[0] = Caption;
	Func=F;
	Flag = FLAG_SWITCH | FLAG_CAN_BE_CLICKED;
	s=size;
}
			// Crée une barre d'entrée de texte
void GUIOBJ::create_textbar(float X1,float Y1,float X2,float Y2,const String &Caption,int MaxChar, void(*F)(int), float size)
{
	Type=OBJ_TEXTBAR;
	x1=X1;							y1=Y1;
	x2=X2;							y2=Y2;
	Etat=false;	
	Focus=false;
	Text.resize(1);
	Text[0] = Caption;
	if( Text[0].size() >= MaxChar && MaxChar > 1 )
		Text[0].resize( MaxChar - 1 );
	Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;
	Func=F;
	Data=MaxChar;
	s=size;
}
			// Crée un menu flottant
void GUIOBJ::create_menu(float X1,float Y1,const Vector<String> &Entry,void (*F)(int), float size)
{
	Type=OBJ_FMENU;
	x1=X1;							y1=Y1;
	x2=X1+168;						y2=(int)(Y1+gui_font_h*size*Entry.size()+gui_font_h*size);
	Etat=false;	
	Focus=false;
	Text=Entry;
	Func=F;
	Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;
	s=size;
}
			// Crée un menu déroulant
void GUIOBJ::create_menu(float X1,float Y1,float X2,float Y2,const Vector<String> &Entry,void (*F)(int), float size)
{
	Type=OBJ_MENU;
	x1=X1;							y1=Y1;
	x2=X2;							y2=Y2;
	Etat=false;	
	Focus=false;
	Text = Entry;
	Pos=0;				// Position sur la liste
	Func=F;
	Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;
	s=size;
}
			// Crée une barre de progression
void GUIOBJ::create_pbar(float X1,float Y1,float X2,float Y2,int PCent, float size)
{
	Type=OBJ_PBAR;
	x1=X1;							y1=Y1;
	x2=X2;							y2=Y2;
	Etat=false;	
	Focus=false;
	Text.clear();
	Func=NULL;
	Data=PCent;
	Flag = 0;
	s=size;
}
			// Crée un objet text
void GUIOBJ::create_text(float X1,float Y1,const String &Caption,int Col, float size)
{
	Type = OBJ_TEXT;
	x1 = X1;								y1 = Y1;
	x2 = (int)(X1+Caption.length()*8*size);	y2 = (int)(Y1+gui_font_h*size);
	Etat = false;	
	Focus = false;
	Text.resize( 1 );
	Text[0] = Caption;
	Func = NULL;
	Data = Col;
	s = size;
	Flag = 0;
}

void GUIOBJ::create_line(float X1,float Y1,float X2,float Y2,int Col)
{
	Type=OBJ_LINE;
	x1=X1;							y1=Y1;
	x2=X2;							y2=Y2;
	Etat=false;	
	Focus=false;
	Text.clear();
	Func=NULL;
	Data=Col;
}

void GUIOBJ::create_box(float X1,float Y1,float X2,float Y2,int Col)
{
	Type=OBJ_BOX;
	x1=X1;							y1=Y1;
	x2=X2;							y2=Y2;
	Etat=false;	
	Focus=false;
	Text.clear();
	Func=NULL;
	Data=Col;
	Flag = FLAG_CAN_BE_CLICKED;
}

void GUIOBJ::create_img(float X1,float Y1,float X2,float Y2,GLuint img)
{
	Type=OBJ_IMG;
	x1=X1;							y1=Y1;
	x2=X2;							y2=Y2;
	Etat=false;	
	Focus=false;
	Text.clear();
	Func=NULL;
	Data = (uint32) img;
	Flag = FLAG_CAN_BE_CLICKED;
}

void GUIOBJ::create_list(float X1,float Y1,float X2,float Y2,const Vector<String> &Entry, float size)
{
	Type = OBJ_LIST;
	x1 = X1;						y1 = Y1;
	x2 = X2;						y2 = Y2;
	Etat = false;	
	Focus = false;
	Text = Entry;
	Func = NULL;
	Data = 0;
	Pos = 0;
	Flag = FLAG_CAN_BE_CLICKED | FLAG_CAN_GET_FOCUS;				// To detect when something has changed
	s=size;
}

void GUIOBJ::set_caption( String caption )
{
	switch( Type )
	{
	case OBJ_OPTIONC:
	case OBJ_OPTIONB:
		x2 = x1+(int)gui_font.length( caption )+4;
	case OBJ_TEXT:
	case OBJ_MENU:
	case OBJ_FMENU:
	case OBJ_TEXTBAR:
	case OBJ_BUTTON:
		Text[0] = caption;
	};
	if( Type == OBJ_TEXTBAR && Text[0].size() >= Data )
		Text[0].resize( Data - 1 );
}

uint32 GUIOBJ::msg( const String &message, WND *wnd )		// Reacts to a message transfered from the Interface
{
	uint32	result	= INTERFACE_RESULT_CONTINUE;
	if( Lowercase( message ) == "hide" )					{	Flag |= FLAG_HIDDEN;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "show" )				{	Flag &= ~FLAG_HIDDEN;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "switch" )				{	Flag |= FLAG_SWITCH;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "unswitch" )			{	Flag &= ~FLAG_SWITCH;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "fill" )				{	Flag |= FLAG_FILL;				result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "unfill" )				{	Flag &= ~FLAG_FILL;				result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "enable" )				{	Flag &= ~FLAG_DISABLED;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "disable" )			{	Flag |= FLAG_DISABLED;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "highlight" )			{	Flag |= FLAG_HIGHLIGHT;			result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "unhighlight" )		{	Flag &= ~FLAG_HIGHLIGHT;		result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "can_get_focus" )		{	Flag |= FLAG_CAN_GET_FOCUS;		result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "cant_get_focus" )		{	Flag &= ~FLAG_CAN_GET_FOCUS;	result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "can_be_clicked" )		{	Flag |= FLAG_CAN_BE_CLICKED;	result = INTERFACE_RESULT_HANDLED;	}
	else if( Lowercase( message ) == "cant_be_clicked" )		{	Flag &= ~FLAG_CAN_BE_CLICKED;	result = INTERFACE_RESULT_HANDLED;	}
	else if( StartsWith( Lowercase( message ) ,"caption=" ) )	{			// Change the GUIOBJ's caption
		if( Text.size() > 0 )
			Text[0] = message.substr( 8, message.size() - 8 );
		result = INTERFACE_RESULT_HANDLED;
		}
	else if( Lowercase( message ) == "focus" ) {
		if( wnd ) {
			for( int i = 0 ; i < wnd->NbObj ; i++ )
				wnd->Objets[i].Focus = false;
			Focus = true;
			}
		result = INTERFACE_RESULT_HANDLED;
		}

	wait_a_turn = true;
	return result;
}

uint32	GUIOBJ::num_entries()
{
	return Text.size();
}

	/*---------------------------------------------------------------------------\
	|               Dessine la fenêtre ainsi que tous ses objets                 |
	\---------------------------------------------------------------------------*/

void WND::draw( String &help_msg, bool Focus, bool Deg, SKIN *skin )
{
	if( hidden )	return;		// If it's hidden don't draw it
	
	EnterCS();
	
	int old_u_format = get_uformat();
	set_uformat( u_format );
			// Fenêtre
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		// Alpha blending activated
	if(background == 0) {
		if( skin && skin->wnd_background ) {
			gfx->set_color( color );
			gfx->drawtexture(skin->wnd_background,x,y,x+width,y+height);
			glBindTexture(GL_TEXTURE_2D, 0);
			}
		else {
			glBindTexture(GL_TEXTURE_2D, 0);
			gfx->rectfill(x,y,x+width,y+height, color );
			}
		}
	else {
		gfx->set_color( color );
		if( repeat_bkg )
			gfx->drawtexture(background,x,y,x+width,y+height, 0.0f, 0.0f, ((float)width)/bkg_w, ((float)height)/bkg_h);
		else
			gfx->drawtexture(background,x,y,x+width,y+height);
		glBindTexture(GL_TEXTURE_2D, 0);
		}
	glDisable( GL_BLEND );
	if( skin ) {
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		// Alpha blending activated
		gfx->set_color( color );
		if( draw_borders && skin->wnd_border.tex )
			skin->wnd_border.draw( x - skin->wnd_border.x1, y - skin->wnd_border.y1, x + width - skin->wnd_border.x2, y + height - skin->wnd_border.y2, false );
		if( show_title && skin->wnd_title_bar.tex ) {
			title_h = (int)(max( 2 + gui_font.height(), (float)skin->wnd_title_bar.y1 ) - skin->wnd_title_bar.y2);
			skin->wnd_title_bar.draw( x+3, y+3, x+width-4, y + 3 + title_h * size_factor );
			gfx->print(gui_font,x+5+skin->wnd_title_bar.x1,y + 3 + (title_h - gui_font.height() )*0.5f * size_factor,0,Blanc,Title,size_factor);
			}
		glDisable( GL_BLEND );
		glBindTexture(GL_TEXTURE_2D, 0);
		}
	else {
		if(draw_borders) {
			gfx->rect(x-2,y-2,x+width+1,y+height+1,Noir);
			gfx->rect(x-1,y-1,x+width,y+height,GrisF);
			gfx->line(x-2,y-2,x+width+1,y-2,Blanc);
			gfx->line(x-2,y-2,x-2,y+height+1,Blanc);
			gfx->line(x-1,y-1,x+width,y-1,GrisC);
			gfx->line(x-1,y-1,x-1,y+height,GrisC);
			}
		if(show_title) {
			title_h = (int)( 2 + gui_font.height() );
			if(Deg) {
				if(Focus)	{
					glBegin(GL_QUADS);
						glColor3f(0.0f,0.0f,1.0f);		glVertex2f(x+3,y+3);
						glColor3f(0.5f,0.5f,0.75f);		glVertex2f(x+width-4,y+3);
						glColor3f(0.5f,0.5f,0.75f);		glVertex2f(x+width-4,y+5+gui_font.height());
						glColor3f(0.0f,0.0f,1.0f);		glVertex2f(x+3,y+5+gui_font.height());
					glEnd();
					}
				else	{
					glBegin(GL_QUADS);
						glColor3f(0.75f,0.75f,0.75f);		glVertex2f(x+3,y+3);
						glColor3f(0.5f,0.5f,0.5f);			glVertex2f(x+width-4,y+3);
						glColor3f(0.5f,0.5f,0.5f);			glVertex2f(x+width-4,y+5+gui_font.height());
						glColor3f(0.75f,0.75f,0.75f);		glVertex2f(x+3,y+5+gui_font.height());
					glEnd();
					}
				}
			else {
				if(Focus)
					gfx->rectfill(x+3,y+3,x+width-4,y+5+gui_font.height(),Bleu);
				else
					gfx->rectfill(x+3,y+3,x+width-4,y+5+gui_font.height(),GrisF);
				}
			gfx->print(gui_font,x+4,y+4,0,Blanc,Title);
			}
		}
			// Objets
	if(NbObj>0 && Objets!=NULL) {
		for(int i=0;i<NbObj;i++)
			if( !(Objets[i].Flag & FLAG_HIDDEN) )	{			// Affiche les objets d'arrière plan
				if( Objets[i].MouseOn && !Objets[i].help_msg.empty() )
					help_msg = Objets[i].help_msg;
				switch(Objets[i].Type)
				{
				case OBJ_TA_BUTTON:
					{
					int cur_img =	( Objets[i].Flag & FLAG_DISABLED ) ?
									Objets[i].gltex_states.size() - 1 :
									( (Objets[i].activated && Objets[i].nb_stages == 1) ? Objets[i].gltex_states.size() - 2 : Objets[i].current_state );
					if( cur_img < Objets[i].gltex_states.size() && cur_img >= 0 ) {
						gfx->set_color( 0xFFFFFFFF );
						gfx->set_alpha_blending();
						Objets[i].gltex_states[ cur_img ].draw( x+Objets[i].x1, y+Objets[i].y1 );
						gfx->unset_alpha_blending();
						}
					}
					break;
				case OBJ_LIST:
					ListBox(x+Objets[i].x1, y+Objets[i].y1, x+Objets[i].x2, y+Objets[i].y2, Objets[i].Text, Objets[i].Pos, Objets[i].Data , skin, Objets[i].s, Objets[i].Flag );
					break;
				case OBJ_LINE:
					gfx->line(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2,Objets[i].Data);
					break;
				case OBJ_BOX:
					gfx->set_alpha_blending();
					if( Objets[i].Flag & FLAG_FILL )
						gfx->rectfill(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2,Objets[i].Data);
					else
						gfx->rect(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2,Objets[i].Data);
					gfx->unset_alpha_blending();
					break;
				case OBJ_IMG:
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D,(GLuint)Objets[i].Data);
					gfx->set_color( 0xFFFFFFFF );
					glBegin(GL_QUADS);
						glTexCoord2f(Objets[i].u1,Objets[i].v1);	glVertex2f(x+Objets[i].x1,y+Objets[i].y1);
						glTexCoord2f(Objets[i].u2,Objets[i].v1);	glVertex2f(x+Objets[i].x2,y+Objets[i].y1);
						glTexCoord2f(Objets[i].u2,Objets[i].v2);	glVertex2f(x+Objets[i].x2,y+Objets[i].y2);
						glTexCoord2f(Objets[i].u1,Objets[i].v2);	glVertex2f(x+Objets[i].x1,y+Objets[i].y2);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);
					break;
				case OBJ_BUTTON:		// Boutton
					if( Objets[i].Text.size() == 0 )
						Objets[i].Text.push_back( "" );
					button(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2,Objets[i].Text[0],Objets[i].activated,Objets[i].s, skin );
					if(Objets[i].Focus && Focus)
						gfx->rectdot(Objets[i].x1+x-2,Objets[i].y1+y-2,Objets[i].x2+x+2,Objets[i].y2+y+2,GrisF);
					break;
				case OBJ_OPTIONC:		// Case à cocher
					if( Objets[i].Text.size() == 0 )
						Objets[i].Text.push_back( "" );
					OptionCase(x+Objets[i].x1,y+Objets[i].y1,Objets[i].Text[0],Objets[i].Etat, skin, Objets[i].s );
					if(Objets[i].Focus && Focus)
						gfx->rectdot(Objets[i].x1+x-2,Objets[i].y1+y-2,Objets[i].x2+x+2,Objets[i].y2+y+2,GrisF);
					break;
				case OBJ_OPTIONB:		// Boutton d'option	
					if( Objets[i].Text.size() == 0 )
						Objets[i].Text.push_back( "" );
					OptionButton(x+Objets[i].x1,y+Objets[i].y1,Objets[i].Text[0],Objets[i].Etat, skin, Objets[i].s );
					if(Objets[i].Focus && Focus)
						gfx->rectdot(Objets[i].x1+x-2,Objets[i].y1+y-2,Objets[i].x2+x+2,Objets[i].y2+y+2,GrisF);
					break;
				case OBJ_PBAR:			// Barre de progression
					ProgressBar(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2,Objets[i].Data, skin, Objets[i].s );
					if(Objets[i].Focus && Focus)
						gfx->rectdot(Objets[i].x1+x-2,Objets[i].y1+y-2,Objets[i].x2+x+2,Objets[i].y2+y+2,GrisF);
					break;
				case OBJ_TEXTBAR:		// Barre de saisie de texte
					if( Objets[i].Text.size() == 0 )
						Objets[i].Text.push_back( "" );
					TextBar(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2,Objets[i].Text[0],Objets[i].Focus, skin, Objets[i].s );
					if(Objets[i].Focus && Focus)
						gfx->rectdot(Objets[i].x1+x-2,Objets[i].y1+y-2,Objets[i].x2+x+2,Objets[i].y2+y+2,GrisF);
					break;
				case OBJ_TEXT:
					if( Objets[i].Text.size() == 0 )
						Objets[i].Text.push_back( "" );
					if( !(Objets[i].Flag & FLAG_TEXT_ADJUST ) )
						gfx->print(gui_font,x+Objets[i].x1,Objets[i].y1+y,0.0f,Objets[i].Data,Objets[i].Text[0],Objets[i].s );
					else {
						Objets[i].Data = draw_text_adjust( x+Objets[i].x1, y+Objets[i].y1, x+Objets[i].x2, y+Objets[i].y2, Objets[i].Text[0], Objets[i].s, Objets[i].Pos, Objets[i].Flag & FLAG_MISSION_MODE );
						if( Objets[i].Data > 0 )
							Objets[i].Pos %= Objets[i].Data;
						}
					break;
				case OBJ_MENU:			// Menu déroulant
					if( Objets[i].Text.size() == 0 )
						Objets[i].Text.push_back( "" );
					if(!Objets[i].Etat)
						button( x+Objets[i].x1, y+Objets[i].y1, x+Objets[i].x2, y+Objets[i].y2, Objets[i].Text[0], Objets[i].activated || Objets[i].Etat, Objets[i].s, skin );
					break;
				};
				if( Objets[i].Type != OBJ_TA_BUTTON && (Objets[i].Flag & FLAG_DISABLED) ) {		// Make it darker when disabled
					glEnable(GL_BLEND);
					glDisable(GL_TEXTURE_2D);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					glColor4f(0.0f,0.0f,0.0f,0.5f);
					gfx->rectfill(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2);
					glEnable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
					}
				if( (Objets[i].Flag & FLAG_HIGHLIGHT) && Objets[i].MouseOn) {		// Highlight the object
					glEnable(GL_BLEND);
					glDisable(GL_TEXTURE_2D);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					glColor4f(1.0f,1.0f,1.0f,0.5f);
					gfx->rectfill(x+Objets[i].x1,y+Objets[i].y1,x+Objets[i].x2,y+Objets[i].y2);
					glEnable(GL_TEXTURE_2D);
					glDisable(GL_BLEND);
					}
				}
		for(int i=0;i<NbObj;i++)			// Affiche les objets de premier plan
			if( !(Objets[i].Flag & FLAG_HIDDEN) )
				switch(Objets[i].Type)
				{
				case OBJ_FMENU:			// Menu flottant
					FloatMenu( x+Objets[i].x1, y+Objets[i].y1, Objets[i].Text, Objets[i].Data, 0, skin, Objets[i].s );
					break;
				case OBJ_MENU:			// Menu déroulant
					if(Objets[i].Etat) {
						button( x+Objets[i].x1, y+Objets[i].y1, x+Objets[i].x2, y+Objets[i].y2, Objets[i].Text[0], Objets[i].activated || Objets[i].Etat, Objets[i].s, skin );
						FloatMenu( x+Objets[i].x1, y+Objets[i].y2+1, Objets[i].Text, Objets[i].Data+1, 1+Objets[i].Pos, skin, Objets[i].s );
						}
					break;
				};
		}
	LeaveCS();
	set_uformat( old_u_format );
}			// Fin de draw(...)

	/*---------------------------------------------------------------------------\
	|        Déplace la fenêtre et détecte si la souris s'y trouve               |
	\---------------------------------------------------------------------------*/

byte WND::WinMov(int AMx,int AMy,int AMb,int Mx,int My,int Mb, SKIN *skin)
{
	EnterCS();
	byte WinMouse=0;
	if(AMb==1&&Mb==1 && !Lock)
		if( AMx >= x + 3 && AMx <= x + width - 4 )
			if( AMy >= y + 3 && AMy <= y + 3 + title_h ) {
				x+=Mx-AMx;
				y+=My-AMy;
				}
	if( skin ) {
		if( Mx >= x - skin->wnd_border.x1 && Mx <= x + width - skin->wnd_border.x2 && My >= y - skin->wnd_border.y1 && My <= y + height - skin->wnd_border.y2 )
			WinMouse=1;
		}
	else
		if( Mx >= x && Mx <= x + width && My >= y && My <= y + height )
			WinMouse=1;
	LeaveCS();
	return WinMouse;
}				// Fin de WinMov

	/*---------------------------------------------------------------------------\
	|             Destroy properly a WND object to prevent memory leak           |
	\---------------------------------------------------------------------------*/
void WND::destroy()
{
	EnterCS();

	Title.clear();
	Name.clear();
	if( delete_gltex ) {
		gfx->destroy_texture( background );
		delete_gltex = false;
		}
	background = 0;
	if(NbObj>0 && Objets!=NULL) {
		delete[] Objets;
		NbObj=0;
		Objets=NULL;
		}

	LeaveCS();
}

	/*---------------------------------------------------------------------------\
	|             Gère les interactions entre l'utilisateur et les objets        |
	\---------------------------------------------------------------------------*/
int WND::check(int AMx,int AMy,int AMz,int AMb,bool timetoscroll, SKIN *skin )
{
	EnterCS();
	if( hidden )	{
		was_hidden = true;
		LeaveCS();
		return 0;		// if it's hidden you cannot interact with it
		}
	if( was_hidden )
		for( int i = 0 ; i < NbObj ; i++ )
			if( Objets[i].Type == OBJ_MENU || Objets[i].Type == OBJ_FMENU )
				Objets[i].Etat = false;
	was_hidden = false;
	int IsOnGUI;
		// Vérifie si la souris est sur la fenêtre et/ou si elle la déplace
	IsOnGUI=WinMov(AMx,AMy,AMb,mouse_x,mouse_y,mouse_b,skin);
			// S'il n'y a pas d'objets, on arrête
	if(NbObj<=0 || Objets==NULL) {
		LeaveCS();
		return IsOnGUI;
		}

			// Interactions utilisateur/objets
	int index,e;
	byte Key;
	bool was_on_floating_menu = false;
	int  on_menu = -1;
	bool close_all = false;
	
	bool already_clicked = false;

	for(int i=0;i<NbObj;i++) {
		if( Objets[i].Type == OBJ_NONE )
			continue;
		if( Objets[i].Type == OBJ_TA_BUTTON && Objets[i].current_state < Objets[i].gltex_states.size() ) {
			Objets[i].x2 = Objets[i].x1 + Objets[i].gltex_states[ Objets[i].current_state ].width-1;
			Objets[i].y2 = Objets[i].y1 + Objets[i].gltex_states[ Objets[i].current_state ].height-1;
			}
					// Vérifie si la souris est sur l'objet
		if(mouse_x>=x+Objets[i].x1 && mouse_x<=x+Objets[i].x2
			&& mouse_y>=y+Objets[i].y1 && mouse_y<=y+Objets[i].y2 )	continue;

		if(Objets[i].Type==OBJ_MENU && Objets[i].Etat && Objets[i].MouseOn && !was_on_floating_menu ) {
			float m_width = 168.0f * Objets[i].s;
			if( skin ) {
				for( int e = 0 ; e < Objets[i].Text.size() - ( 1 + Objets[i].Pos ) ; e++ )
					m_width = max( m_width, gui_font.length( Objets[i].Text[ e ] ) * Objets[i].s );

				m_width += skin->menu_background.x1 - skin->menu_background.x2;
				}
			else
				m_width = 168.0f;

			if( mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x1 + m_width
				&& mouse_y > y + Objets[i].y2 && mouse_y <= y + Objets[i].y2 + 1 + gui_font.height() * Objets[i].s * Objets[i].Text.size() ) {
				was_on_floating_menu = true;
				on_menu = i;
				}
			}
		}

	for(int i=0;i<NbObj;i++) {
		if( Objets[i].Type == OBJ_NONE )
			continue;

		bool MouseWasOn = Objets[i].MouseOn;
		Objets[i].MouseOn = false;
		if( Objets[i].wait_a_turn ) {
			Objets[i].wait_a_turn = false;
			continue;
			}
		if( (Objets[i].Flag & FLAG_HIDDEN) == FLAG_HIDDEN )	continue;		// Object is hidden so don't handle its events

		if( on_menu == i )
			was_on_floating_menu = false;

					// Vérifie si la souris est sur l'objet
		if(mouse_x>=x+Objets[i].x1 && mouse_x<=x+Objets[i].x2
			&& mouse_y>=y+Objets[i].y1 && mouse_y<=y+Objets[i].y2 && !was_on_floating_menu )	Objets[i].MouseOn=true;

		if(Objets[i].Type==OBJ_MENU && Objets[i].Etat && !Objets[i].MouseOn && !was_on_floating_menu ) {
			int e;
			float m_width = 168.0f * Objets[i].s;
			if( skin ) {
				for( e = 0 ; e < Objets[i].Text.size() - ( 1 + Objets[i].Pos ) ; e++ )
					m_width = max( m_width, gui_font.length( Objets[i].Text[ e ] ) * Objets[i].s );

				m_width += skin->menu_background.x1 - skin->menu_background.x2;
				}
			else
				m_width = 168.0f;

			if( mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x1 + m_width
				&& mouse_y > y + Objets[i].y2 && mouse_y <= y + Objets[i].y2 + 1 + gui_font.height() * Objets[i].s * Objets[i].Text.size() )
				Objets[i].MouseOn = true;
			}

		if(Objets[i].MouseOn) IsOnGUI |= 2;

		if(mouse_b!=0 && Objets[i].MouseOn && !was_on_floating_menu ) {		// Obtient le focus
			for(e=0;e<NbObj;e++)
				Objets[e].Focus=false;
			Objets[i].Focus=true;
			}

		if(mouse_b!=0 && !Objets[i].MouseOn)	{	// Perd le focus
			Objets[i].Focus=false;
			switch(Objets[i].Type)
			{
			case OBJ_MENU:
				Objets[i].Etat=false;
				break;
			};
			}

		if(Objets[i].MouseOn && (Objets[i].Type==OBJ_FMENU || Objets[i].Type==OBJ_MENU)) {
			for(e=0;e<NbObj;e++) {
				Objets[e].Focus=false;
				if(Objets[e].Type==OBJ_BUTTON)
					Objets[e].Etat=false;
				}
			was_on_floating_menu = Objets[i].Etat;
			Objets[i].Focus=true;
			}

		if( !(Objets[i].Flag & FLAG_CAN_GET_FOCUS) )
			Objets[i].Focus = false;

		switch(Objets[i].Type)
		{
		case OBJ_MENU:			// Choses à faire quoi qu'il arrive
			Objets[i].Data=-1;		// Pas de séléction
			if( !Objets[i].Etat )	Objets[i].Value = -1;
			{
			float m_width = 168.0f * Objets[i].s;
			if( skin ) {
				for( int e = 0 ; e < Objets[i].Text.size() - ( 1 + Objets[i].Pos ) ; e++ )
					m_width = max( m_width, gui_font.length( Objets[i].Text[ e ] ) * Objets[i].s );

				m_width += skin->menu_background.x1 - skin->menu_background.x2;
				}
			else
				m_width = 168.0f;

			if( Objets[i].MouseOn && mouse_x >= x + Objets[i].x1 && mouse_x <= x + Objets[i].x1 + m_width
				&& mouse_y > y + Objets[i].y2 + 4 && mouse_y <= y + Objets[i].y2 + 1 + gui_font.height() * Objets[i].s * Objets[i].Text.size()
				&& Objets[i].Etat ){

				if(timetoscroll) {
					if(mouse_y<y+Objets[i].y2+12 && Objets[i].Pos>0)
						Objets[i].Pos--;
					if(mouse_y>SCREEN_H-8 && y+Objets[i].y2+1+gui_font.height()*Objets[i].s*(Objets[i].Text.size()-Objets[i].Pos)>SCREEN_H)
						Objets[i].Pos++;
					}
				Objets[i].Data=(int)((mouse_y-y-Objets[i].y2-5)/(gui_font.height()*Objets[i].s)+Objets[i].Pos);
				if(Objets[i].Data>=Objets[i].Text.size()-1) Objets[i].Data=-1;
				}
			}
			break;
		case OBJ_FMENU:
			Objets[i].Data=-1;		// Pas de séléction
				if(Objets[i].MouseOn && mouse_y>=y+Objets[i].y1+4 && mouse_y<=y+Objets[i].y2-4) {
					Objets[i].Data=(int)((mouse_y-y-Objets[i].y1-4)/(gui_font.height()*Objets[i].s));
					if(Objets[i].Data>=Objets[i].Text.size()) Objets[i].Data=-1;
					}
			break;
		case OBJ_TEXTBAR:				// Permet l'entrée de texte
			Objets[i].Etat=false;
			if(Objets[i].Focus && keypressed()) {
				Key=readkey()&0xff;
				switch(Key)
				{
				case 8:
					if(Objets[i].Text[0].length()>0)
						Objets[i].Text[0].resize( Objets[i].Text[0].length()-1 );
					break;
				case 13:
					Objets[i].Etat=true;
					if(Objets[i].Func!=NULL)
						(*Objets[i].Func)(Objets[i].Text[0].length());
					break;
				case 27:
				case 0:
					break;
				default:
				    if( Objets[i].Text[0].length() + 1 < Objets[i].Data )
						Objets[i].Text[0] += Key;
				};
				}
			break;
		case OBJ_LIST:
			if( Objets[i].MouseOn && skin ) {
				if( mouse_x - x <= Objets[i].x1 + skin->text_background.x1
					|| mouse_x - x >= Objets[i].x2 + skin->text_background.x2
					|| mouse_y - y <= Objets[i].y1 + skin->text_background.y1
					|| mouse_y - y >= Objets[i].y2 + skin->text_background.y2 )			// We're on ListBox decoration!
					break;
				int TotalScroll = Objets[i].Text.size() - (int)( (Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2) / ( gui_font.height() * Objets[i].s ) );
				if( TotalScroll < 0 )	TotalScroll = 0;

				if( mouse_b == 1
					&& mouse_x - x >= Objets[i].x2 + skin->text_background.x2 - skin->scroll[0].sw
					&& mouse_x - x <= Objets[i].x2 + skin->text_background.x2
					&& mouse_y - y >= Objets[i].y1 + skin->text_background.y1
					&& mouse_y - y <= Objets[i].y2 + skin->text_background.y2 ) {			// We're on the scroll bar!
					
					if( mouse_y - y > Objets[i].y1 + skin->text_background.y1 + skin->scroll[0].y1
					&& mouse_y - y < Objets[i].y2 + skin->text_background.y2 + skin->scroll[0].y2 ) {		// Set scrolling position
						Objets[i].Data = (int)( 0.5f + TotalScroll * (mouse_y - y - Objets[i].y1 - skin->text_background.y1 - skin->scroll[0].y1 - (skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2 ) * 0.5f )
										/ ( Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2 - skin->scroll[0].y1 + skin->scroll[0].y2 - ( skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2 ) * 0.5f ) );
						}
					if( Objets[i].Data > TotalScroll )
						Objets[i].Data = TotalScroll;
					}
				else {
					int nscroll = (int)Objets[i].Data - mouse_z + AMz;
					if( nscroll < 0 )					nscroll = 0;
					else if( nscroll > TotalScroll )	nscroll = TotalScroll;

					Objets[i].Data = nscroll;
					}
				}
			break;
		};

		if( Objets[i].Flag & FLAG_DISABLED ) {
			Objets[i].activated = false;
			Objets[i].Etat = false;
			}
		else {
			if((mouse_b!=1 || !Objets[i].MouseOn || mouse_b==AMb) && (Objets[i].Flag & FLAG_CAN_BE_CLICKED) && !(Objets[i].Flag & FLAG_SWITCH) && Objets[i].Etat && !was_on_floating_menu) { 
				if(Objets[i].Func!=NULL)
					(*Objets[i].Func)(0);		// Lance la fonction associée
				Objets[i].Etat=false;
				}
			if( !Objets[i].activated && mouse_b==1 && Objets[i].MouseOn && ((Objets[i].Flag & FLAG_CAN_BE_CLICKED) || (Objets[i].Flag & FLAG_SWITCH)) )
				switch( Objets[i].Type )
				{
				case OBJ_BOX:
				case OBJ_BUTTON:
				case OBJ_MENU:
				case OBJ_TA_BUTTON:
				case OBJ_OPTIONB:
				case OBJ_OPTIONC:
					if( sound_manager )
						sound_manager->PlayTDFSoundNow("SPECIALORDERS.sound");
				};
			Objets[i].activated = mouse_b==1 && Objets[i].MouseOn;

			bool clicked = false;
			if( Objets[i].shortcut_key >= 0 && Objets[i].shortcut_key <= 255 && ( Console == NULL || !Console->activated() )
				&& ( key[ ascii_to_scancode[ Objets[i].shortcut_key ] ]
					|| ( Objets[i].shortcut_key >= 65 && Objets[i].shortcut_key <= 90 && key[ ascii_to_scancode[ Objets[i].shortcut_key + 32 ] ] )
					|| ( Objets[i].shortcut_key >= 97 && Objets[i].shortcut_key <= 122 && key[ ascii_to_scancode[ Objets[i].shortcut_key - 32 ] ] ) ) ) {
				if( !Objets[i].Etat )
					clicked = true;
				Objets[i].activated = Objets[i].Etat = true;
				}

			if( ( (mouse_b!=1 && AMb==1) || clicked ) && Objets[i].MouseOn && MouseWasOn
			 && ((Objets[i].Flag & FLAG_CAN_BE_CLICKED) || (Objets[i].Flag & FLAG_SWITCH)) && !already_clicked ) {		// Click sur l'objet
			 	already_clicked = true;
				switch(Objets[i].Type)
				{
				case OBJ_LIST:
					if( skin
						&& mouse_x - x >= Objets[i].x2 + skin->text_background.x2 - skin->scroll[0].sw
						&& mouse_x - x <= Objets[i].x2 + skin->text_background.x2
						&& mouse_y - y >= Objets[i].y1 + skin->text_background.y1
						&& mouse_y - y <= Objets[i].y2 + skin->text_background.y2 ) {			// We're on the scroll bar!

						int TotalScroll = Objets[i].Text.size() - (int)( (Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2) / ( gui_font.height() * Objets[i].s ) );
						if( TotalScroll < 0 )	TotalScroll = 0;
						
						if( mouse_y - y <= Objets[i].y1 + skin->text_background.y1 + skin->scroll[0].y1 ) {		// Scroll up
							if( Objets[i].Data > 0 ) Objets[i].Data--;
							if( sound_manager )
								sound_manager->PlayTDFSoundNow("SPECIALORDERS.sound");
							}
						else if( mouse_y - y >= Objets[i].y2 + skin->text_background.y2 + skin->scroll[0].y2 ) {	// Scroll down
							Objets[i].Data++;
							if( sound_manager )
								sound_manager->PlayTDFSoundNow("SPECIALORDERS.sound");
							}
						else {							// Set scrolling position
							Objets[i].Data = (int)( 0.5f + TotalScroll * (mouse_y - y - Objets[i].y1 - skin->text_background.y1 - skin->scroll[0].y1 - (skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2 ) * 0.5f )
											/ ( Objets[i].y2 - Objets[i].y1 - skin->text_background.y1 + skin->text_background.y2 - skin->scroll[0].y1 + skin->scroll[0].y2 - ( skin->scroll[0].sw - skin->scroll[ 0 ].x1 + skin->scroll[ 0 ].x2 ) * 0.5f ) );
							}
						if( Objets[i].Data > TotalScroll )
							Objets[i].Data = TotalScroll;
						}
					else {
						if( skin && (
							   mouse_x - x <= Objets[i].x1 + skin->text_background.x1
							|| mouse_x - x >= Objets[i].x2 + skin->text_background.x2
							|| mouse_y - y <= Objets[i].y1 + skin->text_background.y1
							|| mouse_y - y >= Objets[i].y2 + skin->text_background.y2) )			// We're on ListBox decoration!
							break;
						Objets[i].Pos = (uint32) ((mouse_y - y - Objets[i].y1 - ( skin ? skin->text_background.y1:4) ) / ( gui_font.height() * Objets[i].s ) + Objets[i].Data);						Objets[i].Etat = true;
						}
					break;
				case OBJ_TA_BUTTON:
					if( Objets[i].nb_stages > 0 )
						Objets[i].current_state = (++Objets[i].current_state) % Objets[i].nb_stages;
					Objets[i].Etat = true;
					break;
				case OBJ_BOX:			// Rectangle
				case OBJ_IMG:			// Image
				case OBJ_BUTTON:		// Boutton
					if(was_on_floating_menu)	break;
					Objets[i].Etat=true;
					break;
				case OBJ_OPTIONC:		// Case à cocher
					if(was_on_floating_menu)	break;
					if( skin && skin->checkbox[0].tex && skin->checkbox[1].tex) {
						if(mouse_x<=x+Objets[i].x1+skin->checkbox[Objets[i].Etat?1:0].sw && mouse_y<=y+Objets[i].y1+skin->checkbox[Objets[i].Etat?1:0].sh)
							Objets[i].Etat^=true;
						}
					else if(mouse_x<=x+Objets[i].x1+12 && mouse_y<=y+Objets[i].y1+12)
						Objets[i].Etat^=true;
					if(Objets[i].Func!=NULL)
						(*Objets[i].Func)(Objets[i].Etat);	// Lance la fonction associée
					break;
				case OBJ_OPTIONB:		// Bouton d'option
					if(was_on_floating_menu)	break;
					if( skin && skin->option[0].tex && skin->option[1].tex) {
						if(mouse_x<=x+Objets[i].x1+skin->option[Objets[i].Etat?1:0].sw && mouse_y <= y + Objets[i].y1+skin->option[Objets[i].Etat?1:0].sh)
							Objets[i].Etat^=true;
						}
					else
					if(mouse_x<=x+Objets[i].x1+12 && mouse_y<=y+Objets[i].y1+12)
						Objets[i].Etat^=true;
					if(Objets[i].Func!=NULL)
						(*Objets[i].Func)(Objets[i].Etat);	// Lance la fonction associée
					break;
				case OBJ_FMENU:			// Menu Flottant
					if(mouse_y>=y+Objets[i].y1+( skin ? skin->menu_background.y1:0)+4 && mouse_y<=y+Objets[i].y2 + ( skin ? skin->menu_background.y2 : 0 )-4) {
						index=(int)((mouse_y-y-Objets[i].y1-4 - ( skin ? skin->menu_background.y1 : 0 ))/(gui_font.height()*Objets[i].s));
						if(index>=Objets[i].Text.size()) index=Objets[i].Text.size()-1;
						if(Objets[i].Func!=NULL)
							(*Objets[i].Func)(index);		// Lance la fonction associée
						}
					break;
				case OBJ_MENU:			// Menu déroulant
					{
					float m_width = 168.0f * Objets[i].s;
					if( skin ) {
						for( int e = 0 ; e < Objets[i].Text.size() - ( 1 + Objets[i].Pos ) ; e++ )
							m_width = max( m_width, gui_font.length( Objets[i].Text[ e ] ) * Objets[i].s );

						m_width += skin->menu_background.x1 - skin->menu_background.x2;
						}
					else
						m_width = 168.0f;
					if( mouse_x >= x + Objets[i].x1 + ( skin ? skin->menu_background.x1 : 0 ) && mouse_x <= x + Objets[i].x1 + m_width + ( skin ? skin->menu_background.x2 : 0 )
						&& mouse_y > y + Objets[i].y2 + ( skin ? skin->menu_background.y1 : 0 ) && mouse_y <= y + Objets[i].y2 + ( skin ? skin->menu_background.y2 : 0 ) + 1 + gui_font.height() * Objets[i].s * Objets[i].Text.size()
						&& Objets[i].Etat ){

						index = (int)((mouse_y - y - Objets[i].y2 - 5 - ( skin ? skin->menu_background.y1 : 0 ))/(Objets[i].s * gui_font.height()) + Objets[i].Pos);
						if( index >= Objets[i].Text.size() - 1 ) index = Objets[i].Text.size()-2;
						if( Objets[i].Func != NULL )
							(*Objets[i].Func)(index);		// Lance la fonction associée
						Objets[i].Value = Objets[i].Data;
						Objets[i].Etat = false;
						close_all = true;
						}
					else
						Objets[i].Etat ^= true;
					}
					break;
				default:
					Objets[i].Etat = true;
				};
						// Send a signal to the interface ( the OnClick signal defined at initialization time)
				for( uint16 cur = 0 ; cur < Objets[i].OnClick.size() ; cur++ )
					I_Msg( TA3D::TA3D_IM_GUI_MSG, (void *)Objets[i].OnClick[cur].c_str(), NULL, NULL );
				}
			else if( Objets[i].MouseOn )			// Send a signal to the interface ( the OnHover signal defined at initialization time)
				for( uint16 cur = 0 ; cur < Objets[i].OnHover.size() ; cur++ )
					I_Msg( TA3D::TA3D_IM_GUI_MSG, (void *)Objets[i].OnHover[cur].c_str(), NULL, NULL );
			}

		for( uint16 cur = 0 ; cur < Objets[i].SendDataTo.size() ; cur++ )	{				// Send Data to an Object
			int e = Objets[i].SendDataTo[cur].find(".");
			if( e != -1 ) {
				int target = atoi( Objets[i].SendDataTo[cur].substr( 0, e ).c_str() );
				if( target >= 0 && target < NbObj ) {
					if( Objets[i].SendDataTo[cur].substr( e+1, Objets[i].SendDataTo[cur].length()-e ) == "data" )
						Objets[target].Data = Objets[i].Data;
					else
						Objets[target].Pos = Objets[i].Data;
					}
				}
			}
		for( uint16 cur = 0 ; cur < Objets[i].SendPosTo.size() ; cur++ )	{				// Send Pos to an Object
			int e = Objets[i].SendPosTo[cur].find(".");
			if( e != -1 ) {
				int target = atoi( Objets[i].SendPosTo[cur].substr( 0, e ).c_str() );
				if( target >= 0 && target < NbObj ) {
					if( Objets[i].SendPosTo[cur].substr( e+1, Objets[i].SendPosTo[cur].length()-e ) == "data" )
						Objets[target].Data = Objets[i].Pos;
					else
						Objets[target].Pos = Objets[i].Pos;
					}
				}
			}
		}
	if( close_all )
		for( int i = 0 ; i < NbObj ; i++ )
			if( Objets[i].Type == OBJ_MENU )
				Objets[i].Etat = false;
	LeaveCS();
	return IsOnGUI;
}			// Fin de check

uint32 WND::msg( const String &message )									// Respond to Interface message
{
	EnterCS();
	int i = message.find( "." );
	uint32	result = INTERFACE_RESULT_CONTINUE;

	if( i != -1 ) {				// When it targets a subobject
		GUIOBJ *obj = get_object( message.substr( 0, i ) );
		if( obj ) {
			result = obj->msg( message.substr( i+1, message.size() - i - 1 ), this );
			LeaveCS();
			return result;
			}
		}
	else						// When it targets the window itself
		if( Lowercase( message ) == "show" )				{	hidden = false;		result = INTERFACE_RESULT_HANDLED;	}
		else if( Lowercase( message ) == "hide" )			{	hidden = true;		result = INTERFACE_RESULT_HANDLED;	}

	LeaveCS();
	return result;
}

bool WND::get_state( const String &message )									// Return the state of given object
{
	EnterCS();
	GUIOBJ *obj = get_object( message );
	if( obj ) {
		bool result = obj->Etat;
		LeaveCS();
		return result;
		}
	if( message == "" ) {
		bool result = !hidden;
		LeaveCS();
		return result;
		}
	LeaveCS();
	return false;
}

sint32 WND::get_value( const String &message )									// Return the state of given object
{
	EnterCS();
	GUIOBJ *obj = get_object( message );
	if( obj ) {
		sint32 v = obj->Value;
		LeaveCS();
		return v;
		}
	LeaveCS();
	return -1;
}

String WND::get_caption( const String &message )									// Return the state of given object
{
	EnterCS();
	GUIOBJ *obj = get_object( message );
	if( obj ) {
		if( obj->Text.size() > 0 ) {
			String result = obj->Text[0];
			LeaveCS();
			return result;
			}
		else {
			LeaveCS();
			return "";
			}
		}
	if( message == "" ) {
		String result = Title;
		LeaveCS();
		return result;
		}
	LeaveCS();
	return "";
}

GUIOBJ *WND::get_object( const String &message )		// Return a pointer to the specified object
{
	EnterCS();
	sint16 e = obj_hashtable.Find( Lowercase( message ) ) - 1;
	if( e >= 0 ) {
		GUIOBJ *the_obj = &(Objets[ e ]);
		LeaveCS();
		return the_obj;
		}
	LeaveCS();
	return NULL;
}

void WND::load_gui( const String &filename, cHashTable< Vector< TA3D::INTERFACES::GFX_TEXTURE >* > &gui_hashtable )	// Load a window from a TA *.GUI file describing the interface
{
	GuardEnter( WND::load_gui );

	if(g_useTextureCompression)
		allegro_gl_set_texture_format( GL_COMPRESSED_RGB_ARB );
	else
		allegro_gl_set_texture_format( GL_RGB8 );

	cTAFileParser *wndFile;

	try { // we need to try catch this cause the config file may not exists
		 // and if it don't exists it will throw an error on reading it, which
		 // will be caught in our main function and the application will exit.
		wndFile = new TA3D::UTILS::cTAFileParser( filename, false, false, true );
	}
	catch( ... )
	{
		GuardLeave();
		return;
	}

	Name = filename;		// Grab the window's name, so we can send signals to it (to hide/show for example)

	sint32 e = Name.find( "." );		// Extracts the file name

	if( e != -1 )	Name = Name.substr( 0, e );

	e = Name.find_last_of( "/\\" );

	if( e != -1 )	Name = Name.substr( e + 1, Name.size() - e - 1 );

	hidden = !wndFile->PullAsBool( "gadget0.common.active" );

	u_format = U_ASCII;

	Title = "";
	x = wndFile->PullAsInt( "gadget0.common.xpos" );
	y = wndFile->PullAsInt( "gadget0.common.ypos" );
	if( x < 0 )	x+=SCREEN_W;
	if( y < 0 ) y+=SCREEN_H;
	width = wndFile->PullAsInt( "gadget0.common.width" );
	height = wndFile->PullAsInt( "gadget0.common.height" );

	if( x + width >= SCREEN_W )		x = SCREEN_W - width;
	if( y + height >= SCREEN_H )	y = SCREEN_H - height;

	float x_factor = 1.0f;
	float y_factor = 1.0f;

	Lock = true;
	draw_borders = false;
	show_title = false;
	delete_gltex = false;

	String panel = wndFile->PullAsString( "gadget0.panel" );			// Look for the panel texture
	int w,h;
	background = gfx->load_texture_from_cache( "anims\\" + Name + ".gaf", FILTER_TRILINEAR, (uint32*)&w, (uint32*)&h );
	if( !background ) {
		background = read_gaf_img( "anims\\" + Name + ".gaf", panel, &w, &h, true );
		if( background == 0 ) {
			List< String >	file_list;
			HPIManager->GetFilelist( "anims\\*.gaf", &file_list );
			for( List< String >::iterator i = file_list.begin() ; i != file_list.end() && background == 0 ; i++ )
				background = read_gaf_img( *i, panel, &w, &h, true );
			}
		if( background )
			gfx->save_texture_to_cache( "anims\\" + Name + ".gaf", background, w, h );
		}

	delete_gltex = background;

	background_wnd = background;

	color = background ? makeacol( 0xFF, 0xFF, 0xFF, 0xFF ) : 0x0;

	NbObj = wndFile->PullAsInt( "gadget0.totalgadgets" );

	Objets = new GUIOBJ[NbObj];

	for( uint16 i = 0 ; i < NbObj ; i++ ) {		// Loads each object
		String obj_key = format( "gadget%d." , i + 1 );
		int obj_type = wndFile->PullAsInt( obj_key + "common.id" );

		Objets[i].Name = wndFile->PullAsString( obj_key + "common.name", format( "gadget%d", i + 1 ) );

		obj_hashtable.Insert( Lowercase( Objets[i].Name ), i + 1 );

		int X1 = (int)(wndFile->PullAsInt( obj_key + "common.xpos" )*x_factor);				// Reads data from TDF
		int Y1 = (int)(wndFile->PullAsInt( obj_key + "common.ypos" )*y_factor);
		int W = (int)(wndFile->PullAsInt( obj_key + "common.width" )*x_factor - 1 );
		int H = (int)(wndFile->PullAsInt( obj_key + "common.height" )*y_factor - 1 );

		float size = min( x_factor, y_factor );
		uint32 obj_flags = 0;

		if( X1<0 )	X1+=SCREEN_W;
		if( Y1<0 )	Y1+=SCREEN_H;

		if( !wndFile->PullAsBool( obj_key + "common.active" ) )			obj_flags |= FLAG_HIDDEN;

		Vector<String> Caption = ReadVectorString( wndFile->PullAsString( obj_key + "text" ) );
		for(uint32 e = 0 ; e < Caption.size() ; e++ )
			Caption[ e ] = TRANSLATE( Caption[ e ] );

		if( obj_type == TA_ID_BUTTON ) {
			int t_w[100], t_h[100];

			String key = Lowercase( Objets[i].Name );
			Vector< TA3D::INTERFACES::GFX_TEXTURE >	*result = gui_hashtable.Find( key );

			Vector< GLuint > gaf_imgs;
			bool found_elsewhere = false;

			if( result == NULL ) {
				gaf_imgs = read_gaf_imgs( "anims\\" + Name + ".gaf", Objets[i].Name, t_w, t_h );
				if( gaf_imgs.size() == 0 ) {
					gaf_imgs = read_gaf_imgs( "anims\\commongui.gaf", Objets[i].Name, t_w, t_h );
					found_elsewhere = true;
					}
				if( gaf_imgs.size() == 0 ) {
					List< String >	file_list;
					HPIManager->GetFilelist( "anims\\*.gaf", &file_list );
					for( List< String >::iterator e = file_list.begin() ; e != file_list.end() && gaf_imgs.size() == 0 ; e++ )
						gaf_imgs = read_gaf_imgs( *e, Objets[i].Name, t_w, t_h );
					if( gaf_imgs.size() > 0 )
						found_elsewhere = true;
					}
				}
			else {
				gaf_imgs.resize( result->size() );
				for( int e = 0 ; e < result->size() ; e++ ) {
					gaf_imgs[ e ] = (*result)[ e ].tex;
					t_w[ e ] = (*result)[ e ].width;
					t_h[ e ] = (*result)[ e ].height;
					}
				}
			int nb_stages = wndFile->PullAsInt( obj_key + "stages" );
			Objets[i].create_ta_button( X1, Y1, Caption, gaf_imgs, nb_stages > 0 ? nb_stages : gaf_imgs.size() - 2 );
			if( result == NULL && found_elsewhere )
				gui_hashtable.Insert( key, &Objets[i].gltex_states );
			for( int e = 0 ; e < Objets[i].gltex_states.size() ; e++ ) {
				Objets[i].gltex_states[e].width = t_w[ e ];
				Objets[i].gltex_states[e].height = t_h[ e ];
				if( result )
					Objets[i].gltex_states[e].destroy_tex = false;
				}
			Objets[i].current_state = wndFile->PullAsInt( obj_key + "status" );
			Objets[i].shortcut_key = wndFile->PullAsInt( obj_key + "quickkey", -1 );
			if( wndFile->PullAsBool( obj_key + "common.grayedout" ) )
				Objets[i].Flag |= FLAG_DISABLED;
//			if( wndFile->PullAsInt( obj_key + "common.commonattribs" ) == 4 ) {
			if( wndFile->PullAsInt( obj_key + "common.attribs" ) == 32 )
				Objets[i].Flag |= FLAG_HIDDEN | FLAG_BUILD_PIC;
			}
		else if( obj_type == TA_ID_TEXT_FIELD )
			Objets[i].create_textbar( X1, Y1, X1 + W, Y1 + H, Caption.size() > 0 ? Caption[0] : "", wndFile->PullAsInt( obj_key + "maxchars" ), NULL );
		else if( obj_type == TA_ID_LABEL )
			Objets[i].create_text( X1, Y1, Caption.size() ? Caption[0] : "", 0xFFFFFFFF, 1.0f );
		else if( obj_type == TA_ID_BLANK_IMG || obj_type == TA_ID_IMG ) {
			Objets[i].create_img( X1, Y1, X1+W, Y1+H, gfx->load_texture( wndFile->PullAsString( obj_key + "source" ) ) );
			Objets[i].destroy_img = Objets[i].Data != 0 ? true : false;
			}
		else if( obj_type == TA_ID_LIST_BOX )
			Objets[i].create_list( X1, Y1, X1+W, Y1+H, Caption );
		else
			Objets[i].Type = OBJ_NONE;

		Objets[i].OnClick.clear();
		Objets[i].OnHover.clear();
		Objets[i].SendDataTo.clear();
		Objets[i].SendPosTo.clear();

		Objets[i].Flag |= obj_flags;
		}

	delete wndFile; 

	GuardLeave();
}

void WND::load_tdf( const String &filename, SKIN *skin )			// Load a window from a TDF file describing the window
{
	GuardEnter( WND::load_tdf );

	cTAFileParser *wndFile;

	try { // we need to try catch this cause the config file may not exists
		 // and if it don't exists it will throw an error on reading it, which
		 // will be caught in our main function and the application will exit.
		wndFile = new TA3D::UTILS::cTAFileParser( filename );
	}
	catch( ... )
	{
		GuardLeave();
		return;
	}

	Name = filename;		// Grab the window's name, so we can send signals to it (to hide/show for example)

	sint32 e = Name.find( "." );		// Extracts the file name

	if( e != -1 )	Name = Name.substr( 0, e );

	e = Name.find_last_of( "/\\" );

	if( e != -1 )	Name = Name.substr( e + 1, Name.size() - e - 1 );

	Name = wndFile->PullAsString( "window.name", Name );

	hidden = wndFile->PullAsBool( "window.hidden" );

	String wnd_uformat = wndFile->PullAsString( "window.uformat" );
	if(	wnd_uformat == "ASCII" )	u_format = U_ASCII;
	if(	wnd_uformat == "ASCII_CP" )	u_format = U_ASCII_CP;
	if(	wnd_uformat == "UNICODE" )	u_format = U_UNICODE;
	if(	wnd_uformat == "UTF8" )		u_format = U_UTF8;
	Title = TRANSLATE( wndFile->PullAsString( "window.title" ) );
	x = wndFile->PullAsInt( "window.x" );
	y = wndFile->PullAsInt( "window.y" );
	width = wndFile->PullAsInt( "window.width" );
	height = wndFile->PullAsInt( "window.height" );
	repeat_bkg = wndFile->PullAsBool( "window.repeat background", false );
	get_focus = wndFile->PullAsBool( "window.get focus", false );

	float x_factor = 1.0f;
	float y_factor = 1.0f;
	if( wndFile->PullAsBool( "window.fullscreen" ) ) {
		int ref_width = wndFile->PullAsInt( "window.screen width", width );
		int ref_height = wndFile->PullAsInt( "window.screen height", height );
		if( ref_width > 0.0f )
			x_factor = ((float)gfx->width) / ref_width;
		if( ref_height > 0.0f )
			y_factor = ((float)gfx->height) / ref_height;
		width = (int)(width * x_factor);
		height = (int)(height * y_factor);
		x = (int)(x * x_factor);
		y = (int)(y * y_factor);
		}
	if( x < 0 )	x+=SCREEN_W;
	if( y < 0 ) y+=SCREEN_H;
	if( width < 0 )		width += SCREEN_W;
	if( height < 0 )	height += SCREEN_H;
	size_factor = gfx->height / 600.0f;			// For title bar

	background_wnd = wndFile->PullAsBool( "window.background window" );
	Lock = wndFile->PullAsBool( "window.lock" );
	draw_borders = wndFile->PullAsBool( "window.draw borders" );
	show_title = wndFile->PullAsBool( "window.show title" );
	delete_gltex = false;
	if( exists( wndFile->PullAsString( "window.background" ).c_str() ) ) {
		background = gfx->load_texture( wndFile->PullAsString( "window.background" ), FILTER_LINEAR, &bkg_w, &bkg_h, false );
		delete_gltex = true;
		}
	else
		background = 0;
	color = wndFile->PullAsInt( "window.color", delete_gltex ?  0xFFFFFFFF : makeacol( 0x7F, 0x7F, 0x7F, 0xFF ) );

	NbObj = wndFile->PullAsInt( "window.number of objects" );

	Objets = new GUIOBJ[NbObj];

	for( uint16 i = 0 ; i < NbObj ; i++ ) {		// Loads each object
		String obj_key = format( "window.object%d." , i );
		String obj_type = wndFile->PullAsString( obj_key + "type" );

		Objets[i].Name = wndFile->PullAsString( obj_key + "name", format( "object%d", i ) );

		obj_hashtable.Insert( Lowercase( Objets[i].Name ), i + 1 );

		Objets[i].help_msg = TRANSLATE( wndFile->PullAsString( obj_key + "help", "" ) );

		float X1 = wndFile->PullAsFloat( obj_key + "x1" )*x_factor;				// Reads data from TDF
		float Y1 = wndFile->PullAsFloat( obj_key + "y1" )*y_factor;
		float X2 = wndFile->PullAsFloat( obj_key + "x2" )*x_factor;
		float Y2 = wndFile->PullAsFloat( obj_key + "y2" )*y_factor;
		String caption = TRANSLATE( wndFile->PullAsString( obj_key + "caption" ) );
		float size = wndFile->PullAsFloat( obj_key + "size", 1.0f ) * min( x_factor, y_factor );
		int val = wndFile->PullAsInt( obj_key + "value" );
		uint32 obj_flags = 0;
		uint32 obj_negative_flags = 0;

		if( X1<0 )	X1+=width;
		if( X2<0 )	X2+=width;
		if( Y1<0 )	Y1+=height;
		if( Y2<0 )	Y2+=height;
//		if( X1<0 )	X1+=SCREEN_W;
//		if( X2<0 )	X2+=SCREEN_W;
//		if( Y1<0 )	Y1+=SCREEN_H;
//		if( Y2<0 )	Y2+=SCREEN_H;

		if( wndFile->PullAsBool( obj_key + "can be clicked" ) )	obj_flags |= FLAG_CAN_BE_CLICKED;
		if( wndFile->PullAsBool( obj_key + "can get focus" ) )	obj_flags |= FLAG_CAN_GET_FOCUS;
		if( wndFile->PullAsBool( obj_key + "highlight" ) )		obj_flags |= FLAG_HIGHLIGHT;
		if( wndFile->PullAsBool( obj_key + "fill" ) )			obj_flags |= FLAG_FILL;
		if( wndFile->PullAsBool( obj_key + "hidden" ) )			obj_flags |= FLAG_HIDDEN;
		if( wndFile->PullAsBool( obj_key + "no border" ) )		obj_flags |= FLAG_NO_BORDER;

		if( wndFile->PullAsBool( obj_key + "cant be clicked" ) )	obj_negative_flags |= FLAG_CAN_BE_CLICKED;
		if( wndFile->PullAsBool( obj_key + "cant get focus" ) )		obj_negative_flags |= FLAG_CAN_GET_FOCUS;

		if( wndFile->PullAsBool( obj_key + "centered" ) ) {
			obj_flags |= FLAG_CENTERED;
			X1 -= gui_font.length( caption ) * size * 0.5f;
			}

		Vector<String> Entry = ReadVectorString( wndFile->PullAsString( obj_key + "entry" ) );
		for(uint32 e = 0 ; e < Entry.size() ; e++ )
			Entry[ e ] = TRANSLATE( Entry[ e ] );

		if( obj_type == "BUTTON" )
			Objets[i].create_button( X1, Y1, X2, Y2, caption, NULL, size );
		else if( obj_type == "FMENU" )
			Objets[i].create_menu( X1, Y1, Entry, NULL, size );
		else if( obj_type == "OPTIONB" )
			Objets[i].create_optionb( X1, Y1, caption, val, NULL, skin, size );
		else if( obj_type == "PBAR" )
			Objets[i].create_pbar( X1, Y1, X2, Y2, val, size );
		else if( obj_type == "TEXTBAR" )
			Objets[i].create_textbar( X1, Y1, X2, Y2, caption, val, NULL, size );
		else if( obj_type == "OPTIONC" )
			Objets[i].create_optionc( X1, Y1, caption, val, NULL, skin, size );
		else if( obj_type == "MENU" )
			Objets[i].create_menu( X1, Y1, X2, Y2, Entry, NULL, size );
		else if( obj_type == "TEXT" ) {
			Objets[i].create_text( X1, Y1, caption, val, size );
			if( X2 > 0 && Y2 > Y1 ) {
				Objets[i].x2 = X2;
				Objets[i].y2 = Y2;
				Objets[i].Flag |= FLAG_TEXT_ADJUST;
				}
			}
		else if( obj_type == "MISSION" ) {
			Objets[i].create_text( X1, Y1, caption, val, size );
			if( X2 > 0 && Y2 > Y1 ) {
				Objets[i].x2 = X2;
				Objets[i].y2 = Y2;
				Objets[i].Flag |= FLAG_TEXT_ADJUST | FLAG_MISSION_MODE | FLAG_CAN_BE_CLICKED;
				}
			}
		else if( obj_type == "LINE" )
			Objets[i].create_line( X1, Y1, X2, Y2, val );
		else if( obj_type == "BOX" )
			Objets[i].create_box( X1, Y1, X2, Y2, val );
		else if( obj_type == "IMG" ) {
			Objets[i].create_img( X1, Y1, X2, Y2, gfx->load_texture( TRANSLATE( wndFile->PullAsString( obj_key + "source" ) ) ) );
			Objets[i].destroy_img = Objets[i].Data != 0 ? true : false;
			}
		else if( obj_type == "LIST" )
			Objets[i].create_list( X1, Y1, X2, Y2, Entry, size );

		Objets[i].OnClick = ReadVectorString( wndFile->PullAsString( obj_key + "on click" ) );
		Objets[i].OnHover = ReadVectorString( wndFile->PullAsString( obj_key + "on hover" ) );
		Objets[i].SendDataTo = ReadVectorString( Lowercase( wndFile->PullAsString( obj_key + "send data to" ) ) );
		Objets[i].SendPosTo = ReadVectorString( Lowercase( wndFile->PullAsString( obj_key + "send pos to" ) ) );

		Objets[i].Flag |= obj_flags;
		Objets[i].Flag &= ~obj_negative_flags;
		}

	delete wndFile; 

	GuardLeave();
}

/*---------------------------------------------------------------------------\
|        Draw a window with the parameters from wnd                          |
\---------------------------------------------------------------------------*/

void draw_Window(wnd Wnd)
{
	gfx->rectfill(Wnd.x,Wnd.y,Wnd.x+Wnd.width-1,Wnd.y+Wnd.height-1,GrisM);
	gfx->rect(Wnd.x,Wnd.y,Wnd.x+Wnd.width-1,Wnd.y+Wnd.height-1,Noir);
	gfx->rect(Wnd.x+1,Wnd.y+1,Wnd.x+Wnd.width-2,Wnd.y+Wnd.height-2,GrisF);
	gfx->line(Wnd.x,Wnd.y,Wnd.x+Wnd.width-1,Wnd.y,Blanc);
	gfx->line(Wnd.x,Wnd.y,Wnd.x,Wnd.y+Wnd.height-1,Blanc);
	gfx->line(Wnd.x+1,Wnd.y+1,Wnd.x+Wnd.width-2,Wnd.y+1,GrisC);
	gfx->line(Wnd.x+1,Wnd.y+1,Wnd.x+1,Wnd.y+Wnd.height-2,GrisC);
	gfx->rectfill(Wnd.x+3,Wnd.y+3,Wnd.x+Wnd.width-4,Wnd.y+11,Bleu);
	gfx->print(gui_font,Wnd.x+4,Wnd.y+4,0.0f,Blanc,Wnd.Title);
}

/*---------------------------------------------------------------------------\
|        Déplace une fenêtre avec les paramètres Wnd de type window          |
\---------------------------------------------------------------------------*/

unsigned char WinMov(int AMx,int AMy,int AMb,int Mx,int My,int Mb,wnd *Wnd)
{
	byte WinMouse=0;
	if(AMb==1&&Mb==1)
	   if(AMx>=(*Wnd).x+3&&AMx<=(*Wnd).x+(*Wnd).width-4)
	      if(AMy>=(*Wnd).y+3&&AMy<=(*Wnd).y+11) {
	         (*Wnd).x+=Mx-AMx;
	         (*Wnd).y+=My-AMy;
	      }
	if(Mx>=(*Wnd).x&&Mx<=(*Wnd).x+(*Wnd).width&&My>=(*Wnd).y&&My<=(*Wnd).y+(*Wnd).height)
	   WinMouse=1;
	return WinMouse;
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

void ListBox(float x1,float y1, float x2, float y2,const Vector<String> &Entry,int Index, int Scroll, SKIN *skin, float size, uint32 flags )
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
	Vector< String > Entry;
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
	Vector< String > Entry;
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

void FloatMenu( float x, float y, const Vector<String> &Entry, int Index, int StartEntry, SKIN *skin, float size )
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

		gfx->print(gui_font,x1+skin->text_background.x1,y1+skin->text_background.y1,0.0f,use_normal_alpha_function ? Blanc : Noir,strtoprint);
		if(Etat) gfx->print(gui_font,x1+skin->text_background.x1+gui_font.length( strtoprint ),y1+skin->text_background.y1,0.0f,use_normal_alpha_function ? Blanc : Noir,"_");

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
List<String> Files;
List<String> Dir;
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
   gfx->print(gui_font,WAsk.x+5,WAsk.y+19,0.0f,Noir, TRANSLATE( "Nom du fichier:" ) );
   TextBar(WAsk.x+125,WAsk.y+16,WAsk.x+WAsk.width-5,WAsk.y+30,(char*)Name.c_str(),(retrace_count%60)>30);

                // Liste des dossiers
   gfx->print(gui_font,WAsk.x+21,WAsk.y+40,0.0f,Noir, TRANSLATE( "Liste des dossiers:" ) );
   TextBar(WAsk.x+5,WAsk.y+50,WAsk.x+190,WAsk.y+WAsk.height-10,"",false);
                // Affiche le nom des dossiers
   if(NbDir>0) {
   		int f=0;
      for(List<String>::iterator e=Dir.begin();e!=Dir.end();e++) {
         i=f-DecD;
         f++;
         if(i>=41) break;
         if(i>=0) {
            gfx->print(gui_font,WAsk.x+20,WAsk.y+56+8*i,0.0f,Noir,*e);
            gfx->line(WAsk.x+10,WAsk.y+53+8*i,WAsk.x+10,WAsk.y+60+i*8,Noir);
            gfx->line(WAsk.x+10,WAsk.y+60+8*i,WAsk.x+17,WAsk.y+60+i*8,Noir);
            }
         }
        }
   if(NbDir>0&&mouse_b==1&&mouse_x>=WAsk.x+7&&mouse_x<=WAsk.x+188
    &&mouse_y>=WAsk.y+56&&mouse_y<=WAsk.y+WAsk.height-10) {
      i=(mouse_y-WAsk.y-56)/8;
      e=i+DecD;
      List<String>::iterator curdir=Dir.begin();
      for(int f=0;f<e;f++)	curdir++;
      if(i<41&&e>=0&&e<NbDir) {
         if(strcmp(curdir->c_str(),".")!=0 && strcmp(curdir->c_str(),"..")!=0)
            CurDir+=String("/")+*curdir;
         else
            CurDir=dirname(CurDir);
         while(mouse_b!=0)
            poll_mouse();
		 gfx->unset_2D_mode();
         goto detect;
         }
      }
         
                // Liste des fichiers
   gfx->print(gui_font,WAsk.x+226,WAsk.y+40,0.0f,Noir, TRANSLATE( "Liste des fichiers:" ) );
   TextBar(WAsk.x+210,WAsk.y+50,WAsk.x+480,WAsk.y+WAsk.height-10,"",false);
                // Affiche le nom des fichiers
   if(NbFiles>0) {
   	  int f=0;
      for(List<String>::iterator e=Files.begin();e!=Files.end();e++) {
         i=f-DecF;
         f++;
         if(i>=41) break;
         if(i>=0) {
            gfx->print(gui_font,WAsk.x+225,WAsk.y+56+8*i,0.0f,Noir,*e);
            gfx->line(WAsk.x+215,WAsk.y+53+8*i,WAsk.x+215,WAsk.y+60+i*8,Noir);
            gfx->line(WAsk.x+215,WAsk.y+60+8*i,WAsk.x+222,WAsk.y+60+i*8,Noir);
            }
         }
      }
   if(NbFiles>0&&mouse_x>=WAsk.x+212&&mouse_x<=WAsk.x+478
    &&mouse_y>=WAsk.y+56&&mouse_y<=WAsk.y+WAsk.height-10) {
      i=(mouse_y-WAsk.y-56)/8;
      e=i+DecF;
      List<String>::iterator curfile=Files.begin();
      for(int f=0;f<e;f++)	curfile++;
      if(mouse_b==1&&i<41&&e>=0&&e<NbFiles)
		 Name=*curfile;
      }

                // Barres de défilement
	if(mouse_x>=WAsk.x+210 && mouse_x<=WAsk.x+480 && mouse_y>=WAsk.y+50 && mouse_y<=WAsk.y+WAsk.height-10) {
		DecF+=AMouseZ-mouse_z;
		if(DecF>NbFiles-40)	DecF=NbFiles-40;
		if(DecF<0)	DecF=0;
		}
	if(mouse_x>=WAsk.x+5 && mouse_x<=WAsk.x+190 && mouse_y>=WAsk.y+50 && mouse_y<=WAsk.y+WAsk.height-10) {
		DecD+=AMouseZ-mouse_z;
		if(DecD>NbDir-40)	DecD=NbDir-40;
		if(DecD<0)	DecD=0;
		}
                // Fichiers
   gfx->rectfill(WAsk.x+481,WAsk.y+50,WAsk.x+493,WAsk.y+WAsk.height-10,0xFF9F9F9F);
                // Haut
   if(mouse_b==1&&mouse_x>=WAsk.x+481&&mouse_x<=WAsk.x+493
    &&mouse_y>=WAsk.y+50&&mouse_y<=WAsk.y+62) {
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

/*---------------------- Functions related to the AREA object --------------------------------------------------------------*/

AREA::AREA( const String area_name ) : gui_hashtable(), cached_key(), wnd_hashtable()
{
	CreateCS();				// Thread safe model

	cached_wnd = NULL;

	name = area_name;		// Gives it a name

	vec_wnd.clear();		// Starts with an empty vector
	vec_z_order.clear();	// No windows at start

	background = 0;			// By default we have no background

	amx = mouse_x;
	amy = mouse_y;
	amz = mouse_z;
	amb = mouse_b;

	skin = NULL;			// Default: no skin

	scroll_timer = msec_timer;
	scrolling = false;

	InitInterface();		// Initialization of the interface
}

void AREA::destroy()
{
	cached_key.clear();
	cached_wnd = NULL;

	gui_hashtable.EmptyHashTable();
	gui_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );

	wnd_hashtable.EmptyHashTable();
	wnd_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );

	name.clear();

	for( uint16 i = 0 ; i < vec_wnd.size() ; i++ )
		delete vec_wnd[i];

	vec_wnd.clear();			// Empty the window vector
	vec_z_order.clear();		// No more windows at end

	gfx->destroy_texture( background );		// Destroy the texture (using safe destroyer)

	if( skin )					// Destroy the skin
		delete skin;
	skin = NULL;
}

AREA::~AREA()
{
	DeleteInterface();			// Shut down the interface

	cached_key.clear();
	cached_wnd = NULL;

	gui_hashtable.EmptyHashTable();
	wnd_hashtable.EmptyHashTable();

	name.clear();

	for( uint16 i = 0 ; i < vec_wnd.size() ; i++ )
		delete vec_wnd[i];

	vec_wnd.clear();			// Empty the window vector
	vec_z_order.clear();		// No more windows at end

	if( background )	gfx->destroy_texture( background );		// Destroy the texture

	if( skin )					// Destroy the skin
		delete skin;

	DeleteCS();					// End the safe thread things
}

uint32 AREA::InterfaceMsg( const lpcImsg msg )
{
	if( msg->MsgID != TA3D_IM_GUI_MSG )				// Only GUI messages
		return INTERFACE_RESULT_CONTINUE;

	if( msg->lpParm1 == NULL )	{
		GuardInfo( "AREA : bad format for interface message!\n" );
		return INTERFACE_RESULT_HANDLED;		// Oups badly written things
		}

	return this->msg( (char*) msg->lpParm1 );
}

int	AREA::msg( String message )				// Send that message to the area
{
	EnterCS();

	uint32	result = INTERFACE_RESULT_CONTINUE;

	message = Lowercase( message );				// Get the string associated with the signal

	int i = message.find( "." );
	if( i != -1 ) {
		String key = message.substr( 0, i );						// Extracts the key
		message = message.substr( i+1, message.size() - i -1 );		// Extracts the end of the message

		WND *the_wnd = get_wnd( key );
		
		if( the_wnd )
			result = the_wnd->msg( message );
		}
	else
		if( message == "clear" )	{
			for( uint16 i = 0 ; i < vec_wnd.size() ; i++ )
				delete vec_wnd[i];

			vec_wnd.clear();
			vec_z_order.clear();

			wnd_hashtable.EmptyHashTable();
			wnd_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );
			}
		else if( message == "end_the_game" )	{	}

	LeaveCS();

	return result;				// Ok we're done with it
}

bool AREA::get_state( const String &message )			// Return the state of specified object in the specified window
{
	EnterCS();

	int i = message.find( "." );
	if( i != -1 ) {
		String key = message.substr( 0, i );						// Extracts the key

		String obj_name = message.substr( i+1, message.size() - i -1 );

		if( key == "*" )
			for( uint16 e = 0 ; e < vec_wnd.size() ; e++ ) {				// Search the window containing the object corresponding to the key
				GUIOBJ *the_obj = vec_wnd[ e ]->get_object( obj_name );
				if( the_obj ) {
					bool result = the_obj->Etat;
					LeaveCS();
					return result;	// Return what we found
					}
				}
		else {
			WND *the_wnd = get_wnd( key );

			if( the_wnd ) {
				bool result = the_wnd->get_state( obj_name );
				LeaveCS();
				return result;
				}
			}
		}
	else {
		WND *the_wnd = get_wnd( message );

		if( the_wnd ) {
			bool result = the_wnd->get_state( "" );
			LeaveCS();
			return result;
			}
		}
	LeaveCS();
	return false;
}

sint32 AREA::get_value( const String &message )			// Return the state of specified object in the specified window
{
	EnterCS();
	int i = message.find( "." );
	if( i != -1 ) {
		String key = message.substr( 0, i );						// Extracts the key

		String obj_name = message.substr( i+1, message.size() - i -1 );

		if( key == "*" )
			for( uint16 e = 0 ; e < vec_wnd.size() ; e++ ) {				// Search the window containing the object corresponding to the key
				GUIOBJ *the_obj = vec_wnd[ e ]->get_object( obj_name );
				if( the_obj ) {
					sint32 v = the_obj->Value;
					LeaveCS();
					return v;	// Return what we found
					}
				}
		else {
			WND *the_wnd = get_wnd( key );

			if( the_wnd ) {
				sint32 v = the_wnd->get_value( obj_name );
				LeaveCS();
				return v;
				}
			}
		}
	LeaveCS();
	return -1;
}

String AREA::get_caption( const String &message )		// Return the caption of specified object in the specified window
{
	EnterCS();
	int i = message.find( "." );
	if( i != -1 ) {
		String key = message.substr( 0, i );						// Extracts the key

		String obj_name = message.substr( i+1, message.size() - i -1 );

		if( key == "*" )
			for( uint16 e = 0 ; e < vec_wnd.size() ; e++ ) {				// Search the window containing the object corresponding to the key
				GUIOBJ *the_obj = vec_wnd[ e ]->get_object( obj_name );
				if( the_obj ) {
					String result;
					if( the_obj->Text.size() > 0 )
						result = the_obj->Text[0];	// Return what we found
					LeaveCS();
					return result;
					}
				}
		else {
			WND *the_wnd = get_wnd( key );

			if( the_wnd ) {
				String result = the_wnd->get_caption( obj_name );
				LeaveCS();
				return result;
				}
			}
		}
	LeaveCS();
	return "";
}

GUIOBJ *AREA::get_object( const String &message, bool skip_hidden )		// Return a pointer to the specified object
{
	EnterCS();
	int i = message.find( "." );
	if( i != -1 ) {
		String key = message.substr( 0, i );						// Extracts the key

		String obj_name = message.substr( i+1, message.size() - i -1 );

		if( key == "*" )
			for( uint16 e = 0 ; e < vec_wnd.size() ; e++ ) {				// Search the window containing the object corresponding to the key
				GUIOBJ *the_obj = vec_wnd[ e ]->get_object( obj_name );
				if( the_obj ) {
					LeaveCS();
					return the_obj;
					}
				}
		else {
			WND *the_wnd = get_wnd( key );

			if( the_wnd ) {
				GUIOBJ *the_obj = the_wnd->get_object( obj_name );
				LeaveCS();
				return the_obj;
				}
			}
		}
	LeaveCS();
	return NULL;
}

WND	*AREA::get_wnd( const String &message )			// Return the specified window
{
	EnterCS();
	String lmsg = Lowercase( message );
	if( lmsg == cached_key && cached_wnd ) {
		WND *the_wnd = cached_wnd;
		LeaveCS();
		return the_wnd;
		}

	sint16 e = wnd_hashtable.Find( lmsg ) - 1;
	if( e >= 0 ) {
		cached_key = lmsg;
		cached_wnd = vec_wnd[ e ];
		WND *the_wnd = vec_wnd[ e ];
		LeaveCS();
		return the_wnd;
		}
	LeaveCS();
	return NULL;
}

void AREA::set_enable_flag( const String &message, const bool &enable )		// Set the enabled/disabled state of specified object in the specified window
{
	EnterCS();
	GUIOBJ	*guiobj = get_object( message );
	if( guiobj ) {
		if( enable )
			guiobj->Flag &= ~FLAG_DISABLED;
		else
			guiobj->Flag |= FLAG_DISABLED;
		}
	LeaveCS();
}

void AREA::set_state( const String &message, const bool &state )			// Set the state of specified object in the specified window
{
	EnterCS();
	GUIOBJ	*guiobj = get_object( message );
	if( guiobj )
		guiobj->Etat = state;
	LeaveCS();
}

void AREA::set_value( const String &message, const sint32 &value )			// Set the value of specified object in the specified window
{
	EnterCS();
	GUIOBJ	*guiobj = get_object( message );
	if( guiobj )
		guiobj->Value = value;
	LeaveCS();
}

void AREA::set_data( const String &message, const sint32 &data )			// Set the value of specified object in the specified window
{
	EnterCS();
	GUIOBJ	*guiobj = get_object( message );
	if( guiobj )
		guiobj->Data = data;
	LeaveCS();
}

void AREA::set_caption( const String &message, const String &caption )		// set the caption of specified object in the specified window
{
	EnterCS();
	GUIOBJ	*guiobj = get_object( message );
	if( guiobj && guiobj->Text.size() > 0 ) {
		if( guiobj->Flag & FLAG_CENTERED ) {
			float length = gui_font.length( guiobj->Text[ 0 ] ) * guiobj->s;
			guiobj->x1 += length * 0.5f;
			guiobj->x2 -= length * 0.5f;
			}
		guiobj->Text[0] = caption;
		if( guiobj->Flag & FLAG_CENTERED ) {
			float length = gui_font.length( guiobj->Text[ 0 ] ) * guiobj->s;
			guiobj->x1 -= length * 0.5f;
			guiobj->x2 += length * 0.5f;
			}
		}
	LeaveCS();
}

uint16 AREA::check()					// Checks events for all windows
{
	poll_mouse();
	poll_keyboard();
	uint16 is_on_gui = 0;
	bool scroll = msec_timer - scroll_timer >= 250;
	if( scroll )
		while( msec_timer - scroll_timer >= 250 )
			scroll_timer += 250;
	EnterCS();
	for( uint16 i = 0 ; i < vec_wnd.size() ; i++ )
		if( !is_on_gui || ( vec_wnd[ vec_z_order[ i ] ]->get_focus && !vec_wnd[ vec_z_order[ i ] ]->hidden ) ) {
			is_on_gui |= vec_wnd[ vec_z_order[ i ] ]->check( amx, amy, amz, amb, scroll, skin );			// Do things in the right order
			if( ( (is_on_gui && mouse_b && !vec_wnd[ vec_z_order[ 0 ] ]->get_focus) || vec_wnd[ vec_z_order[ i ] ]->get_focus ) && i > 0 && !vec_wnd[ vec_z_order[ i ] ]->background_wnd ) {			// Change the focus
				uint16 old = vec_z_order[ i ];
				for( uint16 e = i ; e > 0 ; e-- )
					vec_z_order[ e ] = vec_z_order[ e - 1 ];
				vec_z_order[ 0 ] = old;				// Get the focus
				}
			}
	LeaveCS();

	if( Console == NULL || !Console->activated() )
		clear_keybuf();

	scrolling = scroll;
	amx = mouse_x;
	amy = mouse_y;
	amz = mouse_z;
	amb = mouse_b;

	return is_on_gui;
}

uint16 AREA::load_window( const String &filename )
{
	EnterCS();

	uint16 wnd_idx = vec_wnd.size();
	vec_wnd.push_back( new WND );				// Adds a window to the vector
	vec_z_order.push_back( wnd_idx );

	if( Lowercase( filename.substr( filename.length() - 4, 4 ) ) == ".gui" )
		vec_wnd[ wnd_idx ]->load_gui( filename, gui_hashtable );		// Loads the window from a *.gui file
	else
		vec_wnd[ wnd_idx ]->load_tdf( filename, skin );	// Loads the window from a *.tdf file

	for( uint16 i = wnd_idx ; i > 0 ; i-- )		// The new window appear on top of the others
		vec_z_order[ i ] = vec_z_order[ i-1 ];
	vec_z_order[ 0 ] = wnd_idx;

	wnd_hashtable.Insert( Lowercase( vec_wnd[ wnd_idx ]->Name ), wnd_idx + 1 );		// + 1 because it returns 0 on Find failure

	LeaveCS();

	return wnd_idx;
}

void AREA::draw()
{
	if( background ) {
		gfx->drawtexture( background, 0, 0, gfx->width, gfx->height );
		glDisable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, 0 );
		}

	EnterCS();

	String help_msg = "";
	for( sint32 i = vec_wnd.size() - 1 ; i >=0 ; i-- )			// Draws all the windows in focus reversed order so the focused window is drawn on top of the others
		vec_wnd[ vec_z_order[ i ] ]->draw( help_msg, i == 0, true , skin );
	if( help_msg != "" )
		PopupMenu( mouse_x + 20, mouse_y + 20, help_msg, skin );

	LeaveCS();
}

void AREA::load_tdf( const String &filename )			// Loads a TDF file telling which windows to load and which skin to use
{
	GuardEnter( AREA::load_tdf );

	destroy();		// In case there is an area loaded so we don't waste memory

	cTAFileParser *areaFile;

	String skin_name = ( lp_CONFIG != NULL && !lp_CONFIG->skin_name.empty() ) ? lp_CONFIG->skin_name : "";
	if( skin_name != "" && TA3D_exists( skin_name ) ) {			// Loads a skin
		skin = new SKIN;
		skin->load_tdf( skin_name, 1.0f );
		}

	try { // we need to try catch this cause the config file may not exists
		 // and if it don't exists it will throw an error on reading it, which
		 // will be caught in our main function and the application will exit.
		String real_filename = filename;
		if( skin != NULL && !skin->prefix.empty() ) {
			int name_len = strlen( get_filename( real_filename.c_str() ) );
			if( name_len > 0 )
				real_filename = real_filename.substr( 0, real_filename.size() - name_len ) + skin->prefix + get_filename( real_filename.c_str() );
			else
				real_filename += skin->prefix;
			if( !HPIManager->Exists( real_filename ) )		// If it doesn't exist revert to the default name
				real_filename = filename;
			}
		if( skin )	delete( skin );
		skin = NULL;
		areaFile = new TA3D::UTILS::cTAFileParser( real_filename );
	}
	catch( ... )
	{
		GuardLeave();
		return;
	}

	name = filename;		// Grab the area's name

	sint32 e = name.find( "." );		// Extracts the file name

	if( e != -1 )	name = name.substr( 0, e );

	e = name.find_last_of( "/\\" );

	if( e != -1 )	name = name.substr( e + 1, name.size() - e - 1 );

	name = areaFile->PullAsString( "area.name", name );					// The TDF may override the area name

	skin_name = ( lp_CONFIG != NULL && !lp_CONFIG->skin_name.empty() ) ? lp_CONFIG->skin_name : areaFile->PullAsString( "area.skin" );
	if( TA3D_exists( skin_name ) ) {			// Loads a skin
		int area_width = areaFile->PullAsInt( "area.width", SCREEN_W );
		int area_height = areaFile->PullAsInt( "area.height", SCREEN_W );
		float skin_scale = min( (float)SCREEN_H / area_height, (float)SCREEN_W / area_width );
		skin = new SKIN;
		skin->load_tdf( skin_name, skin_scale );
		}

	Vector< String > windows_to_load = ReadVectorString( areaFile->PullAsString( "area.windows" ) );
	for( uint16 i = 0 ; i < windows_to_load.size() ; i++ )
		load_window( windows_to_load[ i ] );

	String background_name = areaFile->PullAsString( "area.background" );
	if( skin && !skin->prefix.empty() ) {
		int name_len = strlen( get_filename( background_name.c_str() ) );
		if( name_len > 0 )
			background_name = background_name.substr( 0, background_name.size() - name_len ) + skin->prefix + get_filename( background_name.c_str() );
		else
			background_name += skin->prefix;
		}
	if( TA3D_exists( background_name ) )			// Loads a background image
		background = gfx->load_texture( background_name );
	else if( skin && !skin->prefix.empty() ) {
		background_name = areaFile->PullAsString( "area.background" );			// No prefixed version, retry with default background
		if( TA3D_exists( background_name ) )			// Loads a background image
			background = gfx->load_texture( background_name );
		}

	delete areaFile; 

	GuardLeave();
}

/*---------------------- Functions related to the SKIN object --------------------------------------------------------------*/

void SKIN_OBJECT::load( const String filename, const String prefix, cTAFileParser *parser, float border_size )
{
	if( TA3D_exists( filename ) ) {
		tex = gfx->load_texture( filename, FILTER_LINEAR, &w, &h );

		x1 = parser->PullAsInt( prefix + "x1" );
		y1 = parser->PullAsInt( prefix + "y1" );
		x2 = parser->PullAsInt( prefix + "x2" );
		y2 = parser->PullAsInt( prefix + "y2" );

		t_x1 = w ? ((float)x1) / w : 0.0f;
		t_x2 = w ? ((float)x2) / w : 0.0f;
		t_y1 = h ? ((float)y1) / h : 0.0f;
		t_y2 = h ? ((float)y2) / h : 0.0f;

		x2 -= w;
		y2 -= h;

		border_size *= parser->PullAsFloat( prefix + "scale", 1.0f );		// Allow scaling the widgets

		x1 *= border_size;
		y1 *= border_size;
		x2 *= border_size;
		y2 *= border_size;
		sw = w * border_size;
		sh = h * border_size;
		}
}

void SKIN_OBJECT::draw( float X1, float Y1, float X2, float Y2, bool bkg )
{
	gfx->drawtexture( tex , X1, Y1, X1 + x1, Y1 + y1, 0.0f, 0.0f, t_x1, t_y1 );
	gfx->drawtexture( tex , X1 + x1, Y1, X2 + x2, Y1 + y1, t_x1, 0.0f, t_x2, t_y1 );
	gfx->drawtexture( tex , X2 + x2, Y1, X2, Y1 + y1, t_x2, 0.0f, 1.0f, t_y1 );

	gfx->drawtexture( tex , X1, Y1 + y1, X1 + x1, Y2 + y2, 0.0f, t_y1, t_x1, t_y2 );
	gfx->drawtexture( tex , X2 + x2, Y1 + y1, X2, Y2 + y2, t_x2, t_y1, 1.0f, t_y2 );

	gfx->drawtexture( tex , X1, Y2 + y2, X1 + x1, Y2, 0.0f, t_y2, t_x1, 1.0f );
	gfx->drawtexture( tex , X1 + x1, Y2 + y2, X2 + x2, Y2, t_x1, t_y2, t_x2, 1.0f );
	gfx->drawtexture( tex , X2 + x2, Y2 + y2, X2, Y2, t_x2, t_y2, 1.0f, 1.0f );

	if( bkg )
		gfx->drawtexture( tex , X1 + x1, Y1 + y1, X2 + x2, Y2 + y2, t_x1, t_y1, t_x2, t_y2 );
}

void SKIN::init()
{
	prefix = "";

	for( int i = 0 ; i < 2 ; i++ )
		button_img[i].init();
	text_background.init();
	menu_background.init();
	wnd_border.init();
	wnd_title_bar.init();
	selection_gfx.init();
	for( int i = 0 ; i < 2 ; i++ )
		progress_bar[i].init();

	wnd_background = 0;
	checkbox[1].init();
	checkbox[0].init();
	option[1].init();
	option[0].init();
	scroll[2].init();
	scroll[1].init();
	scroll[0].init();
}

void SKIN::destroy()
{
	for( int i = 0 ; i < 2 ; i++ ) {
		progress_bar[i].destroy();
		button_img[i].destroy();
		checkbox[i].destroy();
		option[i].destroy();
		scroll[i].destroy();
		}
	scroll[2].destroy();
	text_background.destroy();
	menu_background.destroy();
	wnd_border.destroy();
	wnd_title_bar.destroy();
	selection_gfx.destroy();

	prefix.clear();

	Name.clear();
	gfx->destroy_texture(  wnd_background );
}

void SKIN::load_tdf( const String &filename, float skin_scale )			// Loads the skin from a TDF file
{
	GuardEnter( SKIN::load_tdf );

	destroy();		// In case there is a skin loaded so we don't waste memory

	cTAFileParser *skinFile;

	try { // we need to try catch this cause the config file may not exists
		 // and if it don't exists it will throw an error on reading it, which
		 // will be caught in our main function and the application will exit.
		skinFile = new TA3D::UTILS::cTAFileParser( filename );
	}
	catch( ... )
	{
		GuardLeave();
		return;
	}

	Name = filename;		// Grab the skin's name, so we can now if a skin is already in use

	sint32 e = Name.find( "." );		// Extracts the file name

	if( e != -1 )	Name = Name.substr( 0, e );

	e = Name.find_last_of( "/\\" );

	if( e != -1 )	Name = Name.substr( e + 1, Name.size() - e - 1 );

	Name = skinFile->PullAsString( "skin.name", Name );					// The TDF may override the skin name

	prefix = skinFile->PullAsString( "skin.prefix", "" );				// The prefix to use for 

	wnd_border.load( skinFile->PullAsString( "skin.window borders" ), "skin.border_", skinFile, skin_scale );
	button_img[0].load( skinFile->PullAsString( "skin.button0" ), "skin.button_", skinFile, skin_scale );
	button_img[1].load( skinFile->PullAsString( "skin.button1" ), "skin.button_", skinFile, skin_scale );
	text_background.load( skinFile->PullAsString( "skin.text background" ), "skin.text_", skinFile, skin_scale );
	menu_background.load( skinFile->PullAsString( "skin.menu background" ), "skin.menu_", skinFile, skin_scale );
	wnd_title_bar.load( skinFile->PullAsString( "skin.title bar" ), "skin.title_", skinFile, skin_scale );
	progress_bar[0].load( skinFile->PullAsString( "skin.progress bar0" ), "skin.bar0_", skinFile, skin_scale );
	progress_bar[1].load( skinFile->PullAsString( "skin.progress bar1" ), "skin.bar1_", skinFile, skin_scale );
	selection_gfx.load( skinFile->PullAsString( "skin.selection" ), "skin.selection_", skinFile, skin_scale );
	option[0].load( skinFile->PullAsString( "skin.option0" ), "skin.option_", skinFile, skin_scale );
	option[1].load( skinFile->PullAsString( "skin.option1" ), "skin.option_", skinFile, skin_scale );
	checkbox[0].load( skinFile->PullAsString( "skin.checkbox0" ), "skin.checkbox_", skinFile, skin_scale );
	checkbox[1].load( skinFile->PullAsString( "skin.checkbox1" ), "skin.checkbox_", skinFile, skin_scale );

	scroll[0].load( skinFile->PullAsString( "skin.v_scroll" ), "skin.v_scroll_", skinFile, skin_scale );
	scroll[1].load( skinFile->PullAsString( "skin.h_scroll" ), "skin.h_scroll_", skinFile, skin_scale );
	scroll[2].load( skinFile->PullAsString( "skin.s_scroll" ), "skin.s_scroll_", skinFile, skin_scale );

	String tex_file_name = skinFile->PullAsString( "skin.window background" );
	if( TA3D_exists( tex_file_name ) )	wnd_background = gfx->load_texture( tex_file_name, FILTER_LINEAR );

	delete skinFile; 

	GuardLeave();
}
