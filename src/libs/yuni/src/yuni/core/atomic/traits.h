#ifndef __YUNI_CORE_ATOMIC_TRAITS_H__
# define __YUNI_CORE_ATOMIC_TRAITS_H__

# include "../static/if.h"
# ifdef YUNI_OS_MAC
#	include <libkern/OSAtomic.h>
# endif
# ifdef YUNI_OS_WINDOWS
#	include "../system/windows.hdr.h"
# endif


// Determine if we must use a mutex or not
# if defined(YUNI_OS_WINDOWS) || YUNI_OS_GCC_VERSION >= 40102
#	if !defined(YUNI_OS_WINDOWS) && !defined(YUNI_HAS_SYNC_ADD_AND_FETCH)
#		define YUNI_ATOMIC_MUST_USE_MUTEX 1
#	else
#		define YUNI_ATOMIC_MUST_USE_MUTEX 0
#	endif
# else
#	define YUNI_ATOMIC_MUST_USE_MUTEX 1
# endif

# if YUNI_ATOMIC_MUST_USE_MUTEX == 1
#	define YUNI_ATOMIC_INHERITS  : public TP<Int<Size,TP> >
# else
#	define YUNI_ATOMIC_INHERITS
# endif



namespace Yuni
{
namespace Atomic
{

	// Forward declaration
	template<int Size, template<class> class TP> class Int;


} // namespace Atomic
} // namespace Yuni



namespace Yuni
{
namespace Private
{
namespace AtomicImpl
{


	template<int ThreadSafe, class C>
	struct ThreadingPolicy
	{
		# if YUNI_ATOMIC_MUST_USE_MUTEX == 1
		// If the class must be thread-safe, we have to provide a lock
		// mecanism to ensure thread-safety
		typedef typename Static::If<ThreadSafe,
			Policy::ObjectLevelLockable<C>, Policy::SingleThreaded<C> >::ResultType Type;
		# else
		// No lock is required, the operating system or the compiler already
		// provides all we need
		typedef Policy::SingleThreaded<C> Type;
		# endif

	}; // class ThreadingPolicy




	template<int ThreadSafe, class T>
	struct Volatile
	{
		# if YUNI_ATOMIC_MUST_USE_MUTEX == 1
		// We have to use our own mutex, we don't care of the volatile keyword
		typedef T Type;
		# else
		// The operating system or the compiler already provides methods
		// to deal with atomic types. However the volatile keyword is
		// required to avoid dangerous optimizations by the compiler
		// when the class must be thread-safe (to avoid cache-optimisations
		// SMP processors for example)
		typedef typename Static::If<ThreadSafe, volatile T, T>::ResultType Type;
		# endif

	}; // class Volatile




	template<int Size>
	struct TypeFromSize {};


	// Int16
	template<> struct TypeFromSize<16>
	{
		// On OS X, there are only routines for int32_t and int64_t
		// With MinGW, it simply does not exist
		// It seems that the best solution is to use int32 everywhere
		enum { size = 32 };
		typedef sint32 Type;
	};

	// Int32
	template<> struct TypeFromSize<32>
	{
		enum { size = 32 };
		typedef sint32 Type;
	};

	// Int64
	template<> struct TypeFromSize<64>
	{
		enum { size = 64 };
		typedef sint64 Type;
	};




	// Thread-safe operations
	template<int Size, template<class> class TP>
	struct Operator {};


	template<template<class> class TP>
	struct Operator<32, TP>
	{
		static typename Yuni::Atomic::Int<32,TP>::Type Increment(Yuni::Atomic::Int<32,TP>& t)
		{
			# ifdef YUNI_OS_WINDOWS
			return ::InterlockedIncrement((LONG*)&t.pValue);
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicIncrement32Barrier(&t.pValue);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<32,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (++t.pValue);
			#		else
			return __sync_add_and_fetch(&t.pValue, 1);
			#		endif
			#	endif
			# endif
		}

		static typename Yuni::Atomic::Int<32,TP>::Type Increment(Yuni::Atomic::Int<32,TP>& t, typename Yuni::Atomic::Int<32,TP>::ScalarType value)
		{
			# ifdef YUNI_OS_WINDOWS
			return InterlockedExchange((LONG*)&t.pValue, (LONG)(t.pValue + value));
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicAdd32Barrier(value, &t.pValue);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<32,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (t.pValue += value);
			#		else
			return __sync_add_and_fetch(&t.pValue, value);
			#		endif
			#	endif
			# endif
		}

		static typename Yuni::Atomic::Int<32,TP>::Type Decrement(Yuni::Atomic::Int<32,TP>& t)
		{
			# ifdef YUNI_OS_WINDOWS
			#   ifdef YUNI_OS_MINGW
			return ::InterlockedDecrement((LONG*)&t.pValue);
			#   else
			return _InterlockedDecrement((LONG*)&t.pValue);
			#   endif
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicDecrement32Barrier(&t.pValue);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<32,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (--t.pValue);
			#		else
			return __sync_add_and_fetch(&t.pValue, -1);
			#		endif
			#	endif
			# endif
		}

		static typename Yuni::Atomic::Int<32,TP>::Type Decrement(Yuni::Atomic::Int<32,TP>& t, typename Yuni::Atomic::Int<32,TP>::ScalarType value)
		{
			# ifdef YUNI_OS_WINDOWS
			return InterlockedExchange((LONG*)&t.pValue, (LONG)(t.pValue - value));
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicAdd32Barrier(&t.pValue, -value);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<32,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (t.pValue -= value);
			#		else
			return __sync_add_and_fetch(&t.pValue, -value);
			#		endif
			#	endif
			# endif
		}

	}; // class Operator<32, TP>



	template<template<class> class TP>
	struct Operator<64, TP>
	{
		static typename Yuni::Atomic::Int<64,TP>::Type Increment(Yuni::Atomic::Int<64,TP>& t)
		{
			# ifdef YUNI_OS_WINDOWS
			#	ifdef YUNI_OS_MINGW
			YUNI_STATIC_ASSERT(false, AtomicOperator_NotImplementedWithMinGW);
			#	else
			return _InterlockedIncrement64((LONGLONG*)&t.pValue);
			#	endif
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicIncrement64Barrier(&t.pValue);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<64,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (++t.pValue);
			#		else
			return __sync_add_and_fetch(&t.pValue, 1);
			#		endif
			#	endif
			# endif
		}

		static typename Yuni::Atomic::Int<64,TP>::Type Increment(Yuni::Atomic::Int<64,TP>& t, typename Yuni::Atomic::Int<64,TP>::ScalarType value)
		{
			# ifdef YUNI_OS_WINDOWS
			#	ifdef YUNI_OS_MINGW
			YUNI_STATIC_ASSERT(false, AtomicOperator_NotImplementedWithMinGW);
			#	else
			return InterlockedExchange64((LONGLONG*)&t.pValue, (LONGLONG)(t.pValue + value));
			#   endif
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicAdd64Barrier(value, &t.pValue);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<64,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (t.pValue += value);
			#		else
			return __sync_add_and_fetch(&t.pValue, value);
			#		endif
			#	endif
			# endif
		}

		static typename Yuni::Atomic::Int<64,TP>::Type Decrement(Yuni::Atomic::Int<64,TP>& t)
		{
			# ifdef YUNI_OS_WINDOWS
			#	ifdef YUNI_OS_MINGW
			YUNI_STATIC_ASSERT(false, AtomicOperator_NotImplementedWithMinGW);
			#	else
			return _InterlockedDecrement64((LONGLONG*)&t.pValue);
			#	endif
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicDecrement64Barrier(&t.pValue);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<64,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (--t.pValue);
			#		else
			return __sync_add_and_fetch(&t.pValue, -1);
			#		endif
			#	endif
			# endif
		}

		static typename Yuni::Atomic::Int<64,TP>::Type Decrement(Yuni::Atomic::Int<64,TP>& t, typename Yuni::Atomic::Int<64,TP>::ScalarType value)
		{
			# ifdef YUNI_OS_WINDOWS
			#	ifdef YUNI_OS_MINGW
			YUNI_STATIC_ASSERT(false, AtomicOperator_NotImplementedWithMinGW);
			#	else
			return InterlockedExchange64((LONGLONG*)&t.pValue, (LONGLONG)(t.pValue - value));
			#   endif
			# else
			#	ifdef YUNI_OS_MAC
			return ::OSAtomicAdd64Barrier(&t.pValue, -value);
			#	else
			#		if YUNI_ATOMIC_MUST_USE_MUTEX == 1
			typename Yuni::Atomic::Int<64,TP>::ThreadingPolicy::MutexLocker locker(t);
			return (t.pValue -= value);
			#		else
			return __sync_add_and_fetch(&t.pValue, -value);
			#		endif
			#	endif
			# endif
		}

	}; // class Operator<32, TP>





} // namespace AtomicImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_ATOMIC_TRAITS_H__
