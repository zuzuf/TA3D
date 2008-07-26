#ifndef _TA3D_TOOLS_FILES_H__
# define _TA3D_TOOLS_FILES_H__

# include "../stdafx.h"
# include "string.h"


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
    ** \brief Get the size of a file
    ** \param filename The file
    ** \param[out] size The size of the file. 0 if any errors has occured
    ** \return True if the operation succeeded, False otherwise
    */
    bool Size(const String& filename, uint64& size);


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
    String ReplaceExtension(const String& filename, const String& newExt);



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
    bool Load(String::Vector& out, const String& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);
    bool Load(String::List& out, const String& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);


    /*!
    ** \brief Load the entierly content of a file into memory
    **
    ** \param filename The filename to open
    ** \param hardlimit If the size of the file exceeds this limit, it will not be loaded 
    ** \return The content of the file, null terminated , NULL if size > hardlimit or if any errors has occured.
    ** If not NULL, this value must be deleted with the keyword `delete[]`
    */
    char* LoadContentInMemory(const String& filename, const uint64 hardlimit = TA3D_FILES_HARD_LIMIT_FOR_SIZE);

    /*!
    ** \brief Load the entierly content of a file into memory
    **
    ** \param filename The filename to open
    ** \param[out] size The size of the file
    ** \param hardlimit If the size of the file exceeds this limit, it will not be loaded 
    ** \return The content of the file, null terminated , NULL if size > hardlimit or if any errors has occured.
    ** If not NULL, this value must be deleted with the keyword `delete[]`
    */
    char* LoadContentInMemory(const String& filename, uint64& size, const uint64 hardlimit = TA3D_FILES_HARD_LIMIT_FOR_SIZE);

    //@}

    
    /*!
    ** \brief Copy a single
    **
    ** \param from The source file
    ** \param to The target file
    ** \param overwrite Overwrite the target file if already exists
    ** \return True if the operation succeeded (or if the target file already exists and `overwrite` = false), false otherwise
    */
    bool Copy(const String& from, const String& to, const bool overwrite = true);



} // namespace Files
} // namespace Paths
} // namespace TA3D

#endif // _TA3D_TOOLS_FILES_H__
