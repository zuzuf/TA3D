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
#ifndef _TA3D_TOOLS_FILES_H__
# define _TA3D_TOOLS_FILES_H__

# include <stdafx.h>
# include "string.h"

class QIODevice;


/*!
** \brief The maximum allowed size for a file
*/
# define TA3D_FILES_HARD_LIMIT_FOR_SIZE  83886080  // 80Mo = 10 * 1024 * 1024




namespace TA3D
{
namespace Paths
{
/*!
** \brief Tools to handle files
*/
namespace Files
{
	/*!
	** \brief Replace the extension
	**
	** if the extension can not be found, the new extension will be
	** appended to it
	** As this function is used when loading OTA or TDF files, the folder
	** separator is not system-dependant.
	**
	** \param filename The orignal filename
	** \param newExt The new extension (ex: `.ota`)
	** \return The filename with the new extension
	*/
    QString ReplaceExtension(const QString& filename, const QString& newExt);



	//! \name Load the content of a file
	//@{

	/*!
	** \brief Open and Read the content of a file and write it into a 1D array
	**
	** \param[out] out The content of the file
	** \param filename Filename to open
	** \param sizeLimit Do not load files with a size > to this value. The value `0` disables this feature.
	** \param emptyListBefore Empty the list before any operation
	** \return True if the operation succeeded, False otherwise
	*/
    bool Load(QStringList& out, const QString& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);


	/*!
	** \brief Load the entierly content of a file into memory
	**
	** \param filename The filename to open
	** \param hardlimit If the size of the file exceeds this limit, it will not be loaded
	** \return The content of the file, null terminated , NULL if size > hardlimit or if any errors has occured.
	** If not NULL, this value must be deleted with the keyword `delete[]`
	*/
    QIODevice* LoadContentInMemory(const QString& filename, const uint64 hardlimit = TA3D_FILES_HARD_LIMIT_FOR_SIZE);

	/*!
	** \brief Save the content of a string iinto a file
	**
	** \param filename The filename to create/overwrite
	** \param content The new content of the file
	** \return True if the operation succeeded, false otherwise
	*/
    bool SaveToFile(const QString& filename, const QByteArray& content);

	//@}


	/*!
	** \brief Copy a single file
	**
	** \param from The source file
	** \param to The target file
	** \param overwrite Overwrite the target file if already exists
	** \return True if the operation succeeded (or if the target file already exists and `overwrite` = false), false otherwise
	*/
    bool Copy(const QString& from, const QString& to, const bool overwrite = true);



} // namespace Files
} // namespace Paths
} // namespace TA3D

#endif // _TA3D_TOOLS_FILES_H__
