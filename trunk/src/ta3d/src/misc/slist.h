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

			slist_node(const T &v, slist_node *next = NULL)
			{
				this->v = v;
				this->next = next;
			}
		};
	public:
		class iterator
		{
		public:
			iterator() :it(NULL)  {}
			iterator(const iterator &x) :it(x.it) {}
			iterator(slist_node *x) :it(x) {}
			iterator &operator=(slist_node *x)  {   it = x;  return *this;   }
			iterator &operator=(const iterator &x)  {   it = x.it;  return *this;   }
			iterator &operator++()  {   if (it)  it = it->next;  return *this;   }
			iterator operator++(int)
			{
				iterator i = *this;
				if (it)
					it = it->next;
				return i;
			}
			iterator &operator+(int n)
			{
				for (; n > 0 && it; --n)
					it = it->next;
				return *this;
			}
			T &operator->() {   return it->v;   }
			T &operator*() {   return it->v;   }
			const T &operator->() const {   return it->v;   }
			const T &operator*() const {   return it->v;   }
			bool operator==(const iterator &x)  const {   return it == x.it;  }
			bool operator!=(const iterator &x)  const {   return it != x.it;  }

			operator bool() const {   return it != NULL;  }

			slist_node *p() {   return it;  }
		private:
			slist_node*  it;
		};

		typedef const iterator const_iterator;

	public:
		slist()
			:head(NULL), last(NULL), _size(0)
		{}

		slist(int n, const T &value = T())
			:head(NULL), last(NULL), _size(n)
		{
			if (n > 0)
			{
				head = new slist_node(value, head);
				last = head;
				--n;
				for(; n > 0 ; --n)
					head = new slist_node(value, head);
			}
		}

		slist(const slist<T> &x)
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

		~slist()
		{
			clear();
		}

		slist<T> & operator= (const slist& x)
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

		iterator begin() {   return iterator(head);    }
		iterator end()   {   return iterator();    }
		const_iterator begin() const {   return iterator(head);    }
		const_iterator end()   const {   return iterator();    }

		void clear()
		{
			while (head)
			{
				slist_node *next = head->next;
				delete head;
				head = next;
			}
			last = head = NULL;
			_size = 0;
		}

		bool empty() const   {   return _size == 0;  }
		int size() const     {   return _size;   }

		void resize(int sz, const T &value = T())
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

		void erase_after(iterator i)
		{
			if (!i) return;
			slist_node *e = i.p()->next;
			i.p()->next = NULL;
			last = i.p();

			while (e)
			{
				slist_node *f = e->next;
				delete e;
				e = f;
				--_size;
			}
		}

		void erase(iterator i)
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

		T &front()
		{
			return head->v;
		}

		const T &front() const
		{
			if (empty())    return T();
			else            return head->v;
		}

		T &back()
		{
			return last->v;
		}

		const T &back() const
		{
			if (empty())    return T();
			else            return last->v;
		}

		void push_front(const T &value)
		{
			head = new slist_node(value, head);
			if (last == NULL)
				last = head;
			++_size;
		}

		void push_back(const T &value)
		{
			if (empty())
				last = head = new slist_node(value,NULL);
			else
			{
				last->next = new slist_node(value, NULL);
				last = last->next;
			}
			++_size;
		}

		void pop_front()
		{
			if (!empty())
				erase(head);
		}

		void pop_back()
		{
			if (!empty())
				erase(last);
		}

		iterator insert(iterator pos, const T &value)
		{
			pos.p()->next = new slist_node(*pos, pos.p()->next);
			*pos = value;
			return pos;
		}

		void swap(slist<T> &x)
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
		slist_node	*head;
		slist_node	*last;
		short		_size;

	}; // class slist



} // namespace TA3D

#endif
