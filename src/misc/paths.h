#ifndef _TA3D_TOOLS_PATHS_H__
# define _TA3D_TOOLS_PATHS_H__

# include "../stdafx.h"
# include <vector>



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
        /*!
        ** \brief Find a relative resource filename in the list of search paths for resources
        **
        ** \param relFilename The relative filename to find
        ** \param[out] out The absolute filename that has been found
        ** \return True if the resource has been found, false otherwise
        */
        static bool FindResources(const String& relFilename, String& out);

        /*!
        ** \brief Add a folder in the search paths for resources
        **
        ** \param folder The folder to add
        ** \return True if the folder has been added, false otherwise
        */
        static bool AddResourcesFolder(const String& folder);


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
