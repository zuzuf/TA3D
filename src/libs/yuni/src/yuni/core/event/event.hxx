#ifndef __YUNI_CORE_EVENT_EVENT_HXX__
# define __YUNI_CORE_EVENT_EVENT_HXX__


namespace Yuni
{


	template<typename P>
	inline Event<P>::Event()
	{}


	template<typename P>
	inline Event<P>::Event(const Event<P>& rhs)
	{}


	template<typename P>
	inline Event<P>::~Event()
	{
		clearWL();
	}


	template<typename P>
	inline unsigned int Event<P>::count() const
	{
		return AncestorType::pBindList.size();
	}

	template<typename P>
	inline unsigned int Event<P>::size() const
	{
		return AncestorType::pBindList.size();
	}


	template<typename P>
	void Event<P>::clearWL()
	{
		if (!AncestorType::pBindList.empty())
		{
			// We will inform all bound objects that we are no longer linked.
			IEvent* baseThis = dynamic_cast<IEvent*>(this);
			const IEventObserverBase* base;
			const typename AncestorType::BindList::iterator end = AncestorType::pBindList.end();
			for (typename AncestorType::BindList::iterator i = AncestorType::pBindList.begin(); i != end; ++i)
			{
				if ((*i).isDescendantOfIEventObserverBase())
				{
					// Getting the object pointer, if any, then decrementing the ref counter
					base = ((*i).observerBaseObject());
					if (base)
						base->boundEventRemoveFromTable(baseThis);
				}
			}
			// Clear our own list
			AncestorType::pBindList.clear();
		}
	}


	template<typename P>
	inline void Event<P>::clear()
	{
		if (!AncestorType::pEmpty)
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			// In this case, the flag `empty` must be set first, to avoid concurrent
			// calls to `invoke()` for nothing.
			AncestorType::pEmpty = true;
			// Cleanup !
			clearWL();
		}
	}


	template<typename P>
	void Event<P>::connect(typename BindType::FunctionType pointer)
	{
		Bind<P> b;
		b.bind(pointer);

		typename ThreadingPolicy::MutexLocker locker(*this);
		AncestorType::pBindList.push_back(b);
		AncestorType::pEmpty = false;
	}


	template<typename P>
	template<typename U>
	void Event<P>::connect(typename BindType::template WithUserData<U>::FunctionType pointer,
		typename BindType::template WithUserData<U>::ParameterType userdata)
	{
		Bind<P> b;
		b.bind(pointer, userdata);

		// locking
		typename ThreadingPolicy::MutexLocker locker(*this);
		// list
		AncestorType::pBindList.push_back(b);
		AncestorType::pEmpty = false;
		// unlocking
	}


	template<typename P>
	template<class C>
	void Event<P>::connect(C* o, typename Event<P>::template PointerToMember<C>::Type method)
	{
		if (o)
		{
			Bind<P> b;
			b.bind(o, method);
			assert(b.isDescendantOfIEventObserverBase() && "Invalid class C. The calling class must inherit from Event::Observer<CRTP, ThreadingPolicy>");

			// Locking
			typename ThreadingPolicy::MutexLocker locker(*this);
			// list + increment ref counter
			AncestorType::pBindList.push_back(b);
			(dynamic_cast<const IEventObserverBase*>(o))->boundEventIncrementReference(dynamic_cast<IEvent*>(this));
			AncestorType::pEmpty = false;
			// Unlocking
		}
	}


	template<typename P>
	template<class C>
	void Event<P>::connect(const C* o, typename Event<P>::template PointerToMember<C>::ConstType method)
	{
		if (o)
		{
			Bind<P> b;
			b.bind(o, method);
			assert(b.isDescendantOfIEventObserverBase() && "Invalid class C");

			// locking
			typename ThreadingPolicy::MutexLocker locker(*this);
			// list + increment ref counter
			AncestorType::pBindList.push_back(b);
			(dynamic_cast<const IEventObserverBase*>(o))->boundEventIncrementReference(dynamic_cast<IEvent*>(this));
			AncestorType::pEmpty = false;
			// unlocking
		}
	}


	template<typename P>
	template<class U>
	void Event<P>::remove(const U* object)
	{
		if (object)
		{
			const IEventObserverBase* base = dynamic_cast<const IEventObserverBase*>(object);
			if (base)
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				if (!AncestorType::pBindList.empty())
				{
					typedef Private::EventImpl::template
						PredicateRemoveObserverBase<typename AncestorType::BindType> RemoveType;
					AncestorType::pBindList.remove(RemoveType(dynamic_cast<IEvent*>(this), base));
					AncestorType::pEmpty = AncestorType::pBindList.empty();
				}
				// unlocking
			}
			else
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				if (!AncestorType::pBindList.empty())
				{
					typedef Private::EventImpl::template
						PredicateRemoveObject<typename AncestorType::BindType> RemoveType;
					AncestorType::pBindList.remove(RemoveType(object));
					AncestorType::pEmpty = AncestorType::pBindList.empty();
				}
				// unlocking
			}
		}
	}


	template<typename P>
	void Event<P>::unregisterObserver(const IEventObserverBase* pointer)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		// When unregistering an observer, we have to remove it without any more checks
		if (!AncestorType::pBindList.empty())
		{
			typedef Private::EventImpl::template
				PredicateRemoveWithoutChecks<typename AncestorType::BindType> RemoveType;
			AncestorType::pBindList.remove(RemoveType(pointer));
		}
		AncestorType::pEmpty = AncestorType::pBindList.empty();
	}


	template<typename P>
	inline Event<P>& Event<P>::operator = (const NullPtr&)
	{
		clear();
		return *this;
	}


	template<typename P>
	inline Event<P>& Event<P>::operator = (const Event<P>& rhs)
	{
		AncestorType::assign(rhs);
		return *this;
	}


	template<typename P>
	inline bool Event<P>::operator ! () const
	{
		return (AncestorType::pEmpty);
	}


	template<typename P>
	inline bool Event<P>::empty() const
	{
		return (AncestorType::pEmpty);
	}


	template<typename P>
	inline bool Event<P>::notEmpty() const
	{
		return !(AncestorType::pEmpty);
	}





} // namespace Yuni

#endif // __YUNI_CORE_EVENT_EVENT_HXX__
