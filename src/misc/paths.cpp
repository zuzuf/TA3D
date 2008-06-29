#include "../stdafx.h"
#include "paths.h"
#ifndef TA3D_PLATFORM_WINDOWS
# include <stdlib.h>
#else
# include <windows.h>
# include <shlobj.h>
#endif 
#include <sys/stat.h>
#include <string>
#include "../TA3D_NameSpace.h"
#include "../logs/logs.h"



namespace TA3D
{

    String Paths::Caches = "";
    String Paths::Savegames = "";
    String Paths::Logs = "";
    String Paths::Preferences = "";
    String Paths::ConfigFile = "";
    String Paths::Screenshots = "";
    #ifdef TA3D_PLATFORM_WINDOWS
    char Paths::Separator = '\\';
    String Paths::SeparatorAsString = "\\";
    #else
    char Paths::Separator = '/';
    String Paths::SeparatorAsString = "/";
    #endif
    Paths::ResourcesFoldersList Paths::pResourcesFolders;


    # ifdef TA3D_PLATFORM_WINDOWS

    /*!
     * \brief Get the absolute path to the local application data folder
     * (from the Windows registry)
     */
    static std::string localAppData()
    {
        LPITEMIDLIST pidl;
        HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
        char szPath[_MAX_PATH];
        BOOL f = SHGetPathFromIDList(pidl, szPath);
        LPMALLOC pMalloc;
        hr = SHGetMalloc(&pMalloc);
        pMalloc->Free(pidl);
        pMalloc->Release();
        return szPath;
    }

    static void initForWindows()
    {
	    String root = localAppData();
	    root += Paths::Separator;
        Paths::Caches = root + "ta3d\\cache\\";
        Paths::Savegames = root + "ta3d\\savegames\\";
        Paths::Logs = root + "ta3d\\logs\\";
        Paths::AddResourcesFolder("./");
        Paths::Preferences = root + "ta3d\\settings\\";
        Paths::Screenshots = root + "ta3d\\screenshots\\";
    }

    # else // ifdef TA3D_PLATFORM_WINDOWS

    # ifndef TA3D_PLATFORM_DARWIN
    static void initForDefaultUnixes()
    {
        String home = getenv("HOME");
        home += "/.ta3d/";
        Paths::Caches = home + "cache/";
        Paths::Savegames = home + "savegames/";
        Paths::Logs = home + "log/";
        Paths::AddResourcesFolder(home + "resources/");
        Paths::AddResourcesFolder("/usr/local/games/ta3d/");
        Paths::AddResourcesFolder("/usr/local/share/ta3d/");
        Paths::AddResourcesFolder("/opt/local/share/ta3d/");
        Paths::AddResourcesFolder("./");
        Paths::Preferences = home;
        Paths::Screenshots = home + "screenshots/";
    }

    # else // ifndef TA3D_PLATFORM_DARWIN

    static void initForDarwin()
    {
        String home = getenv("HOME");
        Paths::Caches = home + "/Library/Caches/ta3d/";
        Paths::Savegames = home + "/Library/Preferences/ta3d/savegames/";
        Paths::Logs = home + "/Library/Logs/ta3d/";
        Paths::AddResourcesFolder("../Resources/");
        Paths::AddResourcesFolder(home + "/Library/Application Support/ta3d/");
        Paths::AddResourcesFolder("/opt/local/share/ta3d/");
        Paths::AddResourcesFolder("./");
        Paths::Preferences = home + "/Library/Preferences/ta3d/";
        Paths::Screenshots = home + "/Downloads/";
    }

    # endif // ifndef TA3D_PLATFORM_DARWIN

    # endif // ifdef TA3D_PLATFORM_WINDOWS



    bool
    Paths::Initialize()
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
        ConfigFile = Preferences;
        ConfigFile += "ta3d.cfg";
        LOG_INFO("Folder: Preferences: `" << Preferences << "`");
        LOG_INFO("Folder: Cache: `" << Caches << "`");
        LOG_INFO("Folder: Savegames: `" << Savegames << "`");
        LOG_INFO("Folder: Screenshots: `" << Screenshots << "`");
        LOG_INFO("Folder: Logs: `" << Logs << "`");
        bool res = MakeDir(Caches) && MakeDir(Savegames)
            && MakeDir(Logs) && MakeDir(Preferences) && MakeDir(Screenshots);
        if (!res)
            LOG_CRITICAL("Aborting now.");
        return res;
    }


    bool
    Paths::Exists(const String& p)
    {
        if (p.empty())
            return true;
	#ifdef TA3D_PLATFORM_WINDOWS
	// ugly workaround with stat under Windows
	// FIXME: Find a better way to find driver letters
	if (p.size() == 2 && ':' == p[1]) 
	   return true;
	#endif
        struct stat s;
        return (stat(p.c_str(), &s) == 0);
    }

    bool
    Paths::MakeDir(const String& p)
    {
        if (p.empty())
            return true;
        // TODO Use the boost library, which has a better implementation that this one
        Vector<String> parts;
        ReadVectorString(parts, p, SeparatorAsString, false);
        String pth = "";
        bool hasBeenCreated(false);

        Vector<String>::const_iterator i = parts.begin();
        for (; i != parts.end(); ++i)
        {
            pth += *i;
	    #ifndef TA3D_PLATFORM_WINDOWS
            pth += Separator;
            #endif
            if (!Paths::Exists(pth))
            {
		    LOG_DEBUG(pth << " does not exist !");
		# ifdef TA3D_PLATFORM_WINDOWS
                if (mkdir(pth.c_str()))
		# else
                if (mkdir(pth.c_str(), 01755))
		# endif
                {
                    // TODO Use the logging system instead
                    LOG_ERROR("Impossible to create the folder `" << pth << "`");
                    return false;
                }
                else
                    hasBeenCreated = true;
            }
	    #ifdef TA3D_PLATFORM_WINDOWS
            pth += Separator;
	    #endif
        }
        if (hasBeenCreated)
            LOG_INFO("Created folder: `" << p << "`");
        return true;
    }


    bool Paths::FindResources(const String& relFilename, String& out)
    {
        for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
        {
            out = *i;
            out += relFilename;
            if (Exists(out))
                return true;
        }
        return false;
    }

    bool Paths::AddResourcesFolder(const String& folder)
    {
        if (!folder.empty())
        {
            for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
            {
                if (folder == *i)
                    return false;
            }
            LOG_DEBUG("Added resources folder: `" << folder << "`");
            pResourcesFolders.push_back(folder);
            return true;
        }
        return false;
    }


} // namespace TA3D


