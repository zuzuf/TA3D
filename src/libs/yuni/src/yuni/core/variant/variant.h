#ifndef __YUNI_CORE_VARIANT_VARIANT_H__
# define __YUNI_CORE_VARIANT_VARIANT_H__

# include "../../yuni.h"
# include "../string.h"
# include "variant.private.h"
# include "../static/remove.h"



namespace Yuni
{

	/*!
	** \brief Variant type implementation.
	**
	** This is an implementation of Variants that handles
	** a limited range of types (classical ones), but that
	** can make automagic conversions between them, if such
	** a conversion is possible.
	**
	** It differs from Any, which can handle almost any type
	** (with the notable exception of arrays), but cannot
	** automatically convert from a type to another.
	**
	** How to use:
	** \code
	** Variant v(int(12));
	**
	** std::cout << v.to<float>() << std::endl;
	** std::cout << v.to<String>() << std::endl;
	** \endcode
	**
	** \ingroup Core
	*/
	class Variant
	{
	public:
		//! \name Constructors
		//@{
		//! Constructs an empty Variant
		Variant();
		//! Constructs a copy of an existing Variant.
		Variant(const Variant& rhs);
		//! Constructs a Variant based on an existing variable of simple type.
		template<typename T> Variant(const T& rhs);

		//! Destructor
		~Variant();
		//@}


		//! \name Alteration methods
		//@{
		//! Assignment from an existing Variant
		void assign(const Variant& rhs);

		//! Assignment from a simple type.
		template <typename T>
		void assign(const T& rhs);

		//! Specific assignment from C strings
		void assign(const char* rhs)
		{ assign(String(rhs)); }

		//! Resets the Variant to an empty one.
		void clear();
		//@}


		//! \name Information methods
		//@{
		//! Returns true if the Variant is empty.
		bool empty() const;
		//@}


		//! \name Operator overloads
		//@{
		Variant & operator = (const Variant& rhs);

		template <typename T>
		Variant & operator = (const T& rhs);
		//@}


		//! \name Retrieval methods
		//@{
		template<typename T> T to() const;
		//@}


	private:
		//! Pointer to storage object
		Private::Variant::AData* pData;

	}; // class Variant



} // namespace Yuni

# include "variant.hxx"


#endif // __YUNI_CORE_VARIANT_VARIANT_H__
