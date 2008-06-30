#ifndef _TA3D_TOOLS_RESOURCES_H__
# define _TA3D_TOOLS_RESOURCES_H__

# include "../stdafx.h"
# include <vector>
# include <list>



namespace TA3D
{

/*!
** \brief Tools to handle Resources  for TA3D
*/
namespace Resources
{


    /*!
    ** \brief Find a relative resource filename in the list of search paths for resources
    **
    ** This method is not thread safe
    **
    ** \param relFilename The relative filename to find
    ** \param[out] out The absolute filename that has been found
    ** \return True if the resource has been found, false otherwise
    */
    bool Find(const String& relFilename, String& out);


    /*!
    ** \brief Add a search path for resources
    **
    ** This method is not thread safe
    **
    ** \param folder The folder to add
    ** \return True if the folder has been added, false otherwise
    */
    bool AddSearchPath(const String& folder);


    /*!
    ** \brief Find pathnames in the resources folders matching a pattern
    **
    ** \param[out] out The list of file that has been found
    ** \param pattern The pattern to use
    ** \param emptyListBefore Empty the list before any operation
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
    bool Glob(std::vector<String>& out, const String& pattern, const bool emptyListBefore = true);
    bool Glob(std::list<String>& out, const String& pattern, const bool emptyListBefore = true);



    /*!
    ** \brief Initialize default search paths
    */
    void Initialize();

} // namespace Resources
} // namespace TA3D

#endif // _TA3D_TOOLS_RESOURCES_H__
