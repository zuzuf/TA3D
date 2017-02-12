#ifndef __YUNI_CORE_SINGLETON_SINGLETON_HXX__
# define __YUNI_CORE_SINGLETON_SINGLETON_HXX__

# include "../static/assert.h"


namespace Yuni
{

	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	typename Singleton<T, CreationT, LifetimeT, ThreadingT>::VolatilePtr
	Singleton<T, CreationT, LifetimeT, ThreadingT>::pInstance = NULL;


	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	bool Singleton<T, CreationT, LifetimeT, ThreadingT>::pDestroyed = false;







	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	inline Singleton<T, CreationT, LifetimeT, ThreadingT>::Singleton()
	{}


	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	Singleton<T, CreationT, LifetimeT, ThreadingT>::Singleton(const Singleton&)
	{
		YUNI_STATIC_ASSERT(false, COPY_OPERATOR_ON_SINGLETON_IS_FORBIDDEN);
	}

	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	Singleton<T, CreationT, LifetimeT, ThreadingT>&
	Singleton<T, CreationT, LifetimeT, ThreadingT>::operator = (const Singleton&)
	{
		YUNI_STATIC_ASSERT(false, SINGLETON_ASSIGNMENT_IS_FORBIDDEN);
	}

	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	Singleton<T, CreationT, LifetimeT, ThreadingT>*
	Singleton<T, CreationT, LifetimeT, ThreadingT>::operator & ()
	{
		YUNI_STATIC_ASSERT(false, ADDRESS_OF_OPERATOR_ON_SINGLETON_IS_FORBIDDEN);
	}

	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	const Singleton<T, CreationT, LifetimeT, ThreadingT>*
	Singleton<T, CreationT, LifetimeT, ThreadingT>::operator & () const
	{
		YUNI_STATIC_ASSERT(false, ADDRESS_OF_OPERATOR_ON_SINGLETON_IS_FORBIDDEN);
	}


	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	typename Singleton<T, CreationT, LifetimeT, ThreadingT>::Reference
	Singleton<T, CreationT, LifetimeT, ThreadingT>::Instance()
	{
		// Double-checked locking pattern
		// Avoids locking on each call, only tests twice on first call
		if (!pInstance)
		{
			typename ThreadingPolicy::MutexLocker lock;
			if (!pInstance)
			{
				// If the instance was destroyed, the lifetime policy
				// is responsible for what to do.
				if (pDestroyed)
				{
					// Manage dead reference
					LifetimePolicy::OnDeadReference();
					// Mark the instance as valid again (e.g. for Phoenix singleton)
					pDestroyed = false;
				}
				// Create the instance
				pInstance = CreationPolicy::Create();
				// Let the lifetime policy manage when to destroy
				LifetimePolicy::ScheduleDestruction(&DestroyInstance);
			}
		}
		return *pInstance;
	}



	template <typename T,
		template <class> class CreationT,
		template <class> class LifetimeT,
		template <class> class ThreadingT>
	void Singleton<T, CreationT, LifetimeT, ThreadingT>::DestroyInstance()
	{
		CreationPolicy::Destroy(pInstance);
		pInstance = NULL;
		pDestroyed = true;
	}





} // namespace Yuni

#endif // __YUNI_CORE_SINGLETON_SINGLETON_HXX__
