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

#include "../stdafx.h"
#include "../gfx/gfx.toolkit.h"
#include "tdf.h"
#include "files.h"
#include "../ta3dbase.h"
#include "resources.h"




namespace TA3D
{


    /*! \class StackInfos
     **
     ** \brief
     */
    struct StackInfos
    {
        StackInfos() :level(0), line(0), gadgetMode(-1)
        {}
        //! The full name of the current section
        String currentSection;
        //! The current key
        String key;
        //! The current value
        String value;

        //! The stack for all sections
        std::stack<String> sections;
        //! Stack Level
        int level;

        //! Current line
        int line;

        //! The gadget mode
        int gadgetMode;

        //! File name (may not be the real filename)
        String caption;

    }; // class StackInfos





    TDFParser::TDFParser()
        :pTableSize(TDFPARSER_HASHTABLE_SIZE), pTableIsEmpty(true), pIgnoreCase(true), special_section()
    {
        pTable.initTable(TDFPARSER_HASHTABLE_SIZE);
    }

    TDFParser::TDFParser(const String& filename, const bool caSensitive, const bool toUTF8, const bool gadgetMode, const bool realFS)
        :pTableSize(TDFPARSER_HASHTABLE_SIZE), pTableIsEmpty(true), pIgnoreCase(!caSensitive), special_section()
    {
        pTable.initTable(TDFPARSER_HASHTABLE_SIZE);
        loadFromFile(filename, true, toUTF8, gadgetMode, realFS);
    }

    TDFParser::~TDFParser()
    {}


    void TDFParser::clear()
    {
        pTable.emptyHashTable();
        pTable.initTable(pTableSize);
        pTableIsEmpty = true;
        special_section.clear();
    }


    bool TDFParser::loadFromFile(const String& filename, const bool clear, const bool toUTF8, const bool gadgetMode, const bool realFS)
    {
        uint64 size;
        char* data;
        if (TA3D::VARS::HPIManager && !realFS)
        {
            uint32 ms;
            data = (char*)TA3D::VARS::HPIManager->PullFromHPI(filename, &ms);
            size = ms;
            if (NULL != data && size != 0)
            {
                bool res = loadFromMemory("hpi://" + filename, data, size, clear, toUTF8, gadgetMode);
                delete[] data;
                return res;
            }
        }
        else
        {
            data = Paths::Files::LoadContentInMemory(filename, size, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
            if (NULL != data && size != 0)
            {
                bool res = loadFromMemory(filename, data, size, clear, toUTF8, gadgetMode);
                delete[] data;
                return res;
            }
        }
        if (data == NULL)
		{
            LOG_ERROR(LOG_PREFIX_TDF << "Unable to open `" << filename << "`");
		}
        else
        {
            delete[] data;
            LOG_WARNING(LOG_PREFIX_TDF << "The file `" << filename << "` is empty (file size=0).");
        }
        return false;
    }



    bool TDFParser::loadFromMemory(const String& caption, const char* data, const bool clear, const bool toUTF8,
                                   const bool gadgetMode)
    {
        return (NULL != data)
            ? loadFromMemory(caption, data, strlen(data), clear, toUTF8, gadgetMode)
            : false;
    }

    bool TDFParser::loadFromMemory(const String& caption, const char* data, uint64 size, const bool clearTable,
                                   const bool toUTF8, const bool gadgetMode)
    {
        if (NULL == data || 0 == size)
            return true;
        // Convert it to UTF8 if required
        if (toUTF8)
        {
            uint32 s;
            char* t = String::ConvertToUTF8(data, /*The current size*/size, /*The new size*/s);
            if (NULL != t)
            {
                size = s;
                // delete[] data; The pointer will be freed by the caller
                data = t;
            }
            else
                LOG_WARNING(LOG_PREFIX_TDF << "The convertion using the UTF8 charset has failed.");
        }
        if (clearTable && !pTableIsEmpty)
            clear();

        StackInfos stack;
        stack.gadgetMode = gadgetMode ? 0 /* The first index */ : -1 /* means disabled */;
        stack.caption = caption;

        uint32 pos(0);
        uint32 lastPos(0);
        bool stringStarted(false);
        for (; pos < size; ++pos)
        {
            if (data[pos] == '\n' || data[pos] == '\r' || data[pos] == '\0' || data[pos] == '{' || data[pos] == '}' || data[pos] == ';' || pos + 1 == size)
            {
                if (data[pos] == '{' || data[pos] == '}')       // Because this is the beginning and the end
                    stringStarted = true;
                if (data[pos] == '{' || data[pos] == '}' || data[pos] == ';')   // Sometimes you can have this syntax : { variable0=value0; variable1=value1; ... }
                    ++pos;
                if (data[pos] == '\n')
                    ++stack.line;
                if (stringStarted)
                {
                    stringStarted = false;

                    if (pos + 1 == size)
                    {
                        String::ToKeyValue(String(data + lastPos, pos - lastPos + 1), stack.key, stack.value,
                                           pIgnoreCase ? String::soIgnoreCase : String::soCaseSensitive);
                        ++stack.line;
                    }
                    else
                        String::ToKeyValue(String(data + lastPos, pos - lastPos), stack.key, stack.value,
                                           pIgnoreCase ? String::soIgnoreCase : String::soCaseSensitive);

                    lastPos = pos + 1;
                    if (!stack.key.empty())
                    {
                        // A new section
                        if ("[" == stack.key)
                        {
                            if (pIgnoreCase)
                                stack.value.toLower();
                            stack.sections.push(stack.currentSection);

                            stack.value = ReplaceString( stack.value, "\\n", "\n", false);
                            stack.value = ReplaceString( stack.value, "\\r", "\r", false);

                            if (!stack.level)
                            {
                                if (stack.gadgetMode >= 0)
                                {
                                    String gadgetKey("gadget");
                                    gadgetKey += stack.gadgetMode;
                                    pTable.insertOrUpdate(gadgetKey, stack.value);
                                    ++stack.gadgetMode;
                                    stack.value = gadgetKey;
                                }
                            }
                            else
                                stack.currentSection += ".";
                            stack.currentSection += stack.value;
                            if (stack.gadgetMode < 0 && !exists(stack.currentSection) )
                                pTable.insertOrUpdate(stack.currentSection, stack.value);
                            ++stack.level;
                            continue;
                        }
                        // Start a new block
                        if ("{" == stack.key)
                            continue; // Can be safely ignored
                        // Close the current block
                        if ("}" == stack.key)
                        {
                            if (stack.level > 0)
                            {
                                if (stack.level == 1)
                                    stack.currentSection.clear();
                                else
                                    stack.currentSection = stack.sections.top();
                                stack.sections.pop();
                                --stack.level;
                            }
                            else
                                LOG_ERROR(LOG_PREFIX_TDF << stack.caption << ":" << stack.line
                                          << " : `}` found outside a section");
                            continue;
                        }
                        // Raise an error if there some text outside a block
                        if (stack.currentSection.empty())
                        {
                            LOG_WARNING(LOG_PREFIX_TDF << stack.caption << ":" << stack.line
                                        << " : The text is outside a section (ignored): " << stack.key);
                            continue;
                        }
                        // Do not store empty keys in the table
                        if (!stack.key.empty())
                        {
                            stack.value = ReplaceString( stack.value, "\\n", "\n", false);
                            stack.value = ReplaceString( stack.value, "\\r", "\r", false);

                            if (!special_section.empty() && (stack.currentSection.match("*." + special_section) || stack.currentSection == special_section))
                                pTable.insertOrUpdate(stack.currentSection, pullAsString(stack.currentSection) << "," << stack.key);

                            String realKey(stack.currentSection);
                            realKey << "." << stack.key;
                            pTable.insertOrUpdate(realKey, stack.value);
                        }
                    }
                    continue;
                }
                lastPos = pos + 1;
            }
            else
                stringStarted = true;
        }
        return true;
    }

    void TDFParser::setSpecialSection(const String &section)
    {
        special_section = section;
    }

    sint32 TDFParser::pullAsInt(const String& key, const sint32 def)
    {
        String keyToFind(key);
        if (pIgnoreCase)
            keyToFind.toLower();
        if (!pTable.exists(keyToFind))
            return def;
        String iterFind = pTable.find(keyToFind);
        return ((iterFind.empty())
                ? def
                : (iterFind.size() == 10 && String(iterFind.substr(0,4)).toUInt32() > 127
                   ? (0xFF000000 | String("0x"+ iterFind.substr(4,6)).toUInt32())
                   : iterFind.toInt32()));		// Uses ustrtol to deal with hexa numbers
    }

    real32 TDFParser::pullAsFloat(const String& key, const real32 def)
    {
        if (pIgnoreCase)
        {
            String keyToFind(key);
            keyToFind.toLower();
            if (!pTable.exists(keyToFind))
                return def;
            return pTable.find(keyToFind).toFloat(def);
        }
        if (!pTable.exists(key))
            return def;
        return pTable.find(key).toFloat(def);

    }


    String TDFParser::pullAsString(const String& key, const String& def)
    {
        if (pIgnoreCase)
        {
            String keyToFind(key);
            keyToFind.toLower();
            if (!pTable.exists(keyToFind))
                return def;
            return pTable.find(keyToFind);
        }
        if (!pTable.exists(key))
            return def;
        return pTable.find(key);
    }


    bool TDFParser::pullAsBool(const String& key, const bool def)
    {
        if (pIgnoreCase)
        {
            String keyToFind(key);
            keyToFind.toLower();
            if (!pTable.exists(keyToFind))
                return def;
            return pTable.find(keyToFind).toBool();
        }
        if (!pTable.exists(key))
            return def;
        return pTable.find(key).toBool();

    }

    uint32 TDFParser::pullAsColor(const String& key, const uint32 def)
    {
        String str = pullAsString(key);
        String::Vector params;
        str.split( params, "," );
        if (params.size() < 3)
            return def;
        if (params.size() == 3)
            return makeacol( params[0].toUInt32(), params[1].toUInt32(), params[2].toUInt32(), 0xFF );
        return makeacol( params[0].toUInt32(), params[1].toUInt32(), params[2].toUInt32(), params[3].toUInt32() );
    }

} // namespace TA3D

