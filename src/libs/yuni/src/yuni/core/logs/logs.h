#ifndef __YUNI_CORE_LOGS_LOGS_H__
# define __YUNI_CORE_LOGS_LOGS_H__

# include "../../yuni.h"
# include "../../thread/policy.h"

# include "null.h"
# include "verbosity.h"

// Default Handler
# include "handler/stdcout.h"
# include "handler/file.h"
// Default decorators
# include "decorators/verbositylevel.h"
# include "decorators/time.h"
# include "decorators/message.h"
# include "../static/assert.h"
# include "buffer.h"


// The default verbosity level according the target mode (debug/release)
# ifdef NDEBUG
#	define YUNI_LOGS_DEFAULT_VERBOSITY  Verbosity::Compatibility::level
# else
#	define YUNI_LOGS_DEFAULT_VERBOSITY  Verbosity::Debug::level
# endif



namespace Yuni
{
namespace Logs
{


	/*!
	** \brief A customizable log facility
	**
	** A simple hello world :
	** \code
	** #include <yuni/core/logs.h>
	**
	** int main()
	** {
	** 		// The logger
	** 		Yuni::Logs::Logger<>  logs;
	** 		// Hello world !
	** 		logs.notice() << "Hello world !";
	** 	return 0;
	** }
	** \endcode
	**
	** This class uses a static decorator pattern to modify its behavior. By default,
	** the output has the following format :
	** \code
	** [date][color][verbosity level][/color] <msg>
	** \endcode
	** The color output from the shell is available for both Unix (Terminal) and
	** Windows (cmd.exe).
	**
	**
	** \tparam TP The Threading Policy
	** \tapram Handlers A static list of all message handlers
	** \tparam Decorators A static list of all decorators
	*/
	template<
		class Handlers = StdCout<>,                            // List of all static handles
		class Decorators = Time< VerbosityLevel<Message<> > >, // List of all static decorators
		template<class> class TP = Policy::ObjectLevelLockable // The Threading Policy
		>
	class Logger
		:public TP<Logger<Handlers,Decorators,TP> >, // inherits from the Threading Policy
		public Decorators,                           // inherits from all decorators
		public Handlers                              // inherits from all handlers
	{
	public:
		//! The full prototype of the logger
		typedef Logger<Handlers, Decorators, TP>  LoggerType;
		//! The Threading Policy
		typedef TP<LoggerType>  ThreadingPolicy;

		//! Handlers
		typedef Handlers HandlersType;
		//! Decorators
		typedef Decorators  DecoratorsType;

		/*!
		** \brief Settings for the logger
		*/
		enum Settings
		{
			//! The default verbose level
			defaultVerbosityLevel = YUNI_LOGS_DEFAULT_VERBOSITY,
		};

	private:
		// Aliases (for code clarity)
		//! Alias for the CheckpointBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Checkpoint>    CheckpointBuffer;
		//! Alias for the NoticeBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Notice>        NoticeBuffer;
		//! Alias for the NoticeBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Info>          InfoBuffer;
		//! Alias for the WarningBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Warning>       WarningBuffer;
		//! Alias for the ErrorBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Error>         ErrorBuffer;
		//! Alias for the ProgressBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Progress>      ProgressBuffer;
		//! Alias for the CompatibilityBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Compatibility> CompatibilityBuffer;
		//! Alias for the FatalBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Fatal>         FatalBuffer;
		//! Alias for the DebugBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Debug>         DebugBuffer;
		//! Alias for a dummy writer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Info, 0>       DummyBuffer;
		//! Alias for the UnknownBuffer
		typedef Private::LogImpl::Buffer<LoggerType, Verbosity::Unknown>       UnknownBuffer;

	public:
		//! \name Constructors & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		Logger();
		/*!
		** \brief Copy constructor
		*/
		Logger(const Logger& rhs);
		/*!
		** \brief Destructor
		*/
		~Logger();
		//@}


		//! \name Checkpoint
		//@{
		CheckpointBuffer checkpoint();
		template<typename U> CheckpointBuffer checkpoint(const U& u);
		//@}

		//! \name Notice
		//@{
		NoticeBuffer notice();
		template<typename U> NoticeBuffer notice(const U& u);
		//@}

		//! \name Info
		//@{
		InfoBuffer info();
		template<typename U> InfoBuffer info(const U& u);
		//@}

		//! \name Warning
		//@{
		WarningBuffer warning();
		template<typename U> WarningBuffer warning(const U& u);
		//@}

		//! \name Error
		//@{
		ErrorBuffer error();
		template<typename U> ErrorBuffer error(const U& u);
		//@}

		//! \name Progress
		//@{
		ProgressBuffer progress();
		template<typename U> ProgressBuffer progress(const U& u);
		//@}

		//! \name Fatal
		//@{
		FatalBuffer fatal();
		template<typename U> FatalBuffer fatal(const U& u);
		//@}

		//! \name Compatibility notice
		//@{
		CompatibilityBuffer compatibility();
		template<typename U> CompatibilityBuffer compatibility(const U& u);
		//@}

		//! \name Debug (disabled if NDEBUG defined)
		//@{
		DebugBuffer debug();
		template<typename U> DebugBuffer debug(const U& u);
		//@}


		//! Start a custom verbosity level message
		template<class C> Private::LogImpl::Buffer<LoggerType,C,C::enabled> custom();

		//! Start a message with no verbosity level (always displayed)
		template<typename U> UnknownBuffer operator << (const U& u);
		//@}

	public:
		/*!
		** \brief The current maximum verbosity level allowed to be displayed
		**
		** \code
		** Logs::Logger<> logs;
		**
		** // Starting with default verbosity level
		** // The following messages will be displayed
		** logs.error() << "An error";
		** logs.notice() << "Hello world";
		**
		** // Changing the verbosity level
		** logs.verbosityLevel = Logs::Verbosity::Error::level; 
		** // Only the 'error' message will be displayed
		** logs.error() << "An error";
		** logs.notice() << "Hello world";
		** \endcode
		*/
		int verbosityLevel;

	private:
		/*!
		** \brief Transmit a message to all handlers
		*/
		template<class VerbosityType, class StringT>
		void dispatchMessageToHandlers(const StringT& s);

		// A friend !
		template<class, class, int> friend class Private::LogImpl::Buffer;

	}; // class Logger





} // namespace Logs
} // namespace Yuni

# include "logs.hxx"

#endif // __YUNI_CORE_LOGS_LOGS_H__
