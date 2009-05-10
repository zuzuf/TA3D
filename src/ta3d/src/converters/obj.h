/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D_IMPORTERS_OBJ_H__
# define __TA3D_IMPORTERS_OBJ_H__

# include "../stdafx.h"
# include "../misc/string.h"
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
