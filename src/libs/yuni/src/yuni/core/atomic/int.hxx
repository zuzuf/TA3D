#ifndef __YUNI_CORE_ATOMIC_INT_HXX__
# define __YUNI_CORE_ATOMIC_INT_HXX__


namespace Yuni
{
namespace Atomic
{


	template<int Size, template<class> class TP>
	inline Int<Size,TP>::Int()
		# if YUNI_ATOMIC_MUST_USE_MUTEX != 1
		:pValue()
		# else
		:TP<Int<Size,TP> >(), pValue()
		# endif
	{}


	template<int Size, template<class> class TP>
	Int<Size,TP>::Int(const sint16 v)
		# if YUNI_ATOMIC_MUST_USE_MUTEX != 1
		:pValue((ScalarType)v)
		# else
		:TP<Int<Size,TP> >(), pValue((ScalarType)v)
		# endif
	{}

	template<int Size, template<class> class TP>
	inline Int<Size,TP>::Int(const sint32 v)
		# if YUNI_ATOMIC_MUST_USE_MUTEX != 1
		:pValue((ScalarType)v)
		# else
		:TP<Int<Size,TP> >(), pValue((ScalarType)v)
		# endif
	{}


	template<int Size, template<class> class TP>
	inline Int<Size,TP>::Int(const sint64 v)
		# if YUNI_ATOMIC_MUST_USE_MUTEX != 1
		:pValue((ScalarType)v)
		# else
		:TP<Int<Size,TP> >(), pValue((ScalarType)v)
		# endif
	{}




	template<int Size, template<class> class TP>
	inline Int<Size,TP>::Int(const Int<Size,TP>& v)
		# if YUNI_ATOMIC_MUST_USE_MUTEX != 1
		:pValue((ScalarType)v)
		# else
		:TP<Int<Size,TP> >(), pValue((ScalarType)v.pValue)
		# endif
	{
	}

	template<int Size, template<class> class TP>
	template<int Size2, template<class> class TP2>
	inline Int<Size,TP>::Int(const Int<Size2,TP2>& v)
		# if YUNI_ATOMIC_MUST_USE_MUTEX == 1
		:TP<Int<Size,TP> >()
		# endif
	{
		# if YUNI_ATOMIC_MUST_USE_MUTEX == 1
		typename ThreadingPolicy::MutexLocker locker(*this);
		typename ThreadingPolicy::MutexLocker locker2(v);
		# endif
		pValue = (ScalarType)v.pValue;
	}


	template<int Size, template<class> class TP>
	inline Int<Size,TP>::operator ScalarType () const
	{
		return pValue;
	}


	template<int Size, template<class> class TP>
	inline typename Int<Size,TP>::ScalarType Int<Size,TP>::operator ++ ()
	{
		return (threadSafe)
			? Private::AtomicImpl::Operator<size,TP>::Increment(*this)
			: (++pValue);
	}


	template<int Size, template<class> class TP>
	inline typename Int<Size,TP>::ScalarType Int<Size,TP>::operator -- ()
	{
		return (threadSafe)
			? Private::AtomicImpl::Operator<size,TP>::Decrement(*this)
			: (--pValue);
	}


	template<int Size, template<class> class TP>
	inline typename Int<Size,TP>::ScalarType Int<Size,TP>::operator ++ (int)
	{
		return (threadSafe)
			? Private::AtomicImpl::Operator<size,TP>::Increment(*this) - 1
			: (pValue++);
	}


	template<int Size, template<class> class TP>
	inline typename Int<Size,TP>::ScalarType Int<Size,TP>::operator -- (int)
	{
		return (threadSafe)
			? Private::AtomicImpl::Operator<size,TP>::Decrement(*this) + 1
			: (pValue--);
	}


	template<int Size, template<class> class TP>
	inline bool Int<Size,TP>::operator ! () const
	{
		return !pValue;
	}


	template<int Size, template<class> class TP>
	inline Int<Size,TP>& Int<Size,TP>::operator = (const ScalarType v)
	{
		pValue = v;
		return *this;
	}


	template<int Size, template<class> class TP>
	inline Int<Size,TP>& Int<Size,TP>::operator += (const ScalarType v)
	{
		if (threadSafe)
			Private::AtomicImpl::Operator<size,TP>::Increment(*this, v);
		else
			pValue += v;
		return *this;
	}


	template<int Size, template<class> class TP>
	inline Int<Size,TP>& Int<Size,TP>::operator -= (const ScalarType v)
	{
		if (threadSafe)
			Private::AtomicImpl::Operator<size,TP>::Decrement(*this, v);
		else
			pValue -= v;
		return *this;
	}





} // namespace Atomic
} // namespace Yuni

#endif // __YUNI_CORE_ATOMIC_INT_HXX__
