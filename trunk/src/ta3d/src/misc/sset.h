#ifndef __TA3D_SSET_H__
#define __TA3D_SSET_H__

#include "slist.h"



namespace TA3D
{


	template< typename T, typename Container = slist<T> >
	class sset
	{
	public:
		typedef typename Container::iterator	iterator;
		typedef const iterator const_iterator;

	public:
		sset() : data()	{}

		sset(const sset<T> &x)
		{
			data = x.data;
		}

		sset<T> &operator= (const sset<T> &x)
		{
			clear();
			data = x.data;
			return *this;
		}

		iterator begin() {   return data.begin();    }
		iterator end()   {   return data.end();    }
		const_iterator begin() const {   return data.begin();    }
		const_iterator end()   const {   return data.end();    }

		void clear()			{	data.clear();	}

		bool empty() const   {   return data.empty();  }
		int size() const     {   return data.size();   }

		void erase(iterator i)
		{
			data.erase(i);
		}

		void add(const T &value)
		{
			data.push_front(value);
		}

		void isIn(const T &value) const
		{
			const const_iterator end = this->end();
			for (const_iterator i = begin(); i != end; ++i)
			{
				if (*i == value)
					return true;
			}
			return false;
		}

		void remove(const T &value)
		{
			const iterator end = this->end();
			for (iterator i = begin(); i != end; ++i)
			{
				if (*i == value)
				{
					data.erase(i);
					return;
				}
			}
		}

		void swap(sset<T> &x)
		{
			data.swap(x.data);
		}

		Container &getData()
		{
			return data;
		}

	private:
		Container data;

	}; // class sset



} // namespace TA3D

#endif
