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

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "intro.h"
#include "console.h"

#define NB_TXT_LINES	15
#define LAST_LINE		14
#define INTRO_TOP		580.0f

void play_intro(const char *txt_file)
{
	GLuint glfond = 0;

	if( !lp_CONFIG->skin_name.empty() && TA3D_exists( lp_CONFIG->skin_name ) ) {			// Loads a skin
		SKIN *skin = new SKIN;
		skin->load_tdf( lp_CONFIG->skin_name );

		if( !skin->prefix.empty() )
			glfond = gfx->load_texture("gfx/" + skin->prefix + "intro.jpg");
		else
			glfond = gfx->load_texture("gfx/intro.jpg");

		delete skin;
		}
	else
		glfond = gfx->load_texture("gfx/intro.jpg");

	bool done=false;

	gfx->set_2D_mode();
	glScalef(SCREEN_W/1280.0f,SCREEN_H/1024.0f,1.0f);

	FILE *gpl=TA3D_OpenFile(txt_file,"rb");

	char *text_line[NB_TXT_LINES];
	int i;
	for(i=0;i<NB_TXT_LINES;i++) {
		text_line[i]=new char[100];
		text_line[i][0]=0;
		}

	float dec=0.0f;
	float dt=0.0f;
	int time = msec_timer;
	float Conv = 0.001f;
	bool read=true;

	float TA_font_size = gfx->TA_font.get_size();
	gfx->TA_font.change_size( 1.5f );
	float h = gfx->TA_font.height();

	do
	{
		do
		{
			dt=(msec_timer-time)*Conv;
			rest(1);
		}while(dt<0.02f);
		time = msec_timer;

		dec-=dt*h;
		if(dec<=-h) {		// Décale tout et lit une autre ligne
			dec=0.0f;
			for(i=0;i<LAST_LINE;i++) {
				text_line[i][0]=0;
				strcat(text_line[i],text_line[i+1]);
				}
			text_line[LAST_LINE][0]=0;
			if(read) {
				if(fgets(text_line[LAST_LINE],100,gpl)==NULL)
					read=false;
				if(read) {
					for(i=0;i<strlen(text_line[LAST_LINE]);i++) {
						if(text_line[LAST_LINE][i]==10 || text_line[LAST_LINE][i]==13)
							text_line[LAST_LINE][i]=0;
						if(text_line[LAST_LINE][i]==9) {
							for(int e=strlen(text_line[LAST_LINE])+4;e>i+4;e--)
								text_line[LAST_LINE][e]=text_line[LAST_LINE][e-4];
							text_line[LAST_LINE][i]=32;
							text_line[LAST_LINE][i+1]=32;
							text_line[LAST_LINE][i+2]=32;
							text_line[LAST_LINE][i+3]=32;
							}
						}
					replace_chars(text_line[LAST_LINE]);
					}
				}
			else {
				bool vide=true;
				for(i=0;i<NB_TXT_LINES;i++)
					if(text_line[i][0]!=0) {
						vide=false;
						break;
						}
				done|=vide;
				}
			}

		if(mouse_b || keypressed())	done=true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

		gfx->drawtexture(glfond,0.0f,0.0f,1280.0f,1024.0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		for(i=0;i<NB_TXT_LINES;i++)
			gfx->print(gfx->TA_font,220.0f,INTRO_TOP+(i+2)*h+dec,0.0f,text_line[i]);

		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D,glfond);
		glBegin(GL_QUADS);
			glColor4f(1.0f,1.0f,1.0f,0.0f);
			glTexCoord2f(0.0f,(INTRO_TOP+h)/1024.0f);					glVertex2f(0.0f,INTRO_TOP+h);
			glTexCoord2f(1.0f,(INTRO_TOP+h)/1024.0f);					glVertex2f(1280.0f,INTRO_TOP+h);
			glTexCoord2f(1.0f,(INTRO_TOP+2*h)/1024.0f);					glVertex2f(1280.0f,INTRO_TOP+2*h);
			glTexCoord2f(0.0f,(INTRO_TOP+2*h)/1024.0f);					glVertex2f(0.0f,INTRO_TOP+2*h);

			glTexCoord2f(0.0f,(INTRO_TOP+2*h)/1024.0f);					glVertex2f(0.0f,INTRO_TOP+2*h);
			glTexCoord2f(1.0f,(INTRO_TOP+2*h)/1024.0f);					glVertex2f(1280.0f,INTRO_TOP+2*h);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glTexCoord2f(1.0f,(INTRO_TOP+4*h)/1024.0f);					glVertex2f(1280.0f,INTRO_TOP+4*h);
			glTexCoord2f(0.0f,(INTRO_TOP+4*h)/1024.0f);					glVertex2f(0.0f,INTRO_TOP+4*h);

			glTexCoord2f(0.0f,(INTRO_TOP+LAST_LINE*h)/1024.0f);			glVertex2f(0.0f,INTRO_TOP+LAST_LINE*h);
			glTexCoord2f(1.0f,(INTRO_TOP+LAST_LINE*h)/1024.0f);			glVertex2f(1280.0f,INTRO_TOP+LAST_LINE*h);
			glColor4f(1.0f,1.0f,1.0f,0.0f);
			glTexCoord2f(1.0f,(INTRO_TOP+(LAST_LINE+2)*h)/1024.0f);		glVertex2f(1280.0f,INTRO_TOP+(LAST_LINE+2)*h);
			glTexCoord2f(0.0f,(INTRO_TOP+(LAST_LINE+2)*h)/1024.0f);		glVertex2f(0.0f,INTRO_TOP+(LAST_LINE+2)*h);

			glTexCoord2f(0.0f,(INTRO_TOP+(LAST_LINE+2)*h)/1024.0f);		glVertex2f(0.0f,INTRO_TOP+(LAST_LINE+2)*h);
			glTexCoord2f(1.0f,(INTRO_TOP+(LAST_LINE+2)*h)/1024.0f);		glVertex2f(1280.0f,INTRO_TOP+(LAST_LINE+2)*h);
			glTexCoord2f(1.0f,(INTRO_TOP+(LAST_LINE+3)*h)/1024.0f);		glVertex2f(1280.0f,INTRO_TOP+(LAST_LINE+3)*h);
			glTexCoord2f(0.0f,(INTRO_TOP+(LAST_LINE+3)*h)/1024.0f);		glVertex2f(0.0f,INTRO_TOP+(LAST_LINE+3)*h);
		glEnd();
		glDisable(GL_BLEND);

		gfx->flip();
	}while(!done);

	gfx->TA_font.change_size( TA_font_size );

	for(i=0;i<NB_TXT_LINES;i++)
		if(text_line[i])
			delete[] text_line[i];

	fclose(gpl);

	gfx->unset_2D_mode();

	glDeleteTextures(1,&glfond);
}

GLuint Glfond = 0;

List< String >	messages;

int last_percent = 0;

void loading(float percent,const String &msg)
{
	if( network_manager.isConnected() && last_percent != (int)percent ) {
		last_percent = (int)percent;
		network_manager.sendAll(format("LOADING %d", last_percent));
		}

	set_uformat(U_UTF8);

	bool init=(Glfond==0);
	if(init) {
		messages.clear();
		if( !lp_CONFIG->skin_name.empty() && TA3D_exists( lp_CONFIG->skin_name ) ) {			// Loads a skin
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

	if( messages.empty() || Lowercase( messages.front() ) != Lowercase( msg ) ) {
		if( !messages.empty() )
			messages.front() = messages.front() + " - " + TRANSLATE( "done" );
		messages.push_front( msg );
		}

	int e = 0;
	for( List< String >::iterator i = messages.begin() ; i != messages.end() ; i++, e++ )
		gfx->print( gfx->TA_font, 105.0f, 175.0f + h * e, 0.0f, 0xFFFFFFFF, *i );

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

	if( lp_CONFIG->draw_console_loading ) {				// If set in config
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
