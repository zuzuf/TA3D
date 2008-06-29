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

/*-----------------------------------------------------------------------------------\
|                                        i18n.h                                      |
|  This file contains the translation manager which uses a hashtable to store it's   |
| data, and the cTAFileParser module to load translations from a TDF file (ta3d.res, |
| 3dmeditor.res) which tells what sentences can be translated. Case insensitive!!    |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "cTAFileParser.h"
#include "i18n.h"

I18N_TRANSLATER::I18N_TRANSLATER() : TA3D::UTILS::cTAFileParser( 8192 )
{
	loaded_language = -1;
}

I18N_TRANSLATER::~I18N_TRANSLATER()
{
	EmptyHashTable();
	loaded_language = -1;
}

void I18N_TRANSLATER::refresh_language()
{
	if( loaded_language == lp_CONFIG->Lang )	return;		// Already loaded

	loaded_language = lp_CONFIG->Lang;

	const char *languages[] = { "english", "french", "german", "spanish", "italian", "japanese" };
	language = format( ".%s", languages[ loaded_language ] );				// Select the corresponding sub-key
}

void I18N_TRANSLATER::load_translations( const String &filename, bool adding, bool inASCII )					// Loads translations for current language
{
	if( loaded_language == lp_CONFIG->Lang && !adding )	return;		// Already loaded

	Load( filename, false, inASCII );			// Don't empty the hash table so we can merge data from different files

	loaded_language = lp_CONFIG->Lang;

	const char *languages[] = { "english", "french", "german", "spanish", "italian", "japanese" };
	language = format( ".%s", languages[ loaded_language ] );				// Select the corresponding sub-key
}

String I18N_TRANSLATER::translate( const String &str )		// Translate str if possible, otherwise return str
{
	if( str == "" )	return str;
	return PullAsString( Lowercase( str ) + language, str );
}

void I18N_TRANSLATER::translate( Vector< String > &vec )						// Translate vec if possible, otherwise leave it as is
{
    for (Vector<String>::iterator i = vec.begin(); i != vec.end(); ++i)
		*i = translate(*i);
}
