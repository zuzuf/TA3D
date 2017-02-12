#ifndef __YUNI_POLICIES_THREADS_H__
# define __YUNI_POLICIES_THREADS_H__

/*!
** \file
** \brief Threading policies
*/

# include "../yuni.h"
# ifdef YUNI_OS_WINDOWS
#	include "../core/system/windows.hdr.h"
# endif
# include "mutex.h"



namespace Yuni
{
namespace Policy
{

	/*!
	** \name Threading policies
	** \ingroup Policies
	*/
	//@{

	/*!
	** \brief Implementation of the Threading Model policy in a single-threaded environnement
	** \ingroup Policies
	**
	** No synchronization is required in this case
	*/
	template<class Host>
	class SingleThreaded
	{
	public:
		/*!
		** \brief Locks a mutex in the constructor and unlocks it in the destructor.
		*/
		class MutexLocker
		{
		public:
			MutexLocker() {}
			template<class C> MutexLocker(const C&) {}
			~MutexLocker() {}
		}; // class MutexLocker


		/*!
		** \brief A volative type
		*/
		template<typename U> struct Volatile { typedef U Type; };

		//! Get if this policy is thread-safe
		enum { threadSafe = false };

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		SingleThreaded() {}
		//! Copy constructor
		SingleThreaded(const SingleThreaded&) {}
		//! Destructor
		~SingleThreaded() {}
		//@}

	}; // class SingleThreaded




	/*!
	** \brief Implementation of the Threading Model policy in a multi-threaded environnement (one mutex per object)
	** \ingroup Policies
	*/
	template<class Host>
	class ObjectLevelLockable
	{
	public:
		/*!
		** \brief Locks a mutex in the constructor and unlocks it in the destructor.
		*/
		class MutexLocker
		{
		public:
			template<class C> MutexLocker(const C& h)
				:pHostToLock(static_cast<const Host&>(h))
			{
				pHostToLock.pMutex.lock();
			}

			~MutexLocker()
			{
				pHostToLock.pMutex.unlock();
			}

		private:
			const Host& pHostToLock;
		}; // class MutexLocker

		/*!
		** \brief A volative type
		*/
		template<typename U> struct Volatile { typedef volatile U Type; };

		//! Get if this policy is thread-safe
		enum { threadSafe = true };

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		ObjectLevelLockable() :pMutex(true) {}
		//! Copy constructor
		ObjectLevelLockable(const ObjectLevelLockable&) :pMutex(true) {}
		//! Destructor
		~ObjectLevelLockable() {}
		//@}

	protected:
		//! Mutex
		mutable Mutex pMutex;

	}; // class ObjectLevelLockable



	/*!
	** \brief Implementation of the Threading Model policy in a multi-threaded environnement (one mutex per object, not recursive)
	** \ingroup Policies
	*/
	template<class Host>
	class ObjectLevelLockableNotRecursive
	{
	public:
		/*!
		** \brief Locks a mutex in the constructor and unlocks it in the destructor.
		*/
		class MutexLocker
		{
		public:
			template<class C> MutexLocker(const C& h)
				:pHostToLock(static_cast<const Host&>(h))
			{
				pHostToLock.pMutex.lock();
			}

			~MutexLocker()
			{
				pHostToLock.pMutex.unlock();
			}

		private:
			const Host& pHostToLock;
		}; // class MutexLocker

		/*!
		** \brief A volative type
		*/
		template<typename U> struct Volatile { typedef volatile U Type; };

		//! Get if this policy is thread-safe
		enum { threadSafe = true };

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		ObjectLevelLockableNotRecursive() :pMutex(false) {}
		//! Copy constructor
		ObjectLevelLockableNotRecursive(const ObjectLevelLockableNotRecursive&) :pMutex(false) {}
		//! Destructor
		~ObjectLevelLockableNotRecursive() {}
		//@}

	protected:
		//! Mutex
		mutable Mutex pMutex;

	}; // class ObjectLevelLockableNotRecursive




	/*!
	** \brief Implementation of the Threading Model policy in a multi-threaded environnement (one mutex per class)
	** \ingroup Policies
	*/
	template<class Host>
	class ClassLevelLockable
	{
	public:
		/*!
		** \brief Locks a mutex in the constructor and unlocks it in the destructor.
		*/
		class MutexLocker
		{
		public:
			MutexLocker()
			{
				Mutex::ClassLevelLockable<Host>::mutex.lock();
			}
			MutexLocker(const MutexLocker&) {}
			template<class C> MutexLocker(const C&)
			{
				Mutex::ClassLevelLockable<Host>::mutex.lock();
			}
			~MutexLocker()
			{
				Mutex::ClassLevelLockable<Host>::mutex.unlock();
			}
		}; // class MutexLocker

		/*!
		** \brief A volative type
		*/
		template<typename U> struct Volatile { typedef volatile U Type; };

		//! Get if this policy is thread-safe
		enum { threadSafe = true };

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		ClassLevelLockable() :pMutex(Mutex::ClassLevelLockable<Host>::mutex) {}
		//! Copy constructor
		ClassLevelLockable(const ClassLevelLockable&) :pMutex(Mutex::ClassLevelLockable<Host>::mutex) {}
		//! Destructor
		~ClassLevelLockable() {}
		//@}

	private:
		//! Mutex
		Mutex& pMutex;

	}; // class ClassLevelLockable



	//@} // Threading Policies




} // namespace Policy
} // namespace Yuni

#endif // __YUNI_POLICIES_THREADS_H__
