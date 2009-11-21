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

# include <EngineClass.h>
# include <yuni/core/smartptr/smartptr.h>
# include <threads/thread.h>
# include <threads/mutex.h>
# include <zlib.h>

#define MAX_PATH_EXEC		1

namespace TA3D
{
	class UnitType;

	class Pathfinder;
	namespace AI
	{
		class Path : public ObjectSync
		{
			friend class TA3D::Pathfinder;

		public:
			typedef SmartPtr<Path>	Ptr;

			class Node
			{
			public:
				Node(int x, int z) : _x(x), _z(z)	{}
				int &x() {	return _x;	}
				int &z() {	return _z;	}
			private:
				int _x, _z;
			};

		public:
			Path();
			void next();
			bool empty();
			void clear();
			bool ready() const	{	return _ready;	}
			Node &front()	{	return nodes.front();	}
			Node &back()	{	return nodes.back();	}
			void push_back(const Node &n) {	nodes.push_back(n);	}
			void push_front(const Node &n) {	nodes.push_front(n);	}
			int length() const	{	return int(nodes.size());	}

			void setPos(const Vector3D &pos)	{	this->pos = pos;	}
			const Vector3D &Pos()	const	{	return pos;	}

			void save(gzFile file);
			void load(gzFile file);

			typedef std::deque<Node>::iterator iterator;
			iterator begin()	{	return nodes.begin();	}
			iterator end()	{	return nodes.end();	}
		private:
			void computeCoord();

		private:
			Vector3D pos;
			std::deque<Node> nodes;
			bool _ready;
		};
	}

	class Pathfinder : public Thread, public ObjectSync
	{
	public:
		struct Task
		{
			int dist;
			int idx;
			uint32 UID;
			Vector3D start;
			Vector3D end;
		};

	public:
		Pathfinder();
		void clear();
		void addTask(int idx, int dist, const Vector3D &start, const Vector3D &end);

	private:
		virtual ~Pathfinder();
		virtual void proc(void* param);

	private:
		typedef std::deque<Task> TaskList;
		TaskList tasks;
		int nbCores;

	private:
		static bool checkRectFull(int x1, int y1, int c, UnitType *pType);
		static Mutex sMutex;
	public:
		static Pathfinder *instance();
		static void findPath( AI::Path &path, const Task &task );
		static AI::Path directPath(const Vector3D &end);
	};
} // namespace TA3D

#endif // __TA3D_AI_PATH_FINDING_H__
