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

#ifndef __LuaData_H__
#define __LuaData_H__

# include <stdafx.h>
# include <misc/string.h>
# include "script.data.h"

namespace TA3D
{
    class LuaData : public ScriptData
    {
    protected:
        QString          name;
        QStringList  piece_name;     // Nom des pièces de l'objet 3d concerné / Name of pieces

    public:
        LuaData();
        virtual ~LuaData();

        /*virtual*/ void load(const QString &filename);                    // Load a lua chunk

		const QString &getName() const;

        virtual int identify(const QString &name);
    private:
        void init();
        void destroy();
    };
}

#endif
