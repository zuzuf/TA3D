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
#include <QProcess>
#include <QSysInfo>
#include <QScreen>
#include <QGuiApplication>

using namespace Yuni;



namespace TA3D
{
namespace System
{


	QString run_command(const QString &cmd)
	{
        if (cmd.isEmpty())
            return QString();
        QProcess proc;
        proc.start(cmd, QIODevice::ReadOnly);
        if (!proc.waitForFinished())
            return QString();

        return QString::fromLocal8Bit(proc.readAllStandardOutput());
	}




    namespace // anonymous
    {
		QString CPUName()
        {
            return QSysInfo::currentCpuArchitecture();
        }


		QString CPUCapabilities()
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
        QScreen *screen = QGuiApplication::primaryScreen();
        if (!screen)
            return false;
        width = screen->size().width();
        height = screen->size().height();
        colorDepth = screen->depth();
        return true;
	}



	void DisplayInformations()
	{
		// Vendor
		QString vendorName;
#ifdef TA3D_PLATFORM_LINUX
		vendorName = run_command("cat /proc/cpuinfo | grep vendor_id | head -n 1 | awk '{ print $3 }' | tr -d \"\\n\"");
#else
#endif
        logs.notice() << LOG_PREFIX_SYSTEM << YUNI_OS_NAME << ", Vendor: " << (vendorName.isEmpty() ? "Unknown" : vendorName)
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
