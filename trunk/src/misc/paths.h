#ifndef _TA3D_TOOLS_PATHS_H__
# define _TA3D_TOOLS_PATHS_H__

# include "../stdafx.h"
# include <vector>
# include <list>



namespace TA3D
{


/*
** \brief Tools to handle paths
*/
namespace Paths
{
    //! Folder separator according to the platform
    extern char Separator;
    extern String SeparatorAsString;

    //! Absolute path where the application is located (extracted from argv)
    extern String ApplicationRoot;

    //! Folder for Caches
    extern String Caches;

    //! Folder for Savegames
    extern String Savegames;

    //! Folder for logs
    extern String Logs;

    //! Folder for preferences
    extern String Preferences;

    //! Configuration file
    extern String ConfigFile;

    //! Folder for screenshots
    extern String Screenshots;

    # ifndef TA3D_PLATFORM_WINDOWS
    //! The folder for local data (Windows only)
    extern String LocalData;
    # endif


    //! \name Resources management
    //{
    //}


    //! \name Files & Folders handling
    //{

    /*!
    ** \brief Test if a file/folder exists
    ** \param p The folder/filename to test
    ** \return True if it exists, false otherwise
    */
    bool Exists(const String& p);

    /*!
    ** \brief Create Path Recursively
    **
    ** \param p The path to create if it does not exist
    ** return True if the operation succeeded, false otherwise
    */
    bool MakeDir(const String& p);

    /*!
    ** \brief Retrieve the current directory
    */
    String CurrentDirectory();

    /*!
    ** \brief Extract the path part of a filename
    **
    ** \code
    **      std::cout << Paths::ExtractFilePath("/tmp/foo.txt") std::endl; // write `/tmp/`
    **      std::cout << Paths::ExtractFilePath("/tmp/") std::endl; // write `/tmp/`
    **      std::cout << Paths::ExtractFilePath("/tmp") std::endl; // write `/`
    ** \endcode
    */
    String ExtractFilePath(const String& p);

    //}



    //! \name Globbing
    //{

    /*!
    ** \brief Find pathnames matching a pattern
    **
    ** \param[out] out The list of file that has been found
    ** \param pattern The pattern to use
    ** \param emptyListBefore True to empty the list before performing the glob function
    ** \return True if the operation succeeded and the list is not empty,
    ** false othewise
    **
    ** \code
    ** std::vector<String> list;
    ** if (Paths::Glob(list, Paths::Savegames + "*.sav"))
    ** {
    **      for (std::vector<String>::const_iterator i = list.begin(); i != list.end(); ++i)
    **          std::cout << "Savegame found: `" << *i << std::endl; 
    ** }
    ** else
    ** {
    **      std::cerr << "No savegame found." << std::endl;
    ** }
    ** \endcode
    */
    bool Glob(std::vector<String>& out, const String& pattern, const bool emptyListBefore = true);
    bool Glob(std::list<String>& out, const String& pattern, const bool emptyListBefore = true);

    //} // Globbing



    /*!
    ** \brief Load all informations about paths
    **
    ** return False if any error has occured
    */
    bool Initialize(int argc, char* argv[]);


} // namespace Paths
} // namespace TA3D



#endif // _TA3D_TOOLS_PATHS_H__
