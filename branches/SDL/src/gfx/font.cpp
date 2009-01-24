
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



    GfxFont::GfxFont()
    {}

    void GfxFont::init()
    {
    }

    void GfxFont::load(const char* filename, const float s)
    {
    }



    void GfxFont::load_gaf_font(const char *filename, const float s)
    {
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
            throw cError( "GfxFont::load_gafFont()", "file could not be read, data = NULL.", true );
            return;
        }

#warning FIXME: fonts not implemented yet!
    }


    void GfxFont::destroy()
    {
        init();
    }

    float GfxFont::length(const String txt) const
    {
        return 0.0f;
    }

    float GfxFont::height() const
    {
        return 0.0f;
    }

    void GfxFont::change_size(const float s)
    {
    }

    float GfxFont::get_size() const
    {
        return 0.0f;
    }
} // namespace TA3D
