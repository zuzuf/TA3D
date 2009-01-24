#ifndef __TA3D_GFX_FONT_H__
# define __TA3D_GFX_FONT_H__

# include "gfx.h"


namespace TA3D
{
    class GFX;


    class GfxFont
    {
    public:
        GfxFont();

        void init();

        float length(const String txt) const;
        float height() const;
        void load( const char *filename, const float s = 1.0f );
        void load_gaf_font( const char *filename, const float s = 1.0f );
        void destroy();
        void change_size(const float s);
        float get_size() const;

    private:
        friend class GFX;
    }; // class GfxFont





} // namespace TA3D

#endif // __TA3D_GFX_FONT_H__
