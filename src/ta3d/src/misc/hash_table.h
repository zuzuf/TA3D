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
# include <zuzuf/hashmap.h>
# include <zuzuf/hashset.h>

namespace TA3D
{
	namespace UTILS
	{
		template <class T>
				class hash
		{
		public:
			inline hash()	{}

			inline size_t operator()(const T& key) const
			{
				return size_t(key);
			}
		};

		template < >
				class hash<QString>
		{
		public:
			inline hash()	{}

			inline size_t operator()(const QString& key) const
			{
                if (key.isEmpty())
					return 0U;

				// implementation of MurmurHash2 by Austin Appleby

				// 'm' and 'r' are mixing constants generated offline.
				// They're not really 'magic', they just happen to work well.

				const uint32 m = 0x5bd1e995;
				const int r = 24;

				// Initialize the hash to a 'random' value
                int len = int(key.size() * sizeof(*key.data()));
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

		template<typename T, typename K = QString>
				class HashMap
		{
		public:
			typedef zuzuf::hashmap<K, T, TA3D::UTILS::hash<K> >			Dense;
			typedef zuzuf::hashmap<K, T, TA3D::UTILS::hash<K>, 250 >	Sparse;
		};

		template<typename K = QString>
				class HashSet
		{
		public:
			typedef zuzuf::hashset<K, TA3D::UTILS::hash<K> >		Dense;
			typedef zuzuf::hashset<K, TA3D::UTILS::hash<K>, 250 >	Sparse;
		};

		template<class T, class U>
				uint32
				wildCardSearch( const T& container, const QString& pattern, U& li)
		{
			if (container.empty())
				return 0;
			QString first;
			QString last;
            QString::size_type iFind = pattern.indexOf('*');
            if (iFind != -1)
			{
				first = Substr(pattern, 0, iFind);
				first.toLower();
				++iFind;
				last = Substr(pattern, iFind);
				last.toLower();
			}
			else
			{
				first = pattern;
				first.toLower();
				last.clear();
			}

			uint32 nb(0);
			QString::size_type firstLen = first.length();
			QString::size_type lastLen = last.length();
			for (typename T::const_iterator cur = container.begin() ; cur != container.end() ; ++cur)
			{
				const QString f = cur.key();
				QString::size_type fLen = f.length();
				if (fLen < firstLen || fLen < lastLen)
					continue;

				if (Substr(f, 0, firstLen) == first)
				{
					if (Substr(f, fLen - lastLen, lastLen) == last)
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
