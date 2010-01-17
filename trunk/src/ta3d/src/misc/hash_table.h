/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2006  Roland BROCHARD

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

#ifndef __TA3D_XX_HASH_TABLE_H__
# define __TA3D_XX_HASH_TABLE_H__

# include <stdafx.h>
# include "string.h"
# include <google/dense_hash_map>
# include <google/sparse_hash_map>
# include <google/dense_hash_set>
# include <google/sparse_hash_set>

namespace TA3D
{
	namespace UTILS
	{
		template <class T>
				class hash
		{
		public:
			inline size_t operator()(const T& key) const
			{
				return size_t(key);
			}
		};

		template < >
				class hash<String>
		{
		public:
			inline size_t operator()(const String& key) const
			{
				// implementation of MurmurHash2 by Austin Appleby

				// 'm' and 'r' are mixing constants generated offline.
				// They're not really 'magic', they just happen to work well.

				const uint32 m = 0x5bd1e995;
				const int r = 24;

				// Initialize the hash to a 'random' value
				int len = int(key.size());
				size_t h = 216 ^ len;

				// Mix 4 bytes at a time into the hash

				const uint8 * data = (const uint8 *)key.data();

				while(len >= 4)
				{
					uint32 k = *(uint32 *)data;

					k *= m;
					k ^= k >> r;
					k *= m;

					h *= m;
					h ^= k;

					data += 4;
					len -= 4;
				}

				// Handle the last few bytes of the input array

				switch(len)
				{
				case 3: h ^= data[2] << 16;
				case 2: h ^= data[1] << 8;
				case 1: h ^= data[0];
					h *= m;
				};

				// Do a few final mixes of the hash to ensure the last few
				// bytes are well-incorporated.

				h ^= h >> 13;
				h *= m;
				h ^= h >> 15;

				return h;
			}
		};

		template<typename T, typename K = String>
				class HashMap
		{
		public:
			typedef google::dense_hash_map<K, T, TA3D::UTILS::hash<K> >	Dense;
			typedef google::sparse_hash_map<K, T, TA3D::UTILS::hash<K> >	Sparse;
		};

		template<typename K = String>
				class HashSet
		{
		public:
			typedef google::dense_hash_set<K, TA3D::UTILS::hash<K> >	Dense;
			typedef google::sparse_hash_set<K, TA3D::UTILS::hash<K> >	Sparse;
		};

		template<class T>
				uint32
				wildCardSearch( const T& container, const String& pattern, String::List& li)
		{
			if (container.empty())
				return 0;
			String first;
			String last;
			String::size_type iFind = pattern.find('*');
			if (iFind != String::npos)
			{
				first = pattern.substr(0, iFind);
				first.toLower();
				++iFind;
				last = pattern.substr(iFind);
				last.toLower();
			}
			else
			{
				first = pattern;
				first.toLower();
				last.clear();
			}

			uint32 nb(0);
			String::size_type firstLen = first.length();
			String::size_type lastLen = last.length();
			for (typename T::const_iterator cur = container.begin() ; cur != container.end() ; ++cur)
			{
				String f = cur->first;
				String::size_type fLen = f.length();
				if (fLen < firstLen || fLen < lastLen)
					continue;

				if (f.substr(0, firstLen) == first)
				{
					if (f.substr(fLen - lastLen, lastLen) == last)
					{
						li.push_back(f);
						++nb;
					}
				}
			}
			return nb;
		}
	}
}

#endif // __TA3D_XX_HASH_TABLE_H__
