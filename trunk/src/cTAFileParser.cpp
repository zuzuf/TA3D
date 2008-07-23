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

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "cTAFileParser.h"
#include "cError.h"
#include <fstream>
#include "misc/paths.h"


namespace TA3D
{
namespace UTILS
{


    String cTAFileParser::GetLine(char **Data)
    {
        uint32 n(0);
        char* result = *Data;
        for (; **Data ; ++(*Data), ++n)
        {
            if (**Data == '\n' || **Data == ';' || **Data == '{' || **Data == '}')
            {
                ++(*Data);
                ++n;
                return std::string(result, n);
            }
        }
        return result;
    }


    bool cTAFileParser::ProcessData(char **Data)
    {
        if (**Data == 0)
            return true;
        String Line = GetLine(Data);         // extract line
        if (Line.empty())
            return false;

        /*
        String key;
        String value;
        Line.toKeyValue(key, value);
        if (!m_bKeysCaseSenstive)
            key.toLower();
        LOG_DEBUG(" >> K=`" << key << "`, V=`" << value << "`  > " << Line);
        */

        String::size_type i = Line.find("//"); // search for start of comment.
        if (i != String::npos) // if we find a comment, we will erase everything
            Line.resize(i); // from the comment to the end of the line.

        Line = String::Trim(Line, " \t\n\r{"); // strip out crap from string.
        i = Line.length();

        if (i > 3) // check for new key.
        {
            if( Line[0] == '[' && Line[i - 1] == ']')
            {
                bool changed = false;
                if (gadget_mode >= 0 && key_level.empty())
                {
                    m_cKey = format("gadget%d", gadget_mode);
                    ++gadget_mode;
                    changed = true;
                    if (!m_bKeysCaseSenstive)
                        TA3D::UTILS::cHashTable<String>::insertOrUpdate(m_cKey, String::ToLower(Line.substr(1, i-2)));		// Link the key with its original name
                    else
                        TA3D::UTILS::cHashTable<String>::insertOrUpdate(m_cKey, Line.substr(1, i - 2));		// Link the key with its original name
                }

                key_level.push_front(m_cKey);
                if (!changed) 
                {
                    m_cKey = ( m_cKey.empty()
                               ? Line.substr( 1, i-2 )
                               : m_cKey + String(".") + Line.substr(1, i - 2));
                }

                m_cKey = ReplaceString( m_cKey, "\\n", "\n", false);
                m_cKey = ReplaceString( m_cKey, "\\r", "\r", false);

                while(**Data)										// Better using the stack this way, otherwise it might crash with huge files
                {
                    if (ProcessData(Data))
                        break;
                }
                return false;
            }
        }
        else
            if (i > 0 && Line[ i - 1 ] == '}')  // close the current active key.
            {
                if (key_level.empty())
                    m_cKey.clear();
                else
                {
                    m_cKey = key_level.front();
                    key_level.pop_front();
                }
                return true;
            }

        // if we get here its possible its a name=value;
        // so we will search for a = and a ; if we find them we have a valid name/value
        String::size_type f1 = Line.find_first_of('=');
        String::size_type f2 = Line.find_last_of(';');

        if (f1 != String::npos && f2 != String::npos)
        {
            String n = Line.substr(0, f1);
            String v = Line.substr((f1 + 1), (f2 - (f1 + 1)));

            if (!n.empty() && n[ n.size() - 1] == ' ')
                n.erase(n.size() - 1, 1);
            if (!v.empty() && v[0] == ' ')
                v.erase(0, 1);

            v = ReplaceString( v, "\\n", "\n", false);
            v = ReplaceString( v, "\\r", "\r", false);

            String t;
            t << m_cKey << '.' << n;
            if (!m_bKeysCaseSenstive)
                t.toLower();
            TA3D::UTILS::cHashTable<String>::insertOrUpdate(t, v);
        }
        return false;
    }



    void cTAFileParser::load(const String& filename,  bool bClearTable, bool toUTF8, bool g_mode )
    {
        uint32 ota_size = 0;
        byte* data = NULL;
        if (bClearTable)
        {
            uint32 old_tablesize = pTableSize;
            emptyHashTable();
            initTable(old_tablesize);
        }

        if (TA3D::VARS::HPIManager)
        {
            LOG_DEBUG("[tdf] Loading `hpi://" << filename << "`");
            data = TA3D::VARS::HPIManager->PullFromHPI(filename, &ota_size);
        }
        else
        {
            LOG_DEBUG("[tdf] Loading `" << filename << "`");
            std::ifstream m_File;
            m_File.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
            if (m_File.is_open())
            {
                ota_size = m_File.tellg();
                data = new byte[ota_size + 1];
                data[ota_size] = 0;
                m_File.seekg(0, std::ios::beg);
                m_File.read((char *)data, ota_size);
                m_File.close();
            }
        }
        if (!data)
        {
            LOG_CRITICAL("[tdf] No data to load `" << filename << "`");
            return;
        }

        if (data != NULL && toUTF8) 
        {
            // Convert from ASCII to UTF8, required because TA3D works with UTF8 and TA with ASCII
            char *tmp = new char[ota_size * 3];
            do_uconvert( (const char*)data, U_ASCII, tmp, U_UTF8, ota_size * 3);
            delete[] data;
            data = (byte*)tmp;
        }

        m_cKey.clear();
        key_level.clear();

        // erase all line feeds. (linear algorithm)
        char *tmp = (char*)data;
        int e = 0, i = 0;
        for( ; tmp[i]; ++i)
        {
            if (tmp[i] != '\r')
            {
                if (e)
                    tmp[i - e] = tmp[i];
            }
            else
                ++e;
        }
        if (e > 0)
            tmp[i] = 0;

        gadget_mode = g_mode ? 0 : -1;

        // now process the remaining.
        while (*tmp)
            ProcessData(&tmp);
        delete[] data;
    }


    cTAFileParser::cTAFileParser(const String& filename, bool bKeysCaseSenstive, bool toUTF8, bool g_mode)
        :TA3D::UTILS::cHashTable<String>(),
        m_bKeysCaseSenstive(bKeysCaseSenstive)
    {
        initTable(4096);
        load(filename, false, toUTF8, g_mode);
    }

    cTAFileParser::cTAFileParser(uint32 TableSize)
        :TA3D::UTILS::cHashTable<String>()
    {
        m_bKeysCaseSenstive = false;
        initTable(TableSize);
    }

    cTAFileParser::~cTAFileParser()
    {
        emptyHashTable();
    }

    sint32 cTAFileParser::pullAsInt(const String& key, const sint32 def)
    {
        String key_to_find;
        if( !m_bKeysCaseSenstive )
            key_to_find = String::ToLower(key);
        else
            key_to_find = key;
        if (!TA3D::UTILS::cHashTable<String>::exists(key_to_find))
            return def;

        String iterFind = TA3D::UTILS::cHashTable<String>::find(key_to_find);

        return ( (iterFind.length() == 0) ? def : ( iterFind.size() == 10 && ustrtol( iterFind.substr(0,4).c_str() , NULL, 0 ) > 127 ? ( 0xFF000000 | ustrtol( ("0x"+iterFind.substr(4,6)).c_str() , NULL, 0 ) ) : ustrtol( iterFind.c_str() , NULL, 0 ) ) );		// Uses ustrtol to deal with hexa numbers
    }

    real32 cTAFileParser::pullAsFloat(const String& key, const real32 def)
    {
        String key_to_find(key);
        if (!m_bKeysCaseSenstive)
            key_to_find.toLower();
        if (!TA3D::UTILS::cHashTable<String>::exists(key_to_find))
            return def;
        return TA3D::UTILS::cHashTable<String>::find(key_to_find).toFloat(def);
    }


    String cTAFileParser::pullAsString(const String& key, const String& def)
    {
        String key_to_find(key);
        if (!m_bKeysCaseSenstive)
            key_to_find.toLower();
        if (!TA3D::UTILS::cHashTable<String>::exists(key_to_find))
            return def;
        return TA3D::UTILS::cHashTable<String>::find(key_to_find);
    }


    bool cTAFileParser::pullAsBool(const String& key, const bool def)
    {
        String key_to_find(key);
        if (!m_bKeysCaseSenstive)
            key_to_find.toLower();
        if (!TA3D::UTILS::cHashTable<String>::exists(key_to_find))
            return def;
        return TA3D::UTILS::cHashTable<String>::find(key_to_find).toBool();
    }


}
} 
