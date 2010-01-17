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

#include <yuni/yuni.h>
#include <yuni/core/system/cpu.h>
#include <stdafx.h>
#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <scripts/cob.h>					// For unit scripts / Pour la lecture et l'éxecution des scripts
#include <tdf.h>					// For map features / Pour la gestion des éléments du jeu
#include <EngineClass.h>			// The engine, also includes pathfinding.h / Inclus le moteur(dont le fichier pathfinding.h)
#include <misc/math.h>
#include <UnitEngine.h>
#include <yuni/thread/thread.h>

#define PATHFINDER_MAX_LENGTH			500000


namespace TA3D
{
	inline int sq(int a)
	{
		return a*a;
	}

	Mutex Pathfinder::sMutex;

	Pathfinder *Pathfinder::instance()
	{
		static Pathfinder sInstance;
		return &sInstance;
	}

	namespace AI
	{
		void Path::next()
		{
			if (nodes.empty())
				return;

			nodes.pop_front();
			computeCoord();
		}

		void Path::clear()
		{
			nodes.clear();
			pos.reset();
			_ready = false;
		}

		void Path::computeCoord()
		{
			if (nodes.empty())
				return;

			pos.x = float((nodes.front().x() << 3) + 4 - the_map->map_w_d);
			pos.z = float((nodes.front().z() << 3) + 4 - the_map->map_h_d);
			pos.y = 0.0f;
		}

#define SAVE( i )	gzwrite( file, (void*)&(i), sizeof( i ) )
#define LOAD( i )	gzread( file, (void*)&(i), sizeof( i ) )

		void Path::save(gzFile file)
		{
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

	Pathfinder::Pathfinder() : tasks(), stasks(), taskOffset(0)
	{
		stasks.set_empty_key(-1);
		stasks.set_deleted_key(-2);
		nbCores = Yuni::System::CPU::Count();
	}

	int Pathfinder::taskCount()
	{
		MutexLocker mLock(pMutex);
		return tasks.size();
	}

	void Pathfinder::clear()
	{
		MutexLocker mLock(pMutex);
		tasks.clear();
		stasks.clear();
		taskOffset = 0;
	}

	void Pathfinder::addTask(int idx, int dist, const Vector3D &start, const Vector3D &end)
	{
		Task t = { dist, idx, units.unit[idx].ID, start, end };

		lock();

		TaskSet::iterator pos = stasks.find(t.UID);
		bool found = (pos != stasks.end());

		if (found)		// If it's in the queue, then change the request
			tasks[pos->second - taskOffset] = t;
		// Otherwise add a new request to the task list
		else
		{
			stasks[t.UID] = tasks.size() + taskOffset;
			tasks.push_back(t);
			LOG_DEBUG(LOG_PREFIX_PATHS << int(tasks.size()) << " path requests queued");
		}

		if (!isRunning())
			this->start();
		unlock();
	}

	AI::Path Pathfinder::directPath(const Vector3D &end)
	{
		int x = ((int)end.x + the_map->map_w_d) >> 3;
		int z = ((int)end.z + the_map->map_h_d) >> 3;
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
		while (!tasks.empty() && !pDead)
		{
			Task cur = tasks.front();
			stasks.erase(cur.UID);
			tasks.pop_front();
			++taskOffset;
			unlock();

			// Here we are free to compute this path
			uint32 start_timer = msec_timer;
			if (cur.idx >= 0
				&& units.unit[cur.idx].ID == cur.UID
				&& (units.unit[cur.idx].flags & 1))
			{
				AI::Path path;
				findPath(path, cur);
				path._ready = true;

				Unit *pUnit = &(units.unit[cur.idx]);
				pUnit->lock();
				if (pUnit->ID == cur.UID
					&& (pUnit->flags & 1)
					&& !pUnit->mission.empty()
					&& pUnit->requesting_pathfinder
					&& (pUnit->mission->getFlags() & MISSION_FLAG_MOVE))
				{
					pUnit->mission->Path().replaceWith(path);
					pUnit->requesting_pathfinder = false;
				}
				pUnit->unlock();
			}
	
			// We don't want to use more than 25% of the CPU here
			if (suspend((nbCores == 1) ? ((msec_timer - start_timer) << 2) : 0))
				// The thread should stop as soon as possible
				return;

			lock();
		}
		unlock();
	}

	void Pathfinder::findPath( AI::Path &path, const Task &task )
	{
		if (task.idx >= 0)
		{
			units.unit[task.idx].lock();
			if (units.unit[task.idx].ID != task.UID || units.unit[task.idx].type_id < 0)
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

		int start_x = ((int)task.start.x + the_map->map_w_d) >> 3;
		int start_z = ((int)task.start.z + the_map->map_h_d) >> 3;
		int end_x = ((int)task.end.x + the_map->map_w_d) >> 3;
		int end_z = ((int)task.end.z + the_map->map_h_d) >> 3;

		std::vector<AI::Path::Node> nodes;
		nodes.push_back(AI::Path::Node(start_x, start_z));
		int n = 0;

		static int order_p1[] = { 1, 2, 3, 4, 5, 6, 7, 0 };
		static int order_p2[] = { 2, 3, 4, 5, 6, 7, 0, 1 };
		static int order_m1[] = { 7, 0, 1, 2, 3, 4, 5, 6 };
		static int order_m2[] = { 6, 7, 0, 1, 2, 3, 4, 5 };
		static int order_dx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
		static int order_dz[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
		static int order_d[] = { 10, 7, 10, 7, 10, 7, 10, 7 };

		int m_dist = task.dist;
		m_dist *= m_dist;

		int smh = pType->FootprintZ;
		int smw = pType->FootprintX;

		int mw_h = smw >> 1;
		int mh_h = smh >> 1;

		if (nodes.back().x() < 0
			|| nodes.back().z() < 0
			|| nodes.back().x() >= the_map->bloc_w_db
			|| nodes.back().z() >= the_map->bloc_h_db)		// Hum we are out !!
		{
			path.clear();
			return;			// So we can't find a path
		}

		Grid<int> &zone = the_map->path;
		Grid<float> &energy = the_map->energy;
		std::deque<AI::Path::Node> qNode;
		std::deque<int> qDistFromStart;

		bool pathFound = true;
		if ((m_dist == 0 && (nodes.back().x() != end_x || nodes.back().z() != end_z)) || (m_dist > 0 && sq( nodes.back().x() - end_x ) + sq( nodes.back().z() - end_z) > m_dist))
		{
			nodes.reserve(PATHFINDER_MAX_LENGTH);
			pathFound = false;
			uint32 curDistFromStart = 0;
			uint32 minPathLength = uint32(-1);
			int nbChoices = 0;
			uint32 depthLimit = PATHFINDER_MAX_LENGTH;
			while (n < depthLimit)
			{
				++zone( nodes.back().x(), nodes.back().z() );

				int m = -1;

				int nx = nodes.back().x() + Math::Sgn( end_x - nodes.back().x() );
				int nz = nodes.back().z() + Math::Sgn( end_z - nodes.back().z() );

				if (nx < 0 || nz < 0 || nx >= the_map->bloc_w_db || nz >= the_map->bloc_h_db)
					break;		// If we have to go out there is a problem ...

				if (zone(nx, nz) >= 2 || curDistFromStart > minPathLength)
				{
					if (qNode.empty() || (!pathFound && nbChoices == 0))		// We're done
						break;
					if (nbChoices > 0)
						--nbChoices;
					nodes.push_back(qNode.back());
					qNode.pop_back();
					curDistFromStart = qDistFromStart.back();
					qDistFromStart.pop_back();
					++n;
					continue;		// Instead of looping we restart from a Node in the qNode
				}

				if (zone( nx, nz ) || !checkRectFull( nx - mw_h, nz - mh_h, task.idx, pType ))
				{
					float dist[ 8 ];
					float rdist[ 8 ];
					bool zoned[ 8 ];
					for( int e = 0 ; e < 8 ; ++e )			// Gather required data
					{
						rdist[ e ] = dist[ e ] = -1.0f;
						nx = nodes.back().x() + order_dx[ e ];
						nz = nodes.back().z() + order_dz[ e ];
						zoned[ e ] = false;
						if (nx < 0 || nz < 0 || nx >= the_map->bloc_w_db || nz >= the_map->bloc_h_db)
							continue;
						zoned[ e ] = zone(nx, nz);
						if (!zone(nx, nz) && !checkRectFull( nx - mw_h, nz - mh_h, task.idx, pType ))
								continue;
						rdist[ e ] = dist[ e ] = float(pType->MaxSlope) * sqrtf(float(sq( end_x - nx ) + sq( end_z - nz ))) + energy(nx, nz);

						if (zoned[ e ])
							dist[ e ] = -1.0f;
					}
					for (int e = 0 ; e < 8 ; ++e)		// Look for a way to go
					{
						if (((dist[ order_m1[ e ] ] < 0.0f && !zoned[ order_m1[ e ] ])
							  || (dist[ order_p1[ e ] ] < 0.0f && !zoned[ order_p1[ e ] ])
							  || (dist[ order_m2[ e ] ] < 0.0f && !zoned[ order_m2[ e ] ])
							  || (dist[ order_p2[ e ] ] < 0.0f && !zoned[ order_p2[ e ] ]))
							&& dist[ e ] >= 0.0f)
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
					}
					if (m >= 0)			// We found something
					{
						nx = nodes.back().x() + order_dx[ m ];
						nz = nodes.back().z() + order_dz[ m ];
						// add unexplored nodes to the queue
						for(int i = 0 ; i < 8 ; ++i)
						{
							if (i != m && dist[i] >= 0.0f)
							{
								if ((dist[ order_m1[ i ] ] < 0.0f && !zoned[ order_m1[ i ] ])
									 || (dist[ order_p1[ i ] ] < 0.0f && !zoned[ order_p1[ i ] ])
									 || (dist[ order_m2[ i ] ] < 0.0f && !zoned[ order_m2[ i ] ])
									 || (dist[ order_p2[ i ] ] < 0.0f && !zoned[ order_p2[ i ] ]))
								{
									qNode.push_back(AI::Path::Node(nodes.back().x() + order_dx[m], nodes.back().z() + order_dz[m]));		// Priority given to possibility to avoid obstacles
									qDistFromStart.push_back(curDistFromStart + order_d[ i ]);
									++nbChoices;
								}
								else
								{
									qNode.push_front(AI::Path::Node(nodes.back().x() + order_dx[m], nodes.back().z() + order_dz[m]));
									qDistFromStart.push_front(curDistFromStart + order_d[ i ]);
								}
							}
						}
						curDistFromStart += order_d[ m ];
					}
				}
				else
					m = -2;

				if (m == -1)
				{
					if (qNode.empty() || (!pathFound && nbChoices == 0))		// We're done
						break;
					if (nbChoices > 0)
						--nbChoices;
					nodes.push_back(qNode.back());
					qNode.pop_back();
					curDistFromStart = qDistFromStart.back();
					qDistFromStart.pop_back();
					++n;
					continue;		// Instead of looping we restart from a Node in the qNode
				}

				nodes.push_back( AI::Path::Node(nx, nz) );

				++n;
				if ((m_dist == 0 && nodes.back().x() == end_x && nodes.back().z() == end_z) || (m_dist > 0 && sq( nodes.back().x() - end_x ) + sq( nodes.back().z() - end_z) <= m_dist))
				{
					minPathLength = Math::Min(minPathLength, curDistFromStart);
					if (!pathFound)
						depthLimit = n << 1;			// Limit search complexity
					pathFound = true;
					if (qNode.empty())		// We're done
						break;
					nodes.push_back(qNode.back());
					qNode.pop_back();
					curDistFromStart = qDistFromStart.back();
					qDistFromStart.pop_back();
					++n;
					break;
				}
			}
		}

		if (!nodes.empty() && pathFound)
		{
			for (std::vector<AI::Path::Node>::iterator cur = nodes.begin() ; cur != nodes.end() ; ++cur)		// Mark the path with a special pattern
				zone(cur->x(), cur->z()) = 1;

			qNode.clear();
			if (m_dist == 0)
			{
				if (zone(end_x, end_z) == 1)
				{
					qNode.push_back(AI::Path::Node(end_x, end_z));
					zone(end_x, end_z) = 2;
				}
			}
			else
			{
				for(int z = -task.dist ; z <= task.dist ; ++z)
				{
					if (end_z + z < 0 || end_z + z >= the_map->bloc_h_db)
						continue;
					int dx = int(sqrtf(float(m_dist - z * z)) + 0.5f);
					for(int x = -dx ; x <= dx && end_x + x < the_map->bloc_w_db ; ++x)
						if (end_x + x >= 0 && zone(end_x + x, end_z + z) == 1)
						{
							qNode.push_back(AI::Path::Node(end_x + x, end_z + z));
							zone(end_x + x, end_z + z) = 2;
						}
				}
			}
			while(!qNode.empty())			// Fill the discovered region with the distance to end
			{
				AI::Path::Node cur = qNode.front();
				qNode.pop_front();

				int ref = zone(cur.x(), cur.z()) + 1;
				for(int i = 0 ; i < 8 ; ++i)
				{
					if (cur.x() + order_dx[i] < 0 || cur.x() + order_dx[i] >= the_map->bloc_w_db
						|| cur.z() + order_dz[i] < 0 || cur.z() + order_dz[i] >= the_map->bloc_h_db)
						continue;
					int t = zone(cur.x() + order_dx[i], cur.z() + order_dz[i]);
					if (t == 1 || t > ref)
					{
						zone(cur.x() + order_dx[i], cur.z() + order_dz[i]) = ref;
						qNode.push_back(AI::Path::Node(cur.x() + order_dx[i], cur.z() + order_dz[i]));
					}
				}
			}

			std::vector<AI::Path::Node> tmp;
			tmp.swap(nodes);
			nodes.reserve(tmp.size());
			nodes.push_back(AI::Path::Node(start_x, start_z));
			while ((m_dist == 0 && (nodes.back().x() != end_x || nodes.back().z() != end_z)) || (m_dist > 0 && sq( nodes.back().x() - end_x ) + sq( nodes.back().z() - end_z) > m_dist))	// Reconstruct the path
			{
				zone(nodes.back().x(), nodes.back().z()) = 0;
				AI::Path::Node next = nodes.back();

				int b = -1;
				int m = -1;
				for(int i = 0 ; i < 8 ; ++i)
				{
					if (next.x() + order_dx[i] < 0 || next.x() + order_dx[i] >= the_map->bloc_w_db
						|| next.z() + order_dz[i] < 0 || next.z() + order_dz[i] >= the_map->bloc_h_db)
						continue;
					int t = zone(next.x() + order_dx[i], next.z() + order_dz[i]);
					if (t > 0 && (t < m || m == -1))
					{
						b = i;
						m = t;
					}
				}

				if (b == -1)		// This should not be possible unless there is no path from start to end
				{
					nodes.clear();
					break;
				}
				next.x() += order_dx[b];
				next.z() += order_dz[b];

				nodes.push_back(next);
			}
			for (std::vector<AI::Path::Node>::iterator cur = tmp.begin() ; cur != tmp.end() ; ++cur)		// Do some cleaning
				zone(cur->x(), cur->z()) = 0;

			path.clear();
			for (std::vector<AI::Path::Node>::iterator cur = nodes.begin() ; cur != nodes.end() ; ++cur)
			{
				if (path.empty())
				{
					path.push_back(*cur);
					continue;
				}
				std::vector<AI::Path::Node>::iterator next = cur;
				++next;
				if (next == nodes.end() ||
					(next->x() - path.back().x()) * (cur->z() - path.back().z()) != (cur->x() - path.back().x()) * (next->z() - path.back().z()))	// Remove useless points
					path.push_back(*cur);
			}

			path.next();   // The unit is already at Start!! So remove it
		}
		else
		{
			for (std::vector<AI::Path::Node>::iterator cur = nodes.begin() ; cur != nodes.end() ; ++cur)		// Clean the map data
				zone(cur->x(), cur->z()) = 0;
			path.clear();
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
				{
					int idx = the_map->map_data[y][x].unit_idx;
					int type_id = idx >= 0 ? units.unit[idx].type_id : -1;
					UnitType *tType = type_id >= 0 ? unit_manager.unit_type[type_id] : NULL;
					if ((idx != c && idx != -1
						 && (type_id == -1 || !(tType->canmove && tType->BMcode)))
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
				}
		y = fy - 1;
		if (y >= 0 && y < the_map->bloc_h_db)
			for(x = x1 ; x < fx ; ++x)
				if (x >= 0 && x < the_map->bloc_w_db)
				{
					int idx = the_map->map_data[y][x].unit_idx;
					int type_id = idx >= 0 ? units.unit[idx].type_id : -1;
					UnitType *tType = type_id >= 0 ? unit_manager.unit_type[type_id] : NULL;
					if ((idx != c && idx != -1
						 && (type_id == -1 || !(tType->canmove && tType->BMcode)))
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
				}
		for(int y = y1 + 1 ; y < fy - 1 ; ++y)
			if (y >= 0 && y < the_map->bloc_h_db)
			{
				x = x1;
				if (x >= 0 && x < the_map->bloc_w_db)
				{
					int idx = the_map->map_data[y][x].unit_idx;
					int type_id = idx >= 0 ? units.unit[idx].type_id : -1;
					UnitType *tType = type_id >= 0 ? unit_manager.unit_type[type_id] : NULL;
					if ((idx != c && idx != -1
						 && (type_id == -1 || !(tType->canmove && tType->BMcode)))
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
				}
				x = fx - 1;
				if (x >= 0 && x < the_map->bloc_w_db)
				{
					int idx = the_map->map_data[y][x].unit_idx;
					int type_id = idx >= 0 ? units.unit[idx].type_id : -1;
					UnitType *tType = type_id >= 0 ? unit_manager.unit_type[type_id] : NULL;
					if ((idx != c && idx != -1
						 && (type_id == -1 || !(tType->canmove && tType->BMcode)))
						|| (the_map->slope(x,y) > dh_max
							&& the_map->h_map[y][x] > hover_h)
						|| the_map->map_data[y][x].lava
						|| the_map->h_map[y][x] < h_min
						|| the_map->h_map[y][x] > h_max)
						return false;
				}
			}
		return true;
	}
} // namespace TA3D

