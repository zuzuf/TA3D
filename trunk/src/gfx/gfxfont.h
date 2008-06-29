#ifndef __TA3D_GFX_FONT_H__
# define __TA3D_GFX_FONT_H__

# include "gfx.h"



namespace TA3D
{
namespace Interfaces
{

    class GFX;


    class GfxFont
    {
    public:
        GfxFont();

        void init();

        float length(const String txt) const 
        { return text_length(pAl, txt.c_str()) * size; }
        
        float height() const
        { return text_height(pAl) * size; }

        void set_clear(const bool val) { clear = val; }
        void load( const char *filename, const float s = 1.0f );
        void load_gaf_font( const char *filename, const float s = 1.0f );
        void copy( FONT *fnt, const float s = 1.0f );
        void destroy();
        void change_size( const float s) { size = s;	}
        float get_size() const { return size; }

    private:
        friend class GFX;
    private:
        FONT* pAl;
        FONT* pGl;
        float size;
        bool clear;

    }; // class GfxFont





} // namespace Interfaces
} // namespace TA3D

#endif // __TA3D_GFX_FONT_H__
