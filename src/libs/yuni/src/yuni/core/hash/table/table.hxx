#ifndef __YUNI_HASH_TABLE_HXX__
# define __YUNI_HASH_TABLE_HXX__


namespace Yuni
{
namespace Hash
{

	template<typename K, typename V, template<class> class TP>
	inline bool Table<K,V,TP>::exists(const K& key)
	{
		typename Table<K,V,TP>::ThreadingPolicy::MutexLocker locker(*this);
		return pTable.find(key) != pTable.end();
	}


	template<typename K, typename V, template<class> class TP>
	inline V& Table<K,V,TP>::operator[] (const K& key)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pTable[key];
	}


	template<typename K, typename V, template<class> class TP>
	inline typename Table<K,V,TP>::iterator Table<K,V,TP>::find(const K& key)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pTable.find(key);
	}


	template<typename K, typename V, template<class> class TP>
	inline V Table<K,V,TP>::value(const K& key, const V& defvalue)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		const_iterator it = pTable.find(key);
		return (it == pTable.end()) ? defvalue : it->second;
	}

	template<typename K, typename V, template<class> class TP>
	inline void Table<K,V,TP>::erase(const K& key)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		iterator it = pTable.find(key);
		if (pTable.end() != it)
			pTable.erase(it);
	}


	template<typename K, typename V, template<class> class TP>
	typename Table<K,V,TP>::iterator Table<K,V,TP>::begin()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pTable.begin();
	}


	template<typename K, typename V, template<class> class TP>
	typename Table<K,V,TP>::iterator Table<K,V,TP>::end()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pTable.end();
	}


	template<typename K, typename V, template<class> class TP>
	void Table<K,V,TP>::clear()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		pTable.clear();
	}


	template<typename K, typename V, template<class> class TP>
	inline int Table<K,V,TP>::size()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pTable.size();
	}


	template<typename K, typename V, template<class> class TP>
	inline std::pair<typename Table<K,V,TP>::iterator, bool> Table<K,V,TP>::insert(const K& key)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pTable.insert(key);
	}




} // namespace Hash
} // namespace Yuni

#endif // __YUNI_HASH_TABLE_HXX__
