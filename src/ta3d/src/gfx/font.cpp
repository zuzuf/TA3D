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
#include <TA3D_NameSpace.h>
#include "glfunc.h"
#include <gaf.h>
#include "gfx.h"
#include <misc/paths.h>
#include <logs/logs.h>
#include <yuni/core/io/file.h>
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;


namespace TA3D
{

	FontManager font_manager;



	Font::Font()
		:ObjectSync(), font((FTFont*)NULL), pFontFilename(), pType(typeTexture), bBold(false)
	{}


	Font::Font(const Font& rhs)
		:ObjectSync(), font((FTFont*)NULL), pFontFilename(rhs.pFontFilename), pType(rhs.pType), bBold(false)
	{
		if (!pFontFilename.empty())
			this->loadWL(pFontFilename, rhs.font->FaceSize(), pType);
	}

	Font::~Font()
	{
        font.reset();
	}


	Font& Font::operator = (const Font& rhs)
	{
		MutexLocker locker(pMutex);
        font.reset();
        pFontFilename = rhs.pFontFilename;
		if (!pFontFilename.empty())
			this->loadWL(pFontFilename, rhs.font->FaceSize(), pType);
		return *this;
	}


	void Font::init()
	{
		MutexLocker locker(pMutex);
        font.reset();
		pFontFilename.clear();
	}



	void Font::print(float x, float y, float z, const String& text)
	{
		if (text.empty())
			return;
		MutexLocker locker(pMutex);
		if (!font)
			return;

		glScalef(1.0f, -1.0f, 1.0f);
		for(int k = 0 ; k < (bBold ? 3 : 1) ; ++k)
		{
#ifdef __FTGL__lower__
			font->Render( text.c_str(), -1,
				FTPoint(x, -(y + 0.5f * (-font->Descender() + font->Ascender())), z),
				FTPoint(), FTGL::RENDER_ALL);
#else
			glPushMatrix();
			glTranslatef( x, -(y + 0.5f * (-font->Descender() + font->Ascender())), z );
# ifndef TA3D_PLATFORM_DARWIN
			WString wstr(text);
			font->Render(wstr.cw_str());
# else
			font->Render(text.c_str());
# endif
			glPopMatrix();
#endif
		}
		glScalef(1.0f, -1.0f, 1.0f);
	}


	void Font::destroy()
	{
		MutexLocker locker(pMutex);
        font.reset();
        pFontFilename.clear();
	}


	float Font::length(const String &txt)
	{
		if (txt.empty())
			return 0.0f;
		if (' ' == txt.last())
			return length(String(txt) << "_") - length("_");

		MutexLocker locker(pMutex);
		if (!font)
			return 0.0f;
#ifdef __FTGL__lower__
		FTBBox box = font->BBox( txt.c_str() );
		return fabsf((box.Upper().Xf() - box.Lower().Xf()));
#else
		float x0, y0, z0, x1, y1, z1;
# ifndef TA3D_PLATFORM_DARWIN
        WString wstr(txt);
        font->BBox(wstr.cw_str(), x0, y0, z0, x1, y1, z1);
# else
		font->BBox(txt.c_str(), x0, y0, z0, x1, y1, z1);
# endif
		return fabsf(x0 - x1);
#endif
	}


	float Font::height()
	{
		MutexLocker locker(pMutex);
		return (font) ? font->Ascender() : 0.f;
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
		font_table.clear();
	}



	static void find_font(String& out, const String &path, const String &name)
	{
		LOG_DEBUG(LOG_PREFIX_FONT << "looking for " << name);
		out.clear();
		String::Vector file_list;
		String tmp;

		String comp_name;
		(comp_name << name << ".ttf").toLower();

		VFS::Instance()->getFilelist(String(path) << "/*", file_list);
		// Looking for the file
		{
			const String::Vector::iterator end = file_list.end();
			for (String::Vector::iterator i = file_list.begin() ; i != end; ++i)
			{
				tmp = Paths::ExtractFileName(*i);
				tmp.toLower();
				if (tmp == comp_name)
				{
					out = *i;
					break;
				}
			}
		}

		if (!out.empty())       // If we have a font in our VFS, then we have to extract it to a temporary location
		{                       // in order to load it with FTGL
			LOG_DEBUG(LOG_PREFIX_FONT << "font found: " << out);
			tmp.clear();
			tmp << TA3D::Paths::Caches << Paths::ExtractFileName(name) << ".ttf";

			File *file = VFS::Instance()->readFile(out);
			if (file)
			{
				if (file->isReal())
					tmp = file->getRealFilename();
				else if (!Yuni::Core::IO::File::Exists(tmp) || Yuni::Core::IO::File::Size(tmp) != (uint32)file->size())
				{
					Stream tmp_file;
					LOG_DEBUG(LOG_PREFIX_FONT << "Creating temporary file for " << name << " (" << tmp << ")");

					tmp_file.open(tmp, Yuni::Core::IO::OpenMode::write);
					if (tmp_file.opened())
					{
						char *buf = new char[10240];
						for(int i = 0 ; i < file->size() ; i += 10240)
						{
							const int l = Math::Min(10240, file->size() - i);
							file->read(buf, l);
							tmp_file.write(buf, l);
						}
						delete[] buf;
						tmp_file.flush();
						tmp_file.close();
					}
					else
						LOG_ERROR(LOG_PREFIX_FONT << "Impossible to create the temporary file `" << tmp << "`");
				}
				else
				{
					LOG_INFO(LOG_PREFIX_FONT << "`" << name << "`: From cache (`" << tmp << "`)");
				}
				out.clear();
				out << tmp;
# ifdef TA3D_PLATFORM_WINDOWS
				out.convertSlashesIntoBackslashes();
# endif
				delete file;
			}
		}
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

        font.reset();
        if (!filename.empty())
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "Loading `" << filename << "`");
			switch(type)
			{
				case typeBitmap:
                    font.reset(new FTBitmapFont(filename.c_str()));
					break;
				case typePixmap:
                    font.reset(new FTPixmapFont(filename.c_str()));
					break;
				case typePolygon:
                    font.reset(new FTPolygonFont(filename.c_str()));
					break;
				case typeTextures:
                    font.reset(new FTTextureFont(filename.c_str()));
					break;
				case typeTexture:
				default:
#ifdef __FTGL__lower__
                    font.reset(new FTBufferFont(filename.c_str()));
#else
                    font.reset(new FTTextureFont(filename.c_str()));
#endif
			}
		}
		if (font)
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "'" << filename << "' loaded");
			font->FaceSize(size);
			font->UseDisplayList(false);
		}
		else
			LOG_ERROR(LOG_PREFIX_FONT << "Could not load file : " << filename);

		return (!font);
	}



	Font *FontManager::find(const String& filename, const int size, const Font::Type type)
	{
		String key(filename);
		key << "_" << int(type) << "_" << size;
		key.toLower();

		return (font_table.count(key) != 0)
			? font_table[key]
			: internalRegisterFont(key, filename, size, type);
	}



	Font* FontManager::internalRegisterFont(const String& key, const String& filename, const int size,
		const Font::Type type)
	{
		String foundFilename;
		find_font(foundFilename, TA3D_FONT_PATH, filename);

		if (foundFilename.empty())
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "font not found : " << filename);
			find_font(foundFilename, TA3D_FONT_PATH, "FreeSerif");
		}

		Font *font = new Font();
		font->load(foundFilename, size, type);

		pFontList.push_back(font);
		font_table[key] = font;

		return font;
	}



} // namespace TA3D
