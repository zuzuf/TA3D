
# ifdef TA3D_PLATFORM_USE_NEW_PATHS_TOOLS // TODO Must be remove

#include "paths.h"
#ifndef TA3D_PLATFORM_WINDOWS
# include <stdlib.h>
# include <sys/stat.h>
#endif 
#include "TA3D_NameSpace.h"



namespace TA3D
{

    String Paths::Caches = "";
    String Paths::Savegames = "";
    String Paths::Logs = "";
    String Paths::Resources = "";
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
        Paths::Caches = "cache";
        Paths::Savegames = "savegame";
        Paths::Logs = "";
        Paths::Resources = "";
    }

    # endif

    # ifndef TA3D_PLATFORM_DARWIN
    static void initForDefaultUnixes()
    {
        String home = getenv("HOME");
        home += "/.ta3d";
        Paths::Caches = home + "/cache";
        Paths::Savegames = home + "/savegame";
        Paths::Logs = home + "/log";
        Paths::Resources = "";
    }

    # else // ifndef TA3D_PLATFORM_DARWIN

    static void initForDarwin()
    {
        String home = getenv("HOME");
        Paths::Caches = home + "/Library/Caches/ta3d";
        Paths::Savegames = home + "/Library/Preferences/ta3d/savegames";
        Paths::Logs = home + "/Library/Logs/ta3d";
        Paths::Resources = "";
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
        return MakeDir(Caches) && MakeDir(Savegames)
            && MakeDir(Logs) && MakeDir(Resources);
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

        Vector<String>::const_iterator i = parts.begin();
        for (; i != parts.end(); ++i)
        {
            pth += *i;
            pth += Separator;
            if (!Exists(pth) && mkdir(pth.c_str(), 01755))
                return false;
        }
        return true;
    }



} // namespace TA3D


# endif // TA3D_PLATFORM_USE_NEW_PATHS_TOOLS
