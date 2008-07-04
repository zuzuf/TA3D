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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "cTAFileParser.h"
#include "cError.h"

namespace TA3D
{
	namespace UTILS
	{
		String cTAFileParser::GetLine( char **Data )
		{
			for( char *result = *Data ; **Data ; (*Data)++ )
				if( **Data == '\n' || **Data == ';' || **Data == '{' || **Data == '}' ) {
					(*Data)++;
					return String( result, *Data );
					}
			
			return *Data;
		}

		bool cTAFileParser::ProcessData(  char **Data )
		{
			if( **Data == 0 )
				return true;

			String Line = GetLine( Data );         // extract line
			if( Line.size() == 0 )	return false;

			int i = (int)Line.find( "//" );        // search for start of comment.
			if( i != -1 )        // if we find a comment, we will erase everything
				Line.resize( i );// from the comment to the end of the line.

 			Line = TrimString( Line, " \t\n\r{" ); // strip out crap from string.
			i = (int)Line.length();

			if( i > 3 ) // check for new key.
			{
				if( Line[0] == '[' && Line[ i-1 ] == ']' )
				{
					bool changed = false;
					if( gadget_mode >= 0 && key_level.empty() ) {
						m_cKey = format("gadget%d", gadget_mode );
						gadget_mode++;
						changed = true;
						if( !m_bKeysCaseSenstive )
							InsertOrUpdate( m_cKey, Lowercase( Line.substr( 1, i-2 ) ) );		// Link the key with its original name
						else
							InsertOrUpdate( m_cKey, Line.substr( 1, i-2 ) );		// Link the key with its original name
						}

					key_level.push_front( m_cKey );
					if( !changed )
						m_cKey = ( ( m_cKey == "" ) ? Line.substr( 1, i-2 ) :
						                        m_cKey + String( "." ) +
						                        Line.substr( 1, i-2 ) );

					m_cKey = ReplaceString( m_cKey, "\\n", "\n", false );
					m_cKey = ReplaceString( m_cKey, "\\r", "\r", false );

					while( **Data )										// Better using the stack this way, otherwise it might crash with huge files
						if( ProcessData( Data ) )	break;
					return false;
				}
			}
			else if( i > 0 && Line[ i - 1 ] == '}' )  // close the current active key.
			{
				if( key_level.empty() )
					m_cKey = "";
				else {
					m_cKey = key_level.front();
					key_level.pop_front();
					}

				return true;
			}

			// if we get here its possible its a name=value;
			// so we will search for a = and a ; if we find them we have a valid name/value
			int f1, f2;
			f1 = (int)Line.find_first_of( "=" );
			f2 = (int)Line.find_last_of( ";" );

			if( f1 != -1 && f2 != -1 )
			{
				String n, v;
				n = Line.substr( 0, f1 );
				v = Line.substr( (f1+1), (f2-(f1+1)) );

				if( n[ n.size() - 1 ] == ' ' )
					n.erase( n.size() - 1, 1 );
				if( v[ 0 ] == ' ' )
					v.erase( 0, 1 );

				v = ReplaceString( v, "\\n", "\n", false );
				v = ReplaceString( v, "\\r", "\r", false );

				n = m_cKey + String( "." ) + n;

				if( !m_bKeysCaseSenstive )
					InsertOrUpdate( Lowercase( n ), v );
				else
					InsertOrUpdate( n, v );
			}

			return false;
		}

		void cTAFileParser::Load( const String &FileName,  bool bClearTable, bool toUTF8, bool g_mode )
		{
			uint32 ota_size=0;
			byte *data = NULL;
			std::ifstream   m_File;
			String tFileName = GetClientPath() + FileName;

			if( bClearTable ) {
				uint32 old_tablesize = m_u32TableSize;
				EmptyHashTable();
				InitTable( old_tablesize );
				}

			if( TA3D::VARS::HPIManager )
				data = TA3D::VARS::HPIManager->PullFromHPI( FileName, &ota_size );
			else {
				m_File.open( tFileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
				if( m_File.is_open() ) {
					ota_size = m_File.tellg();
					data = (byte*) malloc( ota_size+1 );
					data[ ota_size ] = 0;
					m_File.seekg( 0, std::ios::beg );
					m_File.read( (char *)data, ota_size );
					m_File.close();
					}
				}

			if( !data )
				throw ( String( "Unable to load file " ) + FileName );

			if( data != NULL && toUTF8 ) {		// Convert from ASCII to UTF8, required because TA3D works with UTF8 and TA with ASCII
				char *tmp = (char*) malloc( ota_size * 2 );

				do_uconvert( (const char*)data, U_ASCII, tmp, U_UTF8, ota_size * 2 );

				free( data );
				data = (byte*)tmp;
				}

			m_cKey = "";
			key_level.clear();

			// erase all line feeds. (linear algorithm)
			char *tmp = (char*)data;
			int e = 0, i = 0;
			for( ; tmp[i] ; i++ ) {
				if( tmp[ i ] != '\r' ) {
					if( e )	tmp[ i - e ] = tmp[ i ];
					}
				else
					e++;
				}
			if( e > 0 )
				tmp[i] = 0;

			gadget_mode = g_mode ? 0 : -1;

			// now process the remaining.
			while( *tmp )
				ProcessData( &tmp );
			free(data);
		}

		void cTAFileParser::LoadMemory( char *data, bool bClearTable, bool toUTF8, bool g_mode )
		{
			if( bClearTable ) {
				uint32 old_tablesize = m_u32TableSize;
				EmptyHashTable();
				InitTable( old_tablesize );
			}

			if( !data )
				throw ( "no data provided!!" );

			if( toUTF8 ) {		// Convert from ASCII to UTF8, required because TA3D works with UTF8 and TA with ASCII
				uint32 size = strlen( data );
				char *tmp = (char*) malloc( size * 2 );

				do_uconvert( (const char*)data, U_ASCII, tmp, U_UTF8, size * 2 );

				data = (char*)tmp;
			}

			m_cKey = "";
			key_level.clear();

			// erase all line feeds. (linear algorithm)
			char *tmp = data;
			int e = 0, i = 0;
			for( ; tmp[i] ; i++ ) {
				if( tmp[ i ] != '\r' ) {
					if( e )	tmp[ i - e ] = tmp[ i ];
				}
				else
					e++;
			}
			if( e > 0 )
				tmp[i] = 0;

			gadget_mode = g_mode ? 0 : -1;

			// now process the remaining.
			while( *tmp )
				ProcessData( &tmp );
			if( toUTF8 )
				free(data);
		}

		cTAFileParser::cTAFileParser( const String &FileName,  bool bKeysCaseSenstive, bool toUTF8, bool g_mode )
		{
			m_bKeysCaseSenstive = bKeysCaseSenstive;
			InitTable( 4096 );
			Load( FileName, true, toUTF8, g_mode );
		}

		cTAFileParser::cTAFileParser( uint32 TableSize )
		{
			m_bKeysCaseSenstive = false;
			InitTable( TableSize );
		}

		cTAFileParser::~cTAFileParser()
		{
			EmptyHashTable();
		}

		sint32 cTAFileParser::PullAsInt( const String &KeyName , sint32 def )
		{
			String key_to_find;
			if( !m_bKeysCaseSenstive )
				key_to_find = Lowercase( KeyName );
			else
				key_to_find = KeyName;
			if( !Exists( key_to_find ) )	return def;

			String iterFind = Find( key_to_find );

			return ( (iterFind.length() == 0) ? def : ( iterFind.size() == 10 && ustrtol( iterFind.substr(0,4).c_str() , NULL, 0 ) > 127 ? ( 0xFF000000 | ustrtol( ("0x"+iterFind.substr(4,6)).c_str() , NULL, 0 ) ) : ustrtol( iterFind.c_str() , NULL, 0 ) ) );		// Uses ustrtol to deal with hexa numbers
		}

		real32 cTAFileParser::PullAsFloat( const String &KeyName , real32 def )
		{
			String key_to_find;
			if( !m_bKeysCaseSenstive )
				key_to_find = Lowercase( KeyName );
			else
				key_to_find = KeyName;
			if( !Exists( key_to_find ) )	return def;

			String iterFind = Find( key_to_find );

			return ( (iterFind.length() == 0) ? def : (float)atof( iterFind.c_str() ) );
		}

		String cTAFileParser::PullAsString( const String &KeyName , String def )
		{
			String key_to_find;
			if( !m_bKeysCaseSenstive )
				key_to_find = Lowercase( KeyName );
			else
				key_to_find = KeyName;
			if( !Exists( key_to_find ) )	return def;

			String iterFind = Find( key_to_find );

			return iterFind;
		}

		bool cTAFileParser::PullAsBool( const String &KeyName , bool def )
		{
			String key_to_find;
			if( !m_bKeysCaseSenstive )
				key_to_find = Lowercase( KeyName );
			else
				key_to_find = KeyName;
			if( !Exists( key_to_find ) )
				return def;

			String iterFind = Lowercase( Find( key_to_find ) );

			if( iterFind == "false" || iterFind == "0" || iterFind == "" )
				return false;

			return true;
		}
	}
} 
