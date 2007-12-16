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
|                                    menu.cpp                                  |
|       Ce module contient les routines du menu de TA3D                        |
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
#include "3do.h"					// Pour la lecture des fichiers 3D
#include "cob.h"					// Pour la lecture et l'éxecution des scripts
//#include "fbi.h"					// Pour la gestion des unités
#include "EngineClass.h"			// Inclus le moteur
#include "tnt.h"					// Inclus le chargeur de cartes
#include "menu.h"
#include "gui.h"
#include "taconfig.h"

using namespace TA3D::EXCEPTION;

#define p_size			10.0f
#define MENU_NB_PART	200

	// Some functions from main.cpp used to deal with config file

void LoadConfigFile( void );
void makeBackup( const String FileName );
void restoreBackup( const String FileName );
void SaveConfigFile( void );

void main_menu(void)
{
	GuardEnter( main_menu );

	while( mouse_b || key[ KEY_ENTER ] || key[ KEY_ESC ] || key[ KEY_SPACE ] )	rest( 10 );

	GLuint tex_formatA,tex_format;
	if(g_useTextureCompression) {
		tex_formatA=GL_COMPRESSED_RGBA_ARB;
		tex_format=GL_COMPRESSED_RGB_ARB;
		}
	else {
		tex_formatA=GL_RGBA8;
		tex_format=GL_RGB8;
		}

	bool done=false;

	gfx->set_2D_mode();

	gfx->ReInitTexSys();

	glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);

	int i;

	float dt=0.0f;
	int time = msec_timer;
	float Conv = 0.001f;

	BITMAP *tst[3];
	GLuint mnu[3];
	GLuint flame = gfx->load_texture( "gfx/part.tga", FILTER_LINEAR );
	mnu[0] = gfx->load_texture( TRANSLATE( "gfx/en/exit.tga" ).c_str(), FILTER_LINEAR );
	mnu[1] = gfx->load_texture( TRANSLATE( "gfx/en/options.tga" ).c_str(), FILTER_LINEAR );
	mnu[2] = gfx->load_texture( TRANSLATE( "gfx/en/play.tga" ).c_str(), FILTER_LINEAR );
	set_color_depth( 32 );
	tst[0] = load_bitmap( TRANSLATE( "gfx/en/exit.tga" ).c_str(),NULL);
	tst[1] = load_bitmap( TRANSLATE( "gfx/en/options.tga" ).c_str(),NULL);
	tst[2] = load_bitmap( TRANSLATE( "gfx/en/play.tga" ).c_str(),NULL);

	int tst_w[3];
	int tst_h[3];

	for( i = 0 ; i < 3 ; i++ )
		if( tst[i] ) {
			tst_w[i] = tst[i]->w >> 1;
			tst_h[i] = tst[i]->h >> 1;
			}
		else
			tst_w[i] = tst_h[i] = 0;

	int px[3]={	320,	160,	480 };
	int py[3]={	400,	300,	300 };

	VECTOR part[ MENU_NB_PART ];
	VECTOR target[ MENU_NB_PART ];
	for( i = 0 ; i < MENU_NB_PART ; i++ ) {
		part[i].x=-p_size;	part[i].y=-p_size;
		part[i].z=0.0f;
		target[i].x = target[i].y = -p_size;
		target[i].z = 0.0f;
		}

	int n=0;

	cursor_type=CURSOR_DEFAULT;

	String current_mod = TA3D_CURRENT_MOD.length() > 6 ? TA3D_CURRENT_MOD.substr( 5, TA3D_CURRENT_MOD.length() - 6 ) : "";

	do
	{
		poll_keyboard();

		do
		{
			dt = (msec_timer-time)*Conv;
			rest(1);
		}while(dt<0.01f);
		time = msec_timer;

		if(key[KEY_ESC])	done=true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

		gfx->drawtexture(gfx->glfond,0.0f,0.0f,640.0f,480.0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		gfx->print(gfx->TA_font,320.0f-gfx->TA_font.length(TA3D_ENGINE_VERSION)*0.5f,448.0f,0.0f,0xFFFFFFFF,TA3D_ENGINE_VERSION);
		if( !current_mod.empty() )
			gfx->print(gfx->TA_font,320.0f-gfx->TA_font.length("MOD: " + current_mod)*0.5f,430.0f,0.0f,0xFFFFFFFF,"MOD: " + current_mod);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		int index=-1;
		for(i=0;i<3;i++)
			if(px[i]-tst_w[i] < mouse_x*gfx->SCREEN_W_TO_640 && px[i]+tst_w[i] > mouse_x*gfx->SCREEN_W_TO_640 && py[i]-tst_h[i] < mouse_y*gfx->SCREEN_H_TO_480 && py[i]+tst_h[i] > mouse_y*gfx->SCREEN_H_TO_480 ) {
				index=i;
				i=3;
				}

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE);

		for( i = 0 ; i < MENU_NB_PART ; i++ )
			part[i] += 3.0f * dt * (target[i] - part[i]);

		part[ n % MENU_NB_PART ].x = 576 + ( (TA3D_RAND() % 2001) - 1000 ) * 0.002f;
		part[ n % MENU_NB_PART ].y = 256 + ( (TA3D_RAND() % 2001) - 1000 ) * 0.002f;

		switch(index)
		{
		case -1:
			target[ n % MENU_NB_PART ] = part[ n % MENU_NB_PART ];
			break;
		case 0:
		case 1:
		case 2:
			{
				int x;
				int y;
				do
				{
					x = TA3D_RAND() % tst[index]->w;
					y = TA3D_RAND() % tst[index]->h;
				}while( tst[index]->line[y][(x<<2)+3] < 10 );
				target[ n % MENU_NB_PART ].x = px[index] + x - tst_w[ index ];
				target[ n % MENU_NB_PART ].y = py[index] + y - tst_h[ index ];
			}
			break;
		};

		for( i = 0 ; i < MENU_NB_PART ; i++ ) {
			int d = MENU_NB_PART - ( ( n - i ) % MENU_NB_PART );
			float coef = sqrt( ((float)d) / MENU_NB_PART );
			glColor4f( coef, coef, coef, coef );
			glPushMatrix();
			glTranslatef(part[i].x,part[i].y,0.0f);
			glRotatef((i&1) ? n : -n,0.0f,0.0f,1.0f);
			glTranslatef(-part[i].x,-part[i].y,0.0f);
			gfx->drawtexture(flame,part[i].x-p_size,part[i].y-p_size,part[i].x+p_size,part[i].y+p_size);
			glPopMatrix();
			}
		n++;
		glColor4f(1.0f,1.0f,1.0f,1.0f);

		for(i=0;i<3;i++) {
			if(i==index)
				glBlendFunc(GL_SRC_COLOR,GL_ONE_MINUS_SRC_ALPHA);
			else
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				gfx->drawtexture(mnu[i],px[i]-tst_w[i],py[i]-tst_h[i],px[i]+tst_w[i],py[i]+tst_h[i]);
			}
		glColor4f(1.0f,1.0f,1.0f,1.0f);

		glDisable(GL_BLEND);

		draw_cursor();

		if( mouse_b || key[KEY_ENTER] || key[KEY_SPACE] || lp_CONFIG->quickstart ) {
			if( key[KEY_ENTER] )	index = 2;			// Shortcut to game room
			if( key[KEY_SPACE] )	index = 1;			// Shortcut to config menu

			while( key[KEY_ENTER] || key[KEY_SPACE] )	{ rest( 20 );	poll_keyboard();	}
			clear_keybuf();

			gfx->SCREEN_W_TO_640 = 1.0f;				// To have mouse sensibility undependent from the resolution
			gfx->SCREEN_H_TO_480 = 1.0f;
			if( lp_CONFIG->quickstart )
				index = 1;
			if( index >= 0 && index <= 2 ) {		// free some memory
				destroy_bitmap(tst[0]);
				destroy_bitmap(tst[1]);
				destroy_bitmap(tst[2]);

				glDeleteTextures(1,&mnu[0]);
				glDeleteTextures(1,&mnu[1]);
				glDeleteTextures(1,&mnu[2]);
				}
			switch(index)
			{
			case 0:
				done=true;
				break;
			case 1:
				config_menu();
				lp_CONFIG->quickstart = false;
				break;
			case 2:
				setup_game();
				break;
			};
			current_mod = TA3D_CURRENT_MOD.length() > 6 ? TA3D_CURRENT_MOD.substr( 5, TA3D_CURRENT_MOD.length() - 6 ) : "";
			if( index >= 0 && index <= 2 ) {
				mnu[0] = gfx->load_texture( TRANSLATE( "gfx/en/exit.tga" ).c_str(), FILTER_LINEAR );
				mnu[1] = gfx->load_texture( TRANSLATE( "gfx/en/options.tga" ).c_str(), FILTER_LINEAR );
				mnu[2] = gfx->load_texture( TRANSLATE( "gfx/en/play.tga" ).c_str(), FILTER_LINEAR );
				set_color_depth( 32 );
				tst[0] = load_bitmap( TRANSLATE( "gfx/en/exit.tga" ).c_str(),NULL);
				tst[1] = load_bitmap( TRANSLATE( "gfx/en/options.tga" ).c_str(),NULL);
				tst[2] = load_bitmap( TRANSLATE( "gfx/en/play.tga" ).c_str(),NULL);

				for( i = 0 ; i < 3 ; i++ )
					if( tst[i] ) {
						tst_w[i] = tst[i]->w >> 1;
						tst_h[i] = tst[i]->h >> 1;
						}
					else
						tst_w[i] = tst_h[i] = 0;

				for( i = 0 ; i < MENU_NB_PART ; i++ ) {
					part[i].x=-p_size;	part[i].y=-p_size;
					part[i].z=0.0f;
					target[i].x = target[i].y = -p_size;
					target[i].z = 0.0f;
					}
				n = 0;
				}
			gfx->SCREEN_W_TO_640 = 640.0f / SCREEN_W;				// To have mouse sensibility undependent from the resolution
			gfx->SCREEN_H_TO_480 = 480.0f / SCREEN_H;
			cursor_type=CURSOR_DEFAULT;		// Curseur par standard
			while( mouse_b )	rest( 10 );
			}

		gfx->flip();
	}while(!done && !lp_CONFIG->quickrestart);

	destroy_bitmap(tst[0]);
	destroy_bitmap(tst[1]);
	destroy_bitmap(tst[2]);

	gfx->set_2D_mode();

	glDeleteTextures(1,&flame);
	glDeleteTextures(1,&mnu[0]);
	glDeleteTextures(1,&mnu[1]);
	glDeleteTextures(1,&mnu[2]);

	GuardLeave();
}

uint32 GetMultiPlayerMapList( std::list<std::string> *li )
{
	std::list< String > map_list;
	uint32 n = HPIManager->GetFilelist("maps\\*.tnt",&map_list);
	std::list< String >::iterator i_map;
	uint32 count;

	if( n < 1 )
		return 0;

	MAP_OTA	map_data;											// Using MAP_OTA because it's faster than cTAFileParser that fills a hash_table object
	bool isNetworkGame;
	count = 0;

	for( i_map=map_list.begin(); i_map!=map_list.end(); i_map++ )
	{
		*i_map=i_map->substr(5,i_map->length()-9);

		isNetworkGame = false;

		uint32 ota_size=0;
		byte *data = HPIManager->PullFromHPI( String( "maps\\" ) + *i_map + String( ".ota" ), &ota_size);
		if(data) {
			map_data.load((char*)data,ota_size);
			isNetworkGame = map_data.network;
			free(data);
			map_data.destroy();
			}

		if( isNetworkGame )
		{
			li->push_back( *i_map );
			count++;
		}
	}

	li->sort();
	return count;
} 

char *select_map(String *def_choice)		// Cette fonction affiche un menu permettant à l'utilisateur de choisir une carte dans une liste et de la prévisualiser à l'écran
{
	SKIN	skin;
	skin.load_tdf("gui/default.skn");

	gfx->SCREEN_W_TO_640 = 640.0f / SCREEN_W;				// To have mouse sensibility undependent from the resolution
	gfx->SCREEN_H_TO_480 = 480.0f / SCREEN_H;

	bool done=false;

	gfx->set_2D_mode();

	gfx->ReInitTexSys();

	set_uformat(U_ASCII);		// Juste histoire d'avoir un affichage correct des textes

	glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);

	int i;

	float dt=0.0f;
	int time = msec_timer;
	float Conv = 0.001f;

	float h=gfx->TA_font.height();
	float data_h = 24.0f;

	List< String > map_list;
	uint32 n = GetMultiPlayerMapList( &map_list );
	List< String >::iterator i_map;

	if(n==0)	{
		Popup(TRANSLATE("Error"),TRANSLATE("No maps found"));
		Console->AddEntry("no maps found!!");
		reset_mouse();
		return NULL;
		}

	char *choice=NULL;

	int dec=0;			// Pour la gestion de la liste déroulante
	int amz=mouse_z;
	int o_sel=-1;
	int sel_index=0;
	int index=-1;
	int m_timer = msec_timer;

	bool dmini=false;		// Pour la minimap
	GLuint mini;
	int dx=0;
	int dy=0;

	bool ok_status=false;		// Pour les boutons
	bool cancel_status=false;
	bool o_ok_status=false;
	bool o_cancel_status=false;

	MAP_OTA	map_data;

	if( def_choice ) {
		*def_choice = def_choice->substr(5,def_choice->length()-9);
		i = 0;
		for( i_map = map_list.begin() ; i_map != map_list.end() ; i_map++, i++ )
			if( *i_map == *def_choice ) {
				sel_index = i;
				break;
				}
		if(sel_index>=dec+(int)(440.0f/h)-1)
			dec=sel_index-(int)(440.0f/h)+1;
		if(sel_index>=0 && sel_index<dec)
			dec=sel_index;
		}

	int amx = -1;
	int amy = -1;
	int amb = -1;
	int am_z = -1;
	reset_keyboard();

	do
	{
		while( amx == mouse_x && amy == mouse_y && am_z == mouse_z && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done )
			rest( 1 );
		amx = mouse_x;
		amy = mouse_y;
		am_z = mouse_z;
		amb = mouse_b;

		time=msec_timer;

		o_ok_status=ok_status;
		o_cancel_status=cancel_status;
		ok_status=false;
		cancel_status=false;
		if(mouse_b==1 && mouse_y*gfx->SCREEN_H_TO_480>=440 && mouse_y*gfx->SCREEN_H_TO_480<=460) {
			cancel_status=(mouse_x*gfx->SCREEN_W_TO_640>=360 && mouse_x*gfx->SCREEN_W_TO_640<=440);
			ok_status=(mouse_x*gfx->SCREEN_W_TO_640>=520 && mouse_x*gfx->SCREEN_W_TO_640<=600);
			}

		if(mouse_b==0 && !ok_status && o_ok_status) {		// Click sur ok
			if(sel_index!=-1) {
				done=true;
				i_map=map_list.begin();
				for(i=0;i<sel_index && i_map!=map_list.end();i++)	i_map++;
				String tmp = String("maps\\") + *i_map + String(".tnt");
				choice = strdup(tmp.c_str());		// Copie le nom de la carte sélectionnée
				}
			}
		if(mouse_b==0 && !cancel_status && o_cancel_status)			// Click sur retour
			done=true;

		if(key[KEY_ESC]) {
			while(key[KEY_ESC]) {
				rest(1);
				poll_keyboard();
				}
			done=true;
			}
		if(key[KEY_ENTER]) {
			if(sel_index!=-1) {
				done=true;
				i_map=map_list.begin();
				for(i=0;i<sel_index && i_map!=map_list.end();i++)	i_map++;
				String tmp = String("maps\\") + *i_map + String(".tnt");
				choice = strdup(tmp.c_str());		// Copie le nom de la carte sélectionnée
				}
			}
		bool check=false;
		if((msec_timer-m_timer)*Conv>=0.1f) {
			if(key[KEY_DOWN])	{	sel_index++;	m_timer=msec_timer;	check=true;	}
			if(key[KEY_UP])		{	sel_index--;	m_timer=msec_timer;	check=true;	}
			if(o_sel!=sel_index && sel_index==-1)	sel_index=0;
			if(sel_index<-1)	sel_index=-1;
			if(sel_index>=n)	sel_index=n-1;
			}

		if(check) {
			if(sel_index>=dec+(int)(440.0f/h)-1)
				dec=sel_index-(int)(440.0f/h)+1;
			if(sel_index>=0 && sel_index<dec)
				dec=sel_index;
			}

		dec+=amz-mouse_z;
		if((msec_timer-m_timer)*Conv>=0.2f) {
			if(mouse_x*gfx->SCREEN_W_TO_640>=15.0f && mouse_x*gfx->SCREEN_W_TO_640<300.0f && mouse_y*gfx->SCREEN_H_TO_480>=30.0f && mouse_y*gfx->SCREEN_H_TO_480<30.0f+h) {
				m_timer=msec_timer;
				dec--;
				}
			if(mouse_x*gfx->SCREEN_W_TO_640>=15.0f && mouse_x*gfx->SCREEN_W_TO_640<300.0f && mouse_y*gfx->SCREEN_H_TO_480>=470.0f-h && mouse_y*gfx->SCREEN_H_TO_480<470.0f) {
				m_timer=msec_timer;
				dec++;
				}
			}

		if( mouse_x*gfx->SCREEN_W_TO_640 > 300 && mouse_x*gfx->SCREEN_W_TO_640 < 320 && mouse_y*gfx->SCREEN_H_TO_480 > 25 && mouse_y*gfx->SCREEN_H_TO_480 < 470 )
			if( mouse_b == 1 )
				dec = ((int)(mouse_y * gfx->SCREEN_H_TO_480) - 26) * (int)(n+2-440.0f/h) / 443;

		if(dec<0) dec=0;
		if(dec>n+1-440.0f/h) dec=(int)(n+1-440.0f/h);
		amz=mouse_z;

		if(mouse_x*gfx->SCREEN_W_TO_640>=15.0f && mouse_x*gfx->SCREEN_W_TO_640<300.0f && mouse_y*gfx->SCREEN_H_TO_480>=30.0f && mouse_y*gfx->SCREEN_H_TO_480<470.0f) {
			index=(int)((mouse_y*gfx->SCREEN_H_TO_480-30.0f)/h+dec);
			if(index>=n) index=-1;		// On ne sort pas de l'intervalle fixé
			if(mouse_b==1)
				sel_index=index;
			}
		else
			index=-1;

		if(sel_index!=o_sel && sel_index>=0) {
			if(dmini)
				glDeleteTextures(1,&mini);
			i_map=map_list.begin();
			for(i=0;i<sel_index && i_map!=map_list.end();i++)	i_map++;
			String tmp=String("maps\\") + *i_map + String(".tnt");
			mini=load_tnt_minimap_fast((char*)tmp.c_str(),&dx,&dy);
			dmini=(mini!=0);
			tmp=String("maps\\") + *i_map + String(".ota");								// Read the ota file
			uint32 ota_size=0;
			byte *data = HPIManager->PullFromHPI(tmp,&ota_size);
			if(data) {
				map_data.load((char*)data,ota_size);
				free(data);
				}
			else
				map_data.destroy();
			}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		gfx->drawtexture(gfx->glfond,0.0f,0.0f,640.0f,480.0);

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f,0.0f,0.0f,0.5f);
		gfx->rectfill(10.0,25.0f,630.0f,470.0f);
		glColor4f(1.0f,1.0f,1.0f,0.5f);
		gfx->rect(10.0f,25.0f,630.0f,470.0f);
		gfx->rect(347.0f,50.0f,603.0f,306.0f);

		gfx->set_alpha_blending();
		glBegin( GL_QUADS );
			glColor4f( 0.0f, 0.0f, 0.5f, 0.5f );
			glVertex2i( 300, 26 );
			glVertex2i( 300, 469 );
			glColor4f( 0.0f, 0.0f, 1.0f, 0.5f );
			glVertex2i( 310, 469 );
			glVertex2i( 310, 26 );

			glVertex2i( 310, 469 );
			glVertex2i( 310, 26 );
			glColor4f( 0.0f, 0.0f, 0.5f, 0.5f );
			glVertex2i( 320, 26 );
			glVertex2i( 320, 469 );

			{
				int py = dec * 443 / (int)(n+2-440.0f/h);
				if( py < 0 )	py = 0;
				int py_h = (dec + 1) * 443 / (int)(n+2-440.0f/h);
				if( py_h < 0 )	py_h = 443;
				py += 26;
				py_h += 26;

				glColor4f( 0.25f, 0.25f, 0.5f, 0.5f );
				glVertex2i( 300, py );
				glVertex2i( 300, py_h );
				glColor4f( 0.5f, 0.5f, 1.0f, 0.5f );
				glVertex2i( 310, py_h );
				glVertex2i( 310, py );

				glVertex2i( 310, py_h );
				glVertex2i( 310, py );
				glColor4f( 0.25f, 0.25f, 0.5f, 0.5f );
				glVertex2i( 320, py );
				glVertex2i( 320, py_h );
			}
		glEnd();
		gfx->unset_alpha_blending();

		glColor4f(1.0f,1.0f,1.0f,0.5f);
		gfx->line(300.0f,26.0f,300.0f,469.0f);
		gfx->line(320.0f,26.0f,320.0f,469.0f);

		glEnable(GL_TEXTURE_2D);

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		gfx->print(gfx->TA_font,320.0f-gfx->TA_font.length(TRANSLATE("Maps"))*0.5f,6.0f,0.0f,0xFFFFFFFF,TRANSLATE("Maps"));

		i=0;
		for(i_map=map_list.begin();i_map!=map_list.end();i_map++) {
			if(30.0f+h*(i-dec)<470.0f-h && h*(i-dec)>=0) {
				if(i==index || i==sel_index) {
					glDisable(GL_TEXTURE_2D);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					glBegin(GL_QUADS);
						if(i==sel_index)
							glColor4f(0.0f,0.0f,0.5f,0.5f);
						else
							glColor4f(0.25f,0.25f,0.5f,0.5f);
						glVertex2f(15.0f,30.0f+h*(i-dec));
						glVertex2f(299.0f,30.0f+h*(i-dec));
						if(i==sel_index)
							glColor4f(0.0f,0.0f,1.0f,0.5f);
						else
							glColor4f(0.5f,0.5f,1.0f,0.5f);
						glVertex2f(299.0f,30.0f+h*(i-dec+0.5f));
						glVertex2f(15.0f,30.0f+h*(i-dec+0.5f));

						glVertex2f(15.0f,30.0f+h*(i-dec+0.5f));
						glVertex2f(299.0f,30.0f+h*(i-dec+0.5f));
						if(i==sel_index)
							glColor4f(0.0f,0.0f,0.5f,0.5f);
						else
							glColor4f(0.25f,0.25f,0.5f,0.5f);
						glVertex2f(299.0f,30.0f+h*(i-dec+1));
						glVertex2f(15.0f,30.0f+h*(i-dec+1));
					glEnd();
					glColor4f(1.0f,1.0f,1.0f,0.5f);
					glEnable(GL_TEXTURE_2D);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					}
				gfx->print(gfx->TA_font,20.0f,30.0f+h*(i-dec),0.0f,0xFFFFFFFF,i_map->c_str());
				}
			i++;
			}

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		glDisable(GL_BLEND);

		if(dmini) {
			glBindTexture(GL_TEXTURE_2D,mini);
			float ldx=dx*256.0f/252.0f*0.5f;
			float ldy=dy*256.0f/252.0f*0.5f;
			glBegin(GL_QUADS);

				glTexCoord2f(0.0f,0.0f);
				glVertex2f(475.0f-ldx,178.0f-ldy);

				glTexCoord2f(dx/252.0f,0.0f);
				glVertex2f(475.0f+ldx,178.0f-ldy);
					
				glTexCoord2f(dx/252.0f,dy/252.0f);
				glVertex2f(475.0f+ldx,178.0f+ldy);
			
				glTexCoord2f(0.0f,dy/252.0f);
				glVertex2f(475.0f-ldx,178.0f+ldy);

			glEnd();
			}
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		if(map_data.missionname)
			gfx->print(gfx->TA_font,480.0f-gfx->TA_font.length(map_data.missionname)*0.5f,320.0f,0.0f,0xFFFFFFFF,map_data.missionname);
		if(map_data.missiondescription) {
			glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
			if(strstr(map_data.missiondescription,"\n")) {
				char *txt = map_data.missiondescription;
				char *rem = strstr(txt,"\n");
				float y_pos = 320.0f+data_h+3.0f;
				while( rem != NULL ) {
					rem[0] = 0;
					gfx->print(gfx->normal_font,340.0f,y_pos,0.0f,0xFFFFFFFF,txt);
					y_pos += 8.0f;
					rem[0] = '\n';
					txt = rem + 1;
					rem = strstr(txt,"\n");
					}
				gfx->print(gfx->normal_font,340.0f,y_pos,0.0f,0xFFFFFFFF,txt);
				}
			else
				gfx->print(gfx->normal_font,340.0f,320.0f+data_h,0.0f,0xFFFFFFFF,map_data.missiondescription);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		if(map_data.numplayers) {
			float len=(gfx->TA_font.length(TRANSLATE("players: "))+gfx->TA_font.length(map_data.numplayers))*0.5f;
			gfx->print(gfx->TA_font,480.0f-len,320.0f+data_h*3.0f,0.0f,0xFFFFFFFF,TRANSLATE("players: "));
			gfx->print(gfx->TA_font,480.0f-len+gfx->TA_font.length(TRANSLATE("players: ")),320.0f+data_h*3.0f,0.0f,0xFFFFFFFF,map_data.numplayers);
			}

		o_sel=sel_index;

		glDisable(GL_BLEND);

		button( 360, 440, 440, 460, TRANSLATE("back"), cancel_status, 1.0f, &skin );
		button( 520, 440, 600, 460, TRANSLATE("ok"), ok_status, 1.0f, &skin );

		draw_cursor();

		gfx->flip();
	}while(!done);

	skin.destroy();

	if(dmini)
		glDeleteTextures(1,&mini);

	gfx->unset_2D_mode();

	set_uformat(U_UTF8);

	return choice;
}

void config_menu(void)
{
	cursor_type=CURSOR_DEFAULT;

	lp_CONFIG->Lang = LANG;

	if( lp_CONFIG->restorestart ) {
		lp_CONFIG->restorestart = false;
		lp_CONFIG->quickstart = false;
		}

	TA3DCONFIG	saved_config = *lp_CONFIG;

	float resize_w = SCREEN_W / 640.0f;
	float resize_h = SCREEN_H / 480.0f;

	gfx->set_2D_mode();

	AREA config_area("config");
	config_area.load_tdf("gui/config.area");
	if( !config_area.background )	config_area.background = gfx->glfond;

	Vector< String > fps_limits = ReadVectorString( "50,60,70,80,90,100,no limit" );
	for( uint32 e = 0 ; e < fps_limits.size() ; e++ )
		fps_limits[ e ] = TRANSLATE( fps_limits[ e ] );
	int	nb_res = 0;
	int res_width[100];
	int res_height[100];
	int res_bpp[100];

	GFX_MODE_LIST *mode_list = get_gfx_mode_list( GFX_OPENGL_FULLSCREEN );

	for( int i = 0 ; i < mode_list->num_modes ; i++ )
		if( mode_list->mode[ i ].bpp == 32 ) {
			bool found = mode_list->mode[ i ].width < 640 || mode_list->mode[ i ].height < 480;
			if( !found )
				for( int e = 0 ; e < nb_res ; e++ )
					if( res_width[e] == mode_list->mode[ i ].width && res_height[e] == mode_list->mode[ i ].height ) {
						found = true;
						break;
						}

			if( mode_list->mode[ i ].height == 0 ||
				( mode_list->mode[ i ].width * 3 != 4 * mode_list->mode[ i ].height &&
				mode_list->mode[ i ].width * 9 != 16 * mode_list->mode[ i ].height &&
				mode_list->mode[ i ].width * 10 != 16 * mode_list->mode[ i ].height &&
				mode_list->mode[ i ].width * 4 != 5 * mode_list->mode[ i ].height ) )	found = true;

			if( !found ) {
				res_bpp[ nb_res ] = 16;
				res_width[ nb_res ] = mode_list->mode[ i ].width;
				res_height[ nb_res++ ] = mode_list->mode[ i ].height;
				res_bpp[ nb_res ] = 32;
				res_width[ nb_res ] = mode_list->mode[ i ].width;
				res_height[ nb_res++ ] = mode_list->mode[ i ].height;
				}
			}

	destroy_gfx_mode_list( mode_list );

	config_area.set_state("*.showfps", lp_CONFIG->showfps);
	switch( (int)lp_CONFIG->fps_limit )
	{
	case 50:	config_area.set_caption("*.fps_limit", fps_limits[0]); break;
	case 60:	config_area.set_caption("*.fps_limit", fps_limits[1]); break;
	case 70:	config_area.set_caption("*.fps_limit", fps_limits[2]); break;
	case 80:	config_area.set_caption("*.fps_limit", fps_limits[3]); break;
	case 90:	config_area.set_caption("*.fps_limit", fps_limits[4]); break;
	case 100:	config_area.set_caption("*.fps_limit", fps_limits[5]); break;
	default:
		config_area.set_caption("*.fps_limit", fps_limits[6]);
	};
	config_area.set_state("*.wireframe", lp_CONFIG->wireframe);
	config_area.set_state("*.particle", lp_CONFIG->particle);
	config_area.set_state("*.waves", lp_CONFIG->waves);
	config_area.set_state("*.shadow", lp_CONFIG->shadow);
	config_area.set_state("*.height_line", lp_CONFIG->height_line);
	config_area.set_state("*.detail_tex", lp_CONFIG->detail_tex);
	config_area.set_state("*.draw_console_loading", lp_CONFIG->draw_console_loading);
	config_area.set_state("*.fullscreen", lp_CONFIG->fullscreen);
	if( config_area.get_object("*.LANG") )
		config_area.set_caption( "*.LANG", config_area.get_object("*.LANG")->Text[1+lp_CONFIG->Lang] );
	if( config_area.get_object("*.camera_zoom") )
		config_area.set_caption( "*.camera_zoom", config_area.get_object("*.camera_zoom")->Text[1+lp_CONFIG->camera_zoom] );
	config_area.set_caption( "*.camera_def_angle", format( "%f", lp_CONFIG->camera_def_angle ) );
	config_area.set_caption( "*.camera_def_h", format( "%f", lp_CONFIG->camera_def_h ) );
	config_area.set_caption( "*.camera_zoom_speed", format( "%f", lp_CONFIG->camera_zoom_speed ) );
	if( config_area.get_object("*.screenres") ) {
		GUIOBJ *obj = config_area.get_object("*.screenres");
		obj->Text.clear();
		int current = 0;
		while( current < nb_res &&
				( res_width[ current ] != lp_CONFIG->screen_width
				|| res_height[ current ] != lp_CONFIG->screen_height
				|| res_bpp[ current ] != lp_CONFIG->color_depth ) )
					current++;
		if( current >= nb_res )	current = 0;
		obj->Text.push_back( format( "%dx%dx%d", res_width[ current ], res_height[ current ], res_bpp[ current ] ) );
		for( int i = 0 ; i < nb_res ; i++ )
			obj->Text.push_back( format( "%dx%dx%d", res_width[ i ], res_height[ i ], res_bpp[ i ] ) );
		}
	if( config_area.get_object("*.shadow_quality") )
		config_area.set_caption( "*.shadow_quality", config_area.get_object("*.shadow_quality")->Text[1+min( (lp_CONFIG->shadow_quality-1)/3, 2 ) ] );
	config_area.set_caption("*.timefactor", format( "%d", (int)lp_CONFIG->timefactor ) );
	switch( lp_CONFIG->fsaa )
	{
	case 2:	config_area.set_caption("*.fsaa", "x2" );	break;
	case 4:	config_area.set_caption("*.fsaa", "x4" );	break;
	case 6:	config_area.set_caption("*.fsaa", "x6" );	break;
	case 8:	config_area.set_caption("*.fsaa", "x8" );	break;
	default:
		config_area.set_caption("*.fsaa", "no fsaa" );
	};
	if( config_area.get_object("*.water_quality") ) {
		GUIOBJ *obj = config_area.get_object("*.water_quality");
		config_area.set_caption("*.water_quality", obj->Text[ 1 + lp_CONFIG->water_quality ] );
		}

	if( config_area.get_object("*.mod") ) {
		GUIOBJ *obj = config_area.get_object("*.mod");

		obj->Text[0] = obj->Text[1];

		al_ffblk search;
		String current_selection = TA3D_CURRENT_MOD.length() > 6 ? TA3D_CURRENT_MOD.substr( 5, TA3D_CURRENT_MOD.length() - 6 ) : "";

		if( al_findfirst( "mods/*", &search, FA_RDONLY | FA_DIREC ) == 0 ) {
			do
			{
				if( String( search.name ) != ".." && String( search.name ) != "." ) {		// Have to exclude both .. & . because of windows finding . as something interesting
					obj->Text.push_back( search.name );
					if( Lowercase( search.name ) == Lowercase( current_selection ) )
						obj->Text[0] = search.name;
					}
			} while( al_findnext( &search ) == 0 );

			al_findclose(&search);
			}
		}
	config_area.set_caption("*.player_name", lp_CONFIG->player_name );

	if( lp_CONFIG->quickstart )
		I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)("config_confirm.show"), NULL, NULL );

	bool done=false;

	bool save=false;

	int amx = -1;
	int amy = -1;
	int amb = -1;
	uint32 timer = msec_timer;

	do
	{
		bool time_out = false;
		bool key_is_pressed = false;
		do {
			key_is_pressed = keypressed();
			if( lp_CONFIG->quickstart ) {
				GUIOBJ *pbar = config_area.get_object( "config_confirm.p_wait" );
				if( pbar ) {
					int new_value = (msec_timer - timer) / 150;
					if( new_value != pbar->Data ) {
						pbar->Data = new_value;
						key_is_pressed = true;
						if( new_value == 100 )
							time_out = true;
						}
					}
				}
			config_area.check();
			rest( 1 );
		} while( amx == mouse_x && amy == mouse_y && amb == mouse_b && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed );
		amx = mouse_x;
		amy = mouse_y;
		amb = mouse_b;

		if( lp_CONFIG->quickstart ) {
			if( time_out || config_area.get_state("config_confirm.b_cancel_changes" ) ) {
				I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)("config_confirm.hide"), NULL, NULL );
				restoreBackup( TA3D_OUTPUT_DIR + "ta3d.cfg" );
				LoadConfigFile();
				done = true;
				save = true;
				lp_CONFIG->quickrestart = true;
				lp_CONFIG->restorestart = true;
				}
			else if( config_area.get_state("config_confirm.b_confirm" ) ) {
				I_Msg( TA3D::TA3D_IM_GUI_MSG, (void*)("config_confirm.hide"), NULL, NULL );
				SaveConfigFile();
				lp_CONFIG->quickstart = false;
				saved_config.quickstart = false;
				}
			}

		if( config_area.get_state( "*.b_ok" ) ) {
			done = true;		// En cas de click sur "OK", on quitte la fenêtre
			save = true;
			}
		if( config_area.get_state( "*.b_cancel" ) ) done=true;		// En cas de click sur "retour", on quitte la fenêtre

		lp_CONFIG->showfps = config_area.get_state( "*.showfps" );
		if( config_area.get_state( "*.fps_limit" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.fps_limit" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = fps_limits[ obj->Data ];
				switch( obj->Data )
				{
				case 0:		lp_CONFIG->fps_limit = 50;	break;
				case 1:		lp_CONFIG->fps_limit = 60;	break;
				case 2:		lp_CONFIG->fps_limit = 70;	break;
				case 3:		lp_CONFIG->fps_limit = 80;	break;
				case 4:		lp_CONFIG->fps_limit = 90;	break;
				case 5:		lp_CONFIG->fps_limit = 100;	break;
				default:
					lp_CONFIG->fps_limit = -1;
				};
				}
			}
		lp_CONFIG->wireframe = config_area.get_state( "*.wireframe" );
		lp_CONFIG->particle = config_area.get_state( "*.particle" );
		lp_CONFIG->waves = config_area.get_state( "*.waves" );
		lp_CONFIG->shadow = config_area.get_state( "*.shadow" );
		lp_CONFIG->height_line = config_area.get_state( "*.height_line" );
		lp_CONFIG->detail_tex = config_area.get_state( "*.detail_tex" );
		lp_CONFIG->draw_console_loading = config_area.get_state( "*.draw_console_loading" );
		lp_CONFIG->fullscreen = config_area.get_state( "*.fullscreen" );
		if( config_area.get_state( "*.camera_zoom" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.camera_zoom" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->camera_zoom = obj->Data;
				}
			}
		if( config_area.get_state( "*.camera_def_angle" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.camera_def_angle" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->camera_def_angle = atof( obj->Text[0].c_str() );
				}
			}
		if( config_area.get_state( "*.camera_def_h" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.camera_def_h" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->camera_def_h = atof( obj->Text[0].c_str() );
				}
			}
		if( config_area.get_state( "*.camera_zoom_speed" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.camera_zoom_speed" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->camera_zoom_speed = atof( obj->Text[0].c_str() );
				}
			}
		if( config_area.get_state( "*.LANG" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.LANG" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->Lang = obj->Data;
				}
			}
		if( config_area.get_state( "*.screenres" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.screenres" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->screen_width = res_width[ obj->Data ];
				lp_CONFIG->screen_height = res_height[ obj->Data ];
				lp_CONFIG->color_depth = res_bpp[ obj->Data ];
				}
			}
		if( config_area.get_state( "*.shadow_quality" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.shadow_quality" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->shadow_quality = obj->Data * 3 + 1;
				}
			}
		if( config_area.get_state( "*.timefactor" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.timefactor" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->timefactor = obj->Data + 1;
				}
			}
		if( config_area.get_state( "*.fsaa" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.fsaa" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->fsaa = obj->Data << 1;
				}
			}
		if( config_area.get_state( "*.water_quality" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.water_quality" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->water_quality = obj->Data;
				}
			}
		if( config_area.get_state( "*.mod" ) ) {
			GUIOBJ *obj = config_area.get_object( "*.mod" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				lp_CONFIG->last_MOD = obj->Data > 0 ? "mods/" + obj->Text[0] + "/" : "";
				}
			}

		if(key[KEY_ESC]) done=true;			// Quitte si on appuie sur echap

					// Efface tout
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		config_area.draw();

		glEnable(GL_TEXTURE_2D);
		gfx->set_color(0xFFFFFFFF);
		draw_cursor( resize_w, resize_h);
		
					// Affiche
		gfx->flip();
	}while(!done);

	if( config_area.background == gfx->glfond )	config_area.background = 0;

	gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

	reset_mouse();
	while(key[KEY_ESC]) {	rest(1);	poll_keyboard();	}

	bool ask_for_quickrestart = lp_CONFIG->quickrestart;

	if(!save)
		*lp_CONFIG = saved_config;
	else {
		if( lp_CONFIG->screen_width != saved_config.screen_width ||
			lp_CONFIG->screen_height != saved_config.screen_height ||
			lp_CONFIG->color_depth != saved_config.color_depth ||
			lp_CONFIG->fsaa != saved_config.fsaa ||
			lp_CONFIG->fullscreen != saved_config.fullscreen )			// Need to restart
			lp_CONFIG->quickrestart = true;
		lp_CONFIG->player_name = config_area.get_caption( "*.player_name" );
		if( lp_CONFIG->last_MOD != TA3D_CURRENT_MOD ) {			// Refresh the file structure
			TA3D_CURRENT_MOD = lp_CONFIG->last_MOD;
			delete HPIManager;
			HPIManager = new cHPIHandler("");
			ta3d_sidedata.load_data();				// Refresh side data so we load the correct values
			}
		}

	lp_CONFIG->quickrestart |= ask_for_quickrestart;

	config_area.destroy();

	LANG = lp_CONFIG->Lang;

	i18n.refresh_language();   // refresh the language used by the i18n object
}

void stats_menu(void)
{
	cursor_type=CURSOR_DEFAULT;

	float resize_w = SCREEN_W / 640.0f;
	float resize_h = SCREEN_H / 480.0f;

	gfx->set_2D_mode();

	gfx->ReInitTexSys();

	reset_keyboard();
	while(key[KEY_ESC])	rest(1);

	AREA statistics_area("statistics");
	statistics_area.load_tdf("gui/statistics.area");
	if( !statistics_area.background )	statistics_area.background = gfx->glfond;

	for( int i = 0 ; i < players.nb_player ; i++ ) {
		statistics_area.set_caption( format( "statistics.player%d", i ), players.nom[i] );
		statistics_area.set_caption( format( "statistics.side%d", i ), players.side[i] );
		statistics_area.set_caption( format( "statistics.losses%d", i ), format( "%d", players.losses[i] ) );
		statistics_area.set_caption( format( "statistics.kills%d", i ), format( "%d", players.kills[i] ) );
		statistics_area.set_caption( format( "statistics.energy%d", i ), format( "%d", (int)players.energy_total[i] ) );
		statistics_area.set_caption( format( "statistics.metal%d", i ), format( "%d", (int)players.metal_total[i] ) );
		}

	bool done=false;

	int amx = -1;
	int amy = -1;
	int amb = -1;

	do
	{
		bool key_is_pressed = false;
		do {
			key_is_pressed = keypressed();
			statistics_area.check();
			rest( 1 );
		} while( amx == mouse_x && amy == mouse_y && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed );

		amx = mouse_x;
		amy = mouse_y;
		amb = mouse_b;

		if( statistics_area.get_state( "statistics.b_ok" ) || key[KEY_ENTER] ) {
			while( key[KEY_ENTER] )	{	rest( 20 );	poll_keyboard();	}
			clear_keybuf();
			done=true;		// If user click "OK" or hit enter then leave the window
			}

		if(key[KEY_ESC]) done=true;			// Quitte si on appuie sur echap
					// Efface tout
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		statistics_area.draw();

		glEnable(GL_TEXTURE_2D);
		gfx->set_color(0xFFFFFFFF);
		draw_cursor(resize_w,resize_h);
		
					// Affiche
		gfx->flip();
	}while(!done);

	if( statistics_area.background == gfx->glfond )	statistics_area.background = 0;
	statistics_area.destroy();

	gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

	reset_mouse();
	while(key[KEY_ESC]) {	rest(1);	poll_keyboard();	}
}

void setup_game(void)
{
	cursor_type=CURSOR_DEFAULT;

	float resize_w = SCREEN_W / 640.0f;
	float resize_h = SCREEN_H / 480.0f;

	uint16	player_str_n = 3;
	uint16	ai_level_str_n = 4;
	String	player_str[3] = { lp_CONFIG->player_name, TRANSLATE("computer"), TRANSLATE("open") };
	byte	player_control[3] = { PLAYER_CONTROL_LOCAL_HUMAN, PLAYER_CONTROL_LOCAL_AI, PLAYER_CONTROL_NONE };
	String	ai_level_str[4] = { TRANSLATE("easy"), TRANSLATE("medium"), TRANSLATE("hard"), TRANSLATE("bloody") };
	uint16	side_str_n = ta3d_sidedata.nb_side;
	Vector<String>	side_str;
	
	side_str.resize( ta3d_sidedata.nb_side );
	for( int i = 0 ; i < ta3d_sidedata.nb_side ; i++ )			// Get side data
		side_str[ i ] = ta3d_sidedata.side_name[ i ];

	GAME_DATA game_data;

	if( HPIManager->Exists( lp_CONFIG->last_map ) )
		game_data.map_filename = strdup( lp_CONFIG->last_map.c_str() );
	else {
		List<String> map_list;
		uint32 n = HPIManager->GetFilelist("maps\\*.tnt",&map_list);

		if( n == 0 ) {
			Popup(TRANSLATE("Error"),TRANSLATE("No maps found"));
			Console->AddEntry("no maps found!!");
			reset_mouse();
			return;
			}
		game_data.map_filename = strdup( map_list.begin()->c_str() );
		map_list.clear();
		}
	game_data.nb_players = 2;
	if( HPIManager->Exists( lp_CONFIG->last_script ) && Lowercase( lp_CONFIG->last_script.substr( lp_CONFIG->last_script.length() - 3 , 3 ) ) == "lua" )
		game_data.game_script = strdup( lp_CONFIG->last_script.c_str() );
	else {
		List<String> script_list;
		uint32 n = HPIManager->GetFilelist("scripts\\*.lua",&script_list);

		if( n == 0 ) {
			Popup(TRANSLATE("Error"),TRANSLATE("No scripts found"));
			Console->AddEntry("no scripts found!!");
			reset_mouse();
			return;
			}
		game_data.game_script = strdup( script_list.begin()->c_str() );
		script_list.clear();
		}
	game_data.fog_of_war = lp_CONFIG->last_FOW;


	game_data.player_names[0] = player_str[0];
	game_data.player_sides[0] = side_str[0];
	game_data.player_control[0] = player_control[0];
	game_data.ai_level[0] = AI_TYPE_EASY;

	game_data.player_names[1] = player_str[1];
	game_data.player_sides[1] = side_str[1];
	game_data.player_control[1] = player_control[1];
	game_data.ai_level[1] = AI_TYPE_EASY;

	for(uint16 i = 2; i < 10 ; i++) {
		game_data.player_names[i] = player_str[2];
		game_data.player_sides[i] = side_str[0];
		game_data.player_control[i] = player_control[2];
		game_data.ai_level[i] = AI_TYPE_EASY;
		}

	gfx->set_2D_mode();

	int dx, dy;
	GLuint glimg = load_tnt_minimap_fast(game_data.map_filename,&dx,&dy);
	char tmp_char[1024];
	MAP_OTA	map_data;
	map_data.load( replace_extension( (char*)tmp_char, game_data.map_filename, "ota", 1024 ) );
	float ldx = dx*70.0f/252.0f;
	float ldy = dy*70.0f/252.0f;

	AREA setupgame_area("setup");
	setupgame_area.load_tdf("gui/setupgame.area");
	if( !setupgame_area.background )	setupgame_area.background = gfx->glfond;
	for(uint16 i = 0 ; i < 10 ; i++ ) {
		setupgame_area.set_caption( format("gamesetup.name%d", i), game_data.player_names[i] );
		setupgame_area.set_caption( format("gamesetup.side%d", i), game_data.player_sides[i] );
		setupgame_area.set_caption( format("gamesetup.ai%d", i), game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI ? ai_level_str[game_data.ai_level[i]].c_str() : "" );
		GUIOBJ *guiobj = setupgame_area.get_object( format("gamesetup.color%d", i) );
		if( guiobj ) {
			guiobj->Flag |= (game_data.player_control[i] == PLAYER_CONTROL_NONE ? FLAG_HIDDEN : 0 );
			guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3], player_color[player_color_map[i]*3+1], player_color[player_color_map[i]*3+2]);
			}
		setupgame_area.set_caption( format("gamesetup.energy%d", i), format("%d",game_data.energy[i]) );
		setupgame_area.set_caption( format("gamesetup.metal%d", i), format("%d",game_data.metal[i]) );
		}

	GUIOBJ *minimap_obj = setupgame_area.get_object( "gamesetup.minimap" );
	float mini_map_x1 = 0.0f;
	float mini_map_y1 = 0.0f;
	float mini_map_x2 = 0.0f;
	float mini_map_y2 = 0.0f;
	float mini_map_x = 0.0f;
	float mini_map_y = 0.0f;
	if( minimap_obj ) {
		mini_map_x1 = minimap_obj->x1;
		mini_map_y1 = minimap_obj->y1;
		mini_map_x2 = minimap_obj->x2;
		mini_map_y2 = minimap_obj->y2;
		ldx = dx * ( mini_map_x2 - mini_map_x1 ) / 504.0f;
		ldy = dy * ( mini_map_y2 - mini_map_y1 ) / 504.0f;

		mini_map_x = (mini_map_x1 + mini_map_x2) * 0.5f;
		mini_map_y = (mini_map_y1 + mini_map_y2) * 0.5f;

		minimap_obj->Data = glimg;
		minimap_obj->x1 = mini_map_x - ldx;
		minimap_obj->y1 = mini_map_y - ldy;
		minimap_obj->x2 = mini_map_x + ldx;
		minimap_obj->y2 = mini_map_y + ldy;
		minimap_obj->u2 = dx / 252.0f;
		minimap_obj->v2 = dy / 252.0f;
		}

	GUIOBJ *guiobj = setupgame_area.get_object( "scripts.script_list" );
	if( guiobj ) {
		std::list< String > script_list;
		HPIManager->GetFilelist("scripts\\*.lua",&script_list);
		for(std::list< String >::iterator i_script = script_list.begin() ; i_script != script_list.end() ; i_script++ )
			guiobj->Text.push_back( *i_script );
		}
	setupgame_area.set_caption( "gamesetup.script_name", game_data.game_script );
	{
		GUIOBJ *obj = setupgame_area.get_object( "gamesetup.FOW" );
		if( obj )
			obj->Text[0] = obj->Text[ 1 + game_data.fog_of_war ];
	}

	{
		String map_info = "";
		if(map_data.missionname)
			map_info += String( map_data.missionname ) + "\n";
		if(map_data.numplayers)
			map_info += String( map_data.numplayers ) + "\n";
		if(map_data.missiondescription)
			map_info += map_data.missiondescription;
		setupgame_area.set_caption("gamesetup.map_info", map_info );
	}

	bool done=false;

	bool start_game = false;

	int amx = -1;
	int amy = -1;
	int amb = -1;

	do
	{
		bool key_is_pressed = false;
		do {
			key_is_pressed = keypressed();
			setupgame_area.check();
			rest( 1 );
		} while( amx == mouse_x && amy == mouse_y && amb == mouse_b && mouse_b == 0 && !key[ KEY_ENTER ] && !key[ KEY_ESC ] && !done && !key_is_pressed );

		amx = mouse_x;
		amy = mouse_y;
		amb = mouse_b;

		if( setupgame_area.get_state( "gamesetup.FOW" ) ) {
			GUIOBJ *obj = setupgame_area.get_object( "gamesetup.FOW" );
			if( obj && obj->Data != -1 ) {
				obj->Text[0] = obj->Text[ 1 + obj->Data ];
				game_data.fog_of_war = obj->Data;
				}
			}

		if( setupgame_area.get_state( "scripts.b_ok" ) ) {
			guiobj = setupgame_area.get_object( "scripts.script_list" );
			if( guiobj && guiobj->Pos >= 0 && guiobj->Pos < guiobj->num_entries() ) {
				setupgame_area.set_caption( "gamesetup.script_name", guiobj->Text[ guiobj->Pos ] );
				free( game_data.game_script );
				game_data.game_script = strdup( guiobj->Text[ guiobj->Pos ].c_str() );
				}
			}

		if( setupgame_area.get_state( "gamesetup.b_ok" ) || key[KEY_ENTER] ) {
			while( key[KEY_ENTER] )	{	rest( 20 );	poll_keyboard();	}
			clear_keybuf();
			done=true;		// If user click "OK" or hit enter then leave the window
			start_game = true;
			}
		if( setupgame_area.get_state( "gamesetup.b_cancel" ) ) done=true;		// En cas de click sur "retour", on quitte la fenêtre

		for(uint16 i = 0 ; i < 10 ; i++ ) {
			if( setupgame_area.get_state( format("gamesetup.b_name%d", i) ) ) {		// Change player type
				uint16 e = 0;
				for( uint16 f = 0 ; f<player_str_n ; f++ )
					if( setupgame_area.get_caption(format("gamesetup.name%d", i)) == player_str[f].c_str() ) {	e = f;	break;	}
				e = (e+1) % player_str_n;

				if( player_control[e] == PLAYER_CONTROL_LOCAL_HUMAN )					// We can only have one local human player ( or it crashes )
					for( uint16 f = 0 ; f<10 ; f++ )
						if( f!= i && game_data.player_control[f] == PLAYER_CONTROL_LOCAL_HUMAN ) {		// If we already have a local human player pass this player type value
							e = (e+1) % player_str_n;
							break;
							}

				game_data.player_names[i] = player_str[e];								// Update game data
				game_data.player_control[i] = player_control[e];

				setupgame_area.set_caption( format( "gamesetup.name%d", i ),player_str[e]);			// Update gui
				setupgame_area.set_caption( format( "gamesetup.ai%d", i ), (game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI) ? ai_level_str[game_data.ai_level[i]] : String("") );
				guiobj = setupgame_area.get_object( format( "gamesetup.color%d", i ) );
				if( guiobj ) {
					if( player_control[e] == PLAYER_CONTROL_NONE )
						guiobj->Flag |= FLAG_HIDDEN;
					else
						guiobj->Flag &= ~FLAG_HIDDEN;
					}
				}
			if( setupgame_area.get_state( format("gamesetup.b_side%d", i) ) ) {	// Change player side
				uint16 e = 0;
				for( uint16 f = 0 ; f<side_str_n ; f++ )
					if( setupgame_area.get_caption( format("gamesetup.side%d", i) ) == side_str[f].c_str() ) {	e = f;	break;	}
				e = (e+1) % side_str_n;
				setupgame_area.set_caption( format("gamesetup.side%d", i) , side_str[e] );			// Update gui

				game_data.player_sides[i] = side_str[e];								// update game data
				}
			if( setupgame_area.get_state( format("gamesetup.b_ai%d", i) ) ) {	// Change player level (for AI)
				uint16 e = 0;
				for( uint16 f = 0 ; f<ai_level_str_n ; f++ )
					if( setupgame_area.get_caption( format("gamesetup.ai%d", i) ) == ai_level_str[f].c_str() ) {	e = f;	break;	}
				e = (e+1) % ai_level_str_n;
				setupgame_area.set_caption( format("gamesetup.ai%d", i), game_data.player_control[i] & PLAYER_CONTROL_FLAG_AI ? ai_level_str[e] : String("") );			// Update gui

				game_data.ai_level[i] = e;								// update game data
				}
			if( setupgame_area.get_state( format("gamesetup.b_color%d", i) ) ) {	// Change player color
				sint16 e = player_color_map[i];
				sint16 f = -1;
				for( sint16 g = 0; g<10 ; g++ )						// Look for the next color
					if( game_data.player_control[g] == PLAYER_CONTROL_NONE && player_color_map[g] > e && (f == -1 || player_color_map[g] < player_color_map[f]) )
						f = g;
				if( f == -1 )
					for( uint16 g = 0; g<10 ; g++ )
						if( game_data.player_control[g] == PLAYER_CONTROL_NONE && (f == -1 || player_color_map[g] < player_color_map[f]) )
							f = g;
				if( f != -1 ) {
					sint16 g = player_color_map[f];
					player_color_map[i] = g;								// update game data
					player_color_map[f] = e;

					guiobj =  setupgame_area.get_object( format("gamesetup.color%d", i) );
					if( guiobj )
						guiobj->Data = gfx->makeintcol(player_color[player_color_map[i]*3],player_color[player_color_map[i]*3+1],player_color[player_color_map[i]*3+2]);			// Update gui
					guiobj =  setupgame_area.get_object( format("gamesetup.color%d", f) );
					if( guiobj )
						guiobj->Data = gfx->makeintcol(player_color[player_color_map[f]*3],player_color[player_color_map[f]*3+1],player_color[player_color_map[f]*3+2]);			// Update gui
					}
				}
			if( setupgame_area.get_state( format("gamesetup.b_energy%d", i) ) ) {	// Change player energy stock
				game_data.energy[i] = (game_data.energy[i] + 500) % 10500;
				if( game_data.energy[i] == 0 ) game_data.energy[i] = 500;

				setupgame_area.set_caption( format("gamesetup.energy%d", i), format("%d",game_data.energy[i]) );			// Update gui
				}
			if( setupgame_area.get_state( format("gamesetup.b_metal%d", i) ) ) {	// Change player metal stock
				game_data.metal[i] = (game_data.metal[i] + 500) % 10500;
				if( game_data.metal[i] == 0 ) game_data.metal[i] = 500;

				setupgame_area.set_caption( format("gamesetup.metal%d", i), format("%d",game_data.metal[i]) );			// Update gui
				}
			}

		if( minimap_obj != NULL && 
		( setupgame_area.get_state( "gamesetup.minimap" ) || setupgame_area.get_state( "gamesetup.change_map" ) ) ) {		// Clic on the mini-map
			gfx->unset_2D_mode();
			reset_mouse();
			String map_filename = game_data.map_filename;
			char *new_map = select_map( &map_filename );

			gfx->SCREEN_W_TO_640 = 1.0f;				// To have mouse sensibility undependent from the resolution
			gfx->SCREEN_H_TO_480 = 1.0f;
			cursor_type=CURSOR_DEFAULT;
			gfx->set_2D_mode();

			if(new_map) {
				gfx->destroy_texture( glimg );

				if(game_data.map_filename)	free(game_data.map_filename);
				game_data.map_filename = new_map;
				glimg = load_tnt_minimap_fast(game_data.map_filename,&dx,&dy);
				ldx = dx * ( mini_map_x2 - mini_map_x1 ) / 504.0f;
				ldy = dy * ( mini_map_y2 - mini_map_y1 ) / 504.0f;
				minimap_obj->x1 = mini_map_x-ldx;
				minimap_obj->y1 = mini_map_y-ldy;
				minimap_obj->x2 = mini_map_x+ldx;
				minimap_obj->y2 = mini_map_y+ldy;
				minimap_obj->u2 = dx/252.0f;
				minimap_obj->v2 = dy/252.0f;

				map_data.destroy();
				map_data.load( replace_extension( (char*)tmp_char, game_data.map_filename, "ota", 1024 ) );
				String map_info = "";
				if(map_data.missionname)
					map_info += String( map_data.missionname ) + "\n";
				if(map_data.numplayers)
					map_info += String( map_data.numplayers ) + "\n";
				if(map_data.missiondescription)
					map_info += map_data.missiondescription;
				setupgame_area.set_caption("gamesetup.map_info", map_info );
				}

			minimap_obj->Data = glimg;		// Synchronize the picture on GUI
			}

		if(key[KEY_ESC]) done=true;			// Quitte si on appuie sur echap
					// Efface tout
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setupgame_area.draw();

		glEnable(GL_TEXTURE_2D);
		gfx->set_color(0xFFFFFFFF);
		draw_cursor(resize_w,resize_h);
		
					// Affiche
		gfx->flip();
	}while(!done);

	if( setupgame_area.background == gfx->glfond )	setupgame_area.background = 0;
	setupgame_area.destroy();

	map_data.destroy();

	gfx->destroy_texture(glimg);

	gfx->unset_2D_mode();	// Quitte le mode de dessin d'allegro

	reset_mouse();
	while(key[KEY_ESC]) {	rest(1);	poll_keyboard();	}

	if(start_game) {
		if(game_data.map_filename && game_data.game_script) {
			lp_CONFIG->last_script = game_data.game_script;		// Remember the last script we played
			lp_CONFIG->last_map = game_data.map_filename;		// Remember the last map we played
			lp_CONFIG->last_FOW = game_data.fog_of_war;

			game_data.nb_players = 0;
			for(uint16 i = 0 ; i<10 ; i++)		// Move players to the top of the vector, so it's easier to access data
				if( game_data.player_control[i] != PLAYER_CONTROL_NONE ) {
					game_data.player_control[game_data.nb_players] = game_data.player_control[i];
					game_data.player_names[game_data.nb_players] = game_data.player_names[i];
					game_data.player_sides[game_data.nb_players] = game_data.player_sides[i];
					game_data.ai_level[game_data.nb_players] = game_data.ai_level[i];
					game_data.energy[game_data.nb_players] = game_data.energy[i];
					game_data.metal[game_data.nb_players] = game_data.metal[i];
					uint16 e = player_color_map[game_data.nb_players];
					player_color_map[game_data.nb_players] = player_color_map[i];
					player_color_map[i] = e;
					game_data.nb_players++;
					}

			gfx->unset_2D_mode();
			GuardStart( play );
				play(&game_data);
			GuardCatch();
			if( IsExceptionInProgress() ) // if guard threw an error this will be true.
			{
				GuardDisplayAndLogError();   // record and display the error.
				exit(1);                      // we outa here.
			}
			gfx->set_2D_mode();
			gfx->ReInitTexSys();
			glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);
			}

		while(key[KEY_ESC]) {	rest(1);	poll_keyboard();	}
		}
}

void battle_room(void)				// Let players create/join a game
{
	gfx->SCREEN_W_TO_640 = 640.0f / SCREEN_W;				// To have mouse sensibility undependent from the resolution
	gfx->SCREEN_H_TO_480 = 480.0f / SCREEN_H;

	bool done=false;

	gfx->set_2D_mode();

	gfx->ReInitTexSys();

	set_uformat(U_ASCII);		// Juste histoire d'avoir un affichage correct des textes

	glScalef(SCREEN_W/640.0f,SCREEN_H/480.0f,1.0f);

//	Network	TA3D_network;
//	TA3D_network.HostGame("my_game","1234",2,0);

	std::list<String>	servers;					// the server list
	std::list<String>::iterator	l_s_servers;		// iterator to read the server list
	servers.push_front(TRANSLATE("No server found"));

	float dt=0.0f;
	int time = msec_timer;
	float Conv = 0.001f;

	float h=gfx->TA_font.height();

	bool ok_status=false;		// Pour les boutons
	bool cancel_status=false;
	bool o_ok_status=false;
	bool o_cancel_status=false;
	reset_keyboard();

	do
	{
		do
		{
			dt=(msec_timer-time)*Conv;
			rest(1);
		}while(dt<0.02f);
		time=msec_timer;

		o_ok_status=ok_status;
		o_cancel_status=cancel_status;
		ok_status=false;
		cancel_status=false;
		if(mouse_b==1 && mouse_y*gfx->SCREEN_H_TO_480>=440 && mouse_y*gfx->SCREEN_H_TO_480<=460) {
			ok_status=(mouse_x*gfx->SCREEN_W_TO_640>=360 && mouse_x*gfx->SCREEN_W_TO_640<=440);
			cancel_status=(mouse_x*gfx->SCREEN_W_TO_640>=520 && mouse_x*gfx->SCREEN_W_TO_640<=600);
			}

		if(mouse_b==0 && !ok_status && o_ok_status) {		// Click sur ok
			done=true;
			}
		if(mouse_b==0 && !cancel_status && o_cancel_status)			// Click sur retour
			done=true;

		if(key[KEY_ESC]) {
			while(key[KEY_ESC]) {
				rest(1);
				poll_keyboard();
				}
			done=true;
			}
		if(key[KEY_ENTER])
			done=true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		gfx->drawtexture(gfx->glfond,0.0f,0.0f,640.0f,480.0);

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f,0.0f,0.0f,0.5f);
		glBegin(GL_QUADS);
			glVertex2f(10.0f,25.0f);
			glVertex2f(630.0f,25.0f);
			glVertex2f(630.0f,470.0f);
			glVertex2f(10.0f,470.0f);
		glEnd();
		glColor4f(1.0f,1.0f,1.0f,0.5f);
		glBegin(GL_LINE_LOOP);
			glVertex2f(10.0f,25.0f);
			glVertex2f(630.0f,25.0f);
			glVertex2f(630.0f,470.0f);
			glVertex2f(10.0f,470.0f);
		glEnd();
		glBegin(GL_LINE_LOOP);
			glVertex2f(347.0f,50.0f);
			glVertex2f(603.0f,50.0f);
			glVertex2f(603.0f,306.0f);
			glVertex2f(347.0f,306.0f);
		glEnd();
		glBegin(GL_LINES);
			glVertex2f(320.0f,26.0f);
			glVertex2f(320.0f,469.0f);
			glVertex2f(10.0f,32.0f+h);
			glVertex2f(320.0f,32.0f+h);
		glEnd();
		glEnable(GL_TEXTURE_2D);

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		gfx->print(gfx->TA_font,320.0f-gfx->TA_font.length(TRANSLATE("Battle room"))*0.5f,0.0f,0.0f,0xFFFFFFFF,TRANSLATE("Battle room"));
		gfx->print(gfx->TA_font,165.0f-gfx->TA_font.length(TRANSLATE("Server list"))*0.5f,32.0f,0.0f,0xFFFFFFFF,TRANSLATE("Server list"));

		glColor4f(1.0f,1.0f,1.0f,1.0f);

		int i=0;
		for(l_s_servers=servers.begin();l_s_servers!=servers.end();l_s_servers++) {
			gfx->print(gfx->TA_font,20.0f,h*i+64.0f,0.0f,0xFFFFFFFF,l_s_servers->c_str());
			i++;
			}

		glDisable(GL_BLEND);

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_BLEND);

		glbutton(TRANSLATE("ok"),360.0f,440.0f,440.0f,460.0f,ok_status);
		glbutton(TRANSLATE("back"),520.0f,440.0f,600.0f,460.0f,cancel_status);

		draw_cursor();

		gfx->flip();
	}while(!done);

	gfx->unset_2D_mode();

	set_uformat(U_UTF8);
}
