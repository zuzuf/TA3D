#ifndef __YUNI_CORE_SMARTPTR_POLICIES_OWNERSHIP_H__
# define __YUNI_CORE_SMARTPTR_POLICIES_OWNERSHIP_H__

/*!
** \file
** \brief Ownership policies
**
** All available ownership policies :
**
** Policy::Ownership::ReferenceCounted<T>
** Policy::Ownership::ReferenceCountedMT<T>
** Policy::Ownership::COMReferenceCounted<T>
** Policy::Ownership::DestructiveCopy<T>
** Policy::Ownership::NoCopy<T>
*/


# include "../../../thread/policy.h"
# include "../../static/assert.h"
# include "../../atomic/int.h"



namespace Yuni
{
namespace Policy
{

/*!
** \brief Ownership policies
** \ingroup Policies
*/
namespace Ownership
{


	/*!
	** \brief Implementation of the reference counting ownership policy
	** \ingroup Policies
	*/
	template<class T>
	class ReferenceCounted
	{
	public:
		enum
		{
			//! Get if the ownership policy is destructive
			destructiveCopy = false
		};

	public:
		//! \name Constructors
		//@{
		//! Default constructor
		ReferenceCounted()
			:pCount(new unsigned int(1))
		{}

		//! Copy constructor
		ReferenceCounted(const ReferenceCounted& c)
			:pCount(c.pCount)
		{}

		//! Copy constructor for any king of template parameter
		template<typename U> ReferenceCounted(const ReferenceCounted<U>& c)
			:pCount(reinterpret_cast<const ReferenceCounted&>(c).pCount)
		{}
		//@}

		/*!
		** \brief Initialization from a raw pointer
		*/
		void initFromRawPointer(const T&)
		{
		}

		/*!
		** \brief Clone this object
		** \param rhs The original object
		*/
		T clone(const T& rhs)
		{
			++(*pCount);
			return rhs;
		}

		/*!
		** \brief Release the reference
		*/
		bool release(const T&)
		{
			if (!(--(*pCount)))
			{
				delete pCount;
				pCount = NULL;
				return true;
			}
			return false;
		}

		void swapPointer(ReferenceCounted& rhs)
		{
			std::swap(pCount, rhs.pCount);
		}

	private:
		//! The reference count
		unsigned int* pCount;

	}; // class ReferenceCounted





	/*!
	** \brief Implementation of the COM intrusive reference counting ownership policy
	** \ingroup Policies
	*/
	template<class T>
	class COMReferenceCounted
	{
	public:
		enum
		{
			//! Get if the ownership policy is destructive
			destructiveCopy = false
		};

	public:
		//! \name Constructors
		//@{
		//! Default constructor
		COMReferenceCounted()
		{}
		//! Copy constructor for any king of template parameter
		template<typename U> COMReferenceCounted(const COMReferenceCounted<U>&)
		{}
		//@}

		/*!
		** \brief Initialization from a raw pointer
		*/
		static void initFromRawPointer(const T& rhs)
		{
			clone(rhs);
		}

		/*!
		** \brief Clone this object
		** \param rhs The original object
		*/
		static T clone(const T& rhs)
		{
			if (0 != rhs)
				rhs->addRef();
			return rhs;
		}

		/*!
		** \brief Release the reference
		*/
		static bool release(const T& rhs)
		{
			if (0 != rhs)
				rhs->release();
			return false;
		}

		static void swapPointer(COMReferenceCounted&) {}

	}; // class COMReferenceCounted




	/*!
	** \brief Implementation of the thread-safe reference counting ownership policy
	** \ingroup Policies
	*/
	template<class T>
	class ReferenceCountedMT
	{
	public:
		enum
		{
			//! Get if the ownership policy is destructive
			destructiveCopy = false
		};
		typedef Atomic::Int<>  AtomicType;

	public:
		//! \name Constructors
		//@{
		//! Default constructor
		ReferenceCountedMT()
			:pCount(new AtomicType(1))
		{}
		//! Copy constructor
		ReferenceCountedMT(const ReferenceCountedMT& c)
		{
			pCount = c.pCount;
		}
		//! Copy constructor for any king of template parameter
		template<typename U> ReferenceCountedMT(const ReferenceCountedMT<U>& c)
		{
			pCount = reinterpret_cast<const ReferenceCountedMT<T>&>(c).pCount;
		}
		//@}

		/*!
		** \brief Initialization from a raw pointer
		*/
		void initFromRawPointer(const T&)
		{
		}

		/*!
		** \brief Clone this object
		** \param rhs The original object
		*/
		T clone(const T& rhs)
		{
			++(*pCount);
			return rhs;
		}

		/*!
		** \brief Release the reference
		*/
		bool release(const T&)
		{
			if (!(--(*pCount)))
			{
				delete pCount;
				pCount = NULL;
				return true;
			}
			return false;
		}

		void swapPointer(ReferenceCountedMT& rhs)
		{
			std::swap(pCount, rhs.pCount);
		}

	private:
		//! The reference count
		AtomicType* pCount;

	}; // class ReferenceCountedMT





	/*!
	** \brief Implementation of the destructive copy ownership policy
	** \ingroup Policies
	*/
	template<class T>
	class DestructiveCopy
	{
	public:
		enum
		{
			//! Get if the ownership policy is destructive
			destructiveCopy = true
		};

	public:
		//! \name Constructors
		//@{
		//! Default constructor
		DestructiveCopy()
		{}
		//! Copy constructor
		template<class U> DestructiveCopy(const DestructiveCopy<U>&)
		{}
		//@}

		/*!
		** \brief Initialization from a raw pointer
		*/
		void initFromRawPointer(const T&)
		{
		}

		/*!
		** \brief Clone this object
		** \param rhs The original object
		*/
		template<class U> static T clone(U& rhs)
		{
			T result(rhs);
			rhs = U();
			return result;
		}

		static bool release(const T&) {return true;}

		static void swapPointer(DestructiveCopy&) {}

	}; // class DestructiveCopy





	/*!
	** \brief Implementation of the no-copy ownership policy
	** \ingroup Policies
	*/
	template<class T>
	class NoCopy
	{
	public:
		enum
		{
			//! Get if the ownership policy is destructive
			destructiveCopy = false
		};

	public:
		//! \name Constructors
		//@{
		//! Default constructor
		NoCopy() {}
		//! Copy constructor
		template <class U> NoCopy(const NoCopy<U>&) {}
		//@}

		/*!
		** \brief Initialization from a raw pointer
		*/
		void initFromRawPointer(const T&)
		{
		}

		/*!
		** \brief Clone this object
		**
		** This method can not be called at compile time
		*/
		static T clone(const T&)
		{
			YUNI_STATIC_ASSERT(sizeof(T) == 0, OwnershipPolicyNoCopy);
		}

		static bool release(const T&) {return true;}

		static void swapPointer(NoCopy&) {}

	}; // class NoCopy




} // namespace Ownership
} // namespace Policy
} // namespace Yuni

#endif // __YUNI_CORE_SMARTPTR_POLICIES_OWNERSHIP_H__
