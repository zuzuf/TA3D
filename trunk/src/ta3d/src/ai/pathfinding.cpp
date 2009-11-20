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
		if (task.idx >= 0)
		{
			units.unit[task.idx].lock();
			if (units.unit[task.idx].ID != task.UID && units.unit[task.idx].type_id >= 0)
			{
				units.unit[task.idx].unlock();
				return;
			}
		}
		UnitType *pType = task.idx >= 0
						  ? unit_manager.unit_type[units.unit[task.idx].type_id]
						  : unit_manager.unit_type[-task.idx];
		if (task.idx >= 0)
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

		int PATH_MAX_LENGTH = Math::Max(256, Math::Min(int(sqrtf(float(sq(end_x - start_x) + sq(end_z - start_z))) * 5.0f), PATHFINDER_MAX_LENGTH));

		int m_dist = task.dist;
		m_dist *= m_dist;
		m_dist <<= 2;

		int smh = pType->FootprintZ;
		int smw = pType->FootprintX;

		int mw_h = smw >> 1;
		int mh_h = smh >> 1;

		int bloc_w_db = the_map->bloc_w_db << 1;
		int bloc_h_db = the_map->bloc_h_db << 1;

		if (nodes.back().x() < 0 || nodes.back().z() < 0 || nodes.back().x() >= bloc_w_db || nodes.back().z() >= bloc_h_db)		// Hum we are out !!
		{
			path.clear();
			return;			// So we can't find a path
		}

		Grid<byte> &zone = the_map->path;
		Grid<float> &energy = the_map->energy;

		while (n < PATH_MAX_LENGTH && ((m_dist == 0 && (nodes.back().x() != END_X || nodes.back().z() != END_Z)) || (m_dist > 0 && sq( nodes.back().x() - END_X ) + sq( nodes.back().z() - END_Z) > m_dist)))
		{
			++zone( nodes.back().x(), nodes.back().z() );

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
				float dist[ 8 ];
				float rdist[ 8 ];
				bool zoned[ 8 ];
				for( int e = 0 ; e < 8 ; ++e )			// Gather required data
				{
					rdist[ e ] = dist[ e ] = -1.0f;
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
					rdist[ e ] = dist[ e ] = float(pType->MaxSlope) * sqrtf(float(sq( END_X - nx ) + sq( END_Z - nz ))) + energy(nx, nz);

					if (zoned[ e ])
						dist[ e ] = -1.0f;
				}
				for (int e = 0 ; e < 8 ; ++e)		// Look for a way to go
				{
					if (( (dist[ order_m1[ e ] ] < 0.0f && !zoned[ order_m1[ e ] ]) || (dist[ order_p1[ e ] ] < 0.0f && !zoned[ order_p1[ e ] ])
						  || (dist[ order_m2[ e ] ] < 0.0f && !zoned[ order_m2[ e ] ]) || (dist[ order_p2[ e ] ] < 0.0f && !zoned[ order_p2[ e ] ]) ) && dist[ e ] >= 0.0f)
					{
						if (m == -1)	m = e;
						else if (dist[ e ] < dist[ m ])
							m = e;
					}
				}
				if (m == -1)							// Second try
				{
					for (int e = 0 ; e < 8 ; ++e)
					{
						if (dist[ e ] >= 0.0f)
						{
							if (m == -1)	m = e;
							else if (dist[ e ] < dist[ m ])
								m = e;
						}
					}
					if (m == -1)						// Ok we already went everywhere, then compute data differently
					{
						for (int e = 0 ; e < 8 ; ++e)
						{
							if (rdist[ e ] < 0.0f)	continue;
							nx = nodes.back().x() + order_dx[ e ];
							nz = nodes.back().z() + order_dz[ e ];
							dist[ e ] = rdist[ e ] + float(1000 * zone(nx, nz));
						}
						for (int e = 0 ; e < 8 ; ++e)		// Ultimate test
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

	bool Pathfinder::checkRectFull(int x1, int y1, int c, UnitType *pType)
	{
		float dh_max = float(pType->MaxSlope) * H_DIV;
		float h_min = pType->canhover ? -100.0f : the_map->sealvl - float(pType->MaxWaterDepth) * H_DIV;
		float h_max = the_map->sealvl - float(pType->MinWaterDepth) * H_DIV;
		float hover_h = pType->canhover ? the_map->sealvl : -100.0f;
		int fy = y1 + pType->FootprintZ;
		int fx = x1 + pType->FootprintX;
		int x, y;
		y = y1;
		if (y >= 0 && y < the_map->bloc_h_db)
			for(x = x1 ; x < fx ; ++x)
				if (x >= 0 && x < the_map->bloc_w_db)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
		y = fy - 1;
		if (y >= 0 && y < the_map->bloc_h_db)
			for(x = x1 ; x < fx ; ++x)
				if (x >= 0 && x < the_map->bloc_w_db)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
		for(int y = y1 + 1 ; y < fy - 1 ; ++y)
			if (y >= 0 && y < the_map->bloc_h_db)
			{
				x = x1;
				if (x >= 0 && x < the_map->bloc_w_db)
					if ((the_map->map_data[y][x].unit_idx != c
						 && the_map->map_data[y][x].unit_idx != -1)
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
				x = fx - 1;
				if (x >= 0 && x < the_map->bloc_w_db)
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
} // namespace TA3D

