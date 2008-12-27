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
    //! Current log file
    //! \see Logs::writeToFile()
    extern String LogFile;

    //! Folder for preferences
    extern String Preferences;

    //! Configuration file
    extern String ConfigFile;

    //! Folder for screenshots
    extern String Screenshots;

    //! Folder for game resources
    extern String Resources;

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
    ** \param systemDependant Consider only the system-dependant path-separator
    **
    ** \code
    **      std::cout << Paths::ExtractFilePath("/tmp/foo.txt") std::endl; // write `/tmp/`
    **      std::cout << Paths::ExtractFilePath("/tmp/") std::endl; // write `/tmp/`
    **      std::cout << Paths::ExtractFilePath("/tmp") std::endl; // write `/`
    ** \endcode
    **
    ** \see Paths::Separator
    */
    String ExtractFilePath(const String& p, const bool systemDependant = false);

    /*!
    ** \brief Extract the bare file name
    **
    ** The file name will be extracted according the last occurence
    ** of the system-dependant path-separator (if systemDependant = true)
    **
    ** \param systemDependant Consider only the system-dependant path-separator
    **
    ** \see Paths::Separator
    */
    String ExtractFileName(const String& p, const bool systemDependant = true);
    void ExtractFileName(String::List& p, const bool systemDependant = true);
    void ExtractFileName(String::Vector& p, const bool systemDependant = true);

    /*!
    ** \brief Extract the bare file name without its extension
    **
    ** The file name will be extracted according the last occurence
    ** of the system-dependant path-separator
    **
    ** \see Paths::Separator
    */
    String ExtractFileNameWithoutExtension(const String& p);

    /*!
    ** \brief Extract the extention of a file name
    ** \param s Filename
    ** \return The extenion of the filename (with the leading '.') in lowercase, empty if no extension is present
    **
    ** \code
    **     std::cout << Paths::Files::ExtractFileExt("foo.exe") << std::endl; // '.exe'
    **     std::cout << Paths::Files::ExtractFileExt("/usr/folder.foo/file") << std::endl; // ''
    ** \endcode
    */
    String ExtractFileExt(const String& s);


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
    ** String::Vector list;
    ** if (Paths::Glob(list, Paths::Savegames + "*.sav"))
    ** {
    **      for (String::Vector::const_iterator i = list.begin(); i != list.end(); ++i)
    **          std::cout << "Savegame found: `" << *i << std::endl;
    ** }
    ** else
    ** {
    **      std::cerr << "No savegame found." << std::endl;
    ** }
    ** \endcode
    */
    bool Glob(String::Vector& out, const String& pattern, const bool emptyListBefore = true);
    bool Glob(String::List& out, const String& pattern, const bool emptyListBefore = true);

    /*!
    ** \brief Find files matching a pattern
    **
    ** \param[out] out The list of file that has been found
    ** \param pattern The pattern to use
    ** \param emptyListBefore True to empty the list before performing the glob function
    ** \return True if the operation succeeded and the list is not empty,
    ** false othewise
    **
    ** \code
    ** String::Vector list;
    ** if (Paths::GlobFiles(list, Paths::Savegames + "*.sav"))
    ** {
    **      for (String::Vector::const_iterator i = list.begin(); i != list.end(); ++i)
    **          std::cout << "Savegame found: `" << *i << std::endl;
    ** }
    ** else
    ** {
    **      std::cerr << "No savegame found." << std::endl;
    ** }
    ** \endcode
    */
    bool GlobFiles(String::Vector& out, const String& pattern, const bool emptyListBefore = true);
    bool GlobFiles(String::List& out, const String& pattern, const bool emptyListBefore = true);

    /*!
    ** \brief Find directories matching a pattern
    **
    ** \param[out] out The list of directory that has been found
    ** \param pattern The pattern to use
    ** \param emptyListBefore True to empty the list before performing the glob function
    ** \return True if the operation succeeded and the list is not empty,
    ** false othewise
    **
    ** \code
    ** String::Vector list;
    ** if (Paths::GlobDirs(list, Paths::LocalData))
    ** {
    **      for (String::Vector::const_iterator i = list.begin(); i != list.end(); ++i)
    **          std::cout << "Sub directory found: `" << *i << std::endl;
    ** }
    ** else
    ** {
    **      std::cerr << "No sub directory found." << std::endl;
    ** }
    ** \endcode
    */
    bool GlobDirs(String::Vector& out, const String& pattern, const bool emptyListBefore = true);
    bool GlobDirs(String::List& out, const String& pattern, const bool emptyListBefore = true);

    //@} // Globbing



    /*!
    ** \brief Load all informations about paths
    **
    ** \param programName Arbitrary name for the program. It will be used
    ** to create the log file.
    **
    ** return False if any error has occured
    */
    bool Initialize(int argc, char* argv[], const String& programName);


} // namespace Paths
} // namespace TA3D



#endif // _TA3D_TOOLS_PATHS_H__
