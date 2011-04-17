/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D_GFX_TOOLKIT_H__
# define __TA3D_GFX_TOOLKIT_H__

# include <stdafx.h>
# include <misc/string.h>
# include <sdl.h>
# include <logs/logs.h>


# define makeacol(r,g,b,a)   (((uint32)(a)<<24) | ((uint32)(b)<<16) | ((uint32)(g)<<8) | (uint32)(r))
# define makecol24(r,g,b)   (((uint32)0xFF000000) | ((uint32)(b)<<16) | ((uint32)(g)<<8) | (uint32)(r))
# define makecol(r,g,b)   makecol24(r,g,b)
# define makeacol32(r,g,b,a)   makeacol(r,g,b,a)

namespace TA3D
{
	inline byte &SurfaceByte(SDL_Surface *img, int x, int y)
	{
		LOG_ASSERT(x >= 0 && y >= 0 && x < img->pitch && y < img->h);
		return ((byte*)(img->pixels))[y * img->pitch + x];
	}

	inline uint32 &SurfaceInt(SDL_Surface *img, int x, int y)
	{
		LOG_ASSERT(x >= 0 && y >= 0 && (x << 2) < img->pitch && y < img->h);
		return *(uint32*)((byte*)(img->pixels) + (y * img->pitch) + (x << 2));
	}

	inline uint16 &SurfaceShort(SDL_Surface *img, int x, int y)
	{
		LOG_ASSERT(x >= 0 && y >= 0 && (x << 1) < img->pitch && y < img->h);
		return *(uint16*)((byte*)(img->pixels) + (y * img->pitch) + (x << 1));
	}

	template<typename T>
			inline T &SurfaceType(SDL_Surface *img, int x, int y)
	{
		LOG_ASSERT(x >= 0 && y >= 0 && x * sizeof(T) < img->pitch && y < img->h);
		return *(T*)((byte*)(img->pixels) + (y * img->pitch) + (x * sizeof(T)));
	}
}

# define geta32(x) (((x)>>24) & 0xFF)
# define getb32(x) (((x)>>16) & 0xFF)
# define getg32(x) (((x)>>8) & 0xFF)
# define getr32(x) ((x) & 0xFF)

# define getr(x) getr32(x)
# define getg(x) getg32(x)
# define getb(x) getb32(x)
# define geta(x) geta32(x)



namespace TA3D
{

	void masked_blit(SDL_Surface *in, SDL_Surface *out, int x0, int y0, int x1, int y1, int w, int h);
	void blit(SDL_Surface *in, SDL_Surface *out, int x0, int y0, int x1, int y1, int w, int h);
	void stretch_blit( SDL_Surface *in, SDL_Surface *out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 );
	void stretch_blit_smooth( SDL_Surface *in, SDL_Surface *out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 );

	SDL_Surface *shrink(const SDL_Surface *in, const int w, const int h);

	void putpixel(SDL_Surface *bmp, int x, int y, uint32 col);
	uint32 getpixel(SDL_Surface *bmp, int x, int y);

	void circlefill(SDL_Surface *bmp, int x, int y, int r, uint32 col);

	void rectfill(SDL_Surface *bmp, int x0, int y0, int x1, int y1, uint32 col);

	SDL_Surface *convert_format_copy(SDL_Surface *bmp);
	SDL_Surface *convert_format_24_copy(SDL_Surface *bmp);
	SDL_Surface *convert_format_16_copy(SDL_Surface *bmp);

	SDL_Surface *convert_format(SDL_Surface *bmp);
	SDL_Surface *convert_format_24(SDL_Surface *bmp);
	SDL_Surface *convert_format_16(SDL_Surface *bmp);

	void disable_TA_palette();
	void enable_TA_palette();

	void save_bitmap(const String &filename, SDL_Surface* bmp);

    void vflip_bitmap(SDL_Surface* bmp);
    void hflip_bitmap(SDL_Surface* bmp);

	SDL_Surface *LoadTex(const String &filename);
	void SaveTex(SDL_Surface *bmp, const String &filename);

	void save_TGA(const String &filename, SDL_Surface* bmp, bool compress = true);
}


#endif
