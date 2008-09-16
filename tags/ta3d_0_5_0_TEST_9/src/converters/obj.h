#ifndef __TA3D_IMPORTERS_OBJ_H__
# define __TA3D_IMPORTERS_OBJ_H__

# include "../stdafx.h"
# include "../3do.h"


namespace TA3D
{
namespace Converters
{


    class OBJ
    {
    public:
        /*!
        ** \brief Create a Model from an OBJ file
        **
        ** \param filename The OBJ file
        ** \param scale Scale for the new model
        ** \return A valid instance to a model
        */
        static MODEL* ToModel(const String& filename, const float scale = 20.0f);

    }; // class OBJ



} // namespace Converters
} // namespace TA3D

#endif // __TA3D_IMPORTERS_OBJ_H__
