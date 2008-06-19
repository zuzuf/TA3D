#ifndef __XX_LIB_LOGS_H__
# define __XX_LIB_LOGS_H__

# include <iostream>
# include <sstream>
# include <time.h>
# include <errno.h>
# include "../threads/mutex.h"


# ifdef LOGS_USE_DEBUG
#   define LOG_DEBUG(X)     Logs::logger() << X << Logs::debug
# else
#   define LOG_DEBUG(X)     
# endif
# define LOG_INFO(X)        Logs::logger() << X << Logs::info
# define LOG_WARNING(X)     Logs::logger() << X << Logs::warning
# define LOG_ERROR(X)       Logs::logger() << X << Logs::error
# define LOG_CRITICAL(X)    do { Logs::logger() << X << Logs::critical; exit(121); } while(0)

# define LOG_LEVEL_DEBUG        5
# define LOG_LEVEL_INFO         4
# define LOG_LEVEL_WARNING      3
# define LOG_LEVEL_ERROR        2
# define LOG_LEVEL_CRITICAL     1
# define LOG_LEVEL_QUIET        0

# ifdef LOGS_USE_DEBUG
#   define LOG_ASSERT(X)        do { \
                                if (!(X)) \
                                { \
                                    Logs::logger() << "Assertion failed: (" __FILE__  << ", " << __LINE__ << "):" << Logs::critical; \
                                    Logs::logger() << "Constraint: " << #X << Logs::critical; \
                                    exit(120); \
                                } \
                            } while(0)
# else
#   define LOG_ASSERT(X)
# endif




namespace Logs
{
	/*!
	** \brief Log line type interface
	** 
	** Derive from this class to implement a new line terminator (Log::debug etc).
	*/
	class AbstractLogMsg
	{
	public:
        //! Destructor
		virtual ~AbstractLogMsg() {} 

		/*!
		** \brief Implement this to produce a string to put
		** between the date and the log string.
		*/
		virtual std::string	header() = 0;

		/*!
		** \brief Implement this to set the minimal log level for the messages to appear.
		** \return The minimum level. 0 means the log will always appear.
		*/
		virtual int minimalLevel() = 0;


		/*!
		** \brief Reimplement this to color your log lines.
		** This is filtered if output is not std::cout.
		*/
		virtual std::string color()
		{
            		#ifndef TA3D_PLATFORM_WINDOWS
			return "[1m";
			#else
			return "";
			#endif
		}

		//! Reimplement this to reset the color to a default value.
		virtual std::string resetColor()
		{
            		#ifndef TA3D_PLATFORM_WINDOWS
			return "[0m";
			#else
			return "";
			#endif
		}

		//! Used to produce the date string
		virtual std::string date()
		{
			time_t rawtime;

			time ( &rawtime );

            #ifdef TA3D_PLATFORM_WINDOWS
			struct tm* timeinfo = localtime(&rawtime);
			char* asc;
			asc = asctime(timeinfo);
            #else
			struct tm* timeinfo;
            timeinfo = localtime(&rawtime);
			char asc[32];
			asctime_r(timeinfo, asc);
            #endif // WINDOWS
			asc[strlen(asc) - 1] = 0;
			std::ostringstream s;
			s << "[" << asc << "] ";
			return std::string(s.str());
		}
	}; // class AbstractLogMsg



	/*!
	** \brief Line terminator for debug messages, will tag messages as [debug].
	*/
	class LogDebugMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() { return LOG_LEVEL_DEBUG; }
		virtual ~LogDebugMsg() {}
            	#ifndef TA3D_PLATFORM_WINDOWS
		virtual std::string color() { return "[0m"; }
		#else
		virtual std::string color() { return ""; }
		#endif
		virtual std::string header() { return "[debug] "; }
	};

	/*!
	** \brief Line terminator for info messages, will tag messages as [infos].
	*/
	class LogInfoMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() { return LOG_LEVEL_INFO; }
		virtual ~LogInfoMsg() {}
            	#ifndef TA3D_PLATFORM_WINDOWS
		virtual std::string color() { return "[1;32m"; }
		#else
		virtual std::string color() { return ""; }
		#endif
		virtual inline std::string header() { return "[infos] "; }
	};

	/*!
	** \brief Line terminator for warning messages, will tag messages as [warns].
	*/
	class LogWarningMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() { return LOG_LEVEL_WARNING; }
		virtual ~LogWarningMsg() {}
            	#ifndef TA3D_PLATFORM_WINDOWS
		virtual std::string color() { return "[1;33m"; }
		#else
		virtual std::string color() { return ""; }
		#endif
		virtual inline std::string header() { return "[warns] "; }
	};

	/*!
	** \brief Line terminator for error messages, will tag messages as [error].
	*/
	class LogErrorMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() { return LOG_LEVEL_ERROR; }
		virtual ~LogErrorMsg() {}
            	#ifndef TA3D_PLATFORM_WINDOWS
		virtual std::string color() { return "[1;31m"; }
		#else
		virtual std::string color() { return ""; }
		#endif
		virtual inline std::string header() { return "[error] "; }
	};

    /*!
	** \brief Line terminator for critical messages, will tag messages as [critical].
	*/
	class LogCriticalMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() { return LOG_LEVEL_CRITICAL; }
		virtual ~LogCriticalMsg() {}
            	#ifndef TA3D_PLATFORM_WINDOWS
		virtual std::string color() { return "[1;31m"; }
		#else
		virtual std::string color() { return ""; }
		#endif
		virtual inline std::string header() { return "[critical] "; }
	};


	/*!
	** \brief Callback for logging.
	** \param tag The tag, according to the terminator (Logs::debug => "[debug]", etc)
	** \param msg What was passed to us, exactly.
	*/
	typedef void (*Callback)(const char *tag, const char *msg);



	/*!
	** \brief Class for logging
	** 
	** This class is basically an ostringstream, but whose output is not really a string,
	** but a stream or a callback or both. (yeah i know...)
	** \code
	**   logger().streamOutput(&std::cerr); // Output on std::cerr
	** 	 logger().callbackOutput(&my_callback); // Output via the callback AND std::cerr
	** 	 logger().streamOutput(NULL); // Do not output on std::cerr anymore
	** 	 logger().callbackOutput(NULL); // Do not output anything anymore.
	**    
	**   When writing your callback, please do not use the Logs::logger(), as it is locked,
	**   and this will enter a deadlock, followed by the death of your
	**   program/microwave oven/chipmunk/computer/cat/puppy/...
	** 		
	** 	 logger << "Sample debug message" << Log::debug; // Will produce the following
	** 	 // [ Sat May 26 13:37:19 2007 ] [debug] Sample debug message
	** \endcode
	*/
	class Log : public std::ostringstream
	{
	private:

		//! \brief Where do we output ? Anything which is an std::ostream. (cout, cerr, ostringstream...)
		std::ostream *  pOut;
		Callback        pCallback;
        TA3D::Mutex     pMutex;
	public:

		/*!
		** \brief Modify the output callback.
		** \param callback The new output callback. NULL means no callback output.
		** 
		** You can modify the output of the logger at any time.
		**/
		void callbackOutput(Callback callback);


		/*!
		** \brief Modify the output stream.
		** \param stream The new output ostream. NULL means no stream output.
		** 
		** You can modify the output of the logger at any time.
		**/
		void streamOutput(std::ostream *stream);


		/*!
		** \brief Locks the object.
		*/
		void lock() { pMutex.lock(); }

		/*!
		** \brief Unlocks the object.
		*/ 
		void unlock() { pMutex.unlock(); }


		/*!
		** \brief Constructor: you can pass a stream, and a callback, by default it is std::cout and NULL
		**/
		Log(std::ostream * outstream = &std::cerr, Callback = 0);


		/*!
		** \brief The generic << operator, specialized for Log*Msg
		**/
		template <typename RValue> 
		Log& operator << (RValue val)
        {
		    *((std::ostringstream*)this) << val;
            return *this;
        }
	};

	//! Used to make the line terminator Log::debug
	extern LogDebugMsg debug;

	//! Used to make the line terminator Log::info
	extern LogInfoMsg info;

	//! Used to make the line terminator Log::warning
	extern LogWarningMsg warning;

	//! Used to make the line terminator Log::error
	extern LogErrorMsg error;

    //! Used to make the line terminator Log::critical
    extern LogCriticalMsg critical;

	//! Singleton logger. This is not meant to be used carelessly.
	extern Log doNotUseThisLoggerDirectly;

    //! The current log level
    extern int level;

    /*!
    ** \brief Locks and return the logger
    ** \return A logger entity
    */
    Log & logger();


# include "logs.hxx"

} // Logs


#endif // __XX_LIB_LOGS_H__
