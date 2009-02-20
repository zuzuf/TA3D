#include "../stdafx.h"
#include "osinfo.h"
#include "string.h"
#include "../logs/logs.h"


# define CPU_MODEL_ATHLON64_N 15



namespace TA3D
{
namespace System
{


    namespace
    {

    String OSType()
    {
#ifdef TA3D_PLATFORM_WINDOWS
        return "Microsoft Windows";
#elif defined TA3D_PLATFORM_LINUX
        return "GNU/Linux";
#elif defined TA3D_PLATFORM_DARWIN
        return "MacOSX";
#else
        return "Unknown";
#endif
    }



    String CPUName()
    {
        return "Unknown";
    }


    String CPUCapabilities()
    {
        String r;
        return (r.empty() ? "None" : r);
    }

    void displayScreenResolution()
    {
        int w, h, d;
        if (DesktopResolution(w, h, d))
            LOG_INFO(LOG_PREFIX_SYSTEM << "Desktop: " << w << "x" << h << " (" << d << "bits)");
        else
            LOG_ERROR(LOG_PREFIX_SYSTEM << "Error while retrieving information about the desktop resolution");
    }


    } // unnamed namespace





    bool DesktopResolution(int& width, int& height, int& colorDepth)
    {
        const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
        if (videoInfo)
        {
            width = videoInfo->current_w;
            height = videoInfo->current_h;
            colorDepth = videoInfo->vfmt->BitsPerPixel;
            return true;
        }
        return false;
    }



    void DisplayInformations()
    {
         // Vendor
        String vendorName;
        LOG_INFO(LOG_PREFIX_SYSTEM << "Vendor: " << (vendorName.empty() ? "Unknown" : vendorName)
                 << " " << CPUName()
                 << " (" << CPUCapabilities() << ")");
       // OS Name
        LOG_INFO(LOG_PREFIX_SYSTEM << OSType());
        displayScreenResolution();
    }


    void DisplayInformationsAboutSDL()
    {
        const SDL_version * v = SDL_Linked_Version();
        LOG_INFO(LOG_PREFIX_SYSTEM << "SDL version: " << (int)v->major << "." << (int)v->minor << "." << (int)v->patch);
    }


} // namespace System
} // namespace TA3D
