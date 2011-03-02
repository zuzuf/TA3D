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

#ifndef __TA3D_BITMAP_H__
# define __TA3D_BITMAP_H__

#include <vector>

namespace TA3D
{
	//! This class implements a bitmap stored in a std::vector<bool> (which is specialized for efficient storage)
	class BitMap
	{
	public:
		BitMap(size_t w, size_t h) : w(w), h(h)
		{
			data.resize(w * h);
		}

		inline bool operator()(size_t x, size_t y) const
		{
			return data[x + y * w];
		}

		inline void set(size_t x, size_t y, bool b)
		{
			data[x + y * w] = b;
		}

		size_t getMemoryUse() const
		{
			return sizeof(*this) + (data.size() >> 3);
		}

	private:
		const size_t w, h;
		std::vector<bool> data;
	};
}

#endif
