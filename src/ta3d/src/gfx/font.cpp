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
#include <gaf.h>
#include "gfx.h"
#include <misc/paths.h>
#include <logs/logs.h>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QImage>
#include <QFontDatabase>

#define TA3D_FONT_PATH  			"fonts"

namespace TA3D
{
	FontManager font_manager;

	Font::Font()
        : bBold(false)
	{}


	Font::Font(const Font& rhs)
        : pFontFilename(rhs.pFontFilename), bBold(false)
	{
        if (!pFontFilename.isEmpty())
            this->load(pFontFilename, rhs.font->pixelSize());
	}

	Font::~Font()
	{
        destroy();
	}

	Font& Font::operator = (const Font& rhs)
	{
		return *this;
	}


	void Font::init()
	{
        metrics.reset();
        font.reset();
		pFontFilename.clear();
	}



    void Font::print(float x, float y, const quint32 col, const QString& text)
	{
        if (text.isEmpty() || !font)
			return;

        gfx->glEnable(GL_BLEND);
        gfx->glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		for(int k = 0 ; k < (bBold ? 3 : 1) ; ++k)
		{
            QPointF pos(x, y + 0.5f * metrics->height());
            for(int i = 0 ; i < text.size() ; ++i)
            {
                const QChar &c = text[i];
                auto it = glyphs.find(c);
                if (it == glyphs.end())
                {
                    const QRectF &c_bbox = metrics->boundingRect(c);
                    QImage img = gfx->create_surface_ex(32,
                                                        std::ceil(c_bbox.width()),
                                                        std::ceil(metrics->height()));
                    img.fill(0);
                    QPainter painter(&img);
                    painter.setPen(Qt::white);
                    painter.setFont(*font);
                    painter.drawText(QPointF(-metrics->leftBearing(c), metrics->ascent()), QString(c));

                    it = glyphs.insert(c, gfx->make_texture(img));
                }
                const GfxTexture::Ptr &tex = it.value();
                const QPointF origin(pos.x() + metrics->leftBearing(c),
                                     pos.y() - metrics->ascent());
                gfx->drawtexture(tex, origin.x(), origin.y(), col);
                pos += QPointF(metrics->width(c), 0);
            }
        }
	}

    void Font::print_center(float x, float y, const quint32 col, const QString &text)
    {
        const float X = x - 0.5f * length(text);

        print(X, y, col, text);
    }

    void Font::print_right(float x, float y, const quint32 col, const QString &text)
    {
        const float X = x - length(text);
        print(X, y, col, text);
    }

	void Font::destroy()
	{
        metrics.reset();
        font.reset();
        pFontFilename.clear();
        glyphs.clear();
	}


    float Font::length(const QString &txt)
	{
        if (txt.isEmpty())
			return 0.0f;
        if (txt.endsWith(' '))
            return length(txt + "_") - length("_");

		if (!font)
			return 0.0f;
        return metrics->boundingRect(txt).width();
	}


	float Font::height()
	{
        if (!font)
            return 0.f;
        return metrics->ascent();
	}

	int Font::get_size()
	{
        return (font) ? font->pixelSize() : 0;
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
        pFontList.clear();
		font_table.clear();
	}



    static void find_font(QString& out, const QString &path, const QString &name)
	{
		LOG_DEBUG(LOG_PREFIX_FONT << "looking for " << name);
		out.clear();
        QStringList file_list;
        QString tmp;

        const QString comp_name = (name + ".ttf").toLower();

        VFS::Instance()->getFilelist(path + "/*", file_list);
		// Looking for the file
        for (const QString &i : file_list)
        {
            if (Paths::ExtractFileName(i).toLower() == comp_name)
            {
                out = i;
                break;
            }
        }

        if (!out.isEmpty())     // If we have a font in our VFS, then we have to extract it to a temporary location
        {                       // in order to load it to the QFontDatabase
			LOG_DEBUG(LOG_PREFIX_FONT << "font found: " << out);
            tmp = TA3D::Paths::Caches + Paths::ExtractFileName(name) + ".ttf";

            QIODevice *file = VFS::Instance()->readFile(out);
			if (file)
			{
                QFile *real_file = dynamic_cast<QFile*>(file);
                if (real_file)
                    tmp = real_file->fileName();
                else if (!QFileInfo(tmp).exists() || QFileInfo(tmp).size() != (uint32)file->size())
				{
                    QFile tmp_file(tmp);
					LOG_DEBUG(LOG_PREFIX_FONT << "Creating temporary file for " << name << " (" << tmp << ")");

                    tmp_file.open(QIODevice::WriteOnly);
                    if (tmp_file.isOpen())
					{
                        while(file->bytesAvailable())
                            tmp_file.write(file->read(10240));
						tmp_file.flush();
						tmp_file.close();
					}
					else
						LOG_ERROR(LOG_PREFIX_FONT << "Impossible to create the temporary file `" << tmp << "`");
				}
				else
					LOG_INFO(LOG_PREFIX_FONT << "`" << name << "`: From cache (`" << tmp << "`)");
                out = tmp;
				delete file;
			}
		}
	}


    bool Font::load(const QString &filename, const int size)
	{
        destroy();

		pFontFilename = filename;

        if (!filename.isEmpty())
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "Loading `" << filename << "`");
            font.reset(new QFont(filename));
		}
		if (font)
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "'" << filename << "' loaded");
            font->setPixelSize(size);
            metrics.reset(new QFontMetricsF(*font));
        }
		else
			LOG_ERROR(LOG_PREFIX_FONT << "Could not load file : " << filename);

		return (!font);
	}



    Font::Ptr FontManager::find(const QString& filename, const int size)
	{
        const QString &key = filename.toLower() + QString("_%1").arg(size);

		return (font_table.count(key) != 0)
			? font_table[key]
            : internalRegisterFont(key, filename, size);
	}



    Font::Ptr FontManager::internalRegisterFont(const QString& key, const QString& filename, const int size)
	{
        QString foundFilename;
		find_font(foundFilename, TA3D_FONT_PATH, filename);

        if (foundFilename.isEmpty())
		{
			LOG_DEBUG(LOG_PREFIX_FONT << "font not found : " << filename);
			find_font(foundFilename, TA3D_FONT_PATH, "FreeSerif");
		}

        Font::Ptr font = new Font();
        const int font_id = QFontDatabase::addApplicationFont(foundFilename);
        font->load(QFontDatabase::applicationFontFamilies(font_id).front(), size);

		pFontList.push_back(font);
		font_table[key] = font;

		return font;
	}



} // namespace TA3D
