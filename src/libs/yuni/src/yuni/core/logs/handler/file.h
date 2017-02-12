#ifndef __YUNI_CORE_LOGS_HANDLERS_FILE_H__
# define __YUNI_CORE_LOGS_HANDLERS_FILE_H__

# include "../null.h"
# include "../../io/file.h"
# include <cassert>


namespace Yuni
{
namespace Logs
{

	/*!
	** \brief Log Handler: Single Log file
	*/
	template<class NextHandler = NullHandler>
	class File : public NextHandler
	{
	public:
		enum Settings
		{
			// Colors are not allowed in a file
			unixColorsAllowed = 0,
		};

	public:
		/*!
		** \brief Try to (re)open a target log file
		**
		** You should use an absolute filename to be able to safely reopen it.
		** If a log file was already opened, it will be closed before anything else.
		** If the given filename is empty, true will be returned.
		**
		** \param filename A relative or absolute filename
		** \return True if the operation succeeded, false otherwise
		*/
		template<typename U> bool logfile(const U& filename);

		/*!
		** \brief Get the last opened log file
		** \see outputFilename(filename)
		*/
		String logfile() const;

		/*!
		** \brief Reopen the log file
		**
		** It is safe to call several times this routine.
		** True will be returned if the log filename is empty.
		*/
		bool reopenLogfile();

		/*!
		** \brief Close the log file
		**
		** It is safe to call several times this routine.
		*/
		void closeLogfile();

		/*!
		** \brief Get if a log file is opened
		*/
		bool logFileIsOpened();

	public:
		template<class LoggerT, class VerbosityType, class StringT>
		void internalDecoratorWriteWL(LoggerT& logger, const StringT& s);

	private:
		//! The originale filename
		String pOutputFilename;
		//! File
		Core::IO::File::Stream pFile;

	}; // class File






} // namespace Logs
} // namespace Yuni

# include "file.hxx"

#endif // __YUNI_CORE_LOGS_HANDLERS_FILE_H__
