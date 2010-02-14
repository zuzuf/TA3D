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

#ifndef __TA3D_QUADMAP_H__
# define __TA3D_QUADMAP_H__

#include <bitset>

namespace TA3D
{
	//! This class implements a bitmap compressed using a kind of quadtree
	class QuadMap
	{
	private:
		class Node
		{
		public:
			virtual ~Node();
			virtual void set(int x, int y, bool b) = 0;
			virtual bool get(int x, int y) const = 0;
			virtual bool compressible() const = 0;
			virtual bool value() const = 0;
			virtual int getMemoryUse() const = 0;
		};

		class InternalNode : public Node
		{
		public:
			InternalNode(int s, bool b = false);
			virtual ~InternalNode();
			virtual void set(int x, int y, bool b);
			virtual bool get(int x, int y) const;
			virtual bool compressible() const;
			virtual bool value() const;
			virtual int getMemoryUse() const;
		private:
			const int s, hs;
			std::bitset<4> data;
			Node *childs[4];
		};

		class LeafNode : public Node
		{
		public:
			LeafNode(bool b = false);
			virtual ~LeafNode();
			virtual void set(int x, int y, bool b);
			virtual bool get(int x, int y) const;
			virtual bool compressible() const;
			virtual bool value() const;
			virtual int getMemoryUse() const;
		private:
			std::bitset<256> data;
		};

	public:
		QuadMap(int w, int h);

		bool operator()(int x, int y) const
		{
			return root.get(x, y);
		}

		void set(int x, int y, bool b)
		{
			root.set(x, y, b);
		}

		int getMemoryUse()
		{
			return root.getMemoryUse();
		}

	private:
		InternalNode root;
	};
}

#endif
