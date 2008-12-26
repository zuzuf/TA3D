
#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "glfunc.h"
#include "../gui.h"
#include "../gaf.h"
#include "gfx.h"
#include "../misc/paths.h"
#include "../logs/logs.h"
#include <allegro/internal/aintern.h>




namespace TA3D
{



    GfxFont::GfxFont()
        : pAl(NULL), pGl(NULL), size(1.0f), clear(false)
    {}

    void GfxFont::init()
    {
        pAl = NULL;
        pGl = NULL;
        size = 1.0f;
        clear = false;
    }



    void GfxFont::load(const char* filename, const float s)
    {
        size = s;
        pAl = load_font( filename, NULL, NULL );
        if (pAl == NULL ) {
            throw cError( "GfxFont::load()", "font could not be loaded, pAl = NULL.", true );
            return;
        }
        pGl = allegro_gl_convert_allegro_font_ex(pAl, AGL_FONT_TYPE_TEXTURED, -1.0f, GL_RGBA8);
        if(NULL == pGl)
        {
            throw cError( "GfxFont::load()", "font could not be converted to GL font, pGl = NULL.", true );
        }
    }



    void GfxFont::load_gaf_font(const char *filename, const float s)
    {
        LOG_DEBUG("Loading GAF font: `" << filename << "`...");
        size = s;
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

            FONT_COLOR_DATA	*fc = (FONT_COLOR_DATA*) malloc(sizeof(FONT_COLOR_DATA));
            fc->begin = 0;
            fc->end = gafFont.nb_bmp;
            fc->bitmaps = (BITMAP**) malloc(sizeof(BITMAP*) * gafFont.nb_bmp);
            fc->next = NULL;
            for (int i = 0 ; i < gafFont.nb_bmp ; ++i)
            {
                fc->bitmaps[ i ] = create_bitmap_ex( 32, gafFont.bmp[ i ]->w, h);
                clear_to_color( fc->bitmaps[ i ], 0xFF00FF );
                if (i != 32 )		// Spaces must remain blank
                    blit( gafFont.bmp[ i ], fc->bitmaps[ i ], 0,0, -gafFont.ofs_x[ i ] - mx, -gafFont.ofs_y[ i ] - my, gafFont.bmp[ i ]->w, gafFont.bmp[ i ]->h );
                for (int y = 0 ; y < fc->bitmaps[ i ]->h ; y++ )
                {
                    for (int x = 0 ; x < fc->bitmaps[ i ]->w ; ++x)
                    {
                        if(getpixel(fc->bitmaps[ i ], x, y ) == 0)
                            putpixel(fc->bitmaps[ i ], x, y, 0xFF00FF);
                    }
                }
            }
            delete[] data;
            pAl = (FONT*) malloc( sizeof(FONT));
            pAl->data = fc;
            pAl->height = h;
            pAl->vtable = font_vtable_color;
        }
        else 
        {
            pAl = NULL;
            throw cError( "GfxFont::load_gafFont()", "file could not be read, data = NULL.", true );
            return;
        }

        pGl = allegro_gl_convert_allegro_font_ex(pAl,AGL_FONT_TYPE_TEXTURED,-1.0f,GL_RGBA8);
        if(!pGl)
        {
            throw cError( "GfxFont::load_gafFont()", "font could not be converted to GL font, pGl = NULL.", true );
        }
    }


    void GfxFont::destroy()
    {
        if (pAl)
            destroy_font(pAl);
        if (pGl)
            allegro_gl_destroy_font(pGl);
        init();
    }

    void GfxFont::copy( FONT *fnt, const float s)
    {
        size = s;
        pAl = extract_font_range(font, -1, -1);
        if (NULL == pAl)
        {
            throw cError( "GfxFont::copy()", "font could not be copied, pAl = NULL.", true );
            return;
        }
        pGl = allegro_gl_convert_allegro_font_ex(pAl,AGL_FONT_TYPE_TEXTURED,-1.0f,GL_RGBA8);
        if (NULL == pGl)
        {
            throw cError( "GfxFont::copy()", "font could not be converted to GL font, pGl = NULL.", true );
        }
    }


    BITMAP* GFX::LoadMaskedTextureToBmp(const String& file, const String& filealpha)
    {
        // Load the texture (32Bits)
        set_color_depth(32);
        BITMAP* bmp = load_bitmap(file.c_str(), NULL);
        LOG_ASSERT(bmp != NULL);

        // Load the mask
        set_color_depth(8);
        BITMAP* alpha = load_bitmap(filealpha.c_str(), NULL);
        LOG_ASSERT(alpha != NULL);

        // Apply the mask, pixel by pixel
        set_color_depth(32);
        for (int y = 0; y < bmp->h; ++y)
        {
            for (int x = 0; x < bmp->w; ++x)
                bmp->line[y][(x << 2) + 3] = alpha->line[y][x];
        }

        destroy_bitmap(alpha);
        return bmp;
    }



} // namespace TA3D
