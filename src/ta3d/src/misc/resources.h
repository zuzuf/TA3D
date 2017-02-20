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

#ifndef _TA3D_TOOLS_RESOURCES_H__
# define _TA3D_TOOLS_RESOURCES_H__

# include <stdafx.h>
# include "string.h"
# include <vector>
# include <list>



namespace TA3D
{

/*!
** \brief Thread-safe Tools to handle Resources for TA3D
*/
namespace Resources
{


    /*!
    ** \brief Find a relative resource filename in the list of search paths for resources
    **
    ** This method is thread safe
    **
    ** \param relFilename The relative filename to find
    ** \param[out] out The absolute filename that has been found
    ** \return True if the resource has been found, false otherwise
    */
    bool Find(const QString& relFilename, QString& out);


    /*!
    ** \brief Add a search path for resources
    **
    ** This method is thread safe
    **
    ** \param folder The folder to add
    ** \return True if the folder has been added, false otherwise
    */
    bool AddSearchPath(QString folder);


    /*!
    ** \brief Find pathnames in the resources folders matching a pattern
    **
    ** \param[out] out The list of file that has been found
    ** \param pattern The pattern to use
    ** \param emptyListBefore Empty the list before any operation
    ** \return True if the operation succeeded and the list is not empty,
    ** false othewise
    **
    ** QStringList list;
    ** if (Paths::ResourcesGlob(list, "objects/rocks*.3dm"))
    ** {
    **      for (QStringList::const_iterator i = list.begin(); i != list.end(); ++i)
    **          std::cout << "3D object found: `" << *i << std::endl;
    ** }
    ** else
    ** {
    **      std::cerr << "No 3D object found." << std::endl;
    ** }
    ** \endcode
    */
    bool Glob(QStringList& out, const QString& pattern, const bool emptyListBefore = true);
    bool GlobDirs(QStringList& out, const QString& pattern, const bool emptyListBefore = true);


    /*!
    ** \brief Return all resource paths
    **
    ** This method is thread safe
    **
    */
    QStringList GetPaths();


    /*!
    ** \brief Initialize default search paths
    */
    void Initialize();

} // namespace Resources
} // namespace TA3D

#endif // _TA3D_TOOLS_RESOURCES_H__
