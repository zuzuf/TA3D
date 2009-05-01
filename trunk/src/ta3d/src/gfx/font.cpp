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
#include "../TA3D_NameSpace.h"
#include "glfunc.h"
#include "../gui.h"
#include "../gaf.h"
#include "gfx.h"
#include "../misc/paths.h"
#include "../logs/logs.h"
#include <fstream>



namespace TA3D
{

	FontManager font_manager;



	Font::Font()
		:ObjectSync(), font(NULL), pFontFilename(), pType(typeTexture)
	{}


	Font::Font(const Font& rhs)
		:ObjectSync(), font(NULL), pFontFilename(rhs.pFontFilename), pType(rhs.pType)
	{
		if (!pFontFilename.empty())
			this->loadWL(pFontFilename, rhs.font->FaceSize(), pType);
	}

	Font::~Font()
	{
		if (font)
			delete font;
	}


	Font& Font::operator = (const Font& rhs)
	{
		MutexLocker locker(pMutex);
		if (font)
		{
			delete font;
			font = NULL;
		}
		pFontFilename = rhs.pFontFilename;
		if (!pFontFilename.empty())
			this->loadWL(pFontFilename, rhs.font->FaceSize(), pType);
		return *this;
	}


	void Font::init()
	{
		MutexLocker locker(pMutex);
		if (font)
		{
			delete font;
			font = NULL;
		}
		pFontFilename.clear();
	}



	void Font::print(float x, float y, float z, const String &text)
	{
		MutexLocker locker(pMutex);
		if (font == NULL)
			return;

		glScalef(1.0f, -1.0f, 1.0f);
#ifdef __FTGL__lower__
		font->Render( text.c_str(), -1,
			FTPoint(x, -(y + 0.5f * (-font->Descender() + font->Ascender())), z),
			FTPoint(), FTGL::RENDER_ALL);
#else
		glPushMatrix();
		glTranslatef( x, -(y + 0.5f * (-font->Descender() + font->Ascender())), z );
		WString wstr(text);
		font->Render(wstr.cw_str());
		glPopMatrix();
#endif
		glScalef(1.0f, -1.0f, 1.0f);
	}


	void Font::destroy()
	{
		MutexLocker locker(pMutex);
		if (font)
		{
			delete font;
			font = NULL;
		}
		pFontFilename.clear();
	}


	float Font::length(const String &txt)
	{
		if (txt.empty())
			return 0.0f;
		if (' ' == txt[txt.size() - 1])
			return length(txt + "_") - length("_");

		MutexLocker locker(pMutex);
		if (font == NULL)
			return 0.0f;
#ifdef __FTGL__lower__
		FTBBox box = font->BBox( txt.c_str() );
		return fabsf((box.Upper().Xf() - box.Lower().Xf()));
#else
		float x0, y0, z0, x1, y1, z1;
		WString wstr(txt);
		font->BBox(wstr.cw_str(), x0, y0, z0, x1, y1, z1);
		return fabsf(x0 - x1);
#endif
	}


	float Font::height()
	{
		MutexLocker locker(pMutex);
		return (font) ? font->Ascender() : 0.;
	}

	int Font::get_size()
	{
		MutexLocker locker(pMutex);
		return (font) ? font->FaceSize() : 0;
	}


	FontManager::FontManager()
	{
	}

	FontManager::~FontManager()
	{
		destroy();
	}

	void FontManager::destroy()
	{
		if (!pFontList.empty())
		{
			for (FontList::iterator i = pFontList.begin(); i != pFontList.end(); ++i)
				delete *i;
			pFontList.clear();
		}
		font_table.emptyHashTable();
		font_table.initTable(__DEFAULT_HASH_TABLE_SIZE);
	}



	String find_font(const String &path, const String &name)
	{
		LOG_DEBUG(LOG_PREFIX_FONT << "looking for " << name);
		String file_path;
		String::List file_list;
		String comp_name = String::ToLower(name + ".ttf");
		if (HPIManager)
			HPIManager->getFilelist(path + "/*", file_list);
		else
			Paths::GlobFiles(file_list, path + "/*", true, true);
		for (String::List::iterator i = file_list.begin() ; i != file_list.end() && file_path.empty() ; ++i)
		{
			if (String::ToLower( Paths::ExtractFileName(*i) ) == comp_name)
				file_path = HPIManager == NULL ? path + "/" + *i : *i;
		}

		if (file_path.empty() && !HPIManager)
		{
			String::List dir_list;
			Paths::GlobDirs(dir_list, path + "/*", true, true);
			for(String::List::iterator i = dir_list.begin() ; i != dir_list.end() && file_path.empty() ; ++i)
				if (!StartsWith(*i, "."))
					file_path = find_font(path + "/" + *i, name);
		}

		if (HPIManager && !file_path.empty())       // If we have a font in our VFS, then we have to extract it to a temporary location
		{                                           // in order to load it with FTGL
			LOG_DEBUG(LOG_PREFIX_FONT << "font found: " << file_path);
			uint32 font_size = 0;
			byte *data = HPIManager->PullFromHPI(file_path, &font_size);
			if (data)
			{
				LOG_DEBUG(LOG_PREFIX_FONT << "creating temporary file for " << name);
				std::fstream tmp_file;
				tmp_file.open( (TA3D::Paths::Caches + Paths::ExtractFileName(name) + ".ttf").c_str(), std::fstream::out | std::fstream::binary);
				if (tmp_file.is_open())
				{
					tmp_file.write((char*)data, font_size);
					tmp_file.flush();
					tmp_file.close();
#ifdef TA3D_PLATFORM_WINDOWS
					file_path = String::ConvertSlashesIntoAntiSlashes( TA3D::Paths::Caches + Paths::ExtractFileName(name) + ".ttf" );
#else
					file_path = TA3D::Paths::Caches + Paths::ExtractFileName(name) + ".ttf";
#endif
				}
				delete[] data;
			}
		}

		return file_path;
	}


	bool Font::load(const String &filename, const int size, const Font::Type type)
	{
		MutexLocker locker(pMutex);
		return this->loadWL(filename, size, type);
	}


	bool Font::loadWL(const String &filename, const int size, const Font::Type type)
	{
		pFontFilename = filename;
		pType = type;

		if (font)
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "Destroying Font object");
			delete font;
			font = NULL;
		}
		if (!filename.empty())
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "Creating FTFont object for " << filename);
			switch(type)
			{
				case typeBitmap:
					font = new FTBitmapFont(filename.c_str());
					break;
				case typePixmap:
					font = new FTPixmapFont(filename.c_str());
					break;
				case typePolygon:
					font = new FTPolygonFont(filename.c_str());
					break;
				case typeTexture:
				default:
					font = new FTTextureFont(filename.c_str());
			}
		}
		if (font)
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "'" << filename << "' loaded");
			font->FaceSize(size);
			LOG_DEBUG(LOG_PREFIX_FONT << "face size set");
			font->UseDisplayList(false);
			LOG_DEBUG(LOG_PREFIX_FONT << "parameters set '" << filename << "'");
		}
		else
			LOG_ERROR(LOG_PREFIX_FONT << "Could not load file : " << filename);

		return (NULL != font);
	}



	Font *FontManager::find(const String& filename, const int size, const Font::Type type)
	{
		const String& key = String::ToLower(filename + format("_%d_%d", int(type), size));

		return (font_table.exists(key))
			? font_table.find(key)
			: internalRegisterFont(key, filename, size, type);
	}



	Font* FontManager::internalRegisterFont(const String& key, const String& filename, const int size,
		const Font::Type type)
	{
		String foundFilename = (HPIManager)
				? find_font(TA3D_FONT_PATH, filename)
				: find_font(SYSTEM_FONT_PATH, filename);

		if (foundFilename.empty())
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "font not found : " << filename);
			foundFilename = (HPIManager)
					? find_font(TA3D_FONT_PATH, "FreeSerif")
					: find_font(SYSTEM_FONT_PATH, "FreeSerif");
		}

		LOG_DEBUG(LOG_PREFIX_FONT << "creating new Font object for " << foundFilename);
		Font *font = new Font();
		LOG_DEBUG(LOG_PREFIX_FONT << "loading file " << foundFilename);
		font->load(foundFilename, size, type);

		LOG_DEBUG(LOG_PREFIX_FONT << "inserting " << foundFilename << " into Font tables");
		pFontList.push_back(font);
		font_table.insertOrUpdate(key, font);

		LOG_DEBUG(LOG_PREFIX_FONT << "Font loader : job done");
		return font;
	}



} // namespace TA3D
