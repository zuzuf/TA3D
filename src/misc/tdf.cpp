
#include "tdf.h"
#include "files.h"
#include "../ta3dbase.h"


# define TA3D_LOGS_TDF_PREFIX "[tdf] "


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
        :pTableSize(4096), pTableIsEmpty(true), pIgnoreCase(false)
    {
        pTable.initTable(4096);
    }

    TDFParser::TDFParser(const String& filename, const bool caSensitive, const bool toUTF8, const bool gadgetMode)
        :pTableSize(4096), pTableIsEmpty(true), pIgnoreCase(!caSensitive)
    {
        pTable.initTable(4096);
        loadFromFile(filename, true, toUTF8, gadgetMode);
    }

    TDFParser::~TDFParser()
    {}


    void TDFParser::clear()
    {
        pTable.emptyHashTable();
        pTable.initTable(pTableSize);
        pTableIsEmpty = true;
    }


    bool TDFParser::loadFromFile(const String& filename, const bool clear, const bool toUTF8, const bool gadgetMode)
    {
        uint64 size;
        char* data;
        if (TA3D::VARS::HPIManager)
        {
            uint32 ms;
            data = (char*)TA3D::VARS::HPIManager->PullFromHPI(filename, &ms);
            size = ms;
            if (NULL != data && size != 0)
            {
                bool res = loadFromMemory("HPIManager://" + filename, data, size, clear, toUTF8, gadgetMode);
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
            LOG_ERROR(TA3D_LOGS_TDF_PREFIX << "Unable to open `" << filename << "`");
        else
        {
            delete[] data;
            LOG_WARNING(TA3D_LOGS_TDF_PREFIX << "The file `" << filename << "` is empty (file size=0).");
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
            size = s;
            delete[] data;
            data = t;
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
            if (data[pos] == '\n' || data[pos] == '\r' || data[pos] == '\0' || pos + 1 == size)
            {
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
                            stack.sections.push(stack.currentSection);
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
                                LOG_ERROR(TA3D_LOGS_TDF_PREFIX << stack.caption << ":" << stack.line
                                          << " : `}` found outside a section");
                            continue;
                        }
                        // Raise an error if there some text outside a block
                        if (stack.currentSection.empty())
                        {
                            LOG_WARNING(TA3D_LOGS_TDF_PREFIX << stack.caption << ":" << stack.line
                                        << " : The text is outside a section (ignored): " << stack.key);
                            continue;
                        }
                        // Do not store empty keys in the table
                        if (!clearTable || !stack.value.empty())
                        {
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
                : (iterFind.size() == 10 && ustrtol(iterFind.substr(0,4).c_str(), NULL, 0) > 127
                   ? (0xFF000000 | ustrtol( ("0x"+iterFind.substr(4,6)).c_str(), NULL, 0))
                   : ustrtol(iterFind.c_str() , NULL, 0)));		// Uses ustrtol to deal with hexa numbers
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


} // namespace TA3D

