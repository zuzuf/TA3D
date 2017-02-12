#ifndef __YUNI_CORE_EVENT_INTERFACES_HXX__
# define __YUNI_CORE_EVENT_INTERFACES_HXX__

# include "../../yuni.h"
# include <list>
# include <cassert>


namespace Yuni
{


	template<class Derived, template<class> class TP>
	inline IEventObserver<Derived,TP>::IEventObserver()
		:pBoundEventTable(new IEvent::Map())
	{}


	template<class Derived, template<class> class TP>
	inline IEventObserver<Derived,TP>::~IEventObserver()
	{
		// The derived class must remove the boumd events itself
		// to prevent race data and a corrupt vtable
		assert((NULL == pBoundEventTable)
			&& "IEventObserver: The derived class must call `destroyBoundEvents()` by itself to prevent a corrupt vtable when destroying the object");
	}


	template<class Derived, template<class> class TP>
	void IEventObserver<Derived,TP>::boundEventIncrementReference(IEvent* evt) const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (pBoundEventTable)
		{
			++((*pBoundEventTable)[evt]);
		}
	}


	template<class Derived, template<class> class TP>
	void IEventObserver<Derived,TP>::boundEventDecrementReference(IEvent* evt) const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (pBoundEventTable && !pBoundEventTable->empty())
		{
			IEvent::Map::iterator i = pBoundEventTable->find(evt);
			if (i != pBoundEventTable->end())
			{
				if (i->second <= 1)
				{
					// We won't keep an entry with no reference
					pBoundEventTable->erase(i);
				}
				else
				{
					// Decrementing the reference count
					--(i->second);
				}
			}
			else
			{
				assert(false && "Impossible to find IEvent");
			}
		}
		else
		{
			assert(false && "The table is empty");
		}
	}


	template<class Derived, template<class> class TP>
	void IEventObserver<Derived,TP>::boundEventRemoveFromTable(IEvent* evt) const
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (pBoundEventTable && !pBoundEventTable->empty())
		{
			IEvent::Map::iterator i = pBoundEventTable->find(evt);
			if (i != pBoundEventTable->end())
				pBoundEventTable->erase(i);
			else
			{
				# ifndef NDEBUG
				assert(0 && " -- Impossible to find the IEvent ");
				# endif
			}
		}
	}


	template<class Derived, template<class> class TP>
	void IEventObserver<Derived,TP>::destroyBoundEvents()
	{
		// The mutex must stay locked while we unregister all observers
		// to avoid race conditions in SMP processors (see previous versions)
		typename ThreadingPolicy::MutexLocker locker(*this);
		if (pBoundEventTable)
		{
			if (!pBoundEventTable->empty())
			{
				// Unlinking this observer to all events
				const IEvent::Map::iterator end = pBoundEventTable->end();
				for (IEvent::Map::iterator i = pBoundEventTable->begin(); i != end; ++i)
				{
					(i->first)->unregisterObserver(this);
				}
			}

			// We can now delete the table
			delete pBoundEventTable;
			// And to set it to NULL to avoid futur operations
			pBoundEventTable = NULL;
		}
	}




} // namespace Yuni

#endif // __YUNI_CORE_EVENT_INTERFACES_HXX__
