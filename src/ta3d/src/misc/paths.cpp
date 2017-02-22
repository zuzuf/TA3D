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

#include <stdafx.h>
#include "paths.h"
#include <TA3D_NameSpace.h>
#include <logs/logs.h>
#include <unistd.h>
#include <errno.h>
#include <QFileInfo>
#include <QDir>


namespace TA3D
{
	namespace Paths
	{

		QString ApplicationRoot;
		QString Caches;
		QString Savegames;
		QString Logs;
		QString LogFile;
		QString Preferences;
		QString ConfigFile;
		QString Screenshots;
		QString Resources;
# ifdef TA3D_PLATFORM_WINDOWS
		QString LocalData;
# endif


		QString CurrentDirectory()
		{
			char* c = getcwd(NULL, 0);
            QString ret = QString::fromLocal8Bit(c);
			free(c);
            return ret;
		}


		namespace
		{

# ifdef TA3D_PLATFORM_WINDOWS

			/*!
			** \brief Get the absolute path to the local application data folder
			** (from the Windows registry)
			*/
			QString localAppData()
			{
				LPITEMIDLIST pidl;
				HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
				char szPath[_MAX_PATH];
				BOOL f = SHGetPathFromIDList(pidl, szPath);
				LPMALLOC pMalloc;
				hr = SHGetMalloc(&pMalloc);
				pMalloc->Free(pidl);
				pMalloc->Release();
				return QString(szPath, strlen(szPath));
			}

			void initForWindows()
			{
				LocalData = localAppData();
				LocalData += Separator;

                Resources = ApplicationRoot + "resources/";
                Caches = ApplicationRoot + "cache/";
                Savegames = ApplicationRoot + "savegames/";
                Logs = ApplicationRoot + "logs/";

                Preferences = ApplicationRoot + "settings/";
                Screenshots = ApplicationRoot + "screenshots/";
			}

# else // ifdef TA3D_PLATFORM_WINDOWS

# ifndef TA3D_PLATFORM_DARWIN
			void initForDefaultUnixes()
			{
				QString home = getenv("HOME");
                home += "/.ta3d/";
                Resources = home + "resources/";
                Caches = home + "cache/";
                Savegames = home + "savegames/";
                Logs = home + "log/";

				Preferences = home;
                Screenshots = home + "screenshots/";
			}

# else // ifndef TA3D_PLATFORM_DARWIN

			void initForDarwin()
			{
				QString home = getenv("HOME");
                Caches = home + "/Library/Caches/ta3d/";
                Savegames = home + "/Library/Application Support/ta3d/savegames/";
                Resources = home + "/Library/Application Support/ta3d/resources/";
                Logs = home + "/Library/Logs/ta3d/";

                Preferences = home + "/Library/Preferences/ta3d/";
                Screenshots = home + "/Downloads/";
			}

# endif // ifndef TA3D_PLATFORM_DARWIN

# endif // ifdef TA3D_PLATFORM_WINDOWS

			/*!
			** \brief Initialize the ApplicationRoot variable
			** \param argv0 Equivalent to argv[0] from the main
			*/
			void initApplicationRootPath(const char* argv0)
			{
				LOG_ASSERT(NULL != argv0);

# ifdef TA3D_PLATFORM_DARWIN
				if (ExtractFileExt(argv0).toLower() == ".app")
				{
					ApplicationRoot.clear();
					ApplicationRoot << argv0 << "/Contents/MacOS";
					return;
				}
# endif

				if (IsAbsolute(argv0))
					ApplicationRoot = ExtractFilePath(argv0);
				else
				{
					ApplicationRoot.clear();
                    QString r = CurrentDirectory() + '/' + argv0;
                    if (!r.isEmpty())
						ApplicationRoot = ExtractFilePath(r);
				}
				if (ApplicationRoot.endsWith("/./"))
					ApplicationRoot.chop(2);
				else if (ApplicationRoot.endsWith("/."))
					ApplicationRoot.chop(1);
                else if (!ApplicationRoot.isEmpty() && !ApplicationRoot.endsWith('/') && !ApplicationRoot.endsWith('\\'))
                    ApplicationRoot += '/';
			}


		} // namespace



        QString ExtractFilePath(const QString& p)
		{
            return QFileInfo(p).path();
		}

        QString ExtractFileName(const QString& p)
		{
            return QFileInfo(p).fileName();
		}

        void ExtractFileName(QStringList& p)
		{
            for(QString &i : p)
                i = QFileInfo(i).fileName();
		}

        QString ExtractFileNameWithoutExtension(const QString& p)
		{
            return QFileInfo(p).completeBaseName();
		}



		QString ExtractFileExt(const QString& s)
		{
            return '.' + QFileInfo(s).completeSuffix();
		}




		bool Initialize(int /*argc*/, char* argv[], const QString& programName)
		{
			LOG_ASSERT(NULL != argv);
            LOG_ASSERT(!programName.isEmpty());

			initApplicationRootPath(argv[0]);

			# ifdef TA3D_PLATFORM_WINDOWS
			initForWindows();
			# else
			#   ifndef TA3D_PLATFORM_DARWIN
			initForDefaultUnixes();
			#   else
			initForDarwin();
			#   endif
			# endif

			// Initialize the logging mecanism
            logs.openLogFile(Paths::Logs + programName + ".log");

			// Welcome Commander !
			logs.checkpoint() << "Welcome to TA3D";
			logs.checkpoint() << "Version: " << TA3D_VERSION_HI << "." << TA3D_VERSION_LO << "-" << TA3D_VERSION_TAG
                              << " (r" << TA3D_CURRENT_REVISION << ")";
			logs.info();

			LOG_INFO(LOG_PREFIX_PATHS << "Started from: `" << ApplicationRoot << "`");
			ConfigFile = Preferences;
			ConfigFile += "ta3d.cfg";
			LOG_INFO(LOG_PREFIX_PATHS << "Preferences: `" << Preferences << "`");
			LOG_INFO(LOG_PREFIX_PATHS << "Cache: `" << Caches << "`");
			LOG_INFO(LOG_PREFIX_PATHS << "Savegames: `" << Savegames << "`");
			LOG_INFO(LOG_PREFIX_PATHS << "Screenshots: `" << Screenshots << "`");
			LOG_INFO(LOG_PREFIX_PATHS << "Logs: `" << Logs << "`");

			// Informations about the log file
			if (!logs.logFileIsOpened())
				logs.error() << "Logs: Impossible to open `" << logs.logfile() << "`";
			else
				logs.info() << "Opened the log file: `" << logs.logfile() << "`";

			bool res = MakeDir(Caches) && MakeDir(Savegames) && MakeDir(Logs)
				&& MakeDir(Preferences) && MakeDir(Screenshots) && MakeDir(Resources)
                && MakeDir(Savegames + "multiplayer/");
			if (!res)
			{
				logs.fatal() << "Some paths are missing. Aborting now...";
				exit(120);
			}

			return res;
		}


		bool Exists(const QString& p)
		{
            return QDir(p).exists() || QFileInfo(p).exists();
		}

		void RemoveDir(const QString& p)
		{
            QDir dir(p);
            if (dir.isRoot())
                return;
            const QString name = dir.dirName();
            dir.cdUp();
            dir.rmdir(name);
		}


		bool MakeDir(const QString& p)
		{
            if (p.isEmpty())
				return true;

            QDir dir(p);
            QString sub_dir;

            while(!dir.exists())
            {
                if (sub_dir.isEmpty())
                    sub_dir = dir.dirName();
                else
                    sub_dir = dir.dirName() + '/' + sub_dir;
                dir.cdUp();
            }
            dir.mkpath(sub_dir);
            return true;
		}

        bool ImplGlob(QStringList& out, const QString& pattern, const bool emptyListBefore, QDir::Filters filters)
		{
            QString path;
            QString filter;
            if (pattern.endsWith('/'))
                path = pattern;
            else
            {
                const QFileInfo pattern_info(pattern);
                path = pattern_info.path();
                filter = pattern_info.fileName();
            }
            QDir dir(path, filter, QDir::SortFlags( QDir::Name | QDir::IgnoreCase ), filters);
            if (emptyListBefore)
                out.clear();
            QStringList new_entries = dir.entryList();
            path += '/';
            for(QString &entry : new_entries)
                entry = path + entry;
            out << new_entries;
            return !new_entries.isEmpty();
		}


        bool Glob(QStringList &out, const QString& pattern, const bool emptyListBefore)
		{
            return ImplGlob(out, pattern, emptyListBefore, QDir::AllEntries | QDir::NoDotAndDotDot);
		}

        bool GlobFiles(QStringList& out, const QString& pattern, const bool emptyListBefore)
		{
            return ImplGlob(out, pattern, emptyListBefore, QDir::Files | QDir::NoDotAndDotDot);
        }

        bool GlobDirs(QStringList& out, const QString& pattern, const bool emptyListBefore)
		{
            return ImplGlob(out, pattern, emptyListBefore, QDir::Dirs | QDir::NoDotAndDotDot);
        }

		bool IsAbsolute(const QString& p)
		{
            return QFileInfo(p).isAbsolute();
		}




} // namespace Paths
} // namespace TA3D

