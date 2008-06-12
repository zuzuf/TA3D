#ifndef __TA3D_MISC_SETTINGS_H__
# define __TA3D_MISC_SETTINGS_H__

# include "../stdafx.h"



namespace TA3D
{
namespace Settings
{

    /*!
    ** \brief
    **
    ** This function will eventually load our config file if it exists.
    ** config files will be stored as 'tdf' format and thus loaded as text,
    ** using the cTAFileParser class.
    **
    ** If something goes wrong you can safely throw a string for an error.
    ** The call to this function is tried, but it only catches exceptions
    ** and strings, ie throw( "LoadConfigFile: some error occured" );
    */
    bool Load();

    /*!
    ** \brief Save settings
    **
    ** Upon application exit this will write out our config file.
    ** note that if the application fails to startup then a config file
    ** will never be generated.
    ** 
    ** See LoadConfigFile for notes on format.
    **
    ** If something goes wrong you can safely throw a string for an error.
    ** The call to this function is tried, but it only catches exceptions
    ** and strings, ie throw( "LoadConfigFile: some error occured" );
    */
    bool Save();

    /*!
    ** \brief Make a copy of a given file
    **
    ** \param filename File which must be copied
    ** \return True if the operation succeeded, false otherwise
    */
    bool Backup(const String& filename);

    bool Restore(const String& filename);




} // namespace Settings
} // namespace TA3D

#endif // __TA3D_MISC_SETTINGS_H__
