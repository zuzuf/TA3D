#ifndef __YUNI_CORE_ATOMIC_INT_H__
# define __YUNI_CORE_ATOMIC_INT_H__

# include "../../yuni.h"
# include "../../thread/policy.h"
# include "../system/windows.hdr.h"
# include "traits.h"



namespace Yuni
{
namespace Atomic
{


	/*!
	** \brief An atomic scalar type
	**
	** An atomic scalar value is a value that may be updated atomically (means
	** without the use of a mutex).
	**
	** \code
	** Atomic::Int<32> i;
	** ++i;
	** \endcode
	**
	** \tparam Size Size (in bits) of the scalar type (16 or 32 or 64)
	** \tparam TP A threading policy to guarantee thread-safety or not
	*/
	template<
		int Size = 8 * sizeof(int), /* 32 or 64Bits */          // Size in Bits of the scalar type
		template<class> class TP = Policy::ObjectLevelLockable  // The threading policy
		>
	class Int YUNI_ATOMIC_INHERITS
	{
	public:
		enum
		{
			//! Get if the class must be thread-safe
			threadSafe = TP<Int<Size,TP> >::threadSafe,
		};
		enum
		{
			//! Get if we have to guarantee ourselves the thread-safety
			mustUseMutex = YUNI_ATOMIC_MUST_USE_MUTEX,
		};

		/*!
		** \brief The Threading Policy
		**
		** The threading policy will be Policy::SingleThreaded in all cases, except
		** when the compiler or the operating system can not provide methods
		** on atomic scalar types. It is the case when the version of gcc is < 4.1
		** for example.
		*/
		typedef typename Private::AtomicImpl::ThreadingPolicy<threadSafe,Int<Size,TP> >::Type  ThreadingPolicy;
		//! The scalar type
		typedef typename Private::AtomicImpl::TypeFromSize<Size>::Type  Type;
		//! The scalar type
		typedef Type  ScalarType;

		enum
		{
			//! Size (in bits) of the scalar type
			size = Private::AtomicImpl::TypeFromSize<Size>::size
		};

		/*!
		** \brief Type of the inner variable
		**
		** Most of the time the keyword `volatile` is required (to avoid dangerous
		** optimizations by the compiler), except when there is no need for
		** thread-safety or when a mutex is used.
		*/
		typedef typename Private::AtomicImpl::Volatile<threadSafe,Type>::Type InnerType;

	public:
		//! \name Constructors
		//@{
		/*!
		** \brief Default Constructor
		*/
		Int();
		/*!
		** \brief Constructor with an initial value (int16)
		*/
		Int(const sint16 v);
		/*!
		** \brief Constructor with an initial value (int32)
		*/
		Int(const sint32 v);
		/*!
		** \brief Constructor with an initial value (int64)
		*/
		Int(const sint64 v);

		/*!
		** \brief Copy constructor
		*/
		Int(const Int& v);
		/*!
		** \brief Copy constructor from another type and another threading policy
		*/
		template<int Size2, template<class> class TP2> Int(const Int<Size2,TP2>& v);
		//@}


		//! \name Operators
		//@{
		//! Pre increment operator
		ScalarType operator ++ ();
		//! Pre decrement operator
		ScalarType operator -- ();
		//! Post increment operator
		ScalarType operator ++ (int);
		//! Post decrement operator
		ScalarType operator -- (int);

		Int& operator = (const ScalarType v);

		//! Increment
		Int& operator += (const ScalarType v);
		//! Decrement
		Int& operator -= (const ScalarType v);

		//! Cast operator
		operator ScalarType () const;

		//! not
		bool operator ! () const;
		//@}

	private:
		//! The real variable
		InnerType pValue;
		// A friend !
		template<int Size2, template<class> class TP2> friend struct Private::AtomicImpl::Operator;

	}; // class Int




} // namespace Atomic
} // namespace Yuni

# include "int.hxx"
# include "customstring.hxx"

#endif // __YUNI_CORE_ATOMIC_INT_H__
