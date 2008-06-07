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

/*-----------------------------------------------------------------------------------\
|                                         tnt.h                                      |
|  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
| des fichiers tnt de total annihilation qui sont les fichiers contenant les cartes  |
| du jeu.                                                                            |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef ENGINE_CLASS		// Inclus les classes du moteur si ce n'est pas déjà fait
#include "EngineClass.h"
#endif

#ifndef __TNT_CLASSES
#define __TNT_CLASSES

struct TNTHEADER		// Structure de l'en-tête du fichier TNT
{
	int		IDversion;
	int		Width;
	int		Height;
	int		PTRmapdata;
	int		PTRmapattr;
	int		PTRtilegfx;
	int		tiles;
	int		tileanims;
	int		PTRtileanim;
	int		sealevel;
	int		PTRminimap;
	int		unknown1;
	int		pad1,pad2,pad3,pad4;
};

MAP	*load_tnt_map(byte *data );		// Charge une map au format TA, extraite d'une archive HPI/UFO

GLuint load_tnt_minimap(byte *data,int *sw,int *sh);		// Charge une minimap d'une carte, extraite d'une archive HPI/UFO

GLuint load_tnt_minimap_fast(char *filename,int *sw,int *sh);		// Charge une minimap d'une carte contenue dans une archive HPI/UFO

BITMAP *load_tnt_minimap_fast_bmp(char *filename);		// Charge une minimap d'une carte contenue dans une archive HPI/UFO

#endif
