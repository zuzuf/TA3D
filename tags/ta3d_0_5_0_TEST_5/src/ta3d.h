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

int LANG = 1; // Deprecated
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



# include "gfx/glfunc.h" // Must be removed as well as the function below

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
	for (int y = 0; y < pcx->h; ++y)
    {
		int x = 0;
		do
		{
			int c=data[pos++];
			if (c > 191)
            {
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
