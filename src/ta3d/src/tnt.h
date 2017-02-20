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

#ifndef __TA3D_UTILS_TNT_H__
# define __TA3D_UTILS_TNT_H__

# include "misc/string.h"
# include "EngineClass.h"



namespace TA3D
{


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
    union TNTHEADER_U		// For strict-aliasing safety
    {
    	byte bytes[sizeof(TNTHEADER)];
    	TNTHEADER header;
    };

    #define TNTMINIMAP_WIDTH  252
    #define TNTMINIMAP_HEIGHT 252

    struct TNTMINIMAP
    {
	    int w;
	    int h;
	    byte map[TNTMINIMAP_HEIGHT][TNTMINIMAP_WIDTH];
    };
    union TNTMINIMAP_U		// For strict-aliasing safety
    {
    	byte bytes[sizeof(TNTMINIMAP)];
    	TNTMINIMAP map;
    };


                                        // Load a map in TNT format extracted from a HPI archive
	MAP	*load_tnt_map(File *file);		// Charge une map au format TA, extraite d'une archive HPI/UFO

                                                                // Load a minimap from a map file extracted from a HPI archive
	GLuint load_tnt_minimap(File *file,int& sw,int& sh);		// Charge une minimap d'une carte, extraite d'une archive HPI/UFO

                                                                                // Load a minimap from a map file extracted from a HPI archive
    GLuint load_tnt_minimap_fast(const QString& filename, int& sw, int& sh);		// Charge une minimap d'une carte contenue dans une archive HPI/UFO

                                                                    // Load a minimap from a map file extracted from a HPI archive
    SDL_Surface *load_tnt_minimap_fast_bmp(const QString& filename);		// Charge une minimap d'une carte contenue dans une archive HPI/UFO


} // namespace TA3D

#endif // __TA3D_UTILS_TNT_H__
