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

#ifndef HASHMAP_H
#define HASHMAP_H

#include <utility>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "hashfn.h"

namespace zuzuf
{

	template<class K, class T, class HFn = hash<K>, int MaxLoad = 128>
	class hashmap
	{
		template<class HT, class KT, class TT>	class tpl_iterator;
		template<class HT, class KT, class TT> friend class hashmap::tpl_iterator;

	public:
		typedef K key_type;
		typedef T value_type;
		typedef T mapped_type;
		typedef std::pair<K, T> node_type;
		typedef tpl_iterator<hashmap const, const K, const T>	const_iterator;
		typedef tpl_iterator<hashmap, K, T>	iterator;
	private:
		template<class HT, class KT, class TT>
		class tpl_iterator
		{
			friend class hashmap;
			friend class tpl_iterator<hashmap, K, T>;
			friend class tpl_iterator<hashmap const, const K, const T>;
		public:
			tpl_iterator(HT *hmap, size_t idx) : hmap(hmap), idx(idx)	{}
			tpl_iterator(const tpl_iterator &it) : hmap(it.hmap), idx(it.idx)	{}

			tpl_iterator &operator=(const tpl_iterator &it)
			{	hmap = it.hmap;	idx = it.idx;	return *this;	}

			operator const_iterator() const
			{
				return const_iterator(hmap, idx);
			}

			bool operator==(const iterator &it) const		{	return idx == it.idx;	}
			bool operator==(const const_iterator &it) const	{	return idx == it.idx;	}
			bool operator!=(const iterator &it) const		{	return idx != it.idx;	}
			bool operator!=(const const_iterator &it) const	{	return idx != it.idx;	}
			bool operator<(const iterator &it) const		{	return idx < it.idx;	}
			bool operator<(const const_iterator &it) const	{	return idx < it.idx;	}
			bool operator>(const iterator &it) const		{	return idx > it.idx;	}
			bool operator>(const const_iterator &it) const	{	return idx > it.idx;	}

			const KT &key() const {	return hmap->keys[idx];	}

			TT &value() const {	return hmap->values[idx];	}

			TT &operator*() const {	return hmap->values[idx];	}

			TT *operator->() const {	return hmap->values + idx;	}

			void operator++()
			{
				do
				{
					++idx;
				} while(idx < hmap->_capacity && hmap->usemask[idx] != Used);
			}

			tpl_iterator operator++(int)
			{
				tpl_iterator old(*this);
				do
				{
					++idx;
				} while(idx < hmap->_capacity && hmap->usemask[idx] != Used);
				return old;
			}

			tpl_iterator operator+(int n) const
			{
				tpl_iterator ret(*this);
				for(; n > 0 ; --n)
					do
					{
						++ret.idx;
					} while(ret.idx < hmap->_capacity && hmap->usemask[ret.idx] != Used);
				return ret;
			}

			tpl_iterator operator-(int n) const
			{
				tpl_iterator ret(*this);
				for(; n > 0 && ret.idx > 0 ; --n)
					do
					{
						--ret.idx;
					} while(ret.idx > 0 && hmap->usemask[ret.idx] != Used);
				return ret;
			}

			void operator--()
			{
				if (idx > 0)
					do
					{
						--idx;
					} while(idx > 0 && hmap->usemask[idx] != Used);
			}

			tpl_iterator operator--(int)
			{
				tpl_iterator old(*this);
				if (idx > 0)
					do
					{
						--idx;
					} while(idx > 0 && hmap->usemask[idx] != Used);
				return old;
			}

		private:
			HT *hmap;
			size_t idx;
		};

	private:
		enum UseState
		{
			Empty = 0,
			Used = 1,
			Deleted = 2
		};

	public:
		inline hashmap() : values(NULL), keys(NULL), usemask(NULL), _capacity(0U), _size(0U), _mask(0U), _used(0U)	{}
		inline hashmap(const hashmap &v);
		inline ~hashmap();

		hashmap &operator=(const hashmap &v);

		inline void clear();

		bool empty() const {	return _size == 0U;	}

		size_t capacity() const {	return _capacity;	}
		size_t size() const {	return _size;	}

		size_t memory_usage() const
		{
			return _capacity * (sizeof(unsigned char) + sizeof(key_type) + sizeof(value_type)) + sizeof(hashmap);
		}

		iterator begin()
		{
			size_t p = 0U;
			while(p < _capacity && usemask[p] != Used)
				++p;
			return iterator(this, p);
		}
		const_iterator begin() const
		{
			size_t p = 0U;
			while(p < _capacity && usemask[p] != Used)
				++p;
			return const_iterator(this, p);
		}

		iterator end()	{	return iterator(this, _capacity);	}
		const_iterator end() const	{	return const_iterator(this, _capacity);	}

		inline void rehash(size_t size);
		inline void reserve(size_t size)	{	rehash(size);	}

		inline void insert(const node_type &v)	{	insert(v.first, v.second);	}
		inline void insert(const K &key, const T &value);

		inline const T *fast_find(const K &key) const;
		inline T *fast_find(const K &key);

		inline const_iterator find(const K &key) const;
		inline iterator find(const K &key);

		inline T &operator[](const K &key);

		inline void remove(const K &key);
		inline void remove(const iterator &it);
		inline void remove(const const_iterator &it);

		inline void erase(const iterator &it)	{	remove(it);	}
		inline void erase(const const_iterator &it)	{	remove(it);	}
		inline void erase(const K &key)	{	remove(key);	}

		inline bool contains(const K &key) const	{	return fast_find(key);	}
		inline int count(const K &key) const	{	return contains(key);	}
	private:
		value_type *values;
		key_type *keys;
		unsigned char *usemask;
		size_t _capacity;
		size_t _size;
		size_t _mask;
		size_t _used;
	};

	template<class K, class T, class HFn, int MaxLoad>
	hashmap<K,T,HFn,MaxLoad>::hashmap(const hashmap &v) : values(NULL), keys(NULL), usemask(NULL), _capacity(0U), _size(0U), _mask(0U), _used(0U)
	{
		rehash(v.size());
		for(const_iterator it = v.begin(), end = v.end() ; it != end ; ++it)
			insert(it.key(), it.value());
	}

	template<class K, class T, class HFn, int MaxLoad>
	hashmap<K,T,HFn,MaxLoad> &hashmap<K,T,HFn,MaxLoad>::operator=(const hashmap &v)
	{
		clear();
		rehash(v.size());
		for(const_iterator it = v.begin(), end = v.end() ; it != end ; ++it)
			insert(it.key(), it.value());
		return *this;
	}

	template<class K, class T, class HFn, int MaxLoad>
	hashmap<K,T,HFn,MaxLoad>::~hashmap()
	{
		clear();
	}

	template<class K, class T, class HFn, int MaxLoad>
	void hashmap<K,T,HFn,MaxLoad>::clear()
	{
		if (values)
		{
			if (_size > 0U)
				for(size_t i = 0U ; i < _capacity ; ++i)
					if (usemask[i] == Used)
					{
						values[i].value_type::~value_type();
						keys[i].key_type::~key_type();
					}
			free(values);
			free(keys);
			free(usemask);
		}
		values = NULL;
		keys = NULL;
		usemask = NULL;
		_size = 0U;
		_used = 0U;
		_capacity = 0U;
		_mask = 0U;
	}

	template<class K, class T, class HFn, int MaxLoad>
	void hashmap<K,T,HFn,MaxLoad>::rehash(size_t size)
	{
		if (size <= _capacity && _used == _size && _capacity != 0U)
			return;
		size = std::max(size, _size);

		const size_t o_capacity = _capacity;
		if (_capacity < 16U)
			_capacity = 16U;
		while(_capacity < size)
			_capacity <<= 1U;
		_mask = _capacity - 1U;

		if (values == NULL)
		{
			values = (value_type*) malloc(sizeof(value_type) * _capacity);
			keys = (key_type*) malloc(sizeof(key_type) * _capacity);
			usemask = (unsigned char*) malloc(sizeof(unsigned char) * _capacity);
			memset(usemask, Empty, sizeof(unsigned char) * _capacity);
			return;
		}
		value_type *o_values = values;
		key_type *o_keys = keys;
		unsigned char *o_usemask = usemask;

		values = (value_type*) malloc(sizeof(value_type) * _capacity);
		keys = (key_type*) malloc(sizeof(key_type) * _capacity);
		usemask = (unsigned char*) malloc(sizeof(unsigned char) * _capacity);
		memset(usemask, Empty, sizeof(unsigned char) * _capacity);

		_used = 0U;
		if (_size > 0U)
		{
			_size = 0U;
			for(size_t i = 0U ; i < o_capacity ; ++i)
			{
				if (o_usemask[i] == Used)
				{
					insert(o_keys[i], o_values[i]);
					o_values[i].value_type::~value_type();
					o_keys[i].key_type::~key_type();
				}
			}
		}

		free(o_values);
		free(o_keys);
		free(o_usemask);
	}

	template<class K, class T, class HFn, int MaxLoad>
	void hashmap<K,T,HFn,MaxLoad>::insert(const K &key, const T &value)
	{
		if (_used >= (_capacity * MaxLoad >> 8))
			rehash(std::max<size_t>(_capacity << 1U, 16U));

		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		size_t first_suitable_place = -1;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
				{
					values[h].T::~T();
					new (&(values[h])) T(value);
					return;
				}
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				if (first_suitable_place == -1)
					first_suitable_place = h;
				h = (h + 1U) & _mask;
				continue;
			}
			break;
		} while(true);
		if (first_suitable_place != -1)
			h = first_suitable_place;
		++_size;
		++_used;
		usemask[h] = Used;
		new(values + h) value_type(value);
		new(keys + h) key_type(key);
	}

	template<class K, class T, class HFn, int MaxLoad>
	const T *hashmap<K,T,HFn,MaxLoad>::fast_find(const K &key) const
	{
		if (_size == 0U)
			return NULL;
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
					return &(values[h]);
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				return NULL;
			}
		} while(true);
	}

	template<class K, class T, class HFn, int MaxLoad>
	T *hashmap<K,T,HFn,MaxLoad>::fast_find(const K &key)
	{
		if (_size == 0U)
			return NULL;
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
					return &(values[h]);
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				return NULL;
			}
		} while(true);
	}

	template<class K, class T, class HFn, int MaxLoad>
	T &hashmap<K,T,HFn,MaxLoad>::operator [](const K &key)
	{
		if (_used >= (_capacity * MaxLoad >> 8))
			rehash(std::max<size_t>(_capacity << 1U, 16U));

		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		size_t first_suitable_place = -1;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
					return values[h];
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				if (first_suitable_place == -1)
					first_suitable_place = h;
				h = (h + 1U) & _mask;
				continue;
			}
			break;
		} while(true);
		if (first_suitable_place != -1)
			h = first_suitable_place;
		++_size;
		++_used;
		usemask[h] = Used;
		new(values + h) value_type();
		new(keys + h) key_type(key);
		return values[h];
	}

	template<class K, class T, class HFn, int MaxLoad>
	void hashmap<K,T,HFn,MaxLoad>::remove(const K &key)
	{
		if (_size == 0U)
			return;
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
				{
					values[h].value_type::~value_type();
					keys[h].key_type::~key_type();
					usemask[h] = Deleted;
					--_size;
					if ((_size << 1U) <= _used)
						rehash(_size << 1U);
					return;
				}
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			}
			return;
		} while(true);
	}

	template<class K, class T, class HFn, int MaxLoad>
	typename hashmap<K,T,HFn,MaxLoad>::iterator hashmap<K,T,HFn,MaxLoad>::find(const K &key)
	{
		if (_size == 0U)
			return end();
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
					return iterator(this, h);
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				return end();
			}
		} while(true);
	}

	template<class K, class T, class HFn, int MaxLoad>
	typename hashmap<K,T,HFn,MaxLoad>::const_iterator hashmap<K,T,HFn,MaxLoad>::find(const K &key) const
	{
		if (_size == 0U)
			return end();
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (keys[h] == key)
					return const_iterator(this, h);
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				return end();
			}
		} while(true);
	}

	template<class K, class T, class HFn, int MaxLoad>
	void hashmap<K,T,HFn,MaxLoad>::remove(const iterator &it)
	{
		if (it.idx >= _capacity)
			return;
		if (usemask[it.idx] != Used)
			return;
		values[it.idx].value_type::~value_type();
		keys[it.idx].key_type::~key_type();
		usemask[it.idx] = Deleted;
		--_size;
		if ((_size << 1U) <= _used)
			rehash(_size << 1U);
	}

	template<class K, class T, class HFn, int MaxLoad>
	void hashmap<K,T,HFn,MaxLoad>::remove(const const_iterator &it)
	{
		if (it.idx >= _capacity)
			return;
		if (usemask[it.idx] != Used)
			return;
		values[it.idx].value_type::~value_type();
		keys[it.idx].key_type::~key_type();
		usemask[it.idx] = Deleted;
		--_size;
		if ((_size << 1U) <= _used)
			rehash(_size << 1U);
	}
}

#endif // HASHMAP_H
