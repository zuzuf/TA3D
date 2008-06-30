#ifndef _TA3D_TOOLS_PATHS_H__
# define _TA3D_TOOLS_PATHS_H__

# include "../stdafx.h"
# include <vector>
# include <list>



namespace TA3D
{


    /*! \class Paths
    **
    ** \brief Tools to handle paths for TA3D
    **
    ** This tool must be initialized using Initialize()
    **
    ** \see TA3D::Paths::Initialize()
    */
    class Paths
    {
    public:
        //! Folder separator according to the platform
        static char Separator;
        static String SeparatorAsString;

        //! Absolute path where the application is located (extracted from argv)
        static String ApplicationRoot;

        //! Folder for Caches
        static String Caches;
        
        //! Folder for Savegames
        static String Savegames;

        //! Folder for logs
        static String Logs;

        //! Folder for preferences
        static String Preferences;

        //! Configuration file
        static String ConfigFile;

        //! Folder for screenshots
        static String Screenshots;


    public:
        //! \name Resources management
        //{

        /*!
        ** \brief Find a relative resource filename in the list of search paths for resources
        **
        ** This method is not thread safe
        **
        ** \param relFilename The relative filename to find
        ** \param[out] out The absolute filename that has been found
        ** \return True if the resource has been found, false otherwise
        */
        static bool FindResources(const String& relFilename, String& out);

        /*!
        ** \brief Add a folder in the search paths for resources
        **
        ** This method is not thread safe
        **
        ** \param folder The folder to add
        ** \return True if the folder has been added, false otherwise
        */
        static bool AddResourcesFolder(const String& folder);

        //}


        //! \name Files & Folders handling
        //{

        /*!
        ** \brief Test if a file/folder exists
        ** \param p The folder/filename to test
        ** \return True if it exists, false otherwise
        */
        static bool Exists(const String& p);

        /*!
        ** \brief Create Path Recursively
        **
        ** \param p The path to create if it does not exist
        ** return True if the operation succeeded, false otherwise
        */
        static bool MakeDir(const String& p);

        /*!
        ** \brief Retrieve the current directory
        */
        static String CurrentDirectory();

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
        static bool Glob(std::vector<String>& out, const String& pattern, const bool emptyListBefore = true);
        static bool Glob(std::list<String>& out, const String& pattern, const bool emptyListBefore = true);

        /*!
        ** \brief Find pathnames in the resources folders matching a pattern
        **
        ** \param[out] out The list of file that has been found
        ** \param pattern The pattern to use
        ** \return True if the operation succeeded and the list is not empty,
        ** false othewise
        **
        ** std::vector<String> list;
        ** if (Paths::ResourcesGlob(list, "objects/rocks*.3dm"))
        ** {
        **      for (std::vector<String>::const_iterator i = list.begin(); i != list.end(); ++i)
        **          std::cout << "3D object found: `" << *i << std::endl; 
        ** }
        ** else
        ** {
        **      std::cerr << "No 3D object found." << std::endl;
        ** }
        ** \endcode

        */
        static bool ResourcesGlob(std::vector<String>& out, const String& pattern, const bool emptyListBefore = true);
        static bool ResourcesGlob(std::list<String>& out, const String& pattern, const bool emptyListBefore = true);

        //} // Globbing


        /*!
        ** \brief Load all informations about paths
        **
        ** return False if any error has occured
        */
        static bool Initialize(int argc, char* argv[]);


    private:
        //! Definition list of resources folders
        typedef std::vector<String> ResourcesFoldersList;
        
    private:
        //! List of resources folders
        static ResourcesFoldersList pResourcesFolders;

    }; // class Paths



} // namespace TA3D



#endif // _TA3D_TOOLS_PATHS_H__
