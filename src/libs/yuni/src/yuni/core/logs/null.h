#ifndef __YUNI_CORE_LOGS_NULL_H__
# define __YUNI_CORE_LOGS_NULL_H__

# include "../../yuni.h"
# include "../string.h"


namespace Yuni
{
namespace Logs
{


	/*!
	** \brief Log Handler: The Null device
	**
	** This handler does produce nothing and skip all messages
	*/
	class NullHandler
	{
	public:
		template<class LoggerT, class VerbosityType, class StringT>
		static void internalDecoratorWriteWL(LoggerT&, const StringT&)
		{
			/* Do nothing */
		}
	};


	/*!
	** \brief Log Handler: The Null device
	**
	** This handler does produce nothing and skip all messages
	*/
	class NullDecorator
	{
	public:
		template<class Handler, class VerbosityType, class O, class StringT>
		static void internalDecoratorAddPrefix(O&, const StringT&)
		{
			/* Do nothing */
		}

	};





} // namespace Logs
} // namespace Yuni


#endif // __YUNI_CORE_LOGS_NULL_H__
