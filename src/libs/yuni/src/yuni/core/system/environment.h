#ifndef __YUNI_CORE_SYSTEM_ENVIRONMENT_H__
# define __YUNI_CORE_SYSTEM_ENVIRONMENT_H__

# include "../../yuni.h"
# include "../string.h"


namespace Yuni
{
namespace System
{
namespace Environment
{

	/*!
	** \brief Get a value from the current environment
	**
	** \param name Name of the variable
	** \return The value of the variable, empty is an error has occured
	*/
	template<class StringT> String Read(const StringT& name);

	/*!
	** \brief Get a value from the current environment
	**
	** \param name Name of the variable
	** \param out Variable of type 'string'/'container' where the value will be appened
	** \param emptyBefore True to empty the parameter `out` before
	** \return True if the operation succeeded, false otherwise
	*/
	template<class StringT, class StringT2>
	bool Read(const StringT& name, StringT2& out, const bool emptyBefore = true);





} // namespace Environment
} // namespace System
} // namespace Yuni

# include "environment.hxx"

#endif // __YUNI_CORE_SYSTEM_ENVIRONMENT_H__
