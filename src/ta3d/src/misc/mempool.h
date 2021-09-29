/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2011  Roland BROCHARD

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

#ifndef __TA3D_MISC_MEMPOOL_H__
#define __TA3D_MISC_MEMPOOL_H__

#include <cstddef>
#include <cstdlib>
#include <stdexcept>

namespace TA3D
{
	template<class T>
	class MemoryPool
	{
	public:
		MemoryPool(const size_t capacity) : capacity(capacity)
		{
			buffer = (T*)malloc(sizeof(T) * capacity);
			unused = (T**)malloc(sizeof(T*) * capacity);
			reset();
		}

		~MemoryPool()
		{
			free(unused);
			free(buffer);
		}

		void reset()
		{
			for(size_t i = 0U ; i < capacity ; ++i)
				unused[i] = buffer + i;
			used = 0;
		}

		T *alloc()
		{
			if (used == capacity)		// No more pre-allocated slot
				throw std::bad_alloc();
			T *p = unused[used];
			++used;
			new (p) T;
			return p;
		}

		void release(T *p)
		{
			if (used == 0U)
				throw std::invalid_argument(std::string("MemoryPool::release has no slot in use"));
			operator delete (p, nullptr);
			unused[--used] = p;
		}

	private:
		const size_t capacity;
		size_t used;
		T **unused;
		T *buffer;
	};
}

#endif
