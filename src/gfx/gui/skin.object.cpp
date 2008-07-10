
#include "skin.object.h"
#include "../../misc/paths.h"
#include "../../TA3D_NameSpace.h"



namespace TA3D
{

    SKIN_OBJECT::SKIN_OBJECT()
        :tex(0), x1(0), y1(0), x2(0), y2(0),
        t_x1(0.0f), t_y1(0.0f), t_x2(0.0f), t_y2(0.0f),
        w(0), h(h),
        sw(0.0f), sh(0.0f)
    {}


    SKIN_OBJECT::~SKIN_OBJECT()
    {
        gfx->destroy_texture(tex);
    }

    void SKIN_OBJECT::init()
    {
        tex = 0;
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = 0;
        t_x1 = 0.0f;
        t_y1 = 0.0f;
        t_x2 = 0.0f;
        t_y2 = 0.0f;
        w = 0;
        h = 0;
        sw = 0.0f;
        sh = 0.0f;
    }

    void SKIN_OBJECT::destroy()
    {
        gfx->destroy_texture(tex);
        init();
    }




    void SKIN_OBJECT::load(const String& filename, const String& prefix, UTILS::cTAFileParser* parser, float borderSize)
    {
        if (TA3D::Paths::Exists(filename))
        {
            tex = gfx->load_texture(filename, FILTER_LINEAR, &w, &h);

            x1 = parser->PullAsInt(prefix + "x1");
            y1 = parser->PullAsInt(prefix + "y1");
            x2 = parser->PullAsInt(prefix + "x2");
            y2 = parser->PullAsInt(prefix + "y2");

            t_x1 = w ? ((float)x1) / w : 0.0f;
            t_x2 = w ? ((float)x2) / w : 0.0f;
            t_y1 = h ? ((float)y1) / h : 0.0f;
            t_y2 = h ? ((float)y2) / h : 0.0f;

            x2 -= w;
            y2 -= h;

            borderSize *= parser->PullAsFloat(prefix + "scale", 1.0f);		// Allow scaling the widgets

            x1 *= borderSize;
            y1 *= borderSize;
            x2 *= borderSize;
            y2 *= borderSize;
            sw = w * borderSize;
            sh = h * borderSize;
        }
    }



    void SKIN_OBJECT::draw(const float X1, const float Y1, const float X2, const float Y2, const bool bkg) const
    {
        gfx->drawtexture(tex, X1, Y1, X1 + x1, Y1 + y1, 0.0f, 0.0f, t_x1, t_y1);
        gfx->drawtexture(tex, X1 + x1, Y1, X2 + x2, Y1 + y1, t_x1, 0.0f, t_x2, t_y1);
        gfx->drawtexture(tex, X2 + x2, Y1, X2, Y1 + y1, t_x2, 0.0f, 1.0f, t_y1);

        gfx->drawtexture(tex, X1, Y1 + y1, X1 + x1, Y2 + y2, 0.0f, t_y1, t_x1, t_y2);
        gfx->drawtexture(tex, X2 + x2, Y1 + y1, X2, Y2 + y2, t_x2, t_y1, 1.0f, t_y2);

        gfx->drawtexture(tex, X1, Y2 + y2, X1 + x1, Y2, 0.0f, t_y2, t_x1, 1.0f);
        gfx->drawtexture(tex, X1 + x1, Y2 + y2, X2 + x2, Y2, t_x1, t_y2, t_x2, 1.0f);
        gfx->drawtexture(tex, X2 + x2, Y2 + y2, X2, Y2, t_x2, t_y2, 1.0f, 1.0f);

        if (bkg)
            gfx->drawtexture(tex, X1 + x1, Y1 + y1, X2 + x2, Y2 + y2, t_x1, t_y1, t_x2, t_y2);
    }



} // namespace TA3D
