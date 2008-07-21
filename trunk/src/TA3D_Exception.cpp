/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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

/*
**  File: TA3D_Exception.cpp
** Notes:
**   Cire: See notes in TA3D_Exception.h
*/


#include "TA3D_Exception.h"
#include "TA3D_NameSpace.h"
#include "misc/paths.h"
#ifdef TA3D_PLATFORM_LINUX
# include <errno.h>
#endif
#include <fstream>



namespace TA3D
{
namespace Exceptions
{

    # ifdef TA3D_PLATFORM_WINDOWS
    typedef DWORD cErrorType;
    #   define GETSYSERROR() GetLastError()
    # else
    typedef int cErrorType;
    #   define GETSYSERROR() errno
    # endif

    namespace
    {

        //!
        String pGuardLog = "";
        //!
        bool pExceptionStatus = false;
        //!
        uint16 pIndentLevel = 0;

    }



    const String GetGuardLog()
    {
        return pGuardLog;
    }


    void GuardIncrementIndent()
    {
        #ifdef TA3D_USE_INTERNAL_EXCEPTIONS
        ++pIndentLevel;
        #endif
    }


    const String GuardIndentPadding()
    {
        String x;
        for (uint16 i = 0; i < pIndentLevel; ++i)
            x += "   ";
        return x;
    }

    String GuardGetSysError()
    {
        #ifdef TA3D_USE_INTERNAL_EXCEPTIONS
        
        cErrorType m_LastError = GETSYSERROR();
        # ifdef TA3D_PLATFORM_WINDOWS
        char szError[512];
        if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, m_LastError, 0, &szError[0], 511, NULL))
            return "Unable to extract error code from system.";
        return szError;
        # else
        return strerror(m_LastError);
        # endif

        #else
        return "";
        #endif // TA3D_USE_INTERNAL_EXCEPTIONS
    }

    void GuardDecrementIndent (void)
    {
        #ifdef TA3D_USE_INTERNAL_EXCEPTIONS
        if (pIndentLevel != 0)
            --pIndentLevel;
        #endif
    }

    void GuardReset(void)
    {
        #ifdef TA3D_USE_INTERNAL_EXCEPTIONS
        pIndentLevel = 0;
        pGuardLog = "";
        #endif
    }

    void GuardLog(const String& szLog)
    {
        #ifdef TA3D_USE_INTERNAL_EXCEPTIONS
        pGuardLog += szLog;
        #endif
    }


    bool IsExceptionInProgress (void)
    {
        return pExceptionStatus;
    }

    void SetExceptionStatus (bool e)
    {
        pExceptionStatus = e;
        return;
    }

    void GuardDisplayAndLogError()
    {
        #ifdef TA3D_USE_INTERNAL_EXCEPTIONS
        String szErrReport;
        std::ofstream   m_File;
        String FileName = TA3D::Paths::Logs + "error.log";
        bool m_Logged = false;

        set_uformat( U_ASCII );		// Switch to good string format

        m_File.open(FileName.c_str(), std::ios::out | std::ios::trunc);
        if( m_File.is_open())
        {
            m_Logged = true;
            szErrReport = format("***** Error Report *****\n%s", pGuardLog.c_str());
            m_File << szErrReport;
            m_File.flush();
            m_File.close();
        }
        szErrReport = format( "A error has just occured within the application.\n%s\nPlease report this to our forums so that we might fix it.\n\nError:\n%s",
                              ( (m_Logged) ? "It was trapped and logged to error.txt." : "It was trapped but NOT logged." ),
                              pGuardLog.c_str() );

        # ifdef TA3D_PLATFORM_WINDOWS
        ::MessageBoxA( NULL, szErrReport.c_str(), "TA3D Application Error", MB_OK  | MB_TOPMOST | MB_ICONERROR );
        # else
        allegro_message( szErrReport.c_str() );
        # endif

        #endif // TA3D_USE_INTERNAL_EXCEPTIONS
    }


} // namespace Exceptions
} // namespace TA3D 

