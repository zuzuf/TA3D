#ifndef __YUNI_CORE_LOGS_HANDLERS_FILE_HXX__
# define __YUNI_CORE_LOGS_HANDLERS_FILE_HXX__



namespace Yuni
{
namespace Logs
{

	template<class NextHandler>
	template<typename U>
	bool File<NextHandler>::logfile(const U& filename)
	{
		// Assigning the new filename
		pOutputFilename = filename;
		// Opening the log file
		if (pOutputFilename.empty())
		{
			pFile.close();
			return true;
		}
		return pFile.open(pOutputFilename, Core::IO::OpenMode::write | Core::IO::OpenMode::append);
	}


	template<class NextHandler>
	inline void File<NextHandler>::closeLogfile()
	{
		pFile.close();
	}


	template<class NextHandler>
	inline bool File<NextHandler>::reopenLogfile()
	{
		pFile.close();
		return (pOutputFilename.empty())
			? true
			: pFile.open(pOutputFilename, Core::IO::OpenMode::write | Core::IO::OpenMode::append);
	}


	template<class NextHandler>
	inline String File<NextHandler>::logfile() const
	{
		return pOutputFilename;
	}


	template<class NextHandler>
	inline bool File<NextHandler>::logFileIsOpened()
	{
		return (pFile.opened());
	}



	template<class NextHandler>
	template<class LoggerT, class VerbosityType, class StringT>
	void File<NextHandler>::internalDecoratorWriteWL(LoggerT& logger, const StringT& s)
	{
		if (pFile.opened())
		{
			typedef typename LoggerT::DecoratorsType DecoratorsType;
			// Append the message to the file
			logger.DecoratorsType::template internalDecoratorAddPrefix<File, VerbosityType>(pFile, s);

			// Flushing the result
			# ifdef YUNI_OS_WINDOWS
			pFile << "\r\n";
			# else
			pFile << '\n';
			# endif
			pFile.flush();
		}

		// Transmit the message to the next handler
		NextHandler::template internalDecoratorWriteWL<LoggerT, VerbosityType, StringT>(logger, s);
	}





} // namespace Logs
} // namespace Yuni

#endif // __YUNI_CORE_LOGS_HANDLERS_FILE_HXX__
