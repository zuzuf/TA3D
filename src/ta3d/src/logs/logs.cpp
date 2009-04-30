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
#include "../gfx/gfx.toolkit.h"
#include "../misc/paths.h"
#include "logs.h"
#include <fstream>
#include "../console.h"


namespace TA3D
{
namespace Logs
{


	LogDebugMsg debug;

	LogInfoMsg info;

	LogWarningMsg warning;

	LogErrorMsg error;

    LogCriticalMsg critical;

	//! Global object
	Log doNotUseThisLoggerDirectly;

    int level = LOG_LEVEL_INFO;





	Log::Log(std::ostream *outstream, Callback callback)
        :std::ostringstream(),
	    pOut(outstream),
	    pCallback(callback)
	{}

	void Log::callbackOutput(Callback callback)
	{
		pCallback = callback;
		unlock();
	}

    Log & logger()
    {
        doNotUseThisLoggerDirectly.lock();
		return doNotUseThisLoggerDirectly;
    }

    void Log::closeFile()
    {
        if (pOut)
        {
            delete pOut;
            pOut = NULL;
        }
        unlock();
    }

    bool Log::writeToFile(const std::string& filename)
    {
        if (pOut)
        {
            delete pOut;
            pOut = NULL;
        }
        if (!filename.empty())
        {
            std::fstream* o = new std::fstream(filename.c_str(), std::ios::out | std::ios::trunc /*| std::ios::app*/);
            if (!o->is_open())
                delete o;
            else
            {
                pOut = o;
                TA3D::Paths::LogFile = filename;
            }
        }
        bool res = (pOut != NULL);
        unlock();
        return res;
    }



    void LogErrorMsg::forwardToConsole(const std::string& msg) const
    {
        console.addEntry("[error] " + msg);
    }

    void LogWarningMsg::forwardToConsole(const std::string& msg) const
    {
        console.addEntry("[!!] " + msg);
    }

    void LogCriticalMsg::forwardToConsole(const std::string& msg) const
    {
        console.addEntry("[critical] " + msg);
    }

    void LogInfoMsg::forwardToConsole(const std::string& msg) const
    {
        console.addEntry(msg);
    }


} // namespace Logs
} // namespace TA3D
