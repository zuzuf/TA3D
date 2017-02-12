#ifndef __YUNI_HASH_TABLE_TABLE_H__
# define __YUNI_HASH_TABLE_TABLE_H__

# include <map>


namespace Yuni
{
namespace Hash
{


	template<class KeyT, class ValueT, class ImplT = std::map<KeyT,ValueT> >
	class Index : protected ImplT
	{
	public:
		//! Type of the key
		typedef KeyT KeyType;
		//! Type for a value
		typedef ValueT ValueType;

		//! The real implementation
		typedef ImplT ImplementationType;

		//! Size
		typedef typename ImplementationType::size_type size_type;

	public:
		//! \name Constructors & Destructor
		//@{
		Table();
		//! Destructor
		~Table();
		//@}

		template<class K> iterator addOrUpdate(const K& key)
		{
			return ImplementationType::template addOrUpdate<K>(key);
		}

		template<class K, class V> iterator addOrUpdate(const K& key, const V& value)
		{
			return ImplementationType::template addOrUpdate<K,V>(key, value);
		}

		/*!
		** \brief Remove all items
		*/
		void clear()
		{
			ImplementationType::clear();
		}

		/*!
		** \brief Clear all deleted items (if any)
		*/
		void purge()
		{
			ImplementationType::purge();
		}

		template<class K>
		bool exists(const K& key) const
		{
			return ImplementationType::exists(key);
		}

		template<class K> bool remove(const K& key)
		{
			return false;
		}

		bool empty() const
		{
			return ImplementationType::empty();
		}

		size_type size() const
		{
			return ImplementationType::size();
		}

		template<class K> ValueType& operator [] (const K& key)
		{
		}

	}; // class Table<>





} // namespace Hash
} // namespace Yuni


#endif // __YUNI_HASH_TABLE_TABLE_H__
