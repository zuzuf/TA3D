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
