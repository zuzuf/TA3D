#include "../stdafx.h"
#include "paths.h"
#ifndef TA3D_PLATFORM_WINDOWS
# include <stdlib.h>
#else
# include <windows.h>
# include <shlobj.h>
#endif
#include <sys/stat.h>
#include "../TA3D_NameSpace.h"
#include "../logs/logs.h"
#include <unistd.h>
#include <fstream>




namespace TA3D
{
namespace Paths
{

    String ApplicationRoot = "";
    String Caches = "";
    String Savegames = "";
    String Logs = "";
    String LogFile = "";
    String Preferences = "";
    String ConfigFile = "";
    String Screenshots = "";
    String Resources = "";
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


    namespace
    {

    # ifdef TA3D_PLATFORM_WINDOWS

    /*!
     * \brief Get the absolute path to the local application data folder
     * (from the Windows registry)
     */
    std::string localAppData()
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
        String home = getenv("HOME");
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
        String home = getenv("HOME");
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
     * \brief Initialize the ApplicationRoot variable
     * \param argv0 Equivalent to argv[0] from the main
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
                ApplicationRoot = ExtractFilePath(r);
        }
    }


    } // namespace



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
        if (ret.size() > 0 && p.size() > 0 && ret[0] != p[0])        // Make sure the result begins like p
            ret = p[0] + ret;
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

    void ExtractFileName(String::List& p, const bool systemDependant)
    {
        for(String::List::iterator i = p.begin() ; i != p.end() ; i++)
            *i = ExtractFileName( *i, systemDependant );
    }

    void ExtractFileName(String::Vector& p, const bool systemDependant)
    {
        for(String::Vector::iterator i = p.begin() ; i != p.end() ; i++)
            *i = ExtractFileName( *i, systemDependant );
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




    bool Initialize(int argc, char* argv[], const String& programName)
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

        bool logFileOpened = Logs::logger().writeToFile(Paths::Logs + programName + ".log");

        LOG_INFO("*** Welcome to TA3D ***");
        LOG_INFO("Version: " << TA3D_VERSION_HI << "." << TA3D_VERSION_LO << "-" << TA3D_VERSION_TAG
                 << " (r" << TA3D_CURRENT_REVISION << ")");
        LOG_INFO(LOG_PREFIX_PATHS << "Started from: `" << ApplicationRoot << "`");
        ConfigFile = Preferences;
        ConfigFile += "ta3d.cfg";
        LOG_INFO(LOG_PREFIX_PATHS << "Preferences: `" << Preferences << "`");
        LOG_INFO(LOG_PREFIX_PATHS << "Cache: `" << Caches << "`");
        LOG_INFO(LOG_PREFIX_PATHS << "Savegames: `" << Savegames << "`");
        LOG_INFO(LOG_PREFIX_PATHS << "Screenshots: `" << Screenshots << "`");
        LOG_INFO(LOG_PREFIX_PATHS << "Logs: `" << Logs << "`");
        if (!logFileOpened)
            LOG_ERROR("[logs] Impossible to open `" << Paths::Logs << programName << ".log`");
        else
            LOG_INFO("Opened the log file: `" << Paths::LogFile);

        bool res = MakeDir(Caches) && MakeDir(Savegames) && MakeDir(Logs)
            && MakeDir(Preferences) && MakeDir(Screenshots) && MakeDir(Resources)
            && MakeDir(Savegames + "multiplayer" + Paths::Separator);
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
        struct _stat s;
        if ('\\' != p[p.size() -1])
            return (_stat(p.c_str(), &s) == 0);
        return (_stat(String(p, 0, p.size() - 1).c_str(), &s) == 0);
        # else
        struct stat s;
        return (stat(p.c_str(), &s) == 0);
	    # endif
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
                if (mkdir(pth.c_str(), 01755))
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
    bool TmplGlob(T& out, const String& pattern, const bool emptyListBefore, const uint32 fileAttribs = FA_ALL, const uint32 required = 0)
    {
        if (emptyListBefore)
            out.clear();
        struct al_ffblk info;
        if (al_findfirst(pattern.c_str(), &info, fileAttribs) != 0)
            return false;
        String root = ExtractFilePath(pattern);
        do
        {
            if((info.attrib&required) == required && String(info.name) != "." && String(info.name) != "..")
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

    bool GlobFiles(String::List& out, const String& pattern, const bool emptyListBefore)
    {
        return TmplGlob< String::List >(out, pattern, emptyListBefore, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_ARCH);
    }

    bool GlobFiles(String::Vector& out, const String& pattern, const bool emptyListBefore)
    {
        return TmplGlob< String::Vector >(out, pattern, emptyListBefore, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_ARCH);
    }

    bool GlobDirs(String::List& out, const String& pattern, const bool emptyListBefore)
    {
        return TmplGlob< String::List >(out, pattern, emptyListBefore, FA_ALL, FA_DIREC);
    }

    bool GlobDirs(String::Vector& out, const String& pattern, const bool emptyListBefore)
    {
        return TmplGlob< String::Vector >(out, pattern, emptyListBefore, FA_ALL, FA_DIREC);
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

