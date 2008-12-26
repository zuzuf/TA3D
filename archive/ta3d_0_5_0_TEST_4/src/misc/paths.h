#ifndef _TA3D_TOOLS_PATHS_H__
# define _TA3D_TOOLS_PATHS_H__

# include "../stdafx.h"
# include <vector>
# include <list>



namespace TA3D
{


/*
** \brief Path manipulation functions
*/
namespace Paths
{
   
    //! \name Relevant paths used by TA3D
    //@{

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

    //@}


    //! \name System-dependant variables
    //@{

    //! The path-separator character according to the platform
    # ifdef TA3D_PLATFORM_WINDOWS
    const char Separator = '\\';
    # else
    const char Separator = '/';
    # endif

    //! The path-separator character according to the platform (stored in a string instead of a char)
    # ifdef TA3D_PLATFORM_WINDOWS
    const String SeparatorAsString = "\\";
    # else
    const String SeparatorAsString = "/";
    # endif

    # ifdef TA3D_PLATFORM_WINDOWS
    //! The folder for local data (Windows only)
    extern String LocalData;
    # endif

    //@}





    //! \name Folders handling
    //@{

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
    ** The path part will be extracted according the system-dependant path-separator
    **
    ** \code
    **      std::cout << Paths::ExtractFilePath("/tmp/foo.txt") std::endl; // write `/tmp/`
    **      std::cout << Paths::ExtractFilePath("/tmp/") std::endl; // write `/tmp/`
    **      std::cout << Paths::ExtractFilePath("/tmp") std::endl; // write `/`
    ** \endcode
    **
    ** \see Paths::Separator
    */
    String ExtractFilePath(const String& p);

    /*!
    ** \brief Extract the bare file file name
    **
    ** The file name will be extracted according the last occurence
    ** of the system-dependant path-separator
    **
    ** \see Paths::Separator
    */
    String ExtractFileName(const String& p);

    /*!
    ** \brief Get if a path is absolute 
    **
    ** \param p The path or the filename to test
    ** \return True if the path is an absolute path or empty, false otherwise
    */
    bool IsAbsolute(const String& p);

    //@}



    //! \name Globbing
    //@{

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

    //@} // Globbing



    /*!
    ** \brief Load all informations about paths
    **
    ** return False if any error has occured
    */
    bool Initialize(int argc, char* argv[]);


} // namespace Paths
} // namespace TA3D



#endif // _TA3D_TOOLS_PATHS_H__
