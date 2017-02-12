#ifndef __YUNI_CORE_ANY_ANY_H__
#define __YUNI_CORE_ANY_ANY_H__

# include "../../yuni.h"
# include "../exceptions/badcast.h"
# include "../string.h"
# include "any.private.h"



namespace Yuni
{

	/*!
	** \brief Classic Any type implementation.
	** \ingroup Core
	**
	** This is a relatively classic, yet (i hope) fast implementation of
	** a Any type.
	**
	** How to use:
	** \code
	** Any v(3.14);
	**
	** if (v.is<double>())
	**   // Be sure to check the type before, otherwise Any will throw an exception.
	**   std::cout << v.cast<double>();
	** \endcode
	*/
	class Any
	{
	public:
		/*!
		** \brief Swaps a any with another
		** \param[in,out] one Any 1
		** \param[in,out] other Any 2
		*/
		static void Swap(Any& one, Any& other);


	public:
		//! \name Constructors
		//@{

		//! Copy of an existing variable
		template <typename T>
		Any(const T& source);

		//! Copy of an existing const C string
		Any(const char* source)
		{ initFromCString(source); }

		//! Copy of an existing const C string
		Any(char* source)
		{ initFromCString(source); }

		//! Empty constructor
		Any();

		//! Copy constructor
		Any(const Any& rhs);

		//! Destructor
		~Any();

		//@}


		//! \name Alteration methods
		//@{

		/*!
		** \brief Assignment from another Any
		** \param[in] rhs The Any to assign from
		** \return This Any
		*/
		Any& assign(const Any& rhs);

		/*!
		** \brief Assignment from any object
		** \param[in] rhs The object to assign from
		** \return This Any
		*/
		template <typename T>
		Any& assign(const T& rhs);

		/*!
		** \brief Specialized assign for C Strings.
		*/
		Any& assign(const char* rhs)
		{ return assign<String>(rhs); }

		/*!
		** \brief Assignment operator for convenience
		*/
		template <typename T>
		Any& operator = (T const& rhs) {return assign(rhs);}

		/*!
		** \brief Resets the Any to an empty state.
		*/
		void reset();

		//@}

		//! \name Information methods
		//@{

		/*!
		** \brief Returns the type_info of the held variable.
		**
		** Can be used to compare with typeid(MyType).
		*/
		const std::type_info& type() const {return pTable->type();}


		/*!
		** \brief Returns true if the object is of the specified type
		*/
		template <typename T>
		bool is() const {return type() == typeid(T);}


		/*!
		** \brief Checks if the any has been assigned a value.
		** \return True if the any contains no value.
		*/
		bool empty() const
		{ return pTable == Private::Any::Table<Private::Any::Empty>::Get(); }

		//@}

		//! \name Retrieval methods
		//@{

		/*!
		** \brief Casts the Any to the T type. Throws a Yuni::Exceptions::BadCast
		** if not possible.
		** \return A T object.
		*/
		template <typename T>
		const T& to() const;

		//@}

	private:

		//! Special initializer for C Strings copys
		template <typename T>
		void initFromCString(T source);

	private:
		//! Static function pointer table storage
		Private::Any::TypeManipulationTable * pTable;

		//! Object storage.
		void* pObject;
	};




} // namespace Yuni

# include "any.hxx"

#endif /* !__YUNI_CORE_ANY_ANY_H__ */


