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

#ifndef __TA3D_MISC_SETTINGS_H__
# define __TA3D_MISC_SETTINGS_H__

# include <stdafx.h>
# include "string.h"



namespace TA3D
{
namespace Settings
{

    /*!
    ** \brief
    **
    ** This function will eventually load our config file if it exists.
    ** config files will be stored as 'tdf' format and thus loaded as text,
    ** using the TDFParser class.
    **
    ** If something goes wrong you can safely throw a string for an error.
    ** The call to this function is tried, but it only catches exceptions
    ** and strings, ie throw( "LoadConfigFile: some error occured" );
    */
    bool Load();

    /*!
    ** \brief Save settings
    **
    ** Upon application exit this will write out our config file.
    ** note that if the application fails to startup then a config file
    ** will never be generated.
    **
    ** See LoadConfigFile for notes on format.
    **
    ** If something goes wrong you can safely throw a string for an error.
    ** The call to this function is tried, but it only catches exceptions
    ** and strings, ie throw( "LoadConfigFile: some error occured" );
    */
    bool Save();

    /*!
    ** \brief Make a copy of a given file
    **
    ** \param filename File which must be copied
    ** \return True if the operation succeeded, false otherwise
    */
    bool Backup(const QString& filename);

    bool Restore(const QString& filename);




} // namespace Settings
} // namespace TA3D

#endif // __TA3D_MISC_SETTINGS_H__
