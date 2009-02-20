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
|                           pathfinding.cpp                        |
|   contient toutes les fonctions et classes nécessaires à la mise |
| en place du pathfinding de TA3D                                  |
\-----------------------------------------------------------------*/

#include "../stdafx.h"
#include "../misc/matrix.h"
#include "../TA3D_NameSpace.h"
#include "../ta3dbase.h"
#include "../3do.h"					// For 3D models / Pour la lecture des fichiers 3D
#include "../scripts/cob.h"					// For unit scripts / Pour la lecture et l'éxecution des scripts
#include "../tdf.h"					// For map features / Pour la gestion des éléments du jeu
//#include "fbi.h"					// For unit types / Pour la gestion des unités
//#include "weapons.h"				// For weapons / Pour la gestion des armes
#include "../EngineClass.h"			// The engine, also includes pathfinding.h / Inclus le moteur(dont le fichier pathfinding.h)
#include "../misc/math.h"

#define PATHFINDER_MAX_LENGTH			10000


namespace TA3D
{


    PATH_NODE *nodes[ PATHFINDER_MAX_LENGTH + 100 ];			// This array is used to compute a direct path, we need it because it decreases the computation time

    inline int path_len(PATH_NODE *path)
    {
        int l = 0;
        while(path)
        {
            ++l;
            path = path->next;
        }
        return l;
    }

    inline int sq(int a)
    {
        return a*a;
    }

    inline int sgn( int a )
    {
        return a < 0 ? -1 : a > 0 ? 1 : 0;
    }

    void destroy_path(PATH_NODE *path)		// Destroy a path / Détruit un chemin
    {
        while( path )
        {
            PATH_NODE *next = path->next;
            delete path;
            path = next;
        }
    }

    void simplify_path(PATH_NODE *path)
    {
        for( PATH_NODE *i = path ; i && i->next && i->next->next ; )
        {
            if( (i->next->next->x - i->x)*(i->next->y - i->y) == (i->next->next->y - i->y)*(i->next->x - i->x) )
            {
                PATH_NODE *tmp = i->next;
                i->next = i->next->next;
                delete tmp;
            }
            else
                i = i->next;
        }
    }


    PATH_NODE *direct_path(Vector3D End)		// Creates a direct path to the target / Chemin direct vers la cible
    {
        return new PATH_NODE(0, 0, End);
    }

    void make_path_direct(SECTOR **map_data,float **h_map,float dh_max,float h_min,float h_max,PATH_NODE *path,int mw,int mh,int bw,int bh,int u_idx,float hover_h)			// Elimine les étapes inutiles (qui rendent un chemin indirect)
    {
        if(path == NULL || path->next == NULL)	return;			// Let's say it's already direct
        int length = 0;
        PATH_NODE *cur = path;
        do {								// Fill the array
            nodes[ length++ ] = cur;
            cur = cur->next;
        } while( cur );

        int s = 0;
        int e = length - 1;

        while( s < length - 1 )
        {
            if(is_direct( map_data, h_map, dh_max, h_min, h_max, *(nodes[ s ]), *(nodes[ e ]), mw, mh, bw, bh, u_idx, hover_h )) // Si le chemin est direct, on élimine le point du milieu
            {
                if( s + 1 < e ) // Destroy now useless nodes
                {
                    nodes[ e - 1 ]->next = NULL;
                    destroy_path( nodes[ s + 1] );
                }
                nodes[ s ]->next = nodes[ e ];
                nodes[ s ]->made_direct = true;
                s = e;
                e = length - 1;
            }
            else
            {
                e = (e + s) >> 1;
                if( s + 1 >= e )	break;			// We're done
            }
        }
    }

    inline bool check_rect(SECTOR **map_data,int x1,int y1,int w,int h,short c,int bloc_w,int bloc_h)
    {
        int fy=y1+h;
        int fx=x1+w;
        int x,y;
        y=y1;
        if(y>=0 && y<bloc_h)
            for(x=x1;x<fx;x++)
                if(x>=0 && x<bloc_w)
                    if(map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        return false;
        y=fy-1;
        if(y>=0 && y<bloc_h)
            for(x=x1;x<fx;x++)
                if(x>=0 && x<bloc_w)
                    if(map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        return false;
        for(int y=y1+1;y<fy-1;y++)
            if(y>=0 && y<bloc_h) {
                x=x1;
                if(x>=0 && x<bloc_w)
                    if(map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        return false;
                x=fx-1;
                if(x>=0 && x<bloc_w)
                    if(map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        return false;
            }
        return true;
    }

    inline bool check_rect_full(SECTOR **map_data,float **h_map,int x1,int y1,int w,int h,short c,float dh_max,float h_min,float h_max,int bloc_w,int bloc_h,float hover_h)
    {
        int fy=y1+h;
        int fx=x1+w;
        int x,y;
        y=y1;
        if(y>=0 && y<bloc_h)
            for(x=x1;x<fx;x++)
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        || ( map_data[y][x].dh>dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
        y=fy-1;
        if(y>=0 && y<bloc_h)
            for(x=x1;x<fx;x++)
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        || ( map_data[y][x].dh>dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
        for(int y=y1+1;y<fy-1;y++)
            if(y>=0 && y<bloc_h) {
                x=x1;
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        || ( map_data[y][x].dh>dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
                x=fx-1;
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                        || ( map_data[y][x].dh>dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
            }
        return true;
    }

    bool is_direct(SECTOR **map_data,float **h_map,float dh_max,float h_min,float h_max,PATH_NODE &A,PATH_NODE &B,int mw,int mh,int bw,int bh,int u_idx,float hover_h)
    {
        int dx = B.x - A.x;
        int dy = B.y - A.y;
        int dmw = mw >> 1;
        int dmh = mh >> 1;
        if( dx == dy && dx == 0 )	return true;

        int Ax = A.x, Bx = B.x;
        int Ay = A.y, By = B.y;

        if( abs(dx) > abs(dy) ) {			// Vérifie si le chemin en ligne convient
            if( Ax > Bx ) {
                Ax ^= Bx;	Bx ^= Ax;	Ax ^= Bx;
                Ay ^= By;	By ^= Ay;	Ay ^= By;
            }
            float coef = ((float)( By - Ay )) / ( Bx - Ax );

            for( int x = Ax ; x <= Bx ; x ++ ) {
                int y = Ay + (int)((x - Ax)*coef+0.5f);
                if( !check_rect_full( map_data, h_map, x-dmw, y-dmh, mw, mh, u_idx, dh_max, h_min, h_max, bw, bh, hover_h ) )	// Check for obstacles
                    return false;
            }
        }
        else {
            if( Ay > By ) {
                Ax ^= Bx;	Bx ^= Ax;	Ax ^= Bx;
                Ay ^= By;	By ^= Ay;	Ay ^= By;
            }
            float coef = ((float)( Bx - Ax )) / ( By - Ay );
            for( int y = Ay ; y <= By ; y++ ) {
                int x = Ax + (int)((y - Ay)*coef+0.5f);
                if( !check_rect_full( map_data, h_map, x-dmw, y-dmh, mw, mh, u_idx, dh_max, h_min, h_max, bw, bh, hover_h ) )	// Check for obstacles
                    return false;
            }
        }
        return true;
    }

    PATH_NODE *next_node( PATH_NODE *path,						// Passe au noeud suivant
                          SECTOR **map_data, float **map, int bloc_w, int bloc_h, float dh_max, float low_level, float high_level, int mw, int mh, int u_idx, float hover_h )
    {
        if(path) {
            PATH_NODE *tmp = path;
            path = path->next;
            delete tmp;
            if( path != NULL && !path->made_direct )
                make_path_direct( map_data, map, dh_max, low_level, high_level, path, mw, mh, bloc_w, bloc_h, u_idx, hover_h );		// Make the path easier to follow and shorter
        }
        return path;
    }

    void compute_coord(PATH_NODE *path,int map_w,int map_h,int bloc_w,int bloc_h)
    {
        PATH_NODE *tmp = path;
        int h_w = map_w >> 1;
        int h_h = map_h >> 1;
        while(tmp) {
            tmp->Pos.x = (tmp->x << 3) + 4 - h_w;
            tmp->Pos.z = (tmp->y << 3) + 4 - h_h;
            tmp->Pos.y = 0.0f;
            tmp = tmp->next;
        }
    }

    PATH_NODE *find_path( SECTOR **map_data, float **map, byte **zone, int map_w, int map_h, int bloc_w, int bloc_h, float dh_max, float low_level, float high_level, Vector3D Start, Vector3D End, int mw, int mh, int u_idx, int m_dist, float hover_h )
    {
        int start_x = ((int)Start.x + (map_w >> 1) + 4) >> 3;
        int start_y = ((int)Start.z + (map_h >> 1) + 4) >> 3;
        int end_x = ((int)End.x + (map_w >> 1) + 4) >> 3;
        int end_y = ((int)End.z + (map_h >> 1) + 4) >> 3;

        int START_X = start_x << 1;
        int START_Y = start_y << 1;
        int END_X = end_x << 1;
        int END_Y = end_y << 1;

        PATH_NODE *path = new PATH_NODE( START_X, START_Y );
        PATH_NODE *start = path;
        int n = 0;

        byte order_p1[] = { 1, 2, 3, 4, 5, 6, 7, 0 };
        byte order_p2[] = { 2, 3, 4, 5, 6, 7, 0, 1 };
        byte order_m1[] = { 7, 0, 1, 2, 3, 4, 5, 6 };
        byte order_m2[] = { 6, 7, 0, 1, 2, 3, 4, 5 };
        char order_dx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
        char order_dy[] = { -1, -1, -1, 0, 1, 1, 1, 0 };

        int PATH_MAX_LENGTH = Math::Max(256, Math::Min((int)(sqrtf(sq(end_x - start_x) + sq(end_y - start_y)) * 5.0f), PATHFINDER_MAX_LENGTH));

        m_dist *= m_dist;
        m_dist <<= 2;

        int smh = mh;
        int smw = mw;

        int mw_h = smw >> 1;
        int mh_h = smh >> 1;

        int bloc_w_db = bloc_w << 1;
        int bloc_h_db = bloc_h << 1;

        if( path->x < 0 || path->y < 0 || path->x >= bloc_w_db || path->y >= bloc_h_db ) {		// Hum we are out !!
            delete path;
            return NULL;			// So we can't find a path
        }

        while( n < PATH_MAX_LENGTH && ( ( m_dist == 0 && ( path->x != END_X || path->y != END_Y ) ) || ( m_dist > 0 && sq( path->x - END_X ) + sq( path->y - END_Y ) > m_dist ) ) ) {
            zone[ path->y ][ path->x ]++;

            int m = -1;

            int nx = path->x;
            int ny = path->y;
            if( abs( END_X - path->x ) > abs( END_Y - path->y ) )
                nx += sgn( END_X - path->x );
            else
                ny += sgn( END_Y - path->y );

            if( nx < 0 || ny < 0 || nx >= bloc_w_db || ny >= bloc_h_db )
                break;		// If we have to go out there is a problem ...

            if( zone[ ny ][ nx ] >= 3 )	break;		// Looping ...

            int NX = nx >> 1, NY = ny >> 1;

            if( zone[ ny ][ nx ] || !check_rect_full( map_data, map, NX - mw_h, NY - mh_h, smw, smh, u_idx, dh_max, low_level, high_level, bloc_w, bloc_h, hover_h ) ) {
                int dist[ 8 ];
                int rdist[ 8 ];
                bool zoned[ 8 ];
                for( int e = 0 ; e < 8 ; e++ ) {			// Gather required data
                    rdist[ e ] = dist[ e ] = -1;
                    nx = path->x + order_dx[ e ];
                    ny = path->y + order_dy[ e ];
                    NX = nx >> 1;
                    NY = ny >> 1;
                    zoned[ e ] = false;
                    if( nx < 0 || ny < 0 || nx >= bloc_w_db || ny >= bloc_h_db )	continue;
                    zoned[ e ] = zone[ ny ][ nx ];
                    if( ((path->x >> 1) != NX || (path->y >> 1) != NY) && !zone[ NY ][ NX ] )				// No need to do it twice
                        if( !check_rect_full( map_data, map, NX - mw_h, NY - mh_h, smw, smh, u_idx, dh_max, low_level, high_level, bloc_w, bloc_h, hover_h ) )
                            continue;
                    rdist[ e ] = dist[ e ] = sq( END_X - nx ) + sq( END_Y - ny );

                    if( zoned[ e ] )
                        dist[ e ] = -1;
                }
                for( int e = 1 ; e < 8 ; e += 2 ) {		// Look for a way to go
                    if( ( (dist[ order_m1[ e ] ] == -1 && !zoned[ order_m1[ e ] ]) || (dist[ order_p1[ e ] ] == -1 && !zoned[ order_p1[ e ] ])
                          || (dist[ order_m2[ e ] ] == -1 && !zoned[ order_m2[ e ] ]) || (dist[ order_p2[ e ] ] == -1 && !zoned[ order_p2[ e ] ]) ) && dist[ e ] >= 0 ) {
                        if( m == -1 )	m = e;
                        else if( dist[ e ] < dist[ m ] )
                            m = e;
                    }
                }
                if( m == -1 ) {							// Second try
                    for( int e = 1 ; e < 8 ; e += 2 ) {
                        if( dist[ e ] >= 0 ) {
                            if( m == -1 )	m = e;
                            else if( dist[ e ] < dist[ m ] )
                                m = e;
                        }
                    }
                    if( m == -1 ) {						// Ok we already went everywhere, then compute data differently
                        for( int e = 1 ; e < 8 ; e+= 2 ) {
                            if( rdist[ e ] == -1 )	continue;
                            nx = path->x + order_dx[ e ];
                            ny = path->y + order_dy[ e ];
                            dist[ e ] = rdist[ e ] + 1000 * zone[ ny ][ nx ];
                        }
                        for( int e = 1 ; e < 8 ; e += 2 )		// Ultimate test
                            if( dist[ e ] >= 0 ) {
                                if( m == -1 )	m = e;
                                else if( dist[ e ] < dist[ m ] )
                                    m = e;
                            }
                    }
                }
                if( m >= 0 ) {			// We found something
                    nx = path->x + order_dx[ m ];
                    ny = path->y + order_dy[ m ];
                }
            }
            else
                m = -2;

            if( m == -1 )
                break;

            path->next = new PATH_NODE( nx, ny, NULL );
            path = path->next;

            n++;
        }

        path = start;

        if( path ) {
            for( PATH_NODE *cur = path ; cur != NULL ; cur = cur->next ) {		// Do some cleaning
                zone[cur->y][cur->x] = 0;
                cur->x >>= 1;
                cur->y >>= 1;
            }

            for( PATH_NODE *cur = path ; cur->next != NULL ; ) {				// Remove duplicated points
                if( cur->x == cur->next->x && cur->y == cur->next->y ) {
                    PATH_NODE *tmp = cur->next->next;
                    delete cur->next;
                    cur->next = tmp;
                }
                else
                    cur = cur->next;
            }
            simplify_path( path );												// Remove useless points
            make_path_direct( map_data, map, dh_max, low_level, high_level, path, mw, mh, bloc_w, bloc_h, u_idx, hover_h );		// Make the path easier to follow and shorter

            compute_coord( path, map_w, map_h, bloc_w, bloc_h );

            PATH_NODE *tmp = path->next;
            delete path;
            path = tmp;		// The unit is already at Start!! So remove it
        }

        return path;
    }

    float path_length(PATH_NODE *path)
    {
        if(path!=NULL) {
            if(path->next==NULL)
                return 1.0f;
            return (sqrtf((float)((path->x-path->next->x)*(path->x-path->next->x)+(path->y-path->next->y)*(path->y-path->next->y)))+path_length(path->next));
        }
        return 0.0f;
    }


} // namespace TA3D

