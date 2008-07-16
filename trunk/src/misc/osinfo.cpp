#include "../stdafx.h"
#include "osinfo.h"
#include "string.h"
#include "../logs/logs.h"


# define TA3D_LOGS_PREFIX_SYSTEM "[system] "

# define CPU_MODEL_ATHLON64_N 15



namespace TA3D
{
namespace System
{

    
    static String AllegroOSTypeToStr(const int x)
    {
        switch(x)
        {
            case OSTYPE_WIN3:       return "Microsoft Windows 3.1"; break;
            case OSTYPE_WIN95:      return "Microsoft Windows 95"; break;
            case OSTYPE_WIN98:      return "Microsoft Windows 98"; break;
            case OSTYPE_WINME:      return "Microsoft Windows ME"; break;
            case OSTYPE_WINNT:      return "Microsoft Windows NT"; break;
            case OSTYPE_WIN2000:    return "Microsoft Windows 2000"; break;
            case OSTYPE_WINXP:      return "Microsoft Windows XP"; break;
            case OSTYPE_WIN2003:    return "Microsoft Windows 2003"; break;
            case OSTYPE_WINVISTA:   return "Microsoft Windows Vista"; break;
            case OSTYPE_LINUX:      return "GNU/Linux"; break;
            case OSTYPE_OS2:        return "OS/2 Warp 3"; break;
            case OSTYPE_DOSEMU:     return "Linux DOSEMU"; break;
            case OSTYPE_OPENDOS:    return "Caldera OpenDOS"; break;
            case OSTYPE_FREEBSD:    return "BSD/FreeBSD"; break;
            case OSTYPE_NETBSD:	    return "BSD/NetBSD"; break;
            case OSTYPE_UNIX:       return "Unix variant"; break;
            case OSTYPE_BEOS:       return "BeOS"; break;
            case OSTYPE_QNX:        return "QNX"; break;
            case OSTYPE_MACOS:      return "MacOS Classic"; break;
            case OSTYPE_MACOSX:     return "MacOSX"; break;
        }
        return "Unknown";
    }



    static String AllegroCPUName(int cpuFamily, int cpuModel)
    {
        switch(cpuFamily)
        {
            case CPU_FAMILY_I386:       return "i386"; break;
            case CPU_FAMILY_I486:       return "i486"; break;
            case CPU_FAMILY_I586:       return "i586"; break;
            case CPU_FAMILY_I686:       return "i686"; break;
            case CPU_FAMILY_ITANIUM:    return "Itanium"; break;
            case CPU_FAMILY_POWERPC:    return "PowerPC"; break;
            case CPU_FAMILY_EXTENDED:
                switch(cpuModel)
                {
                    case CPU_MODEL_PENTIUMIV: return "Pentium4"; break;
                    case CPU_MODEL_XEON: return "Xeon"; break;
                    case CPU_MODEL_ATHLON64_N:
                    case CPU_MODEL_ATHLON64: return "AMD Athlon64"; break;
                    case CPU_MODEL_OPTERON:  return "AMD Opteron"; break;
                    default: return "Unknown";
                }
                break;
            case CPU_FAMILY_UNKNOWN: return "Unknown"; break;
        }
        return "Unknown";
    }


    static String AllegroCPUCapabilities(int c)
    {
        String r;
        if (c & CPU_AMD64)      r += "amd64";
        if (c & CPU_IA64)       r << (!r.empty() ? ", ": "") << "i64";
        if (c & CPU_ID)	        r << (!r.empty() ? ", ": "") << "cpuid";
        if (c & CPU_FPU)        r << (!r.empty() ? ", ": "") << "x87 FPU";
        if (c & CPU_MMX)        r << (!r.empty() ? ", ": "") << "MMX";
        if (c & CPU_MMXPLUS)    r << (!r.empty() ? ", ": "") << "MMX+";
        if (c & CPU_SSE)        r << (!r.empty() ? ", ": "") << "SSE";
        if (c & CPU_SSE2)       r << (!r.empty() ? ", ": "") << "SSE2";
        if (c & CPU_SSE3)       r << (!r.empty() ? ", ": "") << "SSE3";
        if (c & CPU_3DNOW)      r << (!r.empty() ? ", ": "") << "3DNow!";
        if (c & CPU_ENH3DNOW)   r << (!r.empty() ? ", ": "") << "Enhanced 3DNow!";
        if (c & CPU_CMOV)       r << (!r.empty() ? ", ": "") << "cmov";
        return (r.empty() ? "None" : r);
    }


    bool DesktopResolution(int& width, int& height, int& colorDepth)
    {
        if (0 != get_desktop_resolution(&width, &height))
        {
            // if 0, the values stored in width and height are unspecified, same for the color depth
            width = 640;
            height = 480;
            colorDepth = 16;
            return false;
        }
        colorDepth = desktop_color_depth();
        return true;
    }


    static void displayScreenResolution()
    {
        int w, h, d;
        if (DesktopResolution(w, h, d))
            LOG_INFO(TA3D_LOGS_PREFIX_SYSTEM << "Desktop: " << w << "x" << h << " (" << d << "bits)");
        else
            LOG_ERROR(TA3D_LOGS_PREFIX_SYSTEM << "Error while retrieving information about the desktop resolution");
    }



    void DisplayInformations()
    {
         // Vendor
        String vendorName(cpu_vendor);
        LOG_INFO(TA3D_LOGS_PREFIX_SYSTEM << "Vendor: " << (vendorName.empty() ? "Unknown" : vendorName)
                 << " " << AllegroCPUName(cpu_family, cpu_model)
                 << " (" << AllegroCPUCapabilities(cpu_capabilities) << ")");
       // OS Name
        LOG_INFO(TA3D_LOGS_PREFIX_SYSTEM << AllegroOSTypeToStr(os_type)
                 << " (" << os_version << "." << os_revision << ")");
        displayScreenResolution();
    }


    void DisplayInformationsAboutAllegro()
    {
        LOG_INFO(TA3D_LOGS_PREFIX_SYSTEM << "Allegro version: " << ALLEGRO_VERSION_STR << " (" << ALLEGRO_DATE_STR << ")");

        # ifdef AGL_VERSION // Version d'allegroGL
        LOG_INFO(TA3D_LOGS_PREFIX_SYSTEM << "AllegroGL version: " << AGL_VERSION_STR);
        # else
        LOG_INFO(TA3D_LOGS_PREFIX_SYSTEM << "AllegroGL version: unknown");
        # endif
    }


} // namespace System
} // namespace TA3D
