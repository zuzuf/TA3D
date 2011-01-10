/*  zuzuf::hash*, a set of hash set/map containers
	Copyright (C) 2011  Roland BROCHARD

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef HASHFN_H
#define HASHFN_H

#include <string>

namespace zuzuf
{

	template<class K> struct hash
	{
		inline hash() {}
		inline size_t operator()(const K &v) const
		{
			size_t h(0U);
			const unsigned char *p = (const unsigned char*)(&v);
			for(const unsigned char *end = p + sizeof(K) ; p != end ; ++p)
				h = (h << 5) + h + *p;
			return h;
		}
	};

#define TRIVIAL_HASH(T)\
	template<> inline size_t hash<T>::operator()(const T &v) const	{	return static_cast<T>(v);	}

	TRIVIAL_HASH(bool)
	TRIVIAL_HASH(char)
	TRIVIAL_HASH(unsigned char)
	TRIVIAL_HASH(short)
	TRIVIAL_HASH(unsigned short)
	TRIVIAL_HASH(int)
	TRIVIAL_HASH(unsigned int)
	TRIVIAL_HASH(long)
	TRIVIAL_HASH(unsigned long)

#undef TRIVIAL_HASH

	#include "murmurhash2.h"

	template<> inline size_t hash<std::string>::operator()(const std::string &v) const
	{
		return MurmurHash2(v.data(), v.size());
	}
}

#endif // HASHFN_H
