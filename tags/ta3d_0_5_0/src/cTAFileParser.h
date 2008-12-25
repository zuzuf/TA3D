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

#ifndef __TA3D_X_HASH_TABLE_H__
# define __TA3D_X_HASH_TABLE_H__

#include "misc/hash_table.h"
#include <functional>


namespace TA3D
{
namespace UTILS
{


    class cTAFileParser : protected TA3D::UTILS::cHashTable<String>
    {
    public:
        //! \name Constructors & Destructor
        //@{
        /*!
        ** \brief Constructor
        */
        cTAFileParser(const String& filename, bool bKeysCaseSenstive = false, bool toUTF8 = false, bool g_mode = false );
        /*!
        ** \brief Constructor
        */
        cTAFileParser(const uint32 TableSize = 4096);
        //! Destructor
        ~cTAFileParser();
        //@}

        void clear() {emptyHashTable();}
        void load(const String& filename, bool bClearTable = false, bool toUTF8 = false, bool g_mode = false );

        /*!
        ** \brief Get the value for a given key
        ** \param key The key
        ** \param def The default value if the key could not be found
        ** \return The value of the key that has been found, def otherwise
        */
        sint32  pullAsInt(const String& key, const sint32 def = 0);

        /*!
        ** \brief Get the value for a given key
        ** \param key The key
        ** \param def The default value if the key could not be found
        ** \return The value of the key that has been found, def otherwise
        */
        real32  pullAsFloat(const String& key, const real32 def = 0.0f);

        /*!
        ** \brief Get the value for a given key
        ** \param key The key
        ** \param def The default value if the key could not be found
        ** \return The value of the key that has been found, def otherwise
        */
        String  pullAsString(const String& key, const String& def = "");

        /*!
        ** \brief Get the value for a given key
        ** \param key The key
        ** \param def The default value if the key could not be found
        ** \return The value of the key that has been found, def otherwise
        */
        bool  pullAsBool(const String& key, const bool def = false);

        void insertOrUpdate(const String& key, const String& value) {TA3D::UTILS::cHashTable<String>::insertOrUpdate(key, value);}
        bool exists(const String& key) {return TA3D::UTILS::cHashTable<String>::exists(key);}

        /*!
        ** \brief Call the callback for each entry
        ** \param c The callback
        */
        template<typename T>
        void forEach(T callback);

    private:
        bool ProcessData(char **Data);

        String GetLine(char **Data);

    private:
        //! used when building keys
        String m_cKey;
        //! used when building keys
        std::list< String > key_level;
        //! Is it case sensitive ?
        bool m_bKeysCaseSenstive;
        //!
        int gadget_mode;

    }; // class cTAFileParser


    template<typename T>
    void cTAFileParser::forEach(T callback)
    {
        for (iterator iter = begin(); iter != end(); ++iter)   
        {
            for (std::list< cBucket<String> >::const_iterator cur = iter->begin() ; cur != iter->end() ; ++cur)
            {
                if (!callback(cur->m_szKey, cur->m_T_data))
                    return;
            }
        }

    }


}
} 

#endif // __TA3D_X_HASH_TABLE_H__
