
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
namespace Interfaces
{

    GfxTexture::GfxTexture()
        :width(0), height(0), tex(0), destroy_tex(false)
    {}


    void GfxTexture::init()
    {
        width = 0;
        height = 0;
        tex = 0;
        destroy_tex = false;
    }


    void GfxTexture::draw( const float x1, const float y1 )
    {
        gfx->drawtexture( tex, x1, y1, x1+width-1, y1+height-1 );
    }

    void GfxTexture::draw( const float x1, const float y1, const uint32 col )
    {
        gfx->drawtexture(tex, x1, y1, x1+width-1, y1+height-1, col);
    }

    GfxTexture::GfxTexture( const GLuint gltex )
    {
        destroy_tex = true;
        set( gltex );
    }

    void GfxTexture::set( const GLuint gltex )
    {
        tex = gltex;
        width = gfx->texture_width( tex );
        height = gfx->texture_height( tex );
    }

    void GfxTexture::destroy()
    {
        width = 0;
        height = 0;
        if(destroy_tex)
        {
            gfx->destroy_texture(tex);
            destroy_tex = false;
        }
        else
            tex = 0;
    }




}
}
