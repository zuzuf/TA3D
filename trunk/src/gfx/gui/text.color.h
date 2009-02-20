#ifndef __TA3D_TEXT_COLOR_H__
#define __TA3D_TEXT_COLOR_H__

#include "../../misc/tdf.h"
#include "../../misc/string.h"
#include "../font.h"

class TEXT_COLOR
{
public:
    TEXT_COLOR();

    void load(TA3D::TDFParser& parser, const String &prefix, float scale);

    void print(Font *font, float x, float y, const String &text);

    void print(Font *font, float x, float y, uint32 col, const String &text);

public:
    uint32      font_color;
    uint32      shadow_color;
    bool        shadow;
    float       shadow_dx;
    float       shadow_dy;
};

#endif
