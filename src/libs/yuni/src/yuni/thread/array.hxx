#ifndef __YUNI_THREAD_ARRAY_HXX__
# define __YUNI_THREAD_ARRAY_HXX__


namespace Yuni
{
namespace Thread
{


	template<class T>
	inline Array<T>::Array()
		:pAutoStart(false)
	{}


	template<class T>
	Array<T>::Array(const Array<T>& rhs)
	{
		typename ThreadingPolicy::MutexLocker locker(rhs);
		pAutoStart = rhs.pAutoStart;
		pList = rhs.pList;
	}


	template<class T>
	inline Array<T>::Array(unsigned int n)
		:pAutoStart(false)
	{
		// Bound checks
		if (n > maxThreadsLimit)
			n = maxThreadsLimit;
		appendNThreadsWL(n, false);
	}


	template<class T>
	inline Array<T>::Array(unsigned int n, bool autoStart)
		:pAutoStart(autoStart)
	{
		// Bound checks
		if (n > maxThreadsLimit)
			n = maxThreadsLimit;
		appendNThreadsWL(n, autoStart);
	}

	template<class T>
	inline Array<T>::~Array()
	{
		// We won't stop all remaining threads. They have to do it by themselves
		// when destroyed.
		pList.clear();
	}


	template<class T>
	inline bool Array<T>::autoStart() const
	{
		return (pAutoStart);
	}


	template<class T>
	inline void Array<T>::autoStart(const bool v)
	{
		pAutoStart  = v;
	}


	template<class T>
	void Array<T>::clear()
	{
		// We will make a copy of the list to release the lock
		// as soon as possible since this routine may take some time...
		ThreadList copy;
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			if (pList.empty())
				return;
			copy = pList;
			pList.clear();
		}

		// Removing all threads
		copy.clear();
	}


	template<class T>
	void Array<T>::add(typename T::Ptr thread)
	{
		if (pAutoStart)
			thread->start();
		typename ThreadingPolicy::MutexLocker locker(*this);
		pList.push_back(thread);
	}


	template<class T>
	void Array<T>::add(typename T::Ptr thread, bool autostart)
	{
		if (autostart)
			thread->start();
		// Locking
		typename ThreadingPolicy::MutexLocker locker(*this);
		pList.push_back(thread);
	}


	template<class T>
	inline void Array<T>::push_back(typename T::Ptr thread)
	{
		// Locking
		typename ThreadingPolicy::MutexLocker locker(*this);
		pList.push_back(thread);
	}


	template<class T>
	void Array<T>::resize(unsigned int n)
	{
		if (!n)
		{
			// When resizing to 0 elements, it is exactly equivalent to directly
			// call the method clear(), which should be more efficient
			clear();
			return;
		}

		// Bound checks
		if (n > maxThreadsLimit)
			n = maxThreadsLimit;

		// If we have some thread to remove from the pool, we will use this copy list
		// since it can take some time
		ThreadList copy;

		{
			// Locking
			typename ThreadingPolicy::MutexLocker locker(*this);

			// Keeping the number of existing thread
			const unsigned int count = pList.size();
			if (count == n)
				return;

			if (count < n)
			{
				// We don't have enough threads in pool. Creating a few of them...
				appendNThreadsWL(n - count, pAutoStart);
				return;
			}

			// Asking to the last threads to stop by themselves as soon as possible
			// This should be done early to make them stop asynchronously.
			// We may earn a lot of time like this.
			for (unsigned int i = n; i < count; ++i)
				pList[i]->gracefulStop();

			// Creating a list of all threads that must be removed
			copy.reserve(count - n);
			for (unsigned int i = n; i < count; ++i)
				copy.push_back(pList[i]);
			// We can resize the vector, the removed threads will be stopped soon
			pList.resize(count);
		}

		// Removing all pending threads
		copy.clear();
	}


	template<class T>
	void Array<T>::start()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (!pList.empty())
		{
			const typename ThreadList::iterator end = pList.end();
			for (typename ThreadList::iterator i = pList.begin(); i != end; ++i)
				(*i)->start();
		}
	}


	template<class T>
	void Array<T>::gracefulStop()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (!pList.empty())
		{
			const typename ThreadList::iterator end = pList.end();
			for (typename ThreadList::iterator i = pList.begin(); i != end; ++i)
				(*i)->gracefulStop();
		}
	}


	template<class T>
	void Array<T>::stop(unsigned int timeout)
	{
		// We will make a copy of the list to release the lock as soon as
		// possible since this routine may take some time...
		ThreadList copy;
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			if (pList.empty())
				return;
			copy = pList;
		}

		const typename ThreadList::iterator end = copy.end();

		// Asking to the last threads to stop by themselves as soon as possible
		// This should be done early to make them stop asynchronously.
		// We may earn a lot of time like this.
		for (typename ThreadList::iterator i = copy.begin(); i != end; ++i)
			(*i)->gracefulStop();
		// Now we can kill them if they don't cooperate...
		for (typename ThreadList::iterator i = copy.begin(); i != end; ++i)
			(*i)->stop(timeout);
		// Clear
		copy.clear();
	}


	template<class T>
	void Array<T>::restart(unsigned int timeout)
	{
		// We will make a copy of the list to release the lock as soon as
		// possible since this routine may take some time...
		ThreadList copy;
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			if (pList.empty())
				return;
			copy = pList;
		}

		const typename ThreadList::iterator end = copy.end();

		// Asking to the last threads to stop by themselves as soon as possible
		// This should be done early to make them stop asynchronously.
		// We may earn a lot of time like this.
		for (typename ThreadList::iterator i = copy.begin(); i != end; ++i)
			(*i)->gracefulStop();
		// Now we can kill them if they don't cooperate...
		for (typename ThreadList::iterator i = copy.begin(); i != end; ++i)
			(*i)->stop(timeout);
		// And start them again
		for (typename ThreadList::iterator i = copy.begin(); i != end; ++i)
			(*i)->start();
	}


	template<class T>
	void Array<T>::wakeUp()
	{
		// Locking
		typename ThreadingPolicy::MutexLocker locker(*this);
		// We can wake all threads up at once while locked because this operation
		// is quite fast.
		const typename ThreadList::iterator end = pList.end();
		for (typename ThreadList::iterator i = pList.begin(); i != end; ++i)
			(*i)->wakeUp();
	}


	template<class T>
	inline typename T::Ptr Array<T>::operator [] (unsigned int index) const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return (index < pList.size()) ? pList[index] : T::Ptr();
	}


	template<class T>
	Array<T>& Array<T>::operator = (const Array<T>& rhs)
	{
		typename ThreadingPolicy::MutexLocker lockerR(rhs);
		typename ThreadingPolicy::MutexLocker locker(*this);
		pAutoStart = rhs.pAutoStart;
		pList = rhs.pList;
		return *this;
	}


	template<class T>
	Array<T>& Array<T>::operator = (const typename Array<T>::Ptr& rhs)
	{
		typename Array<T>::Ptr keepReference = rhs;
		typename ThreadingPolicy::MutexLocker lockerR(*keepReference);
		typename ThreadingPolicy::MutexLocker locker(*this);
		pAutoStart = keepReference.pAutoStart;
		pList = keepReference.pList;
		return *this;
	}


	template<class T>
	Array<T>& Array<T>::operator += (const Array<T>& rhs)
	{
		typename ThreadingPolicy::MutexLocker lockerR(rhs);
		typename ThreadingPolicy::MutexLocker locker(*this);
		const typename ThreadList::const_iterator end = rhs.pList.end();
		for (typename ThreadList::const_iterator i = rhs.pList.begin(); i != end; ++i)
			pList.push_back(*i);
		return *this;
	}


	template<class T>
	Array<T>& Array<T>::operator += (const typename Array<T>::Ptr& rhs)
	{
		typename Array<T>::Ptr keepReference = rhs;
		typename ThreadingPolicy::MutexLocker lockerR(*keepReference);
		typename ThreadingPolicy::MutexLocker locker(*this);
		const typename ThreadList::const_iterator end = keepReference->pList.end();
		for (typename ThreadList::const_iterator i = keepReference->pList.begin(); i != end; ++i)
			pList.push_back(*i);
		return *this;
	}


	template<class T>
	Array<T>& Array<T>::operator << (const Array<T>& rhs)
	{
		typename ThreadingPolicy::MutexLocker lockerR(rhs);
		typename ThreadingPolicy::MutexLocker locker(*this);
		const typename ThreadList::const_iterator end = rhs.pList.end();
		for (typename ThreadList::const_iterator i = rhs.pList.begin(); i != end; ++i)
			pList.push_back(*i);
		return *this;
	}


	template<class T>
	Array<T>& Array<T>::operator << (const typename Array<T>::Ptr& rhs)
	{
		typename Array<T>::Ptr keepReference = rhs;
		typename ThreadingPolicy::MutexLocker lockerR(*keepReference);
		typename ThreadingPolicy::MutexLocker locker(*this);
		const typename ThreadList::const_iterator end = keepReference->pList.end();
		for (typename ThreadList::const_iterator i = keepReference->pList.begin(); i != end; ++i)
			pList.push_back(*i);
		return *this;
	}


	template<class T>
	inline Array<T>& Array<T>::operator << (T* thread)
	{
		add(thread);
		return *this;
	}


	template<class T>
	inline Array<T>& Array<T>::operator << (const typename T::Ptr& thread)
	{
		add(thread);
		return *this;
	}


	template<class T>
	inline Array<T>& Array<T>::operator += (T* thread)
	{
		add(thread);
		return *this;
	}


	template<class T>
	inline Array<T>& Array<T>::operator += (const typename T::Ptr& thread)
	{
		add(thread);
		return *this;
	}



	template<class T>
	void Array<T>::appendNThreadsWL(unsigned int n, bool autostart)
	{
		// Keeping the number of existing thread
		const unsigned int count = pList.size();
		if (count < n)
		{
			// We don't have enough threads in pool. Creating a few of them...
			// We should use the variable `pAutoStart` once time only to avoid
			// changes while adding the new threads
			if (autostart)
			{
				for (unsigned int i = count; i < n; ++i)
				{
					T* thread = new T();
					thread->start();
					pList.push_back(thread);
				}
			}
			else
			{
				for (unsigned int i = count; i < n; ++i)
					pList.push_back(new T());
			}
		}
	}


	template<class T>
	inline unsigned int Array<T>::size() const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pList.size();
	}


	template<class T>
	inline unsigned int Array<T>::count() const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pList.size();
	}


	template<class T>
	inline bool Array<T>::empty() const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		return pList.empty();
	}



	template<class T>
	template<class PredicateT>
	void Array<T>::foreachThread(PredicateT& predicate) const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		const typename ThreadList::const_iterator end = pList.end();
		for (typename ThreadList::const_iterator i = pList.begin(); i != end; ++i)
		{
			if (!predicate(*i))
				return;
		}
	}




} // namespace Thread
} // namespace Yuni

#endif // __YUNI_THREAD_ARRAY_HXX__
