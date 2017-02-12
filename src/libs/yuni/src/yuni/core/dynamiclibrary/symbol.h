#ifndef __YUNI_CORE_DYNAMICLIBRARY_SYMBOL_H__
# define __YUNI_CORE_DYNAMICLIBRARY_SYMBOL_H__
/*!
** Header for Yuni::DynamicLibrary::Symbol, a class for representing an exported
** symbol by a (shared) library
*/

# include "../../yuni.h"


namespace Yuni
{
namespace DynamicLibrary
{


	/*!
	** \brief Exported Symbol from a dynamic library
	*/
	class Symbol
	{
	public:
		//! Handle for a symbol
		typedef void* Handle;

	public:
		/*!
		** \brief Default constructor
		*/
		Symbol();

		/*!
		** \brief Constructor with a given handle
		*/
		Symbol(const Handle p);

		/*!
		** \brief Copy constructor
		*/
		Symbol(const Symbol& rhs);

		//! Get if the symbol is invalid
		bool null() const;

		//! Get if the symbol is valid
		bool valid() const;

		//! Copy operator
		Symbol& operator = (const Symbol& rhs);

		//! Copy operator
		Symbol& operator = (const Symbol::Handle hndl);

		/*!
		** \brief Get the handle of the symbol
		*/
		Handle ptr() const;


	private:
		//! Handle
		Handle pPtr;

	}; // class Symbol




} // namespace DynamicLibrary
} // namespace Yuni

# include "symbol.hxx"

#endif // __YUNI_CORE_DYNAMICLIBRARY_SYMBOL_H__
