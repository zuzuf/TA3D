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

#include <stdafx.h>
#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <scripts/cob.h>					// For unit scripts / Pour la lecture et l'éxecution des scripts
#include <tdf.h>					// For map features / Pour la gestion des éléments du jeu
#include <EngineClass.h>			// The engine, also includes pathfinding.h / Inclus le moteur(dont le fichier pathfinding.h)
#include <misc/math.h>
#include <UnitEngine.h>

#define PATHFINDER_MAX_LENGTH			10000


namespace TA3D
{
	inline int sq(int a)
	{
		return a*a;
	}

	inline int sgn( int a )
	{
		return a < 0 ? -1 : a > 0 ? 1 : 0;
	}

	Mutex Pathfinder::sMutex;

	Pathfinder *Pathfinder::instance()
	{
		static Pathfinder sInstance;
		return &sInstance;
	}

	namespace AI
	{
		Path::Path() : pos(), nodes(), _ready(false)
		{
		}

		void Path::next()
		{
			MutexLocker mLock(pMutex);
			if (nodes.empty())
				return;

			nodes.pop_front();
			computeCoord();
		}

		bool Path::empty()
		{
			lock();
			bool b = nodes.empty();
			unlock();
			return b;
		}

		void Path::clear()
		{
			lock();

			nodes.clear();
			pos.reset();
			_ready = false;

			unlock();
		}

		void Path::computeCoord()
		{
			lock();
			if (nodes.empty())
			{
				unlock();
				return;
			}

			pos.x = float((nodes.front().x() << 3) + 4 - the_map->map_w_d);
			pos.z = float((nodes.front().z() << 3) + 4 - the_map->map_h_d);
			pos.y = 0.0f;

			unlock();
		}

#define SAVE( i )	gzwrite( file, (void*)&(i), sizeof( i ) )
#define LOAD( i )	gzread( file, (void*)&(i), sizeof( i ) )

		void Path::save(gzFile file)
		{
			MutexLocker mLock(pMutex);

			int n = int(nodes.size());
			SAVE( n );
			for(iterator i = begin() ; i != end() ; ++i)
			{
				SAVE( i->x() );
				SAVE( i->z() );
			}
		}

		void Path::load(gzFile file)
		{
			MutexLocker mLock(pMutex);

			clear();
			int n(0);
			LOAD( n );
			for(int i = 0 ; i < n ; ++i)
			{
				Node node(0,0);
				LOAD( node.x() );
				LOAD( node.z() );
				nodes.push_back(node);
			}
		}
	}

	Pathfinder::Pathfinder() : tasks()
	{
	}

	void Pathfinder::clear()
	{
		MutexLocker mLock(pMutex);
		tasks.clear();
	}

	void Pathfinder::addTask(int idx, int dist, const Vector3D &start, const Vector3D &end)
	{
		Task t = { dist, idx, units.unit[idx].ID, start, end };

		lock();
		bool found = false;
		// If we have already made a request update it
		for(TaskList::iterator i = tasks.begin() ; i != tasks.end() ; ++i)
		{
			if (i->UID == t.UID)
			{
				found = true;
				*i = t;
			}
		}

		// Otherwise add a new request to the task list
		if (!found)
			tasks.push_back(t);
		if (!isRunning())
			this->start();
		unlock();
	}

	AI::Path Pathfinder::directPath(const Vector3D &end)
	{
		int x = ((int)end.x + the_map->map_w_d + 4) >> 3;
		int z = ((int)end.z + the_map->map_h_d + 4) >> 3;
		AI::Path path;
		path.push_back(AI::Path::Node(x, z));
		path.setPos(end);
		path._ready = true;

		return path;
	}

	Pathfinder::~Pathfinder()
	{
		destroyThread();
	}

	void Pathfinder::proc(void*)
	{
		lock();
		while(!tasks.empty() && !pDead)
		{
			Task cur = tasks.front();
			tasks.pop_front();
			unlock();

			// Here we are free to compute this path
			AI::Path path;
			findPath(path, cur);
			path._ready = true;

			Unit *pUnit = &(units.unit[cur.idx]);
			pUnit->lock();
			if (pUnit->ID == cur.UID
				&& (pUnit->flags & 1)
				&& !pUnit->mission.empty()
				&& (pUnit->mission->getFlags() & MISSION_FLAG_MOVE))
			{
				pUnit->mission->Path() = path;
			}
			pUnit->unlock();

			rest(0);		// We don't want to use all the CPU here
			lock();
		}
		unlock();
	}

	void Pathfinder::findPath( AI::Path &path, const Task &task )
	{
		units.unit[task.idx].lock();
		if (units.unit[task.idx].ID != task.UID && units.unit[task.idx].type_id >= 0)
		{
			units.unit[task.idx].unlock();
			return;
		}
		UnitType *pType = unit_manager.unit_type[units.unit[task.idx].type_id];
		units.unit[task.idx].unlock();

		MutexLocker mLock(sMutex);

		int start_x = ((int)task.start.x + the_map->map_w_d + 4) >> 3;
		int start_z = ((int)task.start.z + the_map->map_h_d + 4) >> 3;
		int end_x = ((int)task.end.x + the_map->map_w_d + 4) >> 3;
		int end_z = ((int)task.end.z + the_map->map_h_d + 4) >> 3;

		int START_X = start_x << 1;
		int START_Z = start_z << 1;
		int END_X = end_x << 1;
		int END_Z = end_z << 1;

		std::deque<AI::Path::Node> nodes;
		nodes.push_back(AI::Path::Node(START_X, START_Z));
		int n = 0;

		int order_p1[] = { 1, 2, 3, 4, 5, 6, 7, 0 };
		int order_p2[] = { 2, 3, 4, 5, 6, 7, 0, 1 };
		int order_m1[] = { 7, 0, 1, 2, 3, 4, 5, 6 };
		int order_m2[] = { 6, 7, 0, 1, 2, 3, 4, 5 };
		int order_dx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
		int order_dz[] = { -1, -1, -1, 0, 1, 1, 1, 0 };

		int PATH_MAX_LENGTH = Math::Max(256, Math::Min((int)(sqrtf(sq(end_x - start_x) + sq(end_z - start_z)) * 5.0f), PATHFINDER_MAX_LENGTH));

		int m_dist = task.dist;
		m_dist *= m_dist;
		m_dist <<= 2;

		int smh = pType->FootprintZ;
		int smw = pType->FootprintX;

		int mw_h = smw >> 1;
		int mh_h = smh >> 1;

		int bloc_w_db = the_map->bloc_w_db;
		int bloc_h_db = the_map->bloc_h_db;

		if (nodes.back().x() < 0 || nodes.back().z() < 0 || nodes.back().x() >= bloc_w_db || nodes.back().z() >= bloc_h_db)		// Hum we are out !!
		{
			path.clear();
			return;			// So we can't find a path
		}

		Grid<byte> &zone = the_map->path;

		while (n < PATH_MAX_LENGTH && ((m_dist == 0 && (nodes.back().x() != END_X || nodes.back().z() != END_Z)) || (m_dist > 0 && sq( nodes.back().x() - END_X ) + sq( nodes.back().z() - END_Z) > m_dist)))
		{
			++zone( nodes.back().z(), nodes.back().x() );

			int m = -1;

			int nx = nodes.back().x();
			int nz = nodes.back().z();
			if (abs( END_X - nodes.back().x() ) > abs( END_Z - nodes.back().z() ))
				nx += sgn( END_X - nodes.back().x() );
			else
				nz += sgn( END_Z - nodes.back().z() );

			if (nx < 0 || nz < 0 || nx >= bloc_w_db || nz >= bloc_h_db)
				break;		// If we have to go out there is a problem ...

			if (zone(nx, nz) >= 3 )	break;		// Looping ...

			int NX = nx >> 1, NZ = nz >> 1;

			if (zone( nx, nz ) || !checkRectFull( NX - mw_h, NZ - mh_h, task.idx, pType ))
			{
				int dist[ 8 ];
				int rdist[ 8 ];
				bool zoned[ 8 ];
				for( int e = 0 ; e < 8 ; ++e )			// Gather required data
				{
					rdist[ e ] = dist[ e ] = -1;
					nx = nodes.back().x() + order_dx[ e ];
					nz = nodes.back().z() + order_dz[ e ];
					NX = nx >> 1;
					NZ = nz >> 1;
					zoned[ e ] = false;
					if (nx < 0 || nz < 0 || nx >= bloc_w_db || nz >= bloc_h_db)	continue;
					zoned[ e ] = zone(nx, nz);
					if (((nodes.back().x() >> 1) != NX || (nodes.back().z() >> 1) != NZ) && !zone(nx, nz))				// No need to do it twice
						if (!checkRectFull( NX - mw_h, NZ - mh_h, task.idx, pType ))
							continue;
					rdist[ e ] = dist[ e ] = sq( END_X - nx ) + sq( END_Z - nz );

					if (zoned[ e ])
						dist[ e ] = -1;
				}
				for ( int e = 1 ; e < 8 ; e += 2 )		// Look for a way to go
				{
					if (( (dist[ order_m1[ e ] ] == -1 && !zoned[ order_m1[ e ] ]) || (dist[ order_p1[ e ] ] == -1 && !zoned[ order_p1[ e ] ])
						  || (dist[ order_m2[ e ] ] == -1 && !zoned[ order_m2[ e ] ]) || (dist[ order_p2[ e ] ] == -1 && !zoned[ order_p2[ e ] ]) ) && dist[ e ] >= 0)
					{
						if (m == -1)	m = e;
						else if (dist[ e ] < dist[ m ])
							m = e;
					}
				}
				if (m == -1)							// Second try
				{
					for( int e = 1 ; e < 8 ; e += 2 )
					{
						if (dist[ e ] >= 0)
						{
							if (m == -1)	m = e;
							else if (dist[ e ] < dist[ m ])
								m = e;
						}
					}
					if (m == -1)						// Ok we already went everywhere, then compute data differently
					{
						for( int e = 1 ; e < 8 ; e+= 2 )
						{
							if (rdist[ e ] == -1)	continue;
							nx = nodes.back().x() + order_dx[ e ];
							nz = nodes.back().z() + order_dz[ e ];
							dist[ e ] = rdist[ e ] + 1000 * zone(nx, nz);
						}
						for( int e = 1 ; e < 8 ; e += 2 )		// Ultimate test
							if (dist[ e ] >= 0)
							{
								if (m == -1)	m = e;
								else if (dist[ e ] < dist[ m ])
									m = e;
							}
					}
				}
				if (m >= 0)			// We found something
				{
					nx = nodes.back().x() + order_dx[ m ];
					nz = nodes.back().z() + order_dz[ m ];
				}
			}
			else
				m = -2;

			if (m == -1)
				break;

			nodes.push_back( AI::Path::Node(nx, nz) );

			++n;
		}

		if (!nodes.empty())
		{
			for( std::deque<AI::Path::Node>::iterator cur = nodes.begin() ; cur != nodes.end() ; ++cur)		// Do some cleaning
			{
				zone(cur->x(), cur->z()) = 0;
				cur->x() >>= 1;
				cur->z() >>= 1;
			}

			path.clear();
			for( std::deque<AI::Path::Node>::iterator cur = nodes.begin() ; cur != nodes.end() ; ++cur)
			{
				if (path.empty())
				{
					path.push_back(*cur);
					continue;
				}
				if (cur->x() != path.back().x() || cur->z() != path.back().z())						// Remove duplicates
				{
					std::deque<AI::Path::Node>::iterator next = cur;
					++next;
					if (next == nodes.end() ||
						(next->x() - path.back().x()) * (cur->z() - path.back().z()) != (cur->x() - path.back().x()) * (next->z() - path.back().z()))	// Remove useless points
						path.push_back(*cur);
				}
			}
//			make_path_direct( map_data, map, dh_max, low_level, high_level, path, mw, mh, bloc_w, bloc_h, u_idx, hover_h );		// Make the path easier to follow and shorter

			path.next();   // The unit is already at Start!! So remove it
		}
	}

	bool Pathfinder::checkRectFull(int x1, int y1, short c, UnitType *pType)
	{
		float dh_max = pType->MaxSlope * H_DIV;
		float h_min = pType->canhover ? -100.0f : the_map->sealvl - pType->MaxWaterDepth * H_DIV;
		float h_max = the_map->sealvl - pType->MinWaterDepth * H_DIV;
		float hover_h = pType->canhover ? the_map->sealvl : -100.0f;
		int fy = y1 + pType->FootprintZ;
		int fx = x1 + pType->FootprintX;
		int x, y;
		y = y1;
		if (y >= 0 && y < the_map->bloc_h)
			for(x = x1 ; x < fx ; ++x)
				if (x >= 0 && x < the_map->bloc_w)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
		y = fy - 1;
		if (y >= 0 && y < the_map->bloc_h)
			for(x = x1 ; x < fx ; ++x)
				if (x >= 0 && x < the_map->bloc_w)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
		for(int y = y1 + 1 ; y < fy - 1 ; ++y)
			if (y >= 0 && y < the_map->bloc_h)
			{
				x = x1;
				if (x >= 0 && x < the_map->bloc_w)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
				x = fx - 1;
				if (x >= 0 && x < the_map->bloc_w)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
			}
		return true;
	}

	//---------- Old code ----------

    inline int path_len(const PATH &path)
    {
		return int(path.size());
    }

    void simplify_path(PATH &path)
    {
        for( PATH::iterator i = path.begin() ; i != path.end() ; )
        {
            PATH::iterator next = i;
            ++next;
            if (next == path.end()) break;
            PATH::iterator next2 = next;
            ++next2;
            if (next2 == path.end()) break;
            if( (next2->x - i->x)*(next->y - i->y) == (next2->y - i->y)*(next->x - i->x) )
                path.erase(i++);
            else
                ++i;
        }
    }


    PATH direct_path(Vector3D End)		// Creates a direct path to the target / Chemin direct vers la cible
    {
        PATH path;
        path.push_back( PATH_NODE(0, 0, End) );
        return path;
    }

    void make_path_direct(SECTOR **map_data, float **h_map, float dh_max, float h_min, float h_max, PATH &path, int mw, int mh, int bw, int bh, int u_idx, float hover_h)			// Elimine les étapes inutiles (qui rendent un chemin indirect)
    {
        if(path.size() < 2)	return;			// Let's say it's already direct

		PATH::iterator nodes[ PATHFINDER_MAX_LENGTH + 100 ];			// This array is used to compute a direct path, we need it because it decreases the computation time

		int length = 0;
        for(PATH::iterator i = path.begin() ; i != path.end() ; ++i)
            nodes[ length++ ] = i;

        int s = 0;
        int e = length - 1;

        while( s < length - 1 )
        {
            if(is_direct( map_data, h_map, dh_max, h_min, h_max, *(nodes[ s ]), *(nodes[ e ]), mw, mh, bw, bh, u_idx, hover_h )) // Si le chemin est direct, on élimine le point du milieu
            {
                if( s + 1 < e ) // Destroy now useless nodes
                    path.erase(nodes[s+1], nodes[e-1]);
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
						|| ( the_map->slope(x,y) > dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
        y=fy-1;
        if(y>=0 && y<bloc_h)
            for(x=x1;x<fx;x++)
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
						|| ( the_map->slope(x,y) > dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
        for(int y=y1+1;y<fy-1;y++)
            if(y>=0 && y<bloc_h) {
                x=x1;
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
						|| ( the_map->slope(x,y) > dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
                        || h_map[y][x]<h_min || h_map[y][x]>h_max )
                        return false;
                x=fx-1;
                if(x>=0 && x<bloc_w)
                    if( (map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
						|| ( the_map->slope(x,y) > dh_max && h_map[y][x] > hover_h ) || map_data[y][x].lava
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
			float coef = float(By - Ay) / float(Bx - Ax);

			for (int x = Ax ; x <= Bx ; ++x)
			{
				int y = Ay + int(float(x - Ax) * coef + 0.5f);
				if (!check_rect_full( map_data, h_map, x-dmw, y-dmh, mw, mh, u_idx, dh_max, h_min, h_max, bw, bh, hover_h ))	// Check for obstacles
                    return false;
            }
        }
		else
		{
			if (Ay > By)
			{
                Ax ^= Bx;	Bx ^= Ax;	Ax ^= Bx;
                Ay ^= By;	By ^= Ay;	Ay ^= By;
            }
			float coef = float(Bx - Ax) / float(By - Ay);
			for (int y = Ay ; y <= By ; ++y)
			{
				int x = Ax + (int)(float(y - Ay) * coef + 0.5f);
				if (!check_rect_full( map_data, h_map, x-dmw, y-dmh, mw, mh, u_idx, dh_max, h_min, h_max, bw, bh, hover_h ))	// Check for obstacles
                    return false;
            }
        }
        return true;
    }

    void next_node( PATH &path,						// Passe au noeud suivant
                    SECTOR **map_data, float **map, int bloc_w, int bloc_h, float dh_max, float low_level, float high_level, int mw, int mh, int u_idx, float hover_h )
    {
        if (!path.empty())
        {
            path.pop_front();
            if ( !path.empty() && !path.front().made_direct )
                make_path_direct( map_data, map, dh_max, low_level, high_level, path, mw, mh, bloc_w, bloc_h, u_idx, hover_h );		// Make the path easier to follow and shorter
        }
    }

	void compute_coord(PATH &path, int map_w, int map_h)
    {
        int h_w = map_w >> 1;
        int h_h = map_h >> 1;
        for(PATH::iterator i = path.begin() ; i != path.end() ; ++i)
        {
			i->Pos.x = float((i->x << 3) + 4 - h_w);
			i->Pos.z = float((i->y << 3) + 4 - h_h);
            i->Pos.y = 0.0f;
        }
    }

	PATH find_path( SECTOR **map_data, float **map, Grid<byte> &zone, int map_w, int map_h, int bloc_w, int bloc_h, float dh_max, float low_level, float high_level, Vector3D Start, Vector3D End, int mw, int mh, int u_idx, int m_dist, float hover_h )
    {
        int start_x = ((int)Start.x + (map_w >> 1) + 4) >> 3;
        int start_y = ((int)Start.z + (map_h >> 1) + 4) >> 3;
        int end_x = ((int)End.x + (map_w >> 1) + 4) >> 3;
        int end_y = ((int)End.z + (map_h >> 1) + 4) >> 3;

        int START_X = start_x << 1;
        int START_Y = start_y << 1;
        int END_X = end_x << 1;
        int END_Y = end_y << 1;

        PATH path;
        PATH_NODE start(START_X, START_Y);
        path.push_back(start);
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

        if (path.back().x < 0 || path.back().y < 0 || path.back().x >= bloc_w_db || path.back().y >= bloc_h_db)		// Hum we are out !!
        {
            path.clear();
            return PATH();			// So we can't find a path
        }

        the_map->lock();
        while (n < PATH_MAX_LENGTH && ((m_dist == 0 && (path.back().x != END_X || path.back().y != END_Y)) || (m_dist > 0 && sq( path.back().x - END_X ) + sq( path.back().y - END_Y) > m_dist)))
        {
			zone( path.back().x, path.back().y )++;

            int m = -1;

            int nx = path.back().x;
            int ny = path.back().y;
            if (abs( END_X - path.back().x ) > abs( END_Y - path.back().y ))
                nx += sgn( END_X - path.back().x );
            else
                ny += sgn( END_Y - path.back().y );

            if (nx < 0 || ny < 0 || nx >= bloc_w_db || ny >= bloc_h_db)
                break;		// If we have to go out there is a problem ...

			if( zone(nx, ny) >= 3 )	break;		// Looping ...

            int NX = nx >> 1, NY = ny >> 1;

			if (zone(nx, ny) || !check_rect_full( map_data, map, NX - mw_h, NY - mh_h, smw, smh, u_idx, dh_max, low_level, high_level, bloc_w, bloc_h, hover_h ))
            {
                int dist[ 8 ];
                int rdist[ 8 ];
                bool zoned[ 8 ];
                for( int e = 0 ; e < 8 ; e++ )			// Gather required data
                {
                    rdist[ e ] = dist[ e ] = -1;
                    nx = path.back().x + order_dx[ e ];
                    ny = path.back().y + order_dy[ e ];
                    NX = nx >> 1;
                    NY = ny >> 1;
                    zoned[ e ] = false;
                    if (nx < 0 || ny < 0 || nx >= bloc_w_db || ny >= bloc_h_db)	continue;
					zoned[ e ] = zone(nx, ny);
					if (((path.back().x >> 1) != NX || (path.back().y >> 1) != NY) && !zone(nx,ny))				// No need to do it twice
                        if (!check_rect_full( map_data, map, NX - mw_h, NY - mh_h, smw, smh, u_idx, dh_max, low_level, high_level, bloc_w, bloc_h, hover_h ))
                            continue;
                    rdist[ e ] = dist[ e ] = sq( END_X - nx ) + sq( END_Y - ny );

                    if (zoned[ e ])
                        dist[ e ] = -1;
                }
                for ( int e = 1 ; e < 8 ; e += 2 )		// Look for a way to go
                {
                    if (( (dist[ order_m1[ e ] ] == -1 && !zoned[ order_m1[ e ] ]) || (dist[ order_p1[ e ] ] == -1 && !zoned[ order_p1[ e ] ])
                          || (dist[ order_m2[ e ] ] == -1 && !zoned[ order_m2[ e ] ]) || (dist[ order_p2[ e ] ] == -1 && !zoned[ order_p2[ e ] ]) ) && dist[ e ] >= 0)
                    {
                        if (m == -1)	m = e;
                        else if (dist[ e ] < dist[ m ])
                            m = e;
                    }
                }
                if (m == -1)							// Second try
                {
                    for( int e = 1 ; e < 8 ; e += 2 )
                    {
                        if (dist[ e ] >= 0)
                        {
                            if (m == -1)	m = e;
                            else if (dist[ e ] < dist[ m ])
                                m = e;
                        }
                    }
                    if (m == -1)						// Ok we already went everywhere, then compute data differently
                    {
                        for( int e = 1 ; e < 8 ; e+= 2 )
                        {
                            if (rdist[ e ] == -1)	continue;
                            nx = path.back().x + order_dx[ e ];
                            ny = path.back().y + order_dy[ e ];
							dist[ e ] = rdist[ e ] + 1000 * zone(nx, ny);
                        }
                        for( int e = 1 ; e < 8 ; e += 2 )		// Ultimate test
                            if (dist[ e ] >= 0)
                            {
                                if (m == -1)	m = e;
                                else if (dist[ e ] < dist[ m ])
                                    m = e;
                            }
                    }
                }
                if (m >= 0)			// We found something
                {
                    nx = path.back().x + order_dx[ m ];
                    ny = path.back().y + order_dy[ m ];
                }
            }
            else
                m = -2;

            if (m == -1)
                break;

            path.push_back( PATH_NODE(nx, ny) );

            n++;
        }

        if (!path.empty())
        {
            for( PATH::iterator cur = path.begin() ; cur != path.end() ; ++cur)		// Do some cleaning
            {
				zone(cur->x, cur->y) = 0;
                cur->x >>= 1;
                cur->y >>= 1;
            }

            for( PATH::iterator cur = path.begin() ; cur != path.end() ; )				// Remove duplicated points
            {
                PATH::iterator next = cur;
                ++next;
                if (next == path.end())
                    break;
                if (cur->x == next->x && cur->y == next->y)
                    path.erase(cur++);
                else
                    ++cur;
            }
            simplify_path( path );												// Remove useless points
            make_path_direct( map_data, map, dh_max, low_level, high_level, path, mw, mh, bloc_w, bloc_h, u_idx, hover_h );		// Make the path easier to follow and shorter

			compute_coord(path, map_w, map_h);

            path.pop_front();   // The unit is already at Start!! So remove it
        }
        the_map->unlock();

        return path;
    }

    float path_length(const PATH &path)
    {
        if (!path.empty())
        {
            float length = 0.0f;
            for(PATH::const_iterator i = path.begin() ; i != path.end() ; ++i)
            {
                PATH::const_iterator next = i;
                ++next;
                if (next == path.end())
                    break;
                length += sqrtf((float)(sq(i->x - next->x) + sq(i->y - next->y)));
            }
            return length;
        }
        return 0.0f;
    }


} // namespace TA3D

