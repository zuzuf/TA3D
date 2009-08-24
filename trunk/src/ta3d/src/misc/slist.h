#ifndef __TA3D_SLIST_H__
#define __TA3D_SLIST_H__

namespace TA3D
{

	template< class T >
			class slist
	{
	private:
		class slist_node
		{
		public:
			slist_node      *next;
			T               v;

			inline slist_node(const T &v, slist_node *next = NULL)
			{
				this->v = v;
				this->next = next;
			}
		};
	public:
		class iterator
		{
		public:
			inline iterator()  {   it = NULL;  }
			inline iterator(const iterator &x) {   it = x.it;  }
			inline iterator(slist_node *x) {   it = x;  }
			inline iterator &operator=(slist_node *x)  {   it = x;  return *this;   }
			inline iterator &operator=(const iterator &x)  {   it = x.it;  return *this;   }
			inline iterator &operator++()  {   if (it)  it = it->next;  return *this;   }
			inline iterator operator++(int)
			{
				iterator i = *this;
				if (it)
					it = it->next;
				return i;
			}
			inline iterator &operator+(int n)
			{
				for(;n > 0 && it; --n)
					it = it->next;
				return *this;
			}
			inline T &operator->() {   return it->v;   }
			inline T &operator*() {   return it->v;   }
			inline const T &operator->() const {   return it->v;   }
			inline const T &operator*() const {   return it->v;   }
			inline bool operator==(const iterator &x)  const {   return it == x.it;  }
			inline bool operator!=(const iterator &x)  const {   return it != x.it;  }

			inline operator bool() const {   return it != NULL;  }

			inline slist_node *p()   {   return it;  }
		private:
			slist_node*  it;
		};

		typedef const iterator const_iterator;

		inline slist()
		{
			head = NULL;
			last = NULL;
			_size = 0;
		}

		inline slist(int n, const T &value = T())
		{
			_size = n;
			last = head = NULL;
			if (n > 0)
			{
				last = head = new slist_node(value, head);
				--n;
				for(; n > 0 ; --n)
					head = new slist_node(value, head);
			}
		}

		inline slist(const slist<T> &x)
		{
			if (x._size > 0)
			{
				head = new slist_node(x.head->v, x.head->next);
				last = head;
				while(last->next)
				{
					last->next = new slist_node(last->next->v, last->next->next);
					last = last->next;
				}
			}
			else
				last = head = NULL;
			_size = x._size;
		}

		inline ~slist()
		{
			clear();
		}

		inline slist<T> &operator=( const slist<T> &x )
								  {
			clear();
			if (x._size > 0)
			{
				head = new slist_node(x.head->v, x.head->next);
				last = head;
				while(last->next)
				{
					last->next = new slist_node(last->next->v, last->next->next);
					last = last->next;
				}
			}
			else
				head = last = NULL;
			_size = x._size;
			return *this;
		}

		inline iterator begin() {   return iterator(head);    }
		inline iterator end()   {   return iterator();    }
		inline const_iterator begin() const {   return iterator(head);    }
		inline const_iterator end()   const {   return iterator();    }

		inline void clear()
		{
			while(head)
			{
				slist_node *next = head->next;
				delete head;
				head = next;
			}
			last = head = NULL;
			_size = 0;
		}

		inline bool empty() const   {   return _size == 0;  }
		inline int size() const     {   return _size;   }

		inline void resize(int sz, const T &value = T())
		{
			if (sz > _size)
				while(sz > _size)
					push_back(value);
			else if (sz < _size)
			{
				last = head;
				while(sz > 0)
				{
					last = last->next;
					sz--;
				}
				erase_after(last);
			}
		}

		inline void erase_after(iterator i)
		{
			if (!i) return;
			slist_node *e = i.p()->next;
			i.p()->next = NULL;
			last = i.p();

			while(e)
			{
				slist_node *f = e->next;
				delete e;
				e = f;
				--_size;
			}
		}

		inline void erase(iterator i)
		{
			if (!i) return;
			if (i.p()->next)
			{
				slist_node *old = i.p()->next;
				*(i.p()) = *old;
				delete old;
				if (last == old)
					last = i.p();
				_size--;
			}
			else
			{
				i = head;
				while(i.p()->next && i.p()->next->next)  ++i;
				if (i.p()->next == NULL)
					clear();
				else
					erase_after(i);
			}
		}

		inline T &front()
		{
			return head->v;
		}

		inline const T &front() const
		{
			if (empty())    return T();
			else            return head->v;
		}

		inline T &back()
		{
			return last->v;
		}

		inline const T &back() const
		{
			if (empty())    return T();
			else            return last->v;
		}

		inline void push_front(const T &value)
		{
			head = new slist_node(value, head);
			if (last == NULL)
				last = head;
			++_size;
		}

		inline void push_back(const T &value)
		{
			if (empty())
				last = head = new slist_node(value,NULL);
			else
			{
				last->next = new slist_node(value, NULL);
				last = last->next;
			}
			_size++;
		}

		inline void pop_front()
		{
			if (!empty())   erase(head);
		}

		inline void pop_back()
		{
			if (!empty())   erase(last);
		}

		inline iterator insert(iterator pos, const T &value)
		{
			pos.p()->next = new slist_node(*pos, pos.p()->next);
			*pos = value;
			return pos;
		}

		inline void swap(slist<T> &x)
		{
			slist_node *s = head;
			head = x.head;
			x.head = s;

			s = last;
			last = x.last;
			x.last = s;

			_size ^= x._size;
			x._size ^= _size;
			_size ^= x._size;
		}
	private:
		slist_node   *head;
		slist_node   *last;
		int          _size;
	};
}

#endif
