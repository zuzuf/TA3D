#ifndef __TA3D_XX__MISC_APPLICATION_H__
# define __TA3D_XX__MISC_APPLICATION_H__


namespace TA3D
{


    /*!
    ** \brief Initialize all modules
    **
    ** This routine installs a atexit function so we can call exit() whenever we want
    */
    void Initialize(int argc, char* argv[], const String& programName);



} // namespace TA3D


#endif // __TA3D_XX__MISC_APPLICATION_H__
