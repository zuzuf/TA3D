#ifndef __TA3D_XX_MISC_OS_INFORMATIONS_H__
# define __TA3D_XX_MISC_OS_INFORMATIONS_H__


namespace TA3D
{

/*!
** \brief System-specific routines
*/
namespace System
{


    /*!
    ** \brief Write informations about the current OS to the console
    ** and the logger
    */
    void DisplayInformations();

    /*!
    ** \brief Write informations about the current version of SDL
    */
    void DisplayInformationsAboutSDL();

    /*!
    ** \brief Desktop resolution
    **
    ** \param width  Width of the screen
    ** \param height Height of the screen
    ** \param colorDepth Color depth of the screen
    ** \return True if the operation succeeded, False otherwise
    */
    bool DesktopResolution(int& width, int& height, int& colorDepth);


} // namespace OS
} // namespace TA3D

#endif // __TA3D_XX_MISC_OS_INFORMATIONS_H__
