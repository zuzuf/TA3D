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

#ifndef __TA3D_LOGS_LOGS_H__
# define __TA3D_LOGS_LOGS_H__

# include <yuni/yuni.h>
# include <yuni/core/logs.h>
# include <yuni/core/logs/handler/file.h>
# include <yuni/core/logs/decorators/applicationname.h>



// FIXME : Those defines are ketp for compatibility reasons and shall be removed as soon as possible
# ifdef LOGS_USE_DEBUG
#   define LOG_DEBUG(X)     logs.debug() << X
# else
#   define LOG_DEBUG(X)
# endif
# define LOG_INFO(X)        logs.info() << X
# define LOG_WARNING(X)     logs.warning() << X
# define LOG_ERROR(X)       logs.error() << X
# define LOG_CRITICAL(X)    do { logs.fatal() << X; exit(121); } while(0)

# ifdef LOGS_USE_DEBUG
#   define LOG_ASSERT(X)        do { \
                                if (!(X)) \
                                { \
									logs.fatal() << "Assertion failed: (" << __FILE__  << ", " << __LINE__ << "):"; \
                                    logs.fatal() << "Constraint: " << #X; \
                                    exit(120); \
                                } \
                                } while(0)
# else
#   define LOG_ASSERT(X)
# endif

// Even those defines, they shall be removed as soon as Yuni::Logs::Logger has an efficient way
// for dealing with components of an application
# define LOG_PREFIX_3DM               "[3dm] "
# define LOG_PREFIX_3DO               "[3do] "
# define LOG_PREFIX_S3O               "[s3o] "
# define LOG_PREFIX_MODEL             "[model] "

# define LOG_PREFIX_AI                "[AI] "

# define LOG_PREFIX_OPENGL            "[OpenGL] "
# define LOG_PREFIX_DIRECTX           "[DirectX] "
# define LOG_PREFIX_GFX               "[gfx] "
# define LOG_PREFIX_SHADER            "[shader] "
# define LOG_PREFIX_FONT              "[font] "

# define LOG_PREFIX_VFS               "[VFS] "

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



namespace TA3D
{


	/*!
	** \brief A static list of logging Handelrs
	**
	** StdCout : Console printing
	** File : Logging to a file
	*/
	typedef Yuni::Logs::StdCout< Yuni::Logs::File<> >  LoggerHandlers;


	/*!
	** \brief A static list of decorators for the logger
	*/
	typedef Yuni::Logs::Time<          // Date/Time when the event occurs
		Yuni::Logs::ApplicationName<   // name of the application
		Yuni::Logs::VerbosityLevel<    // The verbosity level
		Yuni::Logs::Message<>          // The message itself
		> > > LoggerDecorators;



	/*!
	** \brief The TA3D logger type
	*/
	typedef Yuni::Logs::Logger<LoggerHandlers, LoggerDecorators>  Logger;


	/*!
	** \brief The global TA3D Logger
	*/
	extern Logger logs;



	/*!
	** \brief Reset the logging mecanism with the default settings
	**
	** \param logfile The new log file to use
	*/
	template<class U> void ResetTheLoggingMecanism(const U& logfile);


} // namespace TA3D

# include "logs.hxx"

#endif // __TA3D_LOGS_LOGS_H__
