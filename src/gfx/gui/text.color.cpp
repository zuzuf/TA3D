#include "../../stdafx.h"
#include "../../TA3D_NameSpace.h"
#include "text.color.h"

TEXT_COLOR::TEXT_COLOR()
{
    font_color = makeacol(0xFF,0xFF,0xFF,0xFF);
    shadow_color = makeacol(0,0,0,0xFF);
    shadow = false;
    shadow_dx = 1.0f;
    shadow_dy = 1.0f;
}

void TEXT_COLOR::load(TDFParser& parser, const String &prefix, float scale)
{
    font_color = parser.pullAsColor( prefix + "font_color", makeacol(0xFF,0xFF,0xFF,0xFF) );
    shadow_color = parser.pullAsColor( prefix + "shadow_color", makeacol(0,0,0,0xFF) );
    shadow = parser.pullAsBool( prefix + "shadow", false );
    shadow_dx = parser.pullAsFloat( prefix + "shadow_dx", 1.0f ) * scale;
    shadow_dy = parser.pullAsFloat( prefix + "shadow_dy", 1.0f ) * scale;
}

void TEXT_COLOR::print(Font *font, float x, float y, const String &text)
{
    if (font == NULL)
    {
        LOG_WARNING(LOG_PREFIX_GFX << "font == NULL !! cannot render text");
        return;
    }

    if (shadow)
        gfx->print(font, x + shadow_dx, y + shadow_dy, 0.0f, shadow_color, text);

    gfx->print(font, x, y, 0.0f, font_color, text);
}

void TEXT_COLOR::print(Font *font, float x, float y, uint32 col, const String &text)
{
    if (font == NULL)
    {
        LOG_WARNING(LOG_PREFIX_GFX << "font == NULL !! cannot render text");
        return;
    }

    if (shadow)
        gfx->print(font, x + shadow_dx, y + shadow_dy, 0.0f, shadow_color, text);

    gfx->print(font, x, y, 0.0f, col, text);
}
