/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#ifndef __XX_LIB_LOGS_H__
# define __XX_LIB_LOGS_H__

# include "../stdafx.h"
# include <iostream>
# include <sstream>
# include <cstring>
# include <time.h>
# include <errno.h>
# include <string>
# include "../threads/mutex.h"
# include "../misc/string.h"


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


//! \name Prefix for log entries
//@{

# define LOG_PREFIX_3DM               "[3dm] "
# define LOG_PREFIX_3DO               "[3do] "
# define LOG_PREFIX_MODEL             "[model] "

# define LOG_PREFIX_AI                "[AI] "

# define LOG_PREFIX_OPENGL            "[OpenGL] "
# define LOG_PREFIX_DIRECTX           "[DirectX] "
# define LOG_PREFIX_GFX               "[gfx] "
# define LOG_PREFIX_SHADER            "[shader] "
# define LOG_PREFIX_FONT              "[font] "

# define LOG_PREFIX_I18N              "[i18n] "
# define LOG_PREFIX_SYSTEM            "[system] "
# define LOG_PREFIX_PATHS             "[paths] "
# define LOG_PREFIX_RESOURCES         "[resources] "
# define LOG_PREFIX_TDF               "[tdf] "
# define LOG_PREFIX_BATTLE            "[battle] "
# define LOG_PREFIX_SETTINGS          "[settings] "

# define LOG_PREFIX_SCRIPT            "[script] "
# define LOG_PREFIX_LUA               "[script::lua] "

# define LOG_PREFIX_BATTLE            "[battle] "

# define LOG_PREFIX_MENU_INTRO        "[menu::introduction] "
# define LOG_PREFIX_MENU_SOLO         "[menu::solo] "
# define LOG_PREFIX_MENU_LOADING      "[menu::loading] "
# define LOG_PREFIX_MENU_MAIN         "[menu::main] "
# define LOG_PREFIX_MENU_MAPSELECTOR  "[menu::mapselector] "
# define LOG_PREFIX_MENU_UNITSELECTOR "[menu::unitselector] "
# define LOG_PREFIX_MENU_STATS        "[menu::stats] "
# define LOG_PREFIX_MENU_NETMENU      "[menu::netmenu] "
# define LOG_PREFIX_MENU_MULTIMENU    "[menu::multimenu] "

# define LOG_PREFIX_NET               "[network] "
# define LOG_PREFIX_NET_BROADCAST     "[net::broadcast] "
# define LOG_PREFIX_NET_FILE          "[net::file] "
# define LOG_PREFIX_NET_SOCKET        "[net::socket] "
# define LOG_PREFIX_NET_UDP           "[net::udp] "

# define LOG_PREFIX_SOUND             "[audio] "

//@}


namespace TA3D
{
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
		virtual String	header() const = 0;

		/*!
		** \brief Reimplement this to forward the message to the console
		*/
		virtual void forwardToConsole(const String& msg) const = 0;

		/*!
		** \brief Implement this to set the minimal log level for the messages to appear.
		** \return The minimum level. 0 means the log will always appear.
		*/
		virtual int minimalLevel() const = 0;

		/*!
		** \brief Reimplement this to color your log lines.
		** This is filtered if output is not std::cout.
		*/
		virtual String color() const
		{
# ifndef TA3D_PLATFORM_WINDOWS
			return "[1m";
# else
			return "";
# endif
		}

		//! Reimplement this to reset the color to a default value.
		virtual String resetColor() const
		{
# ifndef TA3D_PLATFORM_WINDOWS
			return "[0m";
# else
			return "";
# endif
		}

		//! Used to produce the date string
		virtual String date() const
		{
			time_t rawtime;

			time(&rawtime);

# ifdef TA3D_PLATFORM_WINDOWS
			struct tm* timeinfo = localtime(&rawtime);
			char* asc;
			asc = asctime(timeinfo);
# else
			struct tm* timeinfo;
			timeinfo = localtime(&rawtime);
			char asc[32];
			asctime_r(timeinfo, asc);
# endif // WINDOWS
			asc[strlen(asc) - 1] = 0;
			std::ostringstream s;
			s << "[" << asc << "] ";
			return String(s.str());
		}

	}; // class AbstractLogMsg



	/*!
	** \brief Line terminator for debug messages, will tag messages as [debug].
	*/
	class LogDebugMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() const { return LOG_LEVEL_DEBUG; }
		virtual ~LogDebugMsg() {}
# ifndef TA3D_PLATFORM_WINDOWS
		virtual String color() const { return "[0m"; }
# else
		virtual String color() const { return ""; }
# endif
		virtual String header() const { return "[debug] "; }
		virtual void forwardToConsole(const String&) const {}
	};

	/*!
	** \brief Line terminator for info messages, will tag messages as [infos].
	*/
	class LogInfoMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() const { return LOG_LEVEL_INFO; }
		virtual ~LogInfoMsg() {}
# ifndef TA3D_PLATFORM_WINDOWS
		virtual String color() const { return "[1;32m"; }
# else
		virtual String color() const { return ""; }
# endif
		virtual inline String header() const { return "[infos] "; }
		virtual void forwardToConsole(const String& msg) const;
	};

	/*!
	** \brief Line terminator for warning messages, will tag messages as [warns].
	*/
	class LogWarningMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() const { return LOG_LEVEL_WARNING; }
		virtual ~LogWarningMsg() {}
# ifndef TA3D_PLATFORM_WINDOWS
		virtual String color() const { return "[1;33m"; }
# else
		virtual String color() const { return ""; }
# endif
		virtual inline String header() const { return "[warns] "; }
		virtual void forwardToConsole(const String& msg) const;
	};

	/*!
	** \brief Line terminator for error messages, will tag messages as [error].
	*/
	class LogErrorMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() const { return LOG_LEVEL_ERROR; }
		virtual ~LogErrorMsg() {}
# ifndef TA3D_PLATFORM_WINDOWS
		virtual String color() const { return "[1;31m"; }
# else
		virtual String color() const { return ""; }
# endif
		virtual inline String header() const { return "[error] "; }
		virtual void forwardToConsole(const String& msg) const;
	};

	/*!
	** \brief Line terminator for critical messages, will tag messages as [critical].
	*/
	class LogCriticalMsg : public AbstractLogMsg
	{
	public:
		virtual int minimalLevel() const { return LOG_LEVEL_CRITICAL; }
		virtual ~LogCriticalMsg() {}
# ifndef TA3D_PLATFORM_WINDOWS
		virtual String color() const { return "[1;31m"; }
# else
		virtual String color() const { return ""; }
# endif
		virtual inline String header() const { return "[critical] "; }
		virtual void forwardToConsole(const String& msg) const;
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
	** 	 logger().writeToFile("output.log"); // Redirect the output to a file
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
		std::ostream*   pOut;
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
		** \brief Redirect the output to a file
		** \param filename The filename
		** \return True if the operation succeeded, False otherwise
		*/
		bool writeToFile(const String& filename);

		/*!
		** \brief Close the file where the output was redirected
		**
		** This method can be safely called at any time
		*/
		void closeFile();

		/*!
		** \brief Locks the object.
		*/
		void lock() { pMutex.lock(); }

		/*!
		** \brief Unlocks the object.
		*/
		void unlock() { pMutex.unlock(); }


		/*!
		** \brief Constructor: you can pass a stream, and a callback
		**/
		Log(std::ostream * outstream = NULL, Callback = 0);


		/*!
		** \brief The generic << operator, specialized for Log*Msg
		**/
		template <typename RValue>
			Log& operator << (RValue val)
			{
				*((std::ostringstream*)this) << val;
				return *this;
			}

	}; // class Log


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

} // namespace Logs
} // namespace TA3D

#endif // __XX_LIB_LOGS_H__
