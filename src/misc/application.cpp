
#include "../stdafx.h"
#include "application.h"
#include "osinfo.h"
#include "paths.h"
#include "../logs/logs.h"
#include "../languages/i18n.h"
#include "resources.h"
#include <stdlib.h>
#include "../TA3D_NameSpace.h"




namespace TA3D
{

    void Finalize()
    {
        LOG_INFO("Aborting now. Releasing all resources...");

        // SDL_Quit(); // Should not be needed since SDL_Quit is installed as an atexit() routine

        DELETEANDNIL(TA3D::VARS::lp_CONFIG);

        I18N::Destroy();
        LOG_INFO("Exit.");

        // Close the log file
        Logs::logger().closeFile();
    }




    void Initialize(int argc, char* argv[], const String& programName)
    {
        Logs::level = LOG_LEVEL_DEBUG;
        // Load and prepare output directories
        if (!TA3D::Paths::Initialize(argc, argv, "ta3d"))
            exit(1);
        TA3D::Resources::Initialize();

        // Install our atexit function before SDL
        // Like this, SDL_Quit() will be called before Finalize()
        atexit(Finalize);
        SDL_Init(SDL_INIT_VIDEO);
        atexit(Finalize);

        // Display usefull infos for debugging
        System::DisplayInformations();
        System::DisplayInformationsAboutSDL();
    }


}
