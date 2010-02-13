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
#include <vfs/file.h>


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

		//! The widget mode
		std::stack<int> widgetMode;

		//! File name (may not be the real filename)
		String caption;

	}; // class StackInfos





	TDFParser::TDFParser()
		: pIgnoreCase(true), special_section()
	{
		pTable.set_empty_key(String());
	}


	TDFParser::TDFParser(const String& filename, const bool caSensitive, const bool toUTF8, const bool gadgetMode, const bool realFS, const bool widgetMode)
		: pIgnoreCase(!caSensitive), special_section()
	{
		pTable.set_empty_key(String());
		loadFromFile(filename, true, toUTF8, gadgetMode, realFS, widgetMode);
	}


	TDFParser::~TDFParser()
	{}


	void TDFParser::clear()
	{
		pTable.clear();
		special_section.clear();
	}


	bool TDFParser::loadFromFile(const String& filename, const bool clear, const bool toUTF8, const bool gadgetMode, const bool realFS, const bool widgetMode)
	{
		File* file;
		if (!realFS)
		{
			file = VFS::Instance()->readFile(filename);
			if (file && file->size())
			{
				bool res = loadFromMemory("hpi://" + filename, file->data(), file->size(), clear, toUTF8, gadgetMode, widgetMode);
				delete file;
				return res;
			}
		}
		else
		{
			file = Paths::Files::LoadContentInMemory(filename, TA3D_FILES_HARD_LIMIT_FOR_SIZE);
			if (file && file->size())
			{
				bool res = loadFromMemory(filename, file->data(), file->size(), clear, toUTF8, gadgetMode, widgetMode);
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


	bool TDFParser::loadFromMemory(const String& caption, const char* data, uint64 size, const bool clearTable,
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
				// DELETE_ARRAY(data); The pointer will be freed by the caller
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
						String::ExtractKeyValue(String(data + lastPos, pos - lastPos + 1), stack.key, stack.value,
												pIgnoreCase ? String::soIgnoreCase : String::soCaseSensitive);
						++stack.line;
					}
					else
						String::ExtractKeyValue(String(data + lastPos, pos - lastPos), stack.key, stack.value,
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

							stack.value.replace("\\n", "\n");
							stack.value.replace("\\r", "\r");

							if (!stack.level)
							{
								if (stack.gadgetMode >= 0)
								{
									String gadgetKey("gadget");
									gadgetKey += stack.gadgetMode;
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
									String widgetKey("widget");
									widgetKey += stack.widgetMode.top();
									pTable[widgetKey] = stack.value;
									++stack.widgetMode.top();
									stack.value = widgetKey;
								}
							}
							stack.currentSection += stack.value;
							if (stack.gadgetMode < 0 && !stack.currentSection.empty() && !exists(stack.currentSection))
								pTable[stack.currentSection] = stack.value;
							++stack.level;
							stack.widgetMode.push(0);
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
								stack.widgetMode.pop();
							}
							else
								LOG_ERROR(LOG_PREFIX_TDF << stack.caption << ":" << stack.line << " : `}` found outside a section");
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
							stack.value.replace("\\n", "\n");
							stack.value.replace("\\r", "\r");

							if (!special_section.empty() && (stack.currentSection.glob("*." + special_section) || stack.currentSection == special_section))
								pTable[stack.currentSection] = (pullAsString(stack.currentSection) << "," << stack.key);

							String realKey(stack.currentSection);
							realKey << "." << stack.key;
							pTable[realKey] = stack.value;
						}
					}
					continue;
				}
				lastPos = pos + 1;
			}
			else
				stringStarted = true;
		}
		DELETE_ARRAY(tmpBufferToDelete);
		return true;
	}

	void TDFParser::setSpecialSection(const String &section)
	{
		special_section = section;
	}

	float TDFParser::pullAsFloat(const String& key, const float def)
	{
		float f;
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(keyToFind);
			if (entry == pTable.end() || entry->second.empty())
				return def;
			return entry->second.to<float>(f) ? f : def;
		}
		TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(key);
		if (entry == pTable.end() || entry->second.empty())
			return def;
		return entry->second.to<float>(f) ? f : def;

	}


	bool TDFParser::pullAsBool(const String& key, const bool def)
	{
		if (pIgnoreCase)
		{
			String keyToFind(key);
			keyToFind.toLower();
			TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(keyToFind);
			if (entry == pTable.end() || entry->second.empty())
				return def;
			return entry->second.to<bool>();
		}
		TA3D::UTILS::HashMap<String>::Dense::iterator entry = pTable.find(key);
		if (entry == pTable.end() || entry->second.empty())
			return def;
		return entry->second.to<bool>();

	}

	uint32 TDFParser::pullAsColor(const String& key, const uint32 def)
	{
		const String& str = pullAsString(key);
		String::Vector params;
		str.explode(params, ',');
		if (params.size() < 3)
			return def;
		if (params.size() == 3)
			return makeacol( params[0].to<uint32>(), params[1].to<uint32>(), params[2].to<uint32>(), 0xFF );
		return makeacol( params[0].to<uint32>(), params[1].to<uint32>(), params[2].to<uint32>(), params[3].to<uint32>() );
	}




} // namespace TA3D

