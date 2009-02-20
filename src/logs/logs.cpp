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
