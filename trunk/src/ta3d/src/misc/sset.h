#ifndef __TA3D_SSET_H__
#define __TA3D_SSET_H__

#include "slist.h"

namespace TA3D
{

	template< typename T, typename Container = slist<T> >
			class sset
	{
	private:
		Container data;
	public:
		typedef typename Container::iterator	iterator;
		typedef const iterator const_iterator;

		inline sset() : data()	{}

		inline sset(const sset<T> &x)
		{
			data = x.data;
		}

		inline sset<T> &operator=( const sset<T> &x )
		{
			clear();
			data = x.data;
			return *this;
		}

		inline iterator begin() {   return data.begin();    }
		inline iterator end()   {   return data.end();    }
		inline const_iterator begin() const {   return data.begin();    }
		inline const_iterator end()   const {   return data.end();    }

		inline void clear()			{	data.clear();	}

		inline bool empty() const   {   return data.empty();  }
		inline int size() const     {   return data.size();   }

		inline void erase(iterator i)
		{
			data.erase(i);
		}

		inline void add(const T &value)
		{
			data.push_front(value);
		}

		inline void isIn(const T &value) const
		{
			for(const_iterator i = begin() ; i != end() ; ++i)
				if (*i == value)
					return true;
			return false;
		}

		inline void remove(const T &value)
		{
			for(iterator i = begin() ; i != end() ; ++i)
				if (*i == value)
				{
					data.erase(i);
					return;
				}
		}

		inline void swap(sset<T> &x)
		{
			data.swap(x.data);
		}

		inline Container &getData()
		{
			return data;
		}
	};
}

#endif
