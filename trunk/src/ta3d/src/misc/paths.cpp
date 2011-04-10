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
#include <yuni/core/io/directory.h>


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

		String ApplicationRoot;
		String Caches;
		String Savegames;
		String Logs;
		String LogFile;
		String Preferences;
		String ConfigFile;
		String Screenshots;
		String Resources;
# ifdef TA3D_PLATFORM_WINDOWS
		String LocalData;
# endif


		String CurrentDirectory()
		{
			char* c = getcwd(NULL, 0);
			String ret(c);
			free(c);
			return String(ret);
		}


		namespace
		{

# ifdef TA3D_PLATFORM_WINDOWS

			/*!
			** \brief Get the absolute path to the local application data folder
			** (from the Windows registry)
			*/
			String localAppData()
			{
				LPITEMIDLIST pidl;
				HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
				char szPath[_MAX_PATH];
				BOOL f = SHGetPathFromIDList(pidl, szPath);
				LPMALLOC pMalloc;
				hr = SHGetMalloc(&pMalloc);
				pMalloc->Free(pidl);
				pMalloc->Release();
				return String(szPath, strlen(szPath));
			}

			void initForWindows()
			{
				LocalData = localAppData();
				LocalData += Separator;

				Resources = String(ApplicationRoot) << "resources\\";
				Caches = String(ApplicationRoot) << "cache\\";
				Savegames = String(ApplicationRoot) << "savegames\\";
				Logs = String(ApplicationRoot) << "logs\\";

				Preferences = String(ApplicationRoot) << "settings\\";
				Screenshots = String(ApplicationRoot) << "screenshots\\";
			}

# else // ifdef TA3D_PLATFORM_WINDOWS

# ifndef TA3D_PLATFORM_DARWIN
			void initForDefaultUnixes()
			{
				String home = getenv("HOME");
				home << "/.ta3d/";
				Resources = String(home) << "resources/";
				Caches = String(home) << "cache/";
				Savegames = String(home) << "savegames/";
				Logs = String(home) << "log/";

				Preferences = home;
				Screenshots = String(home) << "screenshots/";
			}

# else // ifndef TA3D_PLATFORM_DARWIN

			void initForDarwin()
			{
				String home = getenv("HOME");
				Caches = String(home) << "/Library/Caches/ta3d/";
				Savegames = String(home) << "/Library/Application Support/ta3d/savegames/";
				Resources = String(home) << "/Library/Application Support/ta3d/resources/";
				Logs = String(home) << "/Library/Logs/ta3d/";

				Preferences = String(home) << "/Library/Preferences/ta3d/";
				Screenshots = String(home) << "/Downloads/";
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
					String r;
					r << CurrentDirectory() << Separator << argv0;
					if (!r.empty())
                    {
						ApplicationRoot = ExtractFilePath(r);
                    }
				}
				if (ApplicationRoot.endsWith("/./"))
					ApplicationRoot.chop(2);
				else if (ApplicationRoot.endsWith("/."))
					ApplicationRoot.chop(1);
				else if (!ApplicationRoot.empty() && ApplicationRoot.last() != '/' && ApplicationRoot.last() != '\\')
					ApplicationRoot << '/';
			}


		} // namespace



		String ExtractFilePath(const String& p, const bool systemDependant)
		{
			String out;
			Yuni::Core::IO::ExtractFilePath(out, p, systemDependant);
			return out;
		}

		String ExtractFileName(const String& p, const bool systemDependant)
		{
			String out;
			Yuni::Core::IO::ExtractFileName(out, p, systemDependant);
			return out;
		}

		void ExtractFileName(String::List& p, const bool systemDependant)
		{
			for(String::List::iterator i = p.begin() ; i != p.end() ; ++i)
			{
				String out;
				Yuni::Core::IO::ExtractFileName(out, *i, systemDependant);
				*i = out;
			}
		}

		void ExtractFileName(String::Vector& p, const bool systemDependant)
		{
			for(String::Vector::iterator i = p.begin() ; i != p.end() ; ++i)
			{
				String out;
				Yuni::Core::IO::ExtractFileName(out, *i, systemDependant);
				*i = out;
			}
		}

		String ExtractFileNameWithoutExtension(const String& p, const bool systemDependant)
		{
			String out;
			Yuni::Core::IO::ExtractFileNameWithoutExtension(out, p, systemDependant);
			return out;
		}



		String ExtractFileExt(const String& s)
		{
			// FIXME This method should be completely removed
			// The prototype in Yuni::Core::IO has been changed to improve performances
			String t;
			Yuni::Core::IO::ExtractExtension(t, s);
			return t;
		}




		bool Initialize(int /*argc*/, char* argv[], const String& programName)
		{
			LOG_ASSERT(NULL != argv);
			LOG_ASSERT(!programName.empty());

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
			ResetTheLoggingMecanism(Yuni::String(Paths::Logs) <<programName << ".log");

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
				&& MakeDir(String(Savegames) << "multiplayer" << Paths::Separator);
			if (!res)
			{
				logs.fatal() << "Some paths are missing. Aborting now...";
				exit(120);
			}

			return res;
		}


		bool Exists(const String& p)
		{
			if (p.empty())
				return false;
			# ifdef TA3D_PLATFORM_WINDOWS
			// ugly workaround with stat under Windows
			// FIXME: Find a better way to find driver letters
			if (p.size() == 2 && ':' == p[1])
				return true;
			struct _stat s;
			if ('\\' != p[p.size() -1])
				return (_stat(p.c_str(), &s) == 0);
			return (_stat(String(p, 0, p.size() - 1).c_str(), &s) == 0);
			# else
			struct stat s;
			return (stat(p.c_str(), &s) == 0);
			# endif
		}

		void RemoveDir(const String& p)
		{
			Yuni::Core::IO::Directory::Remove(p);
		}


		bool MakeDir(const String& p)
		{
			if (p.empty())
				return true;
			// TODO Use the boost library, which has a better implementation that this one
			String::Vector parts;
			p.explode(parts, SeparatorAsString, false);
			String pth;
			bool hasBeenCreated(false);
			if (p[0] == '/' || p[0] == '\\')
				pth += Separator;

			for (String::Vector::const_iterator i = parts.begin(); i != parts.end(); ++i)
			{
				pth += *i;
# ifndef TA3D_PLATFORM_WINDOWS
				pth += Separator;
# endif
				if (!Exists(pth))
				{
					LOG_DEBUG(LOG_PREFIX_PATHS << "`" << pth << "` does not exist !");
# ifdef TA3D_PLATFORM_WINDOWS
					if (mkdir(pth.c_str()))
# else
                        if (mkdir(pth.c_str(), 0755))
# endif
						{
							// TODO Use the logging system instead
							LOG_ERROR(LOG_PREFIX_PATHS << "Impossible to create the folder `" << pth << "`");
							return false;
						}
						else
							hasBeenCreated = true;
				}
# ifdef TA3D_PLATFORM_WINDOWS
				pth += Separator;
# endif
			}
			if (hasBeenCreated)
				LOG_INFO(LOG_PREFIX_PATHS << "Created folder: `" << p << "`");
			return true;
		}

		template<class T>
		bool TmplGlob(T& out, const String& pattern, const bool emptyListBefore, const uint32 fileAttribs = FA_ALL, const uint32 required = 0, const bool relative = false)
		{
			if (emptyListBefore)
				out.clear();

			String root = ExtractFilePath(pattern);
			String root_path = root;
			if (root.size() > 1 && (root.last() == '/' || root.last() == '\\'))
				root_path.removeLast();
			else if (!root.empty())
				root << Separator;

			# ifdef TA3D_PLATFORM_WINDOWS
			String strFilePath; // Filepath
			String strExtension; // Extension
			HANDLE hFile; // Handle to file
			WIN32_FIND_DATA FileInformation; // File information

			hFile = ::FindFirstFile(pattern.c_str(), &FileInformation);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (FileInformation.cFileName[0] != '.')
					{
						String name = (const char*)FileInformation.cFileName;

						if((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (fileAttribs & FA_DIREC) && !(required & FA_FILE))
						{
							if (relative)
								out.push_back(name);
							else
								out.push_back(String(root) << name);
						}
						else if (!(required & FA_DIREC) && (fileAttribs & FA_FILE))
						{
							if (relative)
								out.push_back(name);
							else
								out.push_back(String(root) << name);
						}
					}
				} while(::FindNextFile(hFile, &FileInformation) == TRUE);

				// Close handle
				::FindClose(hFile);
			}

			# else /* ifdef WINDOWS */

			(void)fileAttribs;
			String filename_pattern = ExtractFileName(pattern);
			filename_pattern.toUpper();
			DIR *dp;
			struct dirent *dirp;
			if ((dp  = opendir(root_path.c_str())) == NULL)
			{
				// Following line is commented out because it may be useful later, but for now it only floods the logs
				//            LOG_ERROR( LOG_PREFIX_PATHS << "opening " << root << " failed: " << strerror( errno ) );
				return true;
			}

			while ((dirp = readdir(dp)) != NULL)
			{
				String name = (char*)(dirp->d_name);
				if (dirp->d_type == 0)
				{
					DIR *dp2;
					if ((dp2  = opendir((String(root) << name).c_str())))
					{
						closedir(dp2);
						dirp->d_type |= FA_DIREC;
					}
					else
						dirp->d_type |= FA_FILE;
				}

				if ((dirp->d_type & required) == required && name != "." && name != ".." && ToUpper(name).glob(filename_pattern))
				{
					if (relative)
						out.push_back(name);
					else
						out.push_back(String(root) << name);
				}
			}
			closedir(dp);
			# endif

			return !out.empty();
		}


		bool Glob(String::List& out, const String& pattern, const bool emptyListBefore, const bool relative)
		{
			return TmplGlob< String::List >(out, pattern, emptyListBefore, FA_ALL, 0, relative);
		}

		bool Glob(String::Vector& out, const String& pattern, const bool emptyListBefore, const bool relative)
		{
			return TmplGlob< String::Vector >(out, pattern, emptyListBefore, FA_ALL, 0, relative);
		}

		bool GlobFiles(String::List& out, const String& pattern, const bool emptyListBefore, const bool relative)
		{
			return TmplGlob< String::List >(out, pattern, emptyListBefore, FA_FILE, FA_FILE, relative);
		}

		bool GlobFiles(String::Vector& out, const String& pattern, const bool emptyListBefore, const bool relative)
		{
			return TmplGlob< String::Vector >(out, pattern, emptyListBefore, FA_FILE, FA_FILE, relative);
		}

		bool GlobDirs(String::List& out, const String& pattern, const bool emptyListBefore, const bool relative)
		{
			return TmplGlob< String::List >(out, pattern, emptyListBefore, FA_ALL, FA_DIREC, relative);
		}

		bool GlobDirs(String::Vector& out, const String& pattern, const bool emptyListBefore, const bool relative)
		{
			return TmplGlob< String::Vector >(out, pattern, emptyListBefore, FA_ALL, FA_DIREC, relative);
		}


		bool IsAbsolute(const String& p)
		{
			# ifdef TA3D_PLATFORM_WINDOWS
			return (p.empty() || (p.size() > 2 && ':' == p[1] && ('\\' == p[2] || '/' == p[2])));
			# else
			return ('/' == p.first());
			# endif
		}




} // namespace Paths
} // namespace TA3D

