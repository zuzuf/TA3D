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

#include "resources.h"
#include "paths.h"
#include <logs/logs.h>
#include <threads/mutex.h>
#include <TA3D_NameSpace.h>



namespace TA3D
{
namespace Resources
{


	namespace
	{

		//! Definition list of resources folders
        typedef QStringList ResourcesFoldersList;

		//! Mutex for resources
		Mutex gResourcesMutex;

		//! List of resources folders
		ResourcesFoldersList pResourcesFolders;




# ifdef TA3D_PLATFORM_WINDOWS

		void initForWindows()
		{
            AddSearchPath(Paths::ApplicationRoot + "resources/");
			AddSearchPath(Paths::ApplicationRoot);
		}

# else // ifdef TA3D_PLATFORM_WINDOWS

# ifndef TA3D_PLATFORM_DARWIN
		void initForDefaultUnixes()
		{
			QString home = getenv("HOME");
            home += "/.ta3d/";

            AddSearchPath(home + "resources/");
			AddSearchPath("/usr/local/games/ta3d/");
            AddSearchPath("/usr/local/games/ta3d/resources/");
            AddSearchPath("/usr/local/share/ta3d/");
            AddSearchPath("/usr/local/share/ta3d/resources/");
            AddSearchPath("/opt/local/share/ta3d/");
            AddSearchPath("/opt/local/share/ta3d/resources/");
			AddSearchPath("/usr/local/share/games/ta3d/");
			AddSearchPath("/usr/local/share/games/ta3d/resources/");
			AddSearchPath("/opt/local/share/games/ta3d/");
			AddSearchPath("/opt/local/share/games/ta3d/resources/");
			AddSearchPath("/usr/games/ta3d/");
            AddSearchPath("/usr/games/ta3d/resources/");
            AddSearchPath("/usr/share/ta3d/");
            AddSearchPath("/usr/share/ta3d/resources/");
            AddSearchPath("/opt/share/ta3d/");
            AddSearchPath("/opt/share/ta3d/resources/");
			AddSearchPath("/usr/share/games/ta3d/");
			AddSearchPath("/usr/share/games/ta3d/resources/");
			AddSearchPath("/opt/share/games/ta3d/");
			AddSearchPath("/opt/share/games/ta3d/resources/");
			if (VARS::lp_CONFIG->bUseWorkingDirectory)
			{
				AddSearchPath(Paths::ApplicationRoot);
                AddSearchPath(Paths::ApplicationRoot + "resources/");
			}
		}

# else // ifndef TA3D_PLATFORM_DARWIN

		void initForDarwin()
		{
			QString home = getenv("HOME");

            Paths::MakeDir(home + "/Library/Application Support/ta3d/");
			// Relative folder for the Application bundle
            AddSearchPath(Paths::ApplicationRoot + "/../Resources/");
            AddSearchPath(home + "/Library/Application Support/ta3d/");
			// Unix compatibility
            AddSearchPath(home + "/.ta3d/resources/");
			// If using MacPorts
			AddSearchPath("/opt/local/share/ta3d/");
			if (VARS::lp_CONFIG->bUseWorkingDirectory)
			{
                AddSearchPath(Paths::ApplicationRoot + "/resources/");
				AddSearchPath(Paths::ApplicationRoot);
			}
		}

# endif // ifndef TA3D_PLATFORM_DARWIN

# endif // ifdef TA3D_PLATFORM_WINDOWS

	} // namespace

	void Initialize()
	{
# ifdef TA3D_PLATFORM_WINDOWS
		initForWindows();
# else
#   ifndef TA3D_PLATFORM_DARWIN
		initForDefaultUnixes();
#   else
		initForDarwin();
#   endif
# endif
	}


	bool Find(const QString& relFilename, QString& out)
	{
        MutexLocker locker(gResourcesMutex);
		for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
        {
            out = *i + relFilename;
			if (Paths::Exists(out))
				return true;
		}
		return false;
	}

    bool AddSearchPath(QString folder)
	{
        if (folder.isEmpty())
            return false;

        if (folder.endsWith('/') || folder.endsWith('\\'))
            folder.chop(1);

        if (Paths::Exists(folder))
		{
            MutexLocker locker(gResourcesMutex);
            if (pResourcesFolders.contains(folder))
                return false;
            pResourcesFolders.push_back(folder + '/');
            LOG_INFO(LOG_PREFIX_RESOURCES << "Added `" << folder << "/`");
			return true;
		}
		return false;
	}


    QStringList GetPaths()
	{
        MutexLocker locker(gResourcesMutex);
		return pResourcesFolders;
	}


    bool Glob(QStringList& out, const QString& pattern, const bool emptyListBefore)
	{
		if (emptyListBefore)
			out.clear();
		gResourcesMutex.lock();
        for (const QString &i : pResourcesFolders)
            Paths::Glob(out, i + pattern, false);
		gResourcesMutex.unlock();
		return !out.empty();
	}


    bool GlobDirs(QStringList& out, const QString& pattern, const bool emptyListBefore)
	{
		if (emptyListBefore)
			out.clear();
		gResourcesMutex.lock();
        for (const QString &i : pResourcesFolders)
            Paths::GlobDirs(out, i + pattern, false);
		gResourcesMutex.unlock();
		return !out.empty();
	}


} // namespace Resources
} // namespace TA3D
