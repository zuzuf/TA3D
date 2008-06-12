#include "paths.h"
#ifndef TA3D_PLATFORM_WINDOWS
# include <stdlib.h>
# include <sys/stat.h>
#endif 
#include "TA3D_NameSpace.h"
#include "logs/logs.h"



namespace TA3D
{

    String Paths::Caches = "";
    String Paths::Savegames = "";
    String Paths::Logs = "";
    String Paths::Resources = "";
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


    # ifdef TA3D_PLATFORM_WINDOWS

    static void initForWindows()
    {
        Paths::Caches = "cache\\";
        Paths::Savegames = "savegame\\";
        Paths::Logs = "";
        Paths::Resources = "";
        Paths::Preferences = "";
        Paths::Screenshots = "screenshots\\";
    }

    # endif

    # ifndef TA3D_PLATFORM_DARWIN
    static void initForDefaultUnixes()
    {
        String home = getenv("HOME");
        home += "/.ta3d/";
        Paths::Caches = home + "cache/";
        Paths::Savegames = home + "savegame/";
        Paths::Logs = home + "log/";
        Paths::Resources = "";
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
        Paths::Resources = "";
        Paths::Preferences = home + "/Library/Preferences/ta3d/";
        Paths::Screenshots = home + "/Downloads/";
    }

    # endif // ifndef TA3D_PLATFORM_DARWIN




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
        ConfigFile += Separator;
        ConfigFile += "ta3d.fcg";
        LOG_INFO("Folder: Preferences: `" << Preferences << "`");
        LOG_INFO("Folder: Cache: `" << Caches << "`");
        LOG_INFO("Folder: Savegames: `" << Savegames << "`");
        LOG_INFO("Folder: Resources: `" << Resources << "`");
        LOG_INFO("Folder: Screenshots: `" << Screenshots << "`");
        LOG_INFO("Folder: Logs: `" << Logs << "`");
        bool res = MakeDir(Caches) && MakeDir(Savegames)
            && MakeDir(Logs) && MakeDir(Resources)
            && MakeDir(Preferences)
            && MakeDir(Screenshots);
        if (!res)
            LOG_CRITICAL("Aborting now.");
        return res;
    }


    bool
    Paths::Exists(const String& p)
    {
        if (p.empty())
            return true;
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
            pth += Separator;
            if (!Exists(pth))
            {
                if (mkdir(pth.c_str(), 01755))
                {
                    // TODO Use the logging system instead
                    LOG_ERROR("Impossible to create the folder `" << pth << "`");
                    return false;
                }
                else
                    hasBeenCreated = true;
            }
        }
        if (hasBeenCreated)
            LOG_INFO("Created folder: `" << p << "`");
        return true;
    }



} // namespace TA3D


