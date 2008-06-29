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

#ifndef __CLASS_I18N__

#define __CLASS_I18N__

#include "misc/hash_table.h"


class I18N_TRANSLATER : protected TA3D::UTILS::cTAFileParser
{
private:
	int			loaded_language;	// Translations currently loaded
	String		language;			// Current language

public:

	I18N_TRANSLATER();
	~I18N_TRANSLATER();
	void refresh_language();
	void load_translations( const String &filename, bool adding = false, bool inASCII = false );					// Loads translations for current language
	String translate( const String &str );								// Translate str if possible, otherwise return str
	void translate( Vector< String > &vec );						// Translate vec if possible, otherwise leave it as is
};

#define TRANSLATE(str)	i18n.translate( str )

#endif
