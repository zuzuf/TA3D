#ifndef _TA3D_TOOLS_FILES_H__
# define _TA3D_TOOLS_FILES_H__

# include "../stdafx.h"
# include "string.h"


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
    ** \brief Open and Read the content of a file and write it into a 1D array
    **
    ** \param[out] out The content of the file
    ** \param filename Filename to open
    ** \param sizeLimit Do not load files with a size > to this value. The value `0` disables this feature.
    ** \param emptyListBefore Empty the list before any operation
    ** \return True if the operation succeeded, False otherwise
    */
    bool Load(String::Vector& out, const String& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);
    bool Load(String::List& out, const String& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);




} // namespace Files
} // namespace Paths
} // namespace TA3D

#endif // _TA3D_TOOLS_FILES_H__
