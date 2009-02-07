#ifndef __TA3D_GFX_FONT_H__
# define __TA3D_GFX_FONT_H__

# include "gfx.h"
# include "../misc/hash_table.h"
# include "../threads/thread.h"

#ifdef __FTGL__lower__
    #include <FTGL/ftgl.h>
#else
    #include <FTGL/FTGL.h>
    #include <FTGL/FTPoint.h>
    #include <FTGL/FTBBox.h>

    #include <FTGL/FTGlyph.h>
    #include <FTGL/FTBitmapGlyph.h>
    #include <FTGL/FTExtrdGlyph.h>
    #include <FTGL/FTOutlineGlyph.h>
    #include <FTGL/FTPixmapGlyph.h>
    #include <FTGL/FTPolyGlyph.h>
    #include <FTGL/FTTextureGlyph.h>

    #include <FTGL/FTFont.h>
    #include <FTGL/FTGLBitmapFont.h>
    #include <FTGL/FTGLExtrdFont.h>
    #include <FTGL/FTGLOutlineFont.h>
    #include <FTGL/FTGLPixmapFont.h>
    #include <FTGL/FTGLPolygonFont.h>
    #include <FTGL/FTGLTextureFont.h>

    typedef FTGLTextureFont FTTextureFont;
    typedef FTGLPolygonFont FTPolygonFont;
#endif

#define FONT_TYPE_POLYGON           0x0
#define FONT_TYPE_TEXTURE           0x1
#define FONT_TYPE_BITMAP            0x2
#define FONT_TYPE_PIXMAP            0x3

#define LINUX_FONT_PATH         "/usr/share/fonts"
#define SYSTEM_FONT_PATH        LINUX_FONT_PATH
#define TA3D_FONT_PATH          "fonts"

namespace TA3D
{
    class GFX;


    class Font : ObjectSync
    {
    public:
        Font();

        void init();

        float length(const String &txt);
        float height();
        void load( const String &filename, const int size, const int type);
        void destroy();
        int get_size();
        void print(float x, float y, float z, const String &text);

    private:
        friend class GFX;

        FTFont *font;
    }; // class GfxFont

    class FontManager
    {
    public:
        FontManager();
        ~FontManager();

        void destroy();

        Font *getFont(String filename, int size, int type);


    private:
        std::list<Font*>            font_list;
        UTILS::cHashTable<Font*>    font_table;
    }; // class FontManager

    extern FontManager font_manager;
} // namespace TA3D

#endif // __TA3D_GFX_FONT_H__
