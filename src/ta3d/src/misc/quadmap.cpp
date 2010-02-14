#include <stdafx.h>
#include "quadmap.h"
#include "math.h"

namespace TA3D
{
	QuadMap::QuadMap(int w, int h)
	 : root(Math::Max(1 << (Math::Log2(w << 1) - 1),		// We want the smallest power of two square map bigger than w*h
			1 << (Math::Log2(h << 1) - 1)))
	{
	}

	QuadMap::LeafNode::LeafNode(bool b)
	{
		if (b)
			data.set();
		else
			data.reset();
	}

	bool QuadMap::LeafNode::compressible() const
	{
		return data.none() || data.all();
	}

	bool QuadMap::LeafNode::value() const
	{
		return data.all();
	}

	bool QuadMap::LeafNode::get(int x, int y) const
	{
		return data[x | (y << 2)];
	}

	void QuadMap::LeafNode::set(int x, int y, bool b)
	{
		data[x | (y << 2)] = b;
	}

	QuadMap::InternalNode::InternalNode(int s, bool b) : s(s), hs(1 << Math::Log2(s))
	{
		childs[0] = NULL;
		childs[1] = NULL;
		childs[2] = NULL;
		childs[3] = NULL;
		if (b)
			data.set();
		else
			data.reset();
	}

	QuadMap::InternalNode::~InternalNode()
	{
		for(int i = 0 ; i < 4 ; ++i)
			if (childs[i])
				delete childs[i];
	}

	bool QuadMap::InternalNode::compressible() const
	{
		return childs[0] == NULL && childs[1] == NULL && childs[2] == NULL && childs[3] == NULL && (data.all() || data.none());
	}

	bool QuadMap::InternalNode::value() const
	{
		return data.all();
	}

	bool QuadMap::InternalNode::get(int x, int y) const
	{
		int n(0);
		if (x >= hs)
		{
			n |= 1;
			x -= hs;
		}
		if (y >= hs)
		{
			n |= 2;
			y -= hs;
		}
		if (childs[n] == NULL)
			return data[n];
		return childs[n]->get(x, y);
	}

	void QuadMap::InternalNode::set(int x, int y, bool b)
	{
		int n(0);
		if (x >= hs)
		{
			n |= 1;
			x -= hs;
		}
		if (y >= hs)
		{
			n |= 2;
			y -= hs;
		}
		if (childs[n] == NULL)
		{
			if (data[n] == b)
				return;
			if (hs > 16)
				childs[n] = new InternalNode(hs, data[n]);
			else
				childs[n] = new LeafNode(data[n]);
		}
		childs[n]->set(x, y, b);
		if (childs[n]->compressible())
		{
			data[n] = childs[n]->value();
			delete childs[n];
			childs[n] = NULL;
		}
	}
}
