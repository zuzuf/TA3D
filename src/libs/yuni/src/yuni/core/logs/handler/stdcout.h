#ifndef __YUNI_CORE_LOGS_HANDLERS_STD_COUT_H__
# define __YUNI_CORE_LOGS_HANDLERS_STD_COUT_H__

# include "../null.h"
# include <iostream>


# if defined(YUNI_OS_WINDOWS)
#	define YUNI_LOGS_UNIX_COLORS_ALLOWED  0
# else
#	define YUNI_LOGS_UNIX_COLORS_ALLOWED  1
# endif


namespace Yuni
{
namespace Logs
{



	/*!
	** \brief Log Handler: The standard output (cout & cerr)
	*/
	template<class NextHandler = NullHandler>
	class StdCout : public NextHandler
	{
	public:
		enum Settings
		{
			unixColorsAllowed = YUNI_LOGS_UNIX_COLORS_ALLOWED,
		};

	public:
		template<class LoggerT, class VerbosityType, class StringT>
		void internalDecoratorWriteWL(LoggerT& logger, const StringT& s)
		{
			typedef typename LoggerT::DecoratorsType DecoratorsType;

			// Write the message to the std::cout/cerr
			if (VerbosityType::shouldUsesStdCerr)
			{
				logger.DecoratorsType::template internalDecoratorAddPrefix<StdCout, VerbosityType>(std::cerr, s);
				// Flush
				std::cerr << std::endl;
			}
			else
			{
				logger.DecoratorsType::template internalDecoratorAddPrefix<StdCout, VerbosityType>(std::cout, s);
				// Flush
				std::cout << std::endl;
			}

			// Transmit the message to the next handler
			NextHandler::template internalDecoratorWriteWL<LoggerT, VerbosityType, StringT>(logger, s);
		}

	}; // class StdCout




} // namespace Logs
} // namespace Yuni

#endif // __YUNI_CORE_LOGS_HANDLERS_STD_COUT_H__
