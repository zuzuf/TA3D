#ifndef __YUNI_CORE_EVENT_OBSERVER_HXX__
# define __YUNI_CORE_EVENT_OBSERVER_HXX__

# include <cassert>


namespace Yuni
{
namespace Event
{


	template<class D, template<class> class TP>
	inline Observer<D,TP>::Observer()
		:pCanObserve(true)
	{}


	template<class D, template<class> class TP>
	inline Observer<D,TP>::~Observer()
	{
		// The derived parent class must call the method `destroyingObserver()`
		// from its destructor
		assert((pCanObserve == false)
			&& "All ancestor of the class `Yuni::Observer` must call destroyingObserver() in their destructor");
	}


	template<class D, template<class> class TP>
	void
	Observer<D,TP>::destroyingObserver()
	{
		// Lock
		typename ThreadingPolicy::MutexLocker locker(*this);
		// Prevent against further connection attempts
		pCanObserve = false;
		// Disconnecting from all events
		if (!pEvents.empty())
		{
			const IEvent::List::iterator end = pEvents.end();
			for (IEvent::List::iterator i = pEvents.begin(); i != end; ++i)
				(*i)->internalDetachObserver(this);
			pEvents.clear();
		}
	}


	template<class D, template<class> class TP>
	void
	Observer<D,TP>::disconnectAllEventEmitters()
	{
		// Disconnecting from all events
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (!pEvents.empty())
		{
			const IEvent::List::iterator end = pEvents.end();
			for (IEvent::List::iterator i = pEvents.begin(); i != end; ++i)
				(*i)->internalDetachObserver(this);
			pEvents.clear();
		}
	}

	template<class D, template<class> class TP>
	void
	Observer<D,TP>::disconnectEvent(const IEvent* event)
	{
		if (event)
		{
			// Lock
			typename ThreadingPolicy::MutexLocker locker(*this);
			// Disconnecting from the event
			if (!pEvents.empty() && IEvent::RemoveFromList(pEvents, event))
				event->internalDetachObserver(this);
		}
	}


	template<class D, template<class> class TP>
	void
	Observer<D,TP>::internalAttachEvent(IEvent* evt)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (pCanObserve && !IEvent::Exists(pEvents, evt))
			pEvents.push_back(evt);
	}


	template<class D, template<class> class TP>
	inline void
	Observer<D,TP>::internalDetachEvent(const IEvent* evt)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		IEvent::RemoveFromList(pEvents, evt);
	}




} // namespace Event
} // namespace Yuni

#endif // __YUNI_CORE_EVENT_OBSERVER_HXX__
