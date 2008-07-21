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

/*-----------------------------------------------------------------\
|                           pathfinding.h                          |
|   contient toutes les fonctions et classes nécessaires à la mise |
| en place du pathfinding de TA3D                                  |
\-----------------------------------------------------------------*/

#ifndef __TA3D_AI_PATH_FINDING_H__
# define __TA3D_AI_PATH_FINDING_H__

# include "../EngineClass.h"

#define MAX_PATH_EXEC		1

namespace TA3D
{


    class PATH_NODE			// Noeud d'un chemin
    {
    public:
        int			x,y;				// Position du noeud(sur la carte)
        VECTOR3D		Pos;				// Position du noeud sur le terrain
        PATH_NODE	*next;				// Noeud suivant
        bool		made_direct;		// Flag : have we made this path direct or is it still the old one ?

        PATH_NODE()
            :x(0), y(0), Pos(), next(NULL), made_direct(false)
        {}

        PATH_NODE(const int X, const int Y, VECTOR3D &P, PATH_NODE *N = NULL)
            :x(X), y(Y), next(N), made_direct(false)
        {
            Pos = P;
        }

        PATH_NODE(const int X, const int Y, PATH_NODE *N = NULL)
            :x(X), y(Y), next(N), made_direct(false)
        {}
    };


    void destroy_path(PATH_NODE *path);		// Détruit un chemin
    PATH_NODE *find_path( SECTOR **map_data, float **map, byte **zone, int map_w, int map_h, int bloc_w, int bloc_h, float dh_max, float low_level, float high_level, VECTOR3D Start, VECTOR3D End, int mw, int mh, int u_idx, int m_dist = 0, float hover_h=-100.0f );
    float path_length(PATH_NODE *path);
    void simplify_path(PATH_NODE *path);
    PATH_NODE *next_node( PATH_NODE *path,SECTOR **map_data, float **map, int bloc_w, int bloc_h, float dh_max, float low_level, float high_level, int mw, int mh, int u_idx, float hover_h );
    void compute_coord(PATH_NODE *path,int map_w,int map_h,int bloc_w,int bloc_h);
    PATH_NODE *direct_path(VECTOR3D End);
    bool is_direct(SECTOR **map_data,float **h_map,float dh_max,float h_min,float h_max,PATH_NODE &A,PATH_NODE &B,int mw,int mh,int bw,int bh,int u_idx,float hover_h);
    void make_path_direct(SECTOR **map_data,float **h_map,float dh_max,float h_min,float h_max,PATH_NODE *path,int mw,int mh,int bw,int bh,int u_idx,float hover_h);


} // namespace TA3D

#endif // __TA3D_AI_PATH_FINDING_H__
