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

#ifndef __TA3D_XX_MISC_OS_INFORMATIONS_H__
# define __TA3D_XX_MISC_OS_INFORMATIONS_H__

#include "string.h"



namespace TA3D
{

/*!
** \brief System-specific routines
*/
namespace System
{

	/*!
	** \brief Run the given command and returns its standard output as a String
	*/
	String run_command(const String& cmd);

    /*!
    ** \brief Write informations about the current OS to the console
    ** and the logger
    */
    void DisplayInformations();

    /*!
    ** \brief Write informations about the current version of SDL
    */
    void DisplayInformationsAboutSDL();

    /*!
    ** \brief Desktop resolution
    **
    ** \param width  Width of the screen
    ** \param height Height of the screen
    ** \param colorDepth Color depth of the screen
    ** \return True if the operation succeeded, False otherwise
    */
    bool DesktopResolution(int& width, int& height, int& colorDepth);



} // namespace OS
} // namespace TA3D

#endif // __TA3D_XX_MISC_OS_INFORMATIONS_H__
