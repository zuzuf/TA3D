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
#ifndef TA3D_PLATFORM_WINDOWS
# include <stdlib.h>
#else
# include <windows.h>
# include <shlobj.h>
#endif
#include <sys/stat.h>
#include <TA3D_NameSpace.h>
#include <logs/logs.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <QFileInfo>
#include <QDir>


#ifdef TA3D_PLATFORM_WINDOWS
# define FA_FILE     1
# define FA_DIREC    2
#else
# define FA_FILE     DT_REG
# define FA_DIREC    DT_DIR
#endif

#define FA_ALL      (FA_FILE | FA_DIREC)



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
			QString ret(c);
			free(c);
			return QString(ret);
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

                Resources = ApplicationRoot + "resources\\";
                Caches = ApplicationRoot + "cache\\";
                Savegames = ApplicationRoot + "savegames\\";
                Logs = ApplicationRoot + "logs\\";

                Preferences = ApplicationRoot + "settings\\";
                Screenshots = ApplicationRoot + "screenshots\\";
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
                    QString r = CurrentDirectory() + Separator + argv0;
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



		QString ExtractFilePath(const QString& p, const bool systemDependant)
		{
            return QFileInfo(p).filePath();
		}

		QString ExtractFileName(const QString& p, const bool systemDependant)
		{
            return QFileInfo(p).fileName();
		}

        void ExtractFileName(QStringList& p, const bool systemDependant)
		{
            for(QString &i : p)
                i = QFileInfo(i).fileName();
		}

		QString ExtractFileNameWithoutExtension(const QString& p, const bool systemDependant)
		{
            return QFileInfo(p).completeBaseName();
		}



		QString ExtractFileExt(const QString& s)
		{
            return QFileInfo(s).completeSuffix();
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
                && MakeDir(Savegames + "multiplayer" + Paths::Separator);
			if (!res)
			{
				logs.fatal() << "Some paths are missing. Aborting now...";
				exit(120);
			}

			return res;
		}


		bool Exists(const QString& p)
		{
            return QDir(p).exists();
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

		template<class T>
		bool TmplGlob(T& out, const QString& pattern, const bool emptyListBefore, const uint32 fileAttribs = FA_ALL, const uint32 required = 0, const bool relative = false)
		{
			if (emptyListBefore)
				out.clear();

			QString root = ExtractFilePath(pattern);
			QString root_path = root;
            if (root.size() > 1 && (root.endsWith('/') || root.endsWith('\\')))
                root_path.chop(1);
            else if (!root.isEmpty())
                root += Separator;

			# ifdef TA3D_PLATFORM_WINDOWS
			QString strFilePath; // Filepath
			QString strExtension; // Extension
			HANDLE hFile; // Handle to file
			WIN32_FIND_DATA FileInformation; // File information

            hFile = ::FindFirstFile(pattern.toStdString().c_str(), &FileInformation);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (FileInformation.cFileName[0] != '.')
					{
						QString name = (const char*)FileInformation.cFileName;

						if((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (fileAttribs & FA_DIREC) && !(required & FA_FILE))
						{
							if (relative)
								out.push_back(name);
							else
                                out.push_back(root + name);
						}
						else if (!(required & FA_DIREC) && (fileAttribs & FA_FILE))
						{
							if (relative)
								out.push_back(name);
							else
                                out.push_back(root + name);
						}
					}
				} while(::FindNextFile(hFile, &FileInformation) == TRUE);

				// Close handle
				::FindClose(hFile);
			}

			# else /* ifdef WINDOWS */

			(void)fileAttribs;
            QString filename_pattern = ExtractFileName(pattern);
			DIR *dp;
			struct dirent *dirp;
            if ((dp  = opendir(root_path.toStdString().c_str())) == NULL)
			{
				// Following line is commented out because it may be useful later, but for now it only floods the logs
				//            LOG_ERROR( LOG_PREFIX_PATHS << "opening " << root << " failed: " << strerror( errno ) );
				return true;
			}

            QRegExp reg_exp(filename_pattern, Qt::CaseInsensitive, QRegExp::Wildcard);

			while ((dirp = readdir(dp)) != NULL)
			{
				QString name = (char*)(dirp->d_name);
				if (dirp->d_type == 0)
				{
					DIR *dp2;
                    if ((dp2  = opendir((root + name).toStdString().c_str())))
					{
						closedir(dp2);
						dirp->d_type |= FA_DIREC;
					}
					else
						dirp->d_type |= FA_FILE;
				}

                if ((dirp->d_type & required) == required && name != "." && name != ".." && reg_exp.exactMatch(name))
				{
					if (relative)
						out.push_back(name);
					else
                        out.push_back(root + name);
				}
			}
			closedir(dp);
			# endif

			return !out.empty();
		}


        bool Glob(QStringList &out, const QString& pattern, const bool emptyListBefore, const bool relative)
		{
            return TmplGlob(out, pattern, emptyListBefore, FA_ALL, 0, relative);
		}

        bool GlobFiles(QStringList& out, const QString& pattern, const bool emptyListBefore, const bool relative)
		{
            return TmplGlob(out, pattern, emptyListBefore, FA_FILE, FA_FILE, relative);
		}

        bool GlobDirs(QStringList& out, const QString& pattern, const bool emptyListBefore, const bool relative)
		{
            return TmplGlob(out, pattern, emptyListBefore, FA_ALL, FA_DIREC, relative);
		}

		bool IsAbsolute(const QString& p)
		{
            return QFileInfo(p).isAbsolute();
		}




} // namespace Paths
} // namespace TA3D

