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
#include <unistd.h>
#include <allegro.h>
#include <fstream>
#include <string>



namespace TA3D
{
namespace Paths
{

    String ApplicationRoot = "";
    String Caches = "";
    String Savegames = "";
    String Logs = "";
    String Preferences = "";
    String ConfigFile = "";
    String Screenshots = "";
    # ifdef TA3D_PLATFORM_WINDOWS
    String LocalData = "";
    # endif
    
    
    String CurrentDirectory()
    {
        char* c = getcwd(NULL, 0);
        String ret(c);
        free(c);
        return String(ret);
    }

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
        LocalData = localAppData();
        LocalData += Separator;
        Caches = LocalData + "ta3d\\cache\\";
        Savegames = LocalData + "ta3d\\savegames\\";
        Logs = LocalData + "ta3d\\logs\\";

        Preferences = LocalData + "ta3d\\settings\\";
        Screenshots = LocalData + "ta3d\\screenshots\\";
    }

    # else // ifdef TA3D_PLATFORM_WINDOWS

    # ifndef TA3D_PLATFORM_DARWIN
    static void initForDefaultUnixes()
    {
        String home = getenv("HOME");
        home += "/.ta3d/";
        Caches = home + "cache/";
        Savegames = home + "savegames/";
        Logs = home + "log/";

        Preferences = home;
        Screenshots = home + "screenshots/";
    }

    # else // ifndef TA3D_PLATFORM_DARWIN

    static void initForDarwin()
    {
        String home = getenv("HOME");
        Caches = home + "/Library/Caches/ta3d/";
        Savegames = home + "/Library/Preferences/ta3d/savegames/";
        Logs = home + "/Library/Logs/ta3d/";
        
        Preferences = home + "/Library/Preferences/ta3d/";
        Screenshots = home + "/Downloads/";
    }

    # endif // ifndef TA3D_PLATFORM_DARWIN

    # endif // ifdef TA3D_PLATFORM_WINDOWS

    String ExtractFilePath(const String& p, const bool systemDependant)
    {
        String::Vector parts;
        if (systemDependant)
            p.split(parts, SeparatorAsString, false);
        else
            p.split(parts, "\\/", false);
        
        String ret;
        String::Vector::size_type n = parts.size();
        --n;
        for (String::Vector::size_type i = 0; i != n; ++i)
        {
            if (parts[i] != ".")
                ret << parts[i] << Separator;
        }
        return ret;
    }

    String ExtractFileName(const String& p, const bool systemDependant)
    {
        String::size_type pos;
        if (systemDependant)
            pos = p.find_last_of(Separator);
        else
            pos = p.find_last_of("\\/");
        if (String::npos == pos)
            return p;
        return p.substr(pos+1);
    }

    String ExtractFileNameWithoutExtension(const String& p)
    {
        String::size_type pos = p.find_last_of(Separator);
        String::size_type n = p.find_last_of('.');
        if (String::npos == n && String::npos == pos)
            return p;
        if (n == pos)
            return "";
        if (n == String::npos && n > pos + 1)
        {
            if (String::npos == pos)
                return p;
            return p.substr(pos + 1);
        }
        if (pos == String::npos)
            return p.substr(0, n);
        return p.substr(pos + 1, n - pos - 1);
    }



    String ExtractFileExt(const String& s)
    {
        String::size_type n = s.find_last_of(".\\/");
        if (n == String::npos || '.' != s[n])
            return "";
        return String(s, n).toLower();
    }



    /*!
     * \brief Initialize the ApplicationRoot variable
     * \param argv0 Equivalent to argv[0] from the main
     */
    static void initApplicationRootPath(const char* argv0)
    {
        LOG_ASSERT(NULL != argv0);

        if (IsAbsolute(argv0))
            ApplicationRoot = ExtractFilePath(argv0);
        else
        {
            ApplicationRoot = "";
            String r;
            r << CurrentDirectory() << Separator << argv0;
            if (!r.empty())
                ApplicationRoot = ExtractFilePath(r);
        }
    }




    bool Initialize(int argc, char* argv[])
    {
        LOG_ASSERT(NULL != argv);

        initApplicationRootPath(argv[0]);
        LOG_INFO("Started from: `" << ApplicationRoot << "`");

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


    bool Exists(const String& p)
    {
        if (p.empty())
            return false;
	    # ifdef TA3D_PLATFORM_WINDOWS
	    // ugly workaround with stat under Windows
	    // FIXME: Find a better way to find driver letters
	    if (p.size() == 2 && ':' == p[1]) 
	        return true;
	    # endif
        struct stat s;
        return (stat(p.c_str(), &s) == 0);
    }

    bool MakeDir(const String& p)
    {
        if (p.empty())
            return true;
        // TODO Use the boost library, which has a better implementation that this one
        String::Vector parts;
        p.split(parts, SeparatorAsString, false);
        String pth;
        bool hasBeenCreated(false);

        for (String::Vector::const_iterator i = parts.begin(); i != parts.end(); ++i)
        {
            pth += *i;
	        # ifndef TA3D_PLATFORM_WINDOWS
            pth += Separator;
            # endif
            if (!Exists(pth))
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
	        # ifdef TA3D_PLATFORM_WINDOWS
            pth += Separator;
	        # endif
        }
        if (hasBeenCreated)
            LOG_INFO("Created folder: `" << p << "`");
        return true;
    }

    template<class T>
    bool TmplGlob(T& out, const String& pattern, const bool emptyListBefore)
    {
        if (emptyListBefore)
            out.clear();
        struct al_ffblk info;
        if (al_findfirst(pattern.c_str(), &info, FA_ALL) != 0)
            return false;
        String root = ExtractFilePath(pattern);
        do
        {
            out.push_back(root + (const char*)info.name);
        } while (al_findnext(&info) == 0);
        return !out.empty();
    }

    bool Glob(String::List& out, const String& pattern, const bool emptyListBefore)
    {
        return TmplGlob< String::List >(out, pattern, emptyListBefore);
    }

    bool Glob(String::Vector& out, const String& pattern, const bool emptyListBefore)
    {
        return TmplGlob< String::Vector >(out, pattern, emptyListBefore);
    }


    bool IsAbsolute(const String& p)
    {
        # ifdef TA3D_PLATFORM_WINDOWS
        return (p.empty() || (p.size() > 2 && ':' == p[1] && '\\' == p[2]));
        # else
        return (p.empty() || '/' == p[0]);
        # endif
    }
    

} // namespace Paths
} // namespace TA3D

