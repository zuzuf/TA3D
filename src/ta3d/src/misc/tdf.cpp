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

#include <stdafx.h>
#include <gfx/gfx.toolkit.h>
#include "tdf.h"
#include "files.h"
#include <ta3dbase.h>
#include "resources.h"
#include <QIODevice>


namespace TA3D
{


	/*! \class StackInfos
	**
	** \brief
	*/
	struct StackInfos
	{
		StackInfos()
			:level(0), line(0), gadgetMode(-1)
		{}
		//! The full name of the current section
		QString currentSection;
		//! The current key
		QString key;
		//! The current value
		QString value;

		//! The stack for all sections
		std::stack<QString> sections;
		//! Stack Level
		int level;

		//! Current line
		int line;

		//! The gadget mode
		int gadgetMode;

		//! The widget mode
		std::stack<int> widgetMode;

		//! File name (may not be the real filename)
		QString caption;

	}; // class StackInfos





	TDFParser::TDFParser()
		: pIgnoreCase(true), special_section()
	{
	}


	TDFParser::TDFParser(const QString& filename, const bool caSensitive, const bool toUTF8, const bool gadgetMode, const bool realFS, const bool widgetMode)
		: pIgnoreCase(!caSensitive), special_section()
	{
		loadFromFile(filename, true, toUTF8, gadgetMode, realFS, widgetMode);
	}


	TDFParser::~TDFParser()
	{}


	void TDFParser::clear()
	{
		pTable.clear();
		special_section.clear();
	}


	bool TDFParser::loadFromFile(const QString& filename, const bool clear, const bool toUTF8, const bool gadgetMode, const bool realFS, const bool widgetMode)
	{
        QIODevice* file;
		if (!realFS)
		{
			file = VFS::Instance()->readFile(filename);
			if (file && file->size())
			{
                const QByteArray &buffer = file->readAll();
                bool res = loadFromMemory("hpi://" + filename, buffer.data(), buffer.size(), clear, toUTF8, gadgetMode, widgetMode);
				delete file;
				return res;
			}
		}
		else
		{
			file = Paths::Files::LoadContentInMemory(filename, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
			if (file && file->size())
			{
                const QByteArray &buffer = file->readAll();
                bool res = loadFromMemory(filename, buffer.data(), buffer.size(), clear, toUTF8, gadgetMode, widgetMode);
				delete file;
				return res;
			}
		}
		if (file == NULL)
		{
			LOG_ERROR(LOG_PREFIX_TDF << "Unable to open `" << filename << "`");
		}
		else
		{
			delete file;
			LOG_WARNING(LOG_PREFIX_TDF << "The file `" << filename << "` is empty (file size=0).");
		}
		return false;
	}


	bool TDFParser::loadFromMemory(const QString& caption, const char* data, uint64 size, const bool clearTable,
								   const bool toUTF8, const bool gadgetMode, const bool widgetMode)
	{
		if (NULL == data || 0 == size)
			return true;

		//write(1, data, size);
		// Internally, we may have to dupplicate the given buffer, to apply transformations on it
		// It is our duty to free it
		// This will be done at the end of the method, if the following var is not null
		char* tmpBufferToDelete(NULL);

		// Convert it to UTF8 if required
		if (toUTF8)
		{
			uint32 s;
			char* t = ConvertToUTF8(data, /*The current size*/size, /*The new size*/s);
			if (NULL != t)
			{
				size = s;
				data = t;
				tmpBufferToDelete = t;
			}
			else
				LOG_WARNING(LOG_PREFIX_TDF << "The convertion using the UTF8 charset has failed.");
		}
		if (clearTable)
			clear();

		StackInfos stack;
		stack.gadgetMode = gadgetMode ? 0 /* The first index */ : -1 /* means disabled */;
		stack.caption = caption;

		for (const char *p = data, *end = data + size ; p < end ; ++p)
		{
			switch(*p)
			{
			case ' ':				// White spaces
			case '\t':
			case '\r':
				continue;
			case '\n':				// New line
				++stack.line;
				continue;
			case '/':				// Skip comments
				++p;
				if (p == end)
					continue;
				if (*p == '/')
				{
					while(p < end && *p != '\n')
						++p;
					++stack.line;
				}
				else if (*p == '*')
				{
					++p;
					while(p < end && *p != '/')
					{
						while(p < end && *p != '*')
						{
							if (*p == '\n')
								++stack.line;
							++p;
						}
						++p;
						if (*p == '\n')
							++stack.line;
					}
				}
				continue;
			case '[':		// Start a new section
				stack.value.clear();
				++p;
				for (;p < end && *p != ']' ; ++p)
                    stack.value += *p;

				if (pIgnoreCase)
                    stack.value = stack.value.toLower();
				stack.sections.push(stack.currentSection);

                stack.value.replace("\\n", "\n");
                stack.value.replace("\\r", "\r");

				if (!stack.level)
				{
					if (stack.gadgetMode >= 0)
					{
                        const QString &gadgetKey = QString("gadget%1").arg(stack.gadgetMode);
						pTable[gadgetKey] = stack.value;
						++stack.gadgetMode;
						stack.value = gadgetKey;
					}
				}
				else
				{
					stack.currentSection += '.';
					if (widgetMode)
					{
                        const QString &widgetKey = QString("widget%1").arg(stack.widgetMode.top());
						pTable[widgetKey] = stack.value;
						++stack.widgetMode.top();
						stack.value = widgetKey;
					}
				}
				stack.currentSection += stack.value;
                if (stack.gadgetMode < 0 && !stack.currentSection.isEmpty() && !exists(stack.currentSection))
					pTable[stack.currentSection] = stack.value;
				++stack.level;
				stack.widgetMode.push(0);
				continue;
			case '{':		// Section bloc
				continue;
			case '}':		// End of section bloc
				if (stack.level > 0)
				{
					if (stack.level == 1)
						stack.currentSection.clear();
					else
						stack.currentSection = stack.sections.top();
					stack.sections.pop();
					--stack.level;
					stack.widgetMode.pop();
				}
				else
					LOG_ERROR(LOG_PREFIX_TDF << stack.caption << ":" << stack.line << " : `}` found outside a section");
				continue;
			default:
				if ((*p >= 'a' && *p <= 'z')
					|| (*p >= 'A' && *p <= 'Z')
					|| (*p >= '0' && *p <= '9')
					|| *p == '_')		// Variable name
				{
					stack.key.clear();
					for(; p < end && *p != '=' ; ++p)
                        stack.key += *p;
					if (p >= end)
						continue;
					++p;
					// Skip white spaces
					while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
					{
						if (*p == '\n')
							++stack.line;
						++p;
					}
					stack.value.clear();
					for(; p < end && *p != ';' ; ++p)
                        stack.value += *p;

					// Raise an error if there is text outside a block
                    if (stack.currentSection.isEmpty())
					{
						LOG_WARNING(LOG_PREFIX_TDF << stack.caption << ":" << stack.line
									<< " : The text is outside a section (ignored): " << stack.key);
						continue;
					}
					// Do not store empty keys in the table
                    stack.key = stack.key.trimmed();
                    if (!stack.key.isEmpty())
					{
						if (pIgnoreCase)
                            stack.key = stack.key.toLower();
                        stack.value = stack.value.trimmed();
                        stack.value.replace("\\n", "\n");
                        stack.value.replace("\\r", "\r");

                        if (!special_section.isEmpty() && (QRegExp("*." + special_section, Qt::CaseSensitive, QRegExp::Wildcard).exactMatch(stack.currentSection) || stack.currentSection == special_section))
                            pTable[stack.currentSection] = (pullAsString(stack.currentSection) + "," + stack.key);

                        const QString &realKey = stack.currentSection + "." + stack.key;
						pTable[realKey] = stack.value;
					}
				}
			};
		}

		DELETE_ARRAY(tmpBufferToDelete);
		return true;
	}

	void TDFParser::setSpecialSection(const QString &section)
	{
		special_section = section;
	}

	float TDFParser::pullAsFloat(const QString& key, const float def)
	{
		float f;
        const QString keyToFind(pIgnoreCase ? key.toLower() : key);
        TA3D::UTILS::HashMap<QString>::Dense::iterator entry = pTable.find(keyToFind);
        if (entry == pTable.end() || entry->isEmpty())
            return def;
        bool ok;
        f = entry->toFloat(&ok);
        return ok ? f : def;

	}


	bool TDFParser::pullAsBool(const QString& key, const bool def)
	{
        QString keyToFind(pIgnoreCase ? key.toLower() : key);
        TA3D::UTILS::HashMap<QString>::Dense::iterator entry = pTable.find(keyToFind);
        if (entry == pTable.end() || entry->isEmpty())
            return def;
        return entry->toInt();

	}

	uint32 TDFParser::pullAsColor(const QString& key, const uint32 def)
	{
        const QString& str = pullAsString(key);
        QStringList params = str.split(',', QString::KeepEmptyParts);
		if (params.size() < 3)
			return def;
		if (params.size() == 3)
            return makeacol( params[0].toUInt(), params[1].toUInt(), params[2].toUInt(), 0xFF );
        return makeacol( params[0].toUInt(), params[1].toUInt(), params[2].toUInt(), params[3].toUInt() );
	}


    sint32 TDFParser::pullAsInt(const QString &key, const sint32 def)
    {
        const QString keyToFind(pIgnoreCase ? key.toLower() : key);
        TA3D::UTILS::HashMap<QString>::Dense::iterator entry = pTable.find(keyToFind);
        return (entry == pTable.end() || entry->isEmpty() ? def : entry->toInt());
    }

    sint32 TDFParser::pullAsInt(const QString& key)
    {
        const QString keyToFind(pIgnoreCase ? key.toLower() : key);
        TA3D::UTILS::HashMap<QString>::Dense::iterator entry = pTable.find(keyToFind);
        return (entry == pTable.end() || entry->isEmpty() ? 0 : entry->toInt());
    }


    QString TDFParser::pullAsString(const QString& key, const QString& def)
    {
        const QString keyToFind(pIgnoreCase ? key.toLower() : key);
        TA3D::UTILS::HashMap<QString>::Dense::iterator entry = pTable.find(keyToFind);
        return entry == pTable.end() ? def : entry.value();
    }


    QString TDFParser::pullAsString(const QString& key)
    {
        const QString keyToFind(pIgnoreCase ? key.toLower() : key);
        TA3D::UTILS::HashMap<QString>::Dense::iterator entry = pTable.find(keyToFind);
        return entry == pTable.end() ? nullptr : entry.value();
    }


} // namespace TA3D

