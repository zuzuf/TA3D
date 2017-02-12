#ifndef __YUNI_CORE_SYSTEM_USERNAME_H__
# define __YUNI_CORE_SYSTEM_USERNAME_H__

# include "../../yuni.h"


namespace Yuni
{
namespace System
{


	/*!
	** \brief Retrieves the calling user's name
	**
	** On Unixes, the value will be read from the environment variables
	** `LOGNAME`. On Windows, the method GetUserName will be used.
	**
	** Windows (from MSDN):
	** If the current thread is impersonating another client, the GetUserName
	** function returns the user name of the client that the thread is impersonating.
	** \see http://msdn.microsoft.com/en-us/library/ms724432%28v=vs.85%29.aspx
	**
	** \param out Variable of type 'string' where the value will be appened
	** \param emptyBefore True to empty the parameter `out` before
	** \return True if the operation succeeded (a valid username has been found), false otherwise
	*/
	template<class StringT> bool Username(StringT& out, bool emptyBefore = true);




} // namespace System
} // namespace Yuni

# include "username.hxx"

#endif // __YUNI_CORE_SYSTEM_USERNAME_H__
