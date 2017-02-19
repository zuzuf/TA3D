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

#ifndef __UnitScriptInterface_H__
# define __UnitScriptInterface_H__

# include <misc/string.h>
# include "script.interface.h"
# include "script.data.h"
# include <misc/hash_table.h>


namespace TA3D
{
    /*!
    ** This class is an interface for all unit scripts types
    */
    class UnitScriptInterface : public ScriptInterface
    {
	public:
        typedef zuzuf::smartptr<UnitScriptInterface>	Ptr;
    public:
		static UnitScriptInterface *instanciate( ScriptData::Ptr data );
        static const String get_script_name(int id);
        static int get_script_id(const String &name);

    protected:
        uint32                  unitID;
		UTILS::HashMap<int>::Dense  return_value;
    public:
		inline UnitScriptInterface() {}
        virtual ~UnitScriptInterface()  {}

        virtual void setUnitID(uint32 ID) = 0;

        virtual int getNbPieces() = 0;

        int getReturnValue(const String &name);
        void setReturnValue(const String &name, int value);

    private:
        static const char *script_name[];
    };
}

#endif
