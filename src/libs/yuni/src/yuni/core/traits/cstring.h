#ifndef __YUNI_CORE_TRAITS_C_STRING_H__
# define __YUNI_CORE_TRAITS_C_STRING_H__

# include "../../yuni.h"
# include "../static/remove.h"


namespace Yuni
{
namespace Extension
{

	/*!
	** \brief Extension: Convert any type into a mere C-String
	*/
	template<class C> class IntoCString
	{
	public:
		enum { valid = 0, converted = 0, zeroTerminated = 0, };

	public:
		template<class U> static const char* Perform(const U&) {return NULL;}

	};

} // namespace Extension
} // namespace Yuni


namespace Yuni
{
namespace Traits
{

	/*!
	** \brief Traits: C-String (const char*) representation of an arbitrary contrainer
	**
	** \tparam U Any class. See the specialization of the class Yuni::Extension::IntoCString
	*/
	template<class U>
	class CString
	{
	public:
		//! The original type without its const qualifier
		typedef typename Static::Remove::Const<U>::Type Type;
		//! Extension
		typedef Extension::IntoCString<Type> ExtensionType;
		enum
		{
			//! A non-zero value if the specialization is valid
			valid = ExtensionType::valid,
			//! A non-zero value if the data have been converted
			converted = ExtensionType::converted,
			//! A non-zero value if the data is zero-terminated
			zeroTerminated = ExtensionType::zeroTerminated,
		};

	public:
		/*!
		** \brief Get a C-String representation of an arbitrary contrainer
		**
		** \internal The template T is here to manage some special cases with
		**   the const qualifier, especially when U = const char* const. Your compiler
		**   may complain about the following error :
		**   "invalid conversion from ‘const char* const’ to ‘char*’"
		** \param container A arbitrary container
		** \return The equivalent of a `const char*` pointer (can be NULL)
		*/
		template<class T>
		static const char* Perform(const T& t) { return ExtensionType::Perform(t); }

	}; // class CString<U>




} // namespace Traits
} // namespace Yuni

# include "extension/into-cstring.h"

#endif // __YUNI_CORE_TRAITS_C_STRING_H__
