
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
        font = NULL;
    }

    void Font::print(float x, float y, float z, const String &text)
    {
        if (font == NULL)   return;

        glScalef(1.0f, -1.0f, 1.0f);
        font->Render( text.c_str(), -1, FTPoint(x, -font->Ascender() - y, z), FTPoint(), FTGL::RENDER_ALL);
        glScalef(1.0f, -1.0f, 1.0f);
    }

    void Font::load_gaf_font(const String &filename)
    {
        destroy();

        LOG_DEBUG("Loading GAF font: `" << filename << "`...");
        byte *data = HPIManager->PullFromHPI(filename);
        if(data)
        {
            Gaf::Animation gafFont;
            gafFont.loadGAFFromRawData(data);
            int h = 0, mx = 0, my = 0;
            for (int i = 0; i < gafFont.nb_bmp; ++i)
            {
                if (abs( gafFont.ofs_x[i]) > 50 || abs( gafFont.ofs_y[i]) > 50)
                    continue;
                if (-gafFont.ofs_x[i] < mx)    mx = -gafFont.ofs_x[i];
                if (-gafFont.ofs_y[i] < my)    my = -gafFont.ofs_y[i];
                if (gafFont.bmp[i]->h > h)     h = gafFont.bmp[i]->h;
            }
            my += 2;
            h -= 2;

//            FONT_COLOR_DATA	*fc = (FONT_COLOR_DATA*) malloc(sizeof(FONT_COLOR_DATA));
//            fc->begin = 0;
//            fc->end = gafFont.nb_bmp;
//            fc->bitmaps = (BITMAP**) malloc(sizeof(BITMAP*) * gafFont.nb_bmp);
//            fc->next = NULL;
//            for (int i = 0 ; i < gafFont.nb_bmp ; ++i)
//            {
//                fc->bitmaps[ i ] = create_bitmap_ex( 32, gafFont.bmp[ i ]->w, h);
//                clear_to_color( fc->bitmaps[ i ], 0xFF00FF );
//                if (i != 32 )		// Spaces must remain blank
//                    blit( gafFont.bmp[ i ], fc->bitmaps[ i ], 0,0, -gafFont.ofs_x[ i ] - mx, -gafFont.ofs_y[ i ] - my, gafFont.bmp[ i ]->w, gafFont.bmp[ i ]->h );
//                for (int y = 0 ; y < fc->bitmaps[ i ]->h ; y++ )
//                {
//                    for (int x = 0 ; x < fc->bitmaps[ i ]->w ; ++x)
//                    {
//                        if(getpixel(fc->bitmaps[ i ], x, y ) == 0)
//                            putpixel(fc->bitmaps[ i ], x, y, 0xFF00FF);
//                    }
//                }
//            }
            delete[] data;
//            pAl = (FONT*) malloc( sizeof(FONT));
//            pAl->data = fc;
//            pAl->height = h;
//            pAl->vtable = font_vtable_color;
        }
        else
        {
            throw cError( "Font::load_gafFont()", "file could not be read, data = NULL.", true );
            return;
        }

#warning FIXME: GAF fonts not implemented yet!
    }


    void Font::destroy()
    {
        if (font)
            delete font;
        init();
    }

    float Font::length(const String &txt) const
    {
        if (font == NULL)
            return 0.0f;
#ifdef __FTGL__lower__
        FTBBox box = font->BBox( txt.c_str(), txt.size() );
        return fabsf((box.Upper().Xf() - box.Lower().Xf()));
#else
        float x0, y0, z0, x1, y1, z1;
        font->BBox( txt.c_str(), x0, y0, z0, x1, y1, z1 );
        return fabsf(x0 - x1);
#endif
    }

    float Font::height() const
    {
        if (font == NULL)
            return 0.0f;
        return font->LineHeight();
    }

    float Font::ascender() const
    {
        if (font == NULL)
            return 0.0f;
        return font->Ascender();
    }

    int Font::get_size() const
    {
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
