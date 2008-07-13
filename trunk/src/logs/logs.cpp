#include "logs.h"
#include <fstream>
#include "../misc/paths.h"


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
            std::fstream* o = new std::fstream(filename.c_str(), std::ios::out | std::ios::app);
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


} // Logs
