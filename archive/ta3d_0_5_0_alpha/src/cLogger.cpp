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
#include "cLogger.h"




namespace TA3D
{
	namespace INTERFACES
	{
		cLogger::cLogger( const String &FileName, bool noInterface )
		{
			m_bNoInterface = true;

			m_File = TA3D_OpenFile( FileName, "wt" );

			if( !m_File ) // todo fix this.
				throw( "cLogger::Failed to open file for output" );

			m_bNoInterface = noInterface;

			CreateCS();

			if( !m_bNoInterface )
				InitInterface();
		}

		cLogger::~cLogger( )
		{
			if( !m_File )
				return;

			fclose( m_File );

			if( !m_bNoInterface )
				DeleteInterface();

			DeleteCS();
		}

		void cLogger::LogData( const char *txt )
		{
			EnterCS();
			fputs(txt, m_File);
			fflush( m_File );
			LeaveCS();            
		}

		void cLogger::LogData(const std::string &txt)
		{
			EnterCS();
			fputs( txt.c_str(), m_File );
			fflush( m_File );
			LeaveCS();
		}

		FILE *cLogger::get_log_file()
		{
			return m_File;
		}

		uint32 cLogger::InterfaceMsg( const lpcImsg msg )
		{
			if( msg->MsgID != TA3D_IM_DEBUG_MSG )
				return INTERFACE_RESULT_CONTINUE;

			LogData( (const char *)(msg->lpParm1) );

			return INTERFACE_RESULT_CONTINUE;
		}
	}
}
