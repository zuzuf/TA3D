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

#include "ta3dbase.h"
#include "misc/math.h"

#ifndef TA3D_BASIC_ENGINE
int expected_players;

int LANG=TA3D_LANG_ENGLISH;

int CURSOR_MOVE;
int CURSOR_GREEN;
int CURSOR_CROSS;
int CURSOR_RED;
int CURSOR_LOAD;
int CURSOR_UNLOAD;
int CURSOR_GUARD;
int CURSOR_PATROL;
int CURSOR_REPAIR;
int CURSOR_ATTACK;
int CURSOR_BLUE;
int CURSOR_AIR_LOAD;
int CURSOR_BOMB_ATTACK;
int CURSOR_BALANCE;
int CURSOR_RECLAIM;
int CURSOR_WAIT;
int CURSOR_CANT_ATTACK;
int CURSOR_CROSS_LINK;
int CURSOR_CAPTURE;
int CURSOR_REVIVE;

ANIMS cursor;

int fire;
int build_part;

int start=0;
int cursor_type=CURSOR_DEFAULT;
#endif

/*---------------------------------------------------------------------------------\
|                              void init_rand_table()                              |
|        Build the random table used in critical code                              |
\---------------------------------------------------------------------------------*/

#define RAND_TABLE_SIZE		0x100000
#define RAND_TABLE_MASK		0xFFFFF

int table_pos = 0;
uint32 rand_table[ RAND_TABLE_SIZE ];

void init_rand_table()
{
	srand( (unsigned)time( NULL ) );
	for( int i = 0 ; i < RAND_TABLE_SIZE ; i++ )
		rand_table[i] = TA3D_RAND();				// Make it platform independent
}

/*---------------------------------------------------------------------------------\
|                              int rand_from_table()                               |
|        Get a "random" number from the table                                      |
\---------------------------------------------------------------------------------*/

const int rand_from_table()
{
	table_pos=(table_pos+1)&RAND_TABLE_MASK;
	return rand_table[table_pos];
}

#ifndef TA3D_BASIC_ENGINE

/*---------------------------------------------------------------------------------\
|                              BITMAP *load_memory_pcx(...)                        |
|        Charge le fichier pcx dont data pointe les données                        |
\---------------------------------------------------------------------------------*/
BITMAP *load_memory_pcx(byte *data,RGB *cpal)
{
	if(cpal==NULL) return NULL;		// On ne peut pas charger de fichier 8bits sans palette graphique
	short width,height;
	width=*((short*)(data+8));		// Dimensions
	height=*((short*)(data+10));
	width-=*((short*)(data+4));		// Dimensions
	height-=*((short*)(data+6));

	width++;
	height++;

	BITMAP *pcx = create_bitmap(width,height);
	clear(pcx);

	int pos=128;
	for(int y=0;y<pcx->h;y++) {
		int x=0;
		do
		{
			int c=data[pos++];
			if(c>191) {
				int l=c-192;
				c=data[pos++];
				int col=makecol(cpal[c].r<<2,cpal[c].g<<2,cpal[c].b<<2);
				for(;l>0 && x<pcx->w;l--)
					((int*)(pcx->line[y]))[x++]=col;
				}
			else
				((int*)(pcx->line[y]))[x++]=makecol(cpal[c].r<<2,cpal[c].g<<2,cpal[c].b<<2);
		}while(x<pcx->w);
		}
	return pcx;
}
#endif

/*---------------------------------------------------------------------------------\
|                                   void PutTex(...)                               |
|        Procédure qui affiche la texture Tex dans le rectangle (x1,y1)-(x2,y2)    |
\---------------------------------------------------------------------------------*/

void PutTex(GLuint Tex,float x1,float y1,float x2,float y2)
{
	glBindTexture(GL_TEXTURE_2D,Tex);
	glBegin(GL_QUADS);

		glTexCoord2f(0.0f,0.0f);
		glVertex2f(x1,y1);

		glTexCoord2f(1.0f,0.0f);
		glVertex2f(x2,y1);
					
		glTexCoord2f(1.0f,1.0f);
		glVertex2f(x2,y2);
			
		glTexCoord2f(0.0f,1.0f);
		glVertex2f(x1,y2);

	glEnd();
}

/*
GLuint LoadTex(const String& file)
{
	set_color_depth(32);
	allegro_gl_use_alpha_channel(true);
	BITMAP *bmp=load_bitmap(file.c_str(), NULL);
	GLuint gl_bmp;
	if (g_useTextureCompression)
		allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
	else
		allegro_gl_set_texture_format(GL_RGBA8);
	gl_bmp=allegro_gl_make_texture(bmp);
	allegro_gl_use_alpha_channel(false);
	glBindTexture(GL_TEXTURE_2D, gl_bmp);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	allegro_gl_set_texture_format(-1);
	destroy_bitmap(bmp);
	return gl_bmp;
}
*/

GLuint LoadMaskedTex(const String& file,const String& filealpha)
{
	set_color_depth(32);
	BITMAP *bmp = load_bitmap(file.c_str(), NULL);
	BITMAP *alpha;
	set_color_depth(8);
	alpha=load_bitmap(filealpha.c_str(), NULL);
	set_color_depth(32);
	for (int y = 0; y < bmp->h; ++y)
    {
		for (int x = 0; x < bmp->w; ++x)
			bmp->line[y][(x << 2) + 3] = alpha->line[y][x];
    }
	GLuint gl_bmp;
	allegro_gl_use_alpha_channel(true);
	if(g_useTextureCompression)
		allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
	else
		allegro_gl_set_texture_format(GL_RGBA8);
	gl_bmp=allegro_gl_make_texture(bmp);
	allegro_gl_use_alpha_channel(false);
	glBindTexture(GL_TEXTURE_2D, gl_bmp);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	destroy_bitmap(bmp);
	destroy_bitmap(alpha);
	return gl_bmp;
}

BITMAP* LoadMaskedTexBmp(const String& file, const String& filealpha)
{
	set_color_depth(32);
	BITMAP* bmp = load_bitmap(file.c_str(), NULL);
	set_color_depth(8);
	BITMAP* alpha = load_bitmap(filealpha.c_str(), NULL);
	set_color_depth(32);
	for (int y = 0; y < bmp->h; ++y)
    {
		for (int x = 0; x < bmp->w; ++x)
			bmp->line[y][(x << 2) + 3] = alpha->line[y][x];
    }
	destroy_bitmap(alpha);
	return bmp;
}

#ifndef TA3D_BASIC_ENGINE

#include "3do.h"					// For 3DO/3DM management
#include "scripts/cob.h"					// For unit scripts management
#include "tdf.h"					// For 2D features
#include "EngineClass.h"			// The Core Engine
#include "UnitEngine.h"				// The Unit Engine
#include "tnt.h"					// The TNT loaded
#include "scripts/script.h"					// The game script manager
#include "ai/ai.h"						// AI Engine

#endif


void reset_keyboard()
{
	remove_keyboard();
	install_keyboard();
}
void reset_mouse()
{
	int amx = mouse_x;
	int amy = mouse_y;

	remove_mouse();
	install_mouse();

	position_mouse(amx, amy);
}
