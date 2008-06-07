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
CAMERA *game_cam=NULL;

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

GLuint LoadTex(const char *file)
{
	set_color_depth(32);
	allegro_gl_use_alpha_channel(true);
	BITMAP *bmp=load_bitmap(file,NULL);
	GLuint gl_bmp;
	if(g_useTextureCompression)
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

GLuint LoadMaskedTex(const char *file,const char *filealpha)
{
	set_color_depth(32);
	BITMAP *bmp=load_bitmap(file,NULL);
	BITMAP *alpha;
	set_color_depth(8);
	alpha=load_bitmap(filealpha,NULL);
	set_color_depth(32);
	for(int y=0;y<bmp->h;y++)
		for(int x=0;x<bmp->w;x++)
			bmp->line[y][(x<<2)+3]=alpha->line[y][x];
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

BITMAP *LoadMaskedTexBmp(const char *file,const char *filealpha)
{
	set_color_depth(32);
	BITMAP *bmp=load_bitmap(file,NULL);
	set_color_depth(8);
	BITMAP *alpha=load_bitmap(filealpha,NULL);
	set_color_depth(32);
	for(int y=0;y<bmp->h;y++)
		for(int x=0;x<bmp->w;x++)
			bmp->line[y][(x<<2)+3]=alpha->line[y][x];
	destroy_bitmap(alpha);
	return bmp;
}

#ifndef TA3D_BASIC_ENGINE

#include "3do.h"					// For 3DO/3DM management
#include "cob.h"					// For unit scripts management
#include "tdf.h"					// For 2D features
#include "EngineClass.h"			// The Core Engine
#include "UnitEngine.h"				// The Unit Engine
#include "tnt.h"					// The TNT loaded
#include "script.h"					// The game script manager
#include "ai.h"						// AI Engine

inline INT_ELEMENT read_gui_element( cTAFileParser *parser, const String &element, bool bottom = false )
{
	INT_ELEMENT gui_element;
	gui_element.x1 = parser->PullAsInt( element + ".x1" );
	gui_element.y1 = parser->PullAsInt( element + ".y1" );
	gui_element.x2 = parser->PullAsInt( element + ".x2" );
	gui_element.y2 = parser->PullAsInt( element + ".y2" );

	if( bottom ) {
		gui_element.y1 += SCREEN_H - 480;
		gui_element.y2 += SCREEN_H - 480;
		}

	return gui_element;
}

void SIDEDATA::load_data()
{
	destroy();

	cTAFileParser mod_parser( "ta3d.mod" );
	unit_ext = mod_parser.PullAsString( "MOD.unit_ext", ".fbi" );
	unit_dir = mod_parser.PullAsString( "MOD.unit_dir", "units\\" );
	model_dir = mod_parser.PullAsString( "MOD.model_dir", "objects3d\\" );
	download_dir = mod_parser.PullAsString( "MOD.download_dir", "download\\" );
	weapon_dir = mod_parser.PullAsString( "MOD.weapon_dir", "weapons\\" );
	guis_dir = mod_parser.PullAsString( "MOD.guis_dir", "guis\\" );
	gamedata_dir = mod_parser.PullAsString( "MOD.gamedata_dir", "gamedata\\" );
	if( unit_dir[ unit_dir.length() - 1 ] != '\\' )			unit_dir += "\\";
	if( model_dir[ model_dir.length() - 1 ] != '\\' )		model_dir += "\\";
	if( download_dir[ download_dir.length() - 1 ] != '\\' )	download_dir += "\\";
	if( weapon_dir[ weapon_dir.length() - 1 ] != '\\' )		weapon_dir += "\\";
	if( guis_dir[ guis_dir.length() - 1 ] != '\\' )			guis_dir += "\\";
	if( gamedata_dir[ gamedata_dir.length() - 1 ] != '\\' )	gamedata_dir += "\\";

	cTAFileParser	sidedata_parser( gamedata_dir + "sidedata.tdf" );

	nb_side = 0;

	while( sidedata_parser.PullAsString( format( "side%d.name", nb_side ), "" ) != "" ) {
		side_name[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.name", nb_side ) ).c_str() );
		side_pref[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.nameprefix", nb_side ) ).c_str() );
		side_com[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.commander", nb_side ) ).c_str() );
		side_int[ nb_side ] = strdup( sidedata_parser.PullAsString( format( "side%d.intgaf", nb_side ) ).c_str() );

		int pal_id = sidedata_parser.PullAsInt( format( "side%d.metalcolor", nb_side ) );
		side_int_data[ nb_side ].metal_color = makeacol( pal[ pal_id ].r << 2, pal[ pal_id ].g << 2, pal[ pal_id ].b << 2, 0xFF );
		pal_id = sidedata_parser.PullAsInt( format( "side%d.energycolor", nb_side ) );
		side_int_data[ nb_side ].energy_color = makeacol( pal[ pal_id ].r << 2, pal[ pal_id ].g << 2, pal[ pal_id ].b << 2, 0xFF );

		side_int_data[ nb_side ].EnergyBar = read_gui_element( &sidedata_parser, format( "side%d.energybar", nb_side ) );
		side_int_data[ nb_side ].EnergyNum = read_gui_element( &sidedata_parser, format( "side%d.energynum", nb_side ) );
		side_int_data[ nb_side ].EnergyMax = read_gui_element( &sidedata_parser, format( "side%d.energymax", nb_side ) );
		side_int_data[ nb_side ].Energy0 = read_gui_element( &sidedata_parser, format( "side%d.energy0", nb_side ) );
		side_int_data[ nb_side ].EnergyProduced = read_gui_element( &sidedata_parser, format( "side%d.energyproduced", nb_side ) );
		side_int_data[ nb_side ].EnergyConsumed = read_gui_element( &sidedata_parser, format( "side%d.energyconsumed", nb_side ) );

		side_int_data[ nb_side ].MetalBar = read_gui_element( &sidedata_parser, format( "side%d.metalbar", nb_side ) );
		side_int_data[ nb_side ].MetalNum = read_gui_element( &sidedata_parser, format( "side%d.metalnum", nb_side ) );
		side_int_data[ nb_side ].MetalMax = read_gui_element( &sidedata_parser, format( "side%d.metalmax", nb_side ) );
		side_int_data[ nb_side ].Metal0 = read_gui_element( &sidedata_parser, format( "side%d.metal0", nb_side ) );
		side_int_data[ nb_side ].MetalProduced = read_gui_element( &sidedata_parser, format( "side%d.metalproduced", nb_side ) );
		side_int_data[ nb_side ].MetalConsumed = read_gui_element( &sidedata_parser, format( "side%d.metalconsumed", nb_side ) );

		side_int_data[ nb_side ].UnitName = read_gui_element( &sidedata_parser, format( "side%d.unitname", nb_side ), true );
		side_int_data[ nb_side ].DamageBar = read_gui_element( &sidedata_parser, format( "side%d.damagebar", nb_side ), true );

		side_int_data[ nb_side ].UnitName2 = read_gui_element( &sidedata_parser, format( "side%d.unitname2", nb_side ), true );
		side_int_data[ nb_side ].DamageBar2 = read_gui_element( &sidedata_parser, format( "side%d.damagebar2", nb_side ), true );

		side_int_data[ nb_side ].UnitMetalMake = read_gui_element( &sidedata_parser, format( "side%d.unitmetalmake", nb_side ), true );
		side_int_data[ nb_side ].UnitMetalUse = read_gui_element( &sidedata_parser, format( "side%d.unitmetaluse", nb_side ), true );
		side_int_data[ nb_side ].UnitEnergyMake = read_gui_element( &sidedata_parser, format( "side%d.unitenergymake", nb_side ), true );
		side_int_data[ nb_side ].UnitEnergyUse = read_gui_element( &sidedata_parser, format( "side%d.unitenergyuse", nb_side ), true );

		side_int_data[ nb_side ].Name = read_gui_element( &sidedata_parser, format( "side%d.name", nb_side ), true );
		side_int_data[ nb_side ].Description = read_gui_element( &sidedata_parser, format( "side%d.description", nb_side ), true );
		
		nb_side++;
		}
}

int	SIDEDATA::get_side_id(const char *side)
{
	for(int i=0;i<nb_side;i++)
		if(strcasecmp(side,side_name[i])==0)
			return i;
	return -1;
}

#endif

SIDEDATA ta3d_sidedata;

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
