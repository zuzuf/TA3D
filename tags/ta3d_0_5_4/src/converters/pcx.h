#ifndef __TA3D_IMPORTERS_PCX_H__
# define __TA3D_IMPORTERS_PCX_H__

# include "../stdafx.h"


namespace TA3D
{
namespace Converters
{


    class PCX
    {
    public:
        /*!
        ** \brief Load a PCX file using the HPI manager and convert it into a BITMAP
        **
        ** \param filename The file to find using the HPI manager
        ** \return NULL if not foud
        */
        static BITMAP* FromHPIToBitmap(const String& filename);

        /*!
        ** \brief Convert the raw data of a PCX file
        **
        ** \param data The raw data
        ** \param cpal The palette to use
        ** \return A valid instance. NULL if `data` or cpal is NULL
        */
        static BITMAP* RawDataToBitmap(const byte* data, const RGB* cpal);

    }; // class PCX



} // namespace Converters
} // namespace TA3D

#endif // __TA3D_IMPORTERS_PCX_H__
