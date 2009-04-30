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

#ifndef __STACK_H__
#define __STACK_H__

#include <deque>

namespace TA3D
{
    template< typename T >
    class Stack
    {
        private:
            std::deque<T> container;
        public:

            inline Stack() : container()   {}

            inline ~Stack()    {   container.clear();  }

            inline void clear() {   container.clear();  }

            inline T& top()
            {
                return container.back();
            }

            inline size_t size()
            {
                return container.size();
            }

            inline T& operator[](int idx)
            {
                return container[idx];
            }

            inline T pop()
            {
                if (empty())    return T();
                T t = container.back();
                container.pop_back();
                return t;
            }

            inline bool empty() {   return container.empty();   }

            inline void push(const T &t)
            {
                container.push_back(t);
            }
    };
}
#endif
