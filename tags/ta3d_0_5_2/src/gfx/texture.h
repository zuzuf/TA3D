#ifndef __TA3D_GfxTexture_H__
# define __TA3D_GfxTexture_H__

# include "gfx.h"



namespace TA3D
{
namespace Interfaces
{

    class GfxTexture
    {
    public:
        uint32		width;
        uint32		height;
        GLuint		tex;
        bool		destroy_tex;

        void init();
        GfxTexture();
        GfxTexture(const GLuint gltex);
        void set(const GLuint gltex);
        void draw(const float x1, const float y1);
        void draw(const float x1, const float y1, const uint32 col);

        void destroy();

    }; // class GfxTexture



}
}

#endif // __TA3D_GfxTexture_H__
