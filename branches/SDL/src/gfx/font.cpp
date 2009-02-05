
#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "glfunc.h"
#include "../gui.h"
#include "../gaf.h"
#include "gfx.h"
#include "../misc/paths.h"
#include "../logs/logs.h"




namespace TA3D
{



    Font::Font()
    {
        init();
    }

    void Font::init()
    {
        lock();
        font = NULL;
        unlock();
    }

    void Font::print(float x, float y, float z, const String &text)
    {
        MutexLocker locker(pMutex);
        if (font == NULL)   return;

        glScalef(1.0f, -1.0f, 1.0f);
        font->Render( text.c_str(), -1, FTPoint(x, -(1.5f * font->Ascender() - 0.5f * font->LineHeight()) - y, z), FTPoint(), FTGL::RENDER_ALL);
//        font->Render( text.c_str(), -1, FTPoint(x, -font->Ascender() - y, z), FTPoint(), FTGL::RENDER_ALL);
        glScalef(1.0f, -1.0f, 1.0f);
    }

    void Font::destroy()
    {
        lock();
        if (font)
            delete font;
        unlock();
        init();
    }

    float Font::length(const String &txt)
    {
        if (txt.size() == 0)    return 0.0f;
        if (txt[txt.size()-1] == ' ')
            return length(txt + "_") - length("_");
        MutexLocker locker(pMutex);
        if (font == NULL)
            return 0.0f;
#ifdef __FTGL__lower__
        FTBBox box = font->BBox( txt.c_str() );
        return fabsf((box.Upper().Xf() - box.Lower().Xf()));
#else
        float x0, y0, z0, x1, y1, z1;
        font->BBox( txt.c_str(), x0, y0, z0, x1, y1, z1 );
        return fabsf(x0 - x1);
#endif
    }

    float Font::height()
    {
        MutexLocker locker(pMutex);
        if (font == NULL)
            return 0.0f;
        return font->Ascender();
    }

    int Font::get_size()
    {
        MutexLocker locker(pMutex);
        if (font)
            return font->FaceSize();
        return 0;
    }

    FontManager font_manager;

    FontManager::FontManager()
    {
    }

    FontManager::~FontManager()
    {
        destroy();
    }

    void FontManager::destroy()
    {
        for(std::list<Font*>::iterator i = font_list.begin() ; i != font_list.end() ; ++i)
            delete *i;
        font_list.clear();
        font_table.emptyHashTable();
        font_table.initTable(__DEFAULT_HASH_TABLE_SIZE);
    }

    String find_font(const String &path, const String &name)
    {
        String file_path;
        String::List file_list;
        String comp_name = String::ToLower(name + ".ttf");
        Paths::GlobFiles(file_list, path + "/*", true, true);
        for(String::List::iterator i = file_list.begin() ; i != file_list.end() && file_path.empty() ; ++i)
            if (String::ToLower(*i) == comp_name)
                file_path = path + "/" + *i;

        if (file_path.empty())
        {
            String::List dir_list;
            Paths::GlobDirs(dir_list, path + "/*", true, true);
            for(String::List::iterator i = dir_list.begin() ; i != dir_list.end() && file_path.empty() ; ++i)
                if (!StartsWith(*i, "."))
                    file_path = find_font(path + "/" + *i, name);
        }
        return file_path;
    }

    void Font::load(const String &filename, const int size, const int type)
    {
        destroy();
        font = NULL;
        if (!filename.empty())
            switch(type)
            {
            case FONT_TYPE_POLYGON:
                font = new FTBitmapFont(filename.c_str());
                break;
            case FONT_TYPE_BITMAP:
                font = new FTPixmapFont(filename.c_str());
                break;
            case FONT_TYPE_PIXMAP:
                font = new FTPolygonFont(filename.c_str());
                break;
            case FONT_TYPE_TEXTURE:
            default:
                font = new FTTextureFont(filename.c_str());
            };
        if (font)
        {
            font->FaceSize(size);
            font->UseDisplayList(false);
            LOG_DEBUG(LOG_PREFIX_FONT << "'" << filename << "' loaded");
        }
        else
            LOG_ERROR(LOG_PREFIX_FONT << "could not load file : " << filename);
    }

    Font *FontManager::getFont(String filename, int size, int type)
    {
        String key = filename + format("_%d_%d", type, size);

        if (font_table.exists(key))
            return font_table.find(key);

        String bak_filename = filename;
        filename = find_font("/usr/share/fonts", filename);

        if (filename.empty())
            filename = find_font("/usr/share/fonts", "FreeSerif");
        else
            LOG_DEBUG(LOG_PREFIX_FONT << "font not found : " << bak_filename);

        Font *font = new Font();
        font->load(filename, size, type);

        font_table.insertOrUpdate(key, font);

        return font;
    }
} // namespace TA3D
