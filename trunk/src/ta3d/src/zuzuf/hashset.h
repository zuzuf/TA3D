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

#ifndef HASHSET_H
#define HASHSET_H

#include <utility>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "hashfn.h"

namespace zuzuf
{

	template<class T, class HFn = hash<T>, int MaxLoad = 128>
	class hashset
	{
		template<class HT, class TT>	class tpl_iterator;
		template<class HT, class TT> friend class hashset::tpl_iterator;
	public:
		typedef T key_type;
		typedef T value_type;
		typedef T node_type;
		typedef tpl_iterator<hashset const, const T>	const_iterator;
		typedef tpl_iterator<hashset, T>	iterator;
	private:
		template<class HT, class TT>
		class tpl_iterator
		{
			friend class tpl_iterator<hashset, T>;
			friend class tpl_iterator<hashset const, const T>;
		public:
			tpl_iterator(HT *hset, size_t idx) : hset(hset), idx(idx)	{}
			tpl_iterator(const tpl_iterator &it) : hset(it.hset), idx(it.idx)	{}

			tpl_iterator &operator=(const tpl_iterator &it)
			{	hset = it.hset;	idx = it.idx;	return *this;	}

			operator const_iterator() const
			{
				return const_iterator(hset, idx);
			}

			bool operator==(const iterator &it) const		{	return idx == it.idx;	}
			bool operator==(const const_iterator &it) const	{	return idx == it.idx;	}
			bool operator!=(const iterator &it) const		{	return idx != it.idx;	}
			bool operator!=(const const_iterator &it) const	{	return idx != it.idx;	}
			bool operator<(const iterator &it) const		{	return idx < it.idx;	}
			bool operator<(const const_iterator &it) const	{	return idx < it.idx;	}
			bool operator>(const iterator &it) const		{	return idx > it.idx;	}
			bool operator>(const const_iterator &it) const	{	return idx > it.idx;	}

			TT &operator*() const {	return hset->data[idx];	}
			TT *operator->() const {	return hset->data + idx;	}

			void operator++()
			{
				do
				{
					++idx;
				} while(idx < hset->_capacity && hset->usemask[idx] != Used);
			}

			tpl_iterator operator++(int)
			{
				tpl_iterator old(*this);
				do
				{
					++idx;
				} while(idx < hset->_capacity && hset->usemask[idx] != Used);
				return old;
			}

			tpl_iterator operator+(int n) const
			{
				tpl_iterator ret(*this);
				for(; n > 0 ; --n)
					do
					{
						++ret.idx;
					} while(ret.idx < hset->_capacity && hset->usemask[ret.idx] != Used);
				return ret;
			}

			tpl_iterator operator-(int n) const
			{
				tpl_iterator ret(*this);
				for(; n > 0 && ret.idx > 0 ; --n)
					do
					{
						--ret.idx;
					} while(ret.idx > 0 && hset->usemask[ret.idx] != Used);
				return ret;
			}

			void operator--()
			{
				if (idx > 0)
					do
					{
						--idx;
					} while(idx > 0 && hset->usemask[idx] != Used);
			}

			tpl_iterator operator--(int)
			{
				tpl_iterator old(*this);
				if (idx > 0)
					do
					{
						--idx;
					} while(idx > 0 && hset->usemask[idx] != Used);
				return old;
			}

		private:
			HT *hset;
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
		inline hashset() : data(NULL), usemask(NULL), _capacity(0U), _size(0U), _mask(0U), _used(0U)	{}
		inline hashset(const hashset &v);
		inline ~hashset();

		inline hashset &operator=(const hashset &v);

		inline void clear();

		bool empty() const {	return _size == 0U;	}

		size_t capacity() const {	return _capacity;	}
		size_t size() const {	return _size;	}

		size_t memory_usage() const
		{
			return _capacity * (sizeof(unsigned char) + sizeof(node_type)) + sizeof(hashset);
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

		inline void insert(const T &v);

		inline iterator find(const T &key);
		inline const_iterator find(const T &key) const;
		inline const T &get(const T &key) const;
		inline T &get(const T &key);

		inline bool contains(const T &key) const;
		inline int count(const T &key) const	{	return contains(key);	}

		inline void remove(const T &value);
		inline void remove(const iterator &it);
		inline void remove(const const_iterator &it);
		inline void erase(const iterator &it)		{	remove(it);	}
		inline void erase(const const_iterator &it)	{	remove(it);	}
		inline void erase(const T &value)			{	remove(value);	}

	private:
		node_type *data;
		unsigned char *usemask;
		size_t _capacity;
		size_t _size;
		size_t _mask;
		size_t _used;
	};

	template<class T, class HFn, int MaxLoad>
	hashset<T,HFn,MaxLoad>::hashset(const hashset &v) : data(NULL), usemask(NULL), _capacity(0U), _size(0U), _mask(0U), _used(0U)
	{
		rehash(v.size());
		for(const_iterator it = v.begin(), end = v.end() ; it != end ; ++it)
			insert(*it);
	}

	template<class T, class HFn, int MaxLoad>
	hashset<T,HFn,MaxLoad> &hashset<T,HFn,MaxLoad>::operator=(const hashset &v)
	{
		clear();
		rehash(v.size());
		for(const_iterator it = v.begin(), end = v.end() ; it != end ; ++it)
			insert(*it);
		return *this;
	}

	template<class T, class HFn, int MaxLoad>
	hashset<T,HFn,MaxLoad>::~hashset()
	{
		clear();
	}

	template<class T, class HFn, int MaxLoad>
	void hashset<T,HFn,MaxLoad>::clear()
	{
		if (data)
		{
			if (_size > 0U)
				for(size_t i = 0U ; i < _capacity ; ++i)
					if (usemask[i] == Used)
						data[i].node_type::~node_type();
			free(data);
			free(usemask);
		}
		data = NULL;
		usemask = NULL;
		_size = 0U;
		_used = 0U;
		_capacity = 0U;
		_mask = 0U;
	}

	template<class T, class HFn, int MaxLoad>
	void hashset<T,HFn,MaxLoad>::rehash(size_t size)
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

		if (data == NULL)
		{
			data = (node_type*) malloc(sizeof(node_type) * _capacity);
			usemask = (unsigned char*) malloc(sizeof(unsigned char) * _capacity);
			memset(usemask, Empty, sizeof(unsigned char) * _capacity);
			return;
		}
		node_type *o_data = data;
		unsigned char *o_usemask = usemask;

		data = (node_type*) malloc(sizeof(node_type) * _capacity);
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
					insert(o_data[i]);
					o_data[i].node_type::~node_type();
				}
			}
		}

		free(o_data);
		free(o_usemask);
	}

	template<class T, class HFn, int MaxLoad>
	void hashset<T,HFn,MaxLoad>::insert(const T &v)
	{
		if (_used >= (_capacity * MaxLoad >> 8))
			rehash(std::max<size_t>(_capacity << 1U, 16U));

		const HFn hash;
		size_t h = (hash(v) << 1) & _mask;
		size_t first_suitable_place = size_t(-1);
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (data[h] == v)
					return;
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				if (first_suitable_place == size_t(-1))
					first_suitable_place = h;
				h = (h + 1U) & _mask;
				continue;
			}
			break;
		} while(true);
		if (first_suitable_place != size_t(-1))
			h = first_suitable_place;
		++_size;
		++_used;
		usemask[h] = Used;
		new(data + h) node_type(v);
	}

	template<class T, class HFn, int MaxLoad>
	bool hashset<T,HFn,MaxLoad>::contains(const T &key) const
	{
		if (_size == 0U)
			return false;
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (data[h] == key)
					return true;
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				return false;
			}
		} while(true);
	}

	template<class T, class HFn, int MaxLoad>
	void hashset<T,HFn,MaxLoad>::remove(const T &key)
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
				if (data[h] == key)
				{
					data[h].node_type::~node_type();
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

	template<class T, class HFn, int MaxLoad>
	typename hashset<T,HFn,MaxLoad>::iterator hashset<T,HFn,MaxLoad>::find(const T &key)
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
				if (data[h] == key)
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

	template<class T, class HFn, int MaxLoad>
	typename hashset<T,HFn,MaxLoad>::const_iterator hashset<T,HFn,MaxLoad>::find(const T &key) const
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
				if (data[h] == key)
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

	template<class T, class HFn, int MaxLoad>
	const T &hashset<T,HFn,MaxLoad>::get(const T &key) const
	{
		if (_size == 0U)
		{
			insert(key);
			return get(key);
		}
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (data[h] == key)
					return data[h];
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				insert(key);
				return get(key);
			}
		} while(true);
	}

	template<class T, class HFn, int MaxLoad>
	T &hashset<T,HFn,MaxLoad>::get(const T &key)
	{
		if (_size == 0U)
		{
			insert(key);
			return get(key);
		}
		const HFn hash;
		size_t h = (hash(key) << 1) & _mask;
		do
		{
			switch(usemask[h])
			{
			case Used:
				if (data[h] == key)
					return data[h];
				h = (h + 1U) & _mask;
				continue;
			case Deleted:
				h = (h + 1U) & _mask;
				continue;
			default:
				insert(key);
				return get(key);
			}
		} while(true);
	}

	template<class T, class HFn, int MaxLoad>
	void hashset<T,HFn,MaxLoad>::remove(const iterator &it)
	{
		if (it.idx >= _capacity)
			return;
		if (usemask[it.idx] != Used)
			return;
		data[it.idx].node_type::~node_type();
		usemask[it.idx] = Deleted;
		--_size;
		if ((_size << 1U) <= _used)
			rehash(_size << 1U);
	}
}

#endif // HASHSET_H
