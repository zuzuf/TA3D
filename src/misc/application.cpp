
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

        // allegro_exit(); // Should not be needed since allegro_init() installs it as an atexit() routine 

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

        // Install our atexit function before allegro
        // Like this, allegro_exit() will be called before Finalize()
        atexit(Finalize);
        allegro_init();

        // Display usefull infos for debugging
        System::DisplayInformations();
        System::DisplayInformationsAboutAllegro();
    }


}
