#ifndef __YUNI_CORE_SMARTPTR_POLICIES_STORAGE_H__
# define __YUNI_CORE_SMARTPTR_POLICIES_STORAGE_H__

/*!
** \file
** \brief Storage policies
*/


namespace Yuni
{
namespace Policy
{

/*!
** \brief Storage policies
** \ingroup Policies
*/
namespace Storage
{



	/*!
	** \brief Implementation of the default storage policy
	** \ingroup Policies
	**
	** \tparam T The data type
	*/
	template<class T>
	class Pointer
	{
	public:
		//! The type stored
		typedef T* StoredType;
		//! The type used by the operator ->
		typedef T* PointerType;
		//! The type used by the operator *
		typedef T& ReferenceType;

		//! The default value for this type
		static StoredType DefaultValue() {return NULL;}

	public:
		//! \name Constructors
		//@{
		Pointer() :pData(DefaultValue()) {}
		Pointer(const Pointer&) :pData(0) {}
		template<class U> Pointer(const Pointer<U>&) :pData(0) {}
		Pointer(const StoredType& p) :pData(p) {}
		//@}

		//! Swap the data
		void swapPointer(Pointer& rhs) {std::swap(pData, rhs.pData);}

		//! \name Get the data
		//@{
		friend PointerType storagePointer(const Pointer& rhs) {return rhs.pData;}
		friend StoredType& storageReference(Pointer& rhs) {return rhs.pData;}
		friend const StoredType& storageReference(const Pointer& rhs) {return rhs.pData;}
		//@}

		//! \name Operators
		//@{
		//! The operator ->
		PointerType   operator -> () const {return pData;}
		//! The operator *
		ReferenceType operator * () const {return *pData;}
		//@}

	protected:
		//! Destroy the inner data
		void destroy() { delete pData; }

	private:
		//! The data
		StoredType pData;

	}; // class Pointer





	/*!
	** \brief Implementation of the Array storage policy
	** \ingroup Policies
	**
	** \tparam T The data type
	*/
	template<class T>
	class Array
	{
	public:
		//! The type stored
		typedef T* StoredType;
		//! The type used by the operator ->
		typedef T* PointerType;
		//! The type used by the operator *
		typedef T& ReferenceType;

		//! The default value for this type
		static StoredType DefaultValue() {return NULL;}

	public:
		//! \name Constructors
		//@{
		Array() :pData(DefaultValue()) {}
		Array(const Array&) :pData(0) {}
		template<class U> Array(const Pointer<U>&) :pData(0) {}
		Array(const StoredType& p) :pData(p) {}
		//@}

		//! Swap the data
		void swapPointer(Array& rhs) {std::swap(pData, rhs.pData);}

		//! \name Get the data
		//@{
		friend PointerType storagePointer(const Array& rhs) {return rhs.pData;}
		friend StoredType& storageReference(Array& rhs) {return rhs.pData;}
		friend const StoredType& storageReference(const Array& rhs) {return rhs.pData;}
		//@}

		//! \name Operators
		//@{
		//! The operator ->
		PointerType   operator -> () const {return pData;}
		//! The operator *
		ReferenceType operator * () const {return *pData;}
		//@}

	protected:
		//! Destroy the inner data
		void destroy() { delete[] pData; }

	private:
		//! The data
		StoredType pData;

	}; // class Array





} // namespace Storage
} // namespace Policy
} // namespace Yuni

#endif // __YUNI_CORE_SMARTPTR_POLICIES_STORAGE_H__
