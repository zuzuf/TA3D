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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "cError.h"

#ifdef TA3D_PLATFORM_LINUX
	#include <errno.h>
#endif

namespace TA3D
{
	/* private */ void cError::MakeSysErrMsg()
	{
#ifdef TA3D_PLATFORM_WINDOWS
		char szError[512];

		if( !FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM,
			0, m_LastError, 0, &szError[0], 511, NULL) )
			m_szDesc = String( "Unable to extract error code from system." );
		else
			m_szDesc = szError;
#else
		m_szDesc = String( strerror( m_LastError ) );
#endif

	}

	void cError::LogError()
	{
		if( TA3D::VARS::InterfaceManager == NULL )
			return;

		String szErrReport;

		szErrReport = format( "***** Error Report *****\nWhere:%s\n Desc:%s\n",
			m_szWhere.c_str(), m_szDesc.c_str() );

		I_Msg( TA3D_IM_DEBUG_MSG, (void *)szErrReport.c_str(), NULL, NULL );
		m_Logged = true;
	}

	// Constructors:
	cError::cError( const String &Where, cErrorType LastError, bool bLogIt ) :
	    m_szWhere(Where), m_szDesc( String( "" ) ), m_LastError( LastError ), m_Logged( false )
	{
		MakeSysErrMsg();

		if( bLogIt )
			LogError();
	}

	cError::cError( const String &Where, const String &Desc, bool bLogIt ) :
	    m_szWhere(Where), m_szDesc( Desc ), m_LastError( 0 ), m_Logged( false )
	{
		if( bLogIt )
			LogError();
	}
   
	cError::cError( const String &Where, bool bLogIt ) :
	    m_szWhere(Where), m_szDesc( String( "None" ) ), m_LastError( 0 ), m_Logged( false )
	{
		if( bLogIt )
			LogError();
	}

	/* public */void cError::DisplayError()
	{
		String szErrReport;

		szErrReport = format( "A error has just occured within the application.\n%s\nPlease report this to our forums so that we might fix it.\n\nWhere:%s\n Desc:%s\n",
		    ( (m_Logged) ? "It was trapped and logged." : "It was trapped but NOT logged." ),
		    m_szWhere.c_str(), m_szDesc.c_str() );

#ifdef TA3D_PLATFORM_WINDOWS
		::MessageBoxA( NULL, szErrReport.c_str(), "TA3D Application Error", MB_OK  | MB_TOPMOST | MB_ICONERROR );
#else
		allegro_message( szErrReport.c_str() );
#endif
	}

/* public */bool cError::LogTo( const String &FileName, bool DisplayErr )
	{
		std::ofstream   m_File;

		m_File.open( FileName.c_str(), std::ios::out | std::ios::trunc );

		if( !m_File.is_open() )
		{
			if( DisplayErr )
				DisplayError();
			return false;
		}

		m_Logged = true;
		String szErrReport = format( "***** Error Report *****\n%s%s\n",
		    m_szWhere.c_str(), m_szDesc.c_str() );

		m_File << szErrReport;

		m_File.flush();

		m_File.close();

		if( DisplayErr )
			DisplayError();

		return true;
	}

} // namespace TA3D

