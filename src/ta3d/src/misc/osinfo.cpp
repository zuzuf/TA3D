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

#include <stdafx.h>
#include "osinfo.h"
#include "string.h"
#include <logs/logs.h>
#include <sdl.h>

#ifdef YUNI_OS_MACOS
# include <stdio.h>
# define TA3D_POPEN   popen
# define TA3D_PCLOSE  pclose
#endif
#ifdef TA3D_PLATFORM_LINUX
# define TA3D_POPEN   popen
# define TA3D_PCLOSE  pclose
#elif defined TA3D_PLATFORM_WINDOWS
# define TA3D_POPEN   _popen
# define TA3D_PCLOSE	 _pclose
#endif


using namespace Yuni;



namespace TA3D
{
namespace System
{


	String run_command(const String &cmd)
	{
		if (!cmd)
			return nullptr;
		# ifdef TA3D_POPEN
		String result;
		FILE* pipe = TA3D_POPEN(cmd.c_str(), "r");
		if (!pipe)
			return result;

		while (!feof(pipe))
		{
			const int c = fgetc(pipe);
			if (c == -1)
				return result;
			result << (char)c;
		}

		TA3D_PCLOSE(pipe);
		return result;

		# else
		PleaseImplementRunCommandForTheCurrentOS;
		return nullptr;
		# endif
	}




    namespace // anonymous
    {
		String CPUName()
        {
			# ifdef TA3D_PLATFORM_LINUX
			return run_command("cat /proc/cpuinfo | grep \"model name\" | head -n 1 | tail -c +14 | tr -d \"\\n\"");
			# else
			return "Unknown";
			# endif
        }


		String CPUCapabilities()
        {
			# ifdef TA3D_PLATFORM_LINUX
			return run_command("cat /proc/cpuinfo | grep flags | head -n 1 | tail -c +10 | tr -d \"\\n\"");
			# else
            return "None";
			# endif
        }

        void displayScreenResolution()
        {
            int w, h, d;
            if (DesktopResolution(w, h, d))
                logs.notice() << LOG_PREFIX_SYSTEM << "Desktop: " << w << "x" << h << " (" << d << "bits)";
            else
                logs.error()  << LOG_PREFIX_SYSTEM << "Error while retrieving information about the desktop resolution";
        }

    } // anonymous namespace






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
#ifdef TA3D_PLATFORM_LINUX
		vendorName = run_command("cat /proc/cpuinfo | grep vendor_id | head -n 1 | awk '{ print $3 }' | tr -d \"\\n\"");
#else
#endif
		logs.notice() << LOG_PREFIX_SYSTEM << YUNI_OS_NAME << ", Vendor: " << (vendorName.empty() ? "Unknown" : vendorName)
			<< " " << CPUName()
			<< " (" << CPUCapabilities() << ")";
		displayScreenResolution();
	}




	void DisplayInformationsAboutSDL()
	{
		const SDL_version * v = SDL_Linked_Version();
		logs.info() << LOG_PREFIX_SYSTEM << "SDL version: " << (int)v->major << "." << (int)v->minor << "." << (int)v->patch;
	}




} // namespace System
} // namespace TA3D
