
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
namespace Interfaces
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



    void GfxFont::load_gaf_font(const char *filename, const float s )
    {
        LOG_DEBUG("Loading GAF font: `" << filename << "`...");
        size = s;
        byte *data = HPIManager->PullFromHPI( filename );
        if(data)
        {
            ANIM gaf_font;
            gaf_font.load_gaf(data);
            int h = 0, mx = 0, my = 0;
            for(int i = 0 ; i < gaf_font.nb_bmp ; ++i)
            {
                if (abs( gaf_font.ofs_x[ i ] ) > 50 || abs( gaf_font.ofs_y[ i ] ) > 50 ) continue;
                if (-gaf_font.ofs_x[ i ] < mx )	mx = -gaf_font.ofs_x[ i ];
                if (-gaf_font.ofs_y[ i ] < my )	my = -gaf_font.ofs_y[ i ];
                if (gaf_font.bmp[ i ]->h > h )	h = gaf_font.bmp[ i ]->h;
            }
            my += 2;
            h -= 2;

            FONT_COLOR_DATA	*fc = ( FONT_COLOR_DATA* ) malloc( sizeof(FONT_COLOR_DATA) );
            fc->begin = 0;
            fc->end = gaf_font.nb_bmp;
            fc->bitmaps = (BITMAP**) malloc( sizeof( BITMAP* ) * gaf_font.nb_bmp );
            fc->next = NULL;
            for (int i = 0 ; i < gaf_font.nb_bmp ; ++i)
            {
                fc->bitmaps[ i ] = create_bitmap_ex( 32, gaf_font.bmp[ i ]->w, h);
                clear_to_color( fc->bitmaps[ i ], 0xFF00FF );
                if (i != 32 )		// Spaces must remain blank
                    blit( gaf_font.bmp[ i ], fc->bitmaps[ i ], 0,0, -gaf_font.ofs_x[ i ] - mx, -gaf_font.ofs_y[ i ] - my, gaf_font.bmp[ i ]->w, gaf_font.bmp[ i ]->h );
                for (int y = 0 ; y < fc->bitmaps[ i ]->h ; y++ )
                {
                    for (int x = 0 ; x < fc->bitmaps[ i ]->w ; ++x)
                    {
                        if(getpixel(fc->bitmaps[ i ], x, y ) == 0)
                            putpixel(fc->bitmaps[ i ], x, y, 0xFF00FF);
                    }
                }
            }
            gaf_font.destroy();					// Destroy the gaf data, we don't need this any more
            delete[] data;
            pAl = (FONT*) malloc( sizeof(FONT));
            pAl->data = fc;
            pAl->height = h;
            pAl->vtable = font_vtable_color;
        }
        else 
        {
            pAl = NULL;
            throw cError( "GfxFont::load_gaf_font()", "file could not be read, data = NULL.", true );
            return;
        }

        pGl = allegro_gl_convert_allegro_font_ex(pAl,AGL_FONT_TYPE_TEXTURED,-1.0f,GL_RGBA8);
        if(!pGl)
        {
            throw cError( "GfxFont::load_gaf_font()", "font could not be converted to GL font, pGl = NULL.", true );
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




} // namespace Interfaces
} // namespace TA3D
