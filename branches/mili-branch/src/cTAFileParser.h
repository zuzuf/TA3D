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

#ifndef __TA3D_X_HASH_TABLE_H__
# define __TA3D_X_HASH_TABLE_H__

#include "misc/hash_table.h"


namespace TA3D
{
namespace UTILS
{


    class cTAFileParser : protected TA3D::UTILS::cHashTable<String>
    {
    private:
        std::string m_cKey;     // used when building keys.
        std::list< std::string > key_level;	// used when building keys.
        bool m_bKeysCaseSenstive;	// Is it case sensitive ?
        int gadget_mode;

        bool ProcessData( char **Data );
        String GetLine( char **Data );

    protected:
        // A hash map of key/values

    public:
        void Load( const String &FileName, bool bClearTable=false, bool toUTF8 = false, bool g_mode = false ); 
        cTAFileParser( const String &FileName, bool bKeysCaseSenstive = false, bool toUTF8 = false, bool g_mode = false );
        cTAFileParser( uint32 TableSize = 4096 );
        ~cTAFileParser();

        void LoadMemory( char *data, bool bClearTable=false, bool toUTF8 = false, bool g_mode = false );

        sint32	PullAsInt( const std::string &KeyName , sint32 def = 0 );
        real32	PullAsFloat( const std::string &KeyName , real32 def = 0.0f );
        String	PullAsString( const std::string &KeyName , String def = "" );
        bool	PullAsBool( const std::string &KeyName , bool def = false );

    }; // class cTAFileParser

}
} 

#endif // __TA3D_X_HASH_TABLE_H__
