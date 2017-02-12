
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/renderapi.h>
#include <SDL/sgui/font.h>
#include <cmath>

#define _X0(surface)	((surface)->clip_rect.x)
#define _Y0(surface)	((surface)->clip_rect.y)
#define _X1(surface)	((surface)->clip_rect.x + (surface)->clip_rect.w)
#define _Y1(surface)	((surface)->clip_rect.y + (surface)->clip_rect.h)

#define RENDERALL()\
switch(dst->format->BytesPerPixel)\
{\
case 1:\
 {\
  RENDER(8);\
  break;\
}\
case 2:\
{\
 RENDER(16);\
 break;\
}\
case 3:\
{\
 RENDER(24);\
 break;\
}\
case 4:\
{\
 RENDER(32);\
 break;\
}\
}

using namespace std;

namespace Gui
{

	uint8 colormap[32768U];
	uint32 black;
	uint32 darkgrey;
	uint32 grey;
	uint32 lightgrey;
	uint32 verylightgrey;
	uint32 white;
	uint32 blue;
	uint32 red;
	uint32 green;
	uint32 yellow;
	uint32 darkblue;
	uint32 darkgreen;
	uint32 darkred;
	uint32 darkyellow;
	uint32 lightblue;
	uint32 lightgreen;
	uint32 lightred;
	uint32 lightyellow;

	SDL_Cursor *cursor_arrow = NULL;
	SDL_Cursor *cursor_edit = NULL;

	void updateGUIColors()
	{
		SDL_Surface *screen = SDL_GetVideoSurface();
		const SDL_PixelFormat *format = screen->format;
		if (format->BytesPerPixel == 1)
		{
			static SDL_Color pal[256];
			static bool bInit = true;
			if (bInit)
			{
				for(int i = 0 ; i < 32 ; ++i)
				{
					const int j = i * 255 / 31;
					pal[i].r = j;
					pal[i].g = j;
					pal[i].b = j;
					pal[i + 32].r = j;
					pal[i + 32].g = 0;
					pal[i + 32].b = 0;
					pal[i + 64].r = 0;
					pal[i + 64].g = j;
					pal[i + 64].b = 0;
					pal[i + 96].r = 0;
					pal[i + 96].g = 0;
					pal[i + 96].b = j;
					pal[i + 128].r = j;
					pal[i + 128].g = j;
					pal[i + 128].b = 0;
					pal[i + 160].r = j;
					pal[i + 160].g = 0;
					pal[i + 160].b = j;
					pal[i + 192].r = 0;
					pal[i + 192].g = j;
					pal[i + 192].b = j;
				}
				for(int i = 0 ; i < 8 ; ++i)
				{
					const int j = i * 255 / 7;
					pal[i + 224].r = j;
					pal[i + 224].g = j >> 1;
					pal[i + 224].b = j >> 1;
					pal[i + 232].r = j >> 1;
					pal[i + 232].g = j;
					pal[i + 232].b = j >> 1;
					pal[i + 240].r = j >> 1;
					pal[i + 240].g = j >> 1;
					pal[i + 240].b = j;
					pal[i + 248].r = j;
					pal[i + 248].g = j;
					pal[i + 248].b = j >> 1;
				}
			}
			SDL_SetColors(screen, pal, 0, 256);
			if (bInit)
			{
				for(int r = 0 ; r < 32 ; ++r)
					for(int g = 0 ; g < 32 ; ++g)
						for(int b = 0 ; b < 32 ; ++b)
							colormap[(r << 10) | (g << 5) | b] = SDL_MapRGB(format, r << 3, g << 3, b << 3);
				bInit = false;
			}
		}
		black = mapRGBA(format, 0x0, 0x0, 0x0, 0x0);
		darkgrey = mapRGBA(format, 0x3F, 0x3F, 0x3F, 0x0);
		grey = mapRGBA(format, 0x7F, 0x7F, 0x7F, 0x0);
		lightgrey = mapRGBA(format, 0xCF, 0xCF, 0xCF, 0x0);
		verylightgrey = mapRGBA(format, 0xDF, 0xDF, 0xDF, 0x0);
		white = mapRGBA(format, 0xFF, 0xFF, 0xFF, 0x0);
		blue = mapRGBA(format, 0x0, 0x0, 0xFF, 0x0);
		red = mapRGBA(format, 0xFF, 0x0, 0x0, 0x0);
		green = mapRGBA(format, 0x0, 0xFF, 0x0, 0x0);
		yellow = mapRGBA(format, 0xFF, 0xFF, 0x0, 0x0);
		darkblue = mapRGBA(format, 0x0, 0x0, 0x7F, 0x0);
		darkred = mapRGBA(format, 0x7F, 0x0, 0x0, 0x0);
		darkgreen = mapRGBA(format, 0x0, 0x7F, 0x0, 0x0);
		darkyellow = mapRGBA(format, 0x7F, 0x7F, 0x0, 0x0);
		lightblue = mapRGBA(format, 0x7F, 0x7F, 0xFF, 0x0);
		lightred = mapRGBA(format, 0xFF, 0x7F, 0x7F, 0x0);
		lightgreen = mapRGBA(format, 0x7F, 0xFF, 0x7F, 0x0);
		lightyellow = mapRGBA(format, 0xFF, 0xFF, 0x7F, 0x0);
	}

	SDL_Surface *createNativeSurface(int w, int h)
	{
		SDL_PixelFormat *format = SDL_GetVideoSurface()->format;
		SDL_Surface *buf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);
		if (format->palette)
			SDL_SetPalette(buf, SDL_LOGPAL, format->palette->colors, 0, format->palette->ncolors);
		return buf;
	}

	SDL_Surface *createSubSurface(SDL_Surface *src, int x, int y, int w, int h)
	{
		SDL_PixelFormat *format = src->format;
		w = min(src->w - x, w);
		h = min(src->h - y, h);
		SDL_Surface *buf = SDL_CreateRGBSurfaceFrom((char*)src->pixels + x * format->BytesPerPixel + y * src->pitch,
													w, h,
													format->BitsPerPixel, src->pitch,
													format->Rmask, format->Gmask, format->Bmask, format->Amask);
		buf->clip_rect = src->clip_rect;
		const int x0 = buf->clip_rect.x;
		const int y0 = buf->clip_rect.y;
		const int x1 = x0 + buf->clip_rect.w;
		const int y1 = y0 + buf->clip_rect.h;
		buf->clip_rect.x = max<int>(x, buf->clip_rect.x);
		buf->clip_rect.y = max<int>(y, buf->clip_rect.y);
		buf->clip_rect.w = min<int>(x1 - buf->clip_rect.x, x + w - buf->clip_rect.x);
		buf->clip_rect.h = min<int>(y1 - buf->clip_rect.y, y + h - buf->clip_rect.y);
		buf->clip_rect.x -= x;
		buf->clip_rect.y -= y;

		if (format->palette)
			SDL_SetPalette(buf, SDL_LOGPAL, format->palette->colors, 0, format->palette->ncolors);
		return buf;
	}

	SDL_Surface SubSurface(SDL_Surface *src, int x, int y, int w, int h)
	{
		SDL_Surface sub = *src;
		sub.pixels = (byte*)sub.pixels + y * sub.pitch + x * sub.format->BytesPerPixel;
		sub.w = min<int>(w, sub.w - x);
		sub.h = min<int>(h, sub.h - y);
		const int x0 = sub.clip_rect.x;
		const int y0 = sub.clip_rect.y;
		const int x1 = x0 + sub.clip_rect.w;
		const int y1 = y0 + sub.clip_rect.h;
		sub.clip_rect.x = max<int>(x, sub.clip_rect.x);
		sub.clip_rect.y = max<int>(y, sub.clip_rect.y);
		sub.clip_rect.w = min<int>(x1 - sub.clip_rect.x, x + w - sub.clip_rect.x);
		sub.clip_rect.h = min<int>(y1 - sub.clip_rect.y, y + h - sub.clip_rect.y);
		sub.clip_rect.x -= x;
		sub.clip_rect.y -= y;

		return sub;
	}

	void blit(SDL_Surface *src, SDL_Surface *dst, int x, int y)
	{
		if (dst->w <= 0 || dst->h <= 0 || src->w <= 0 || src->h <= 0)
			return;
		SDL_Rect rsrc, rdst;
		rsrc.x = 0;
		rsrc.y = 0;
		rsrc.w = src->w;
		rsrc.h = src->h;

		rdst.x = x;
		rdst.y = y;
		rdst.w = rsrc.w;
		rdst.h = rsrc.h;

		SDL_BlitSurface(src, &rsrc, dst, &rdst);
	}

	void blit(SDL_Surface *src, SDL_Surface *dst, int x0, int y0, int x1, int y1, int w, int h)
	{
		if (w <= 0 || h <= 0 || dst->w <= 0 || dst->h <= 0 || src->w <= 0 || src->h <= 0)
			return;
		SDL_Rect rsrc, rdst;
		rsrc.x = x0;
		rsrc.y = y0;
		rsrc.w = w;
		rsrc.h = h;

		rdst.x = x1;
		rdst.y = y1;
		rdst.w = w;
		rdst.h = h;

		SDL_BlitSurface(src, &rsrc, dst, &rdst);
	}

	inline bool memdiff(const void *s1, const void *s2, const size_t l)
	{
		const size_t *pl1 = (const size_t*)s1;
		const size_t *pl2 = (const size_t*)s2;
		const size_t *lend = pl1 + (l / sizeof(size_t));
		while((pl1 != lend) & (*pl1 == *pl2))
			++pl1, ++pl2;
		if (pl1 != lend)
			return true;
		const byte *bend = (const byte*)s1 + l;
		const byte *pb1 = (const byte*)pl1;
		const byte *pb2 = (const byte*)pl2;
		for( ; pb1 != bend ; ++pb1, ++pb2)
			if (*pb1 != *pb2)
				return true;
		return false;
	}

	bool compareSurfaces(SDL_Surface *src, SDL_Surface *dst, int x, int y)
	{
		if (src->format->BitsPerPixel != dst->format->BitsPerPixel || dst->w <= 0 || dst->h <= 0 || src->w <= 0 || src->h <= 0)
			return true;
		if (x < _X0(dst)
			|| y < _Y0(dst)
			|| src->w + x > _X1(dst)
			|| src->h + y > _Y1(dst))
			return true;
		switch(src->format->BytesPerPixel)
		{
		case 1:
			for(int dy = 0 ; dy < src->h ; ++dy)
			{
				const byte *p = (byte*)src->pixels + dy * src->pitch;
				const byte *q = (byte*)dst->pixels + (dy + y) * dst->pitch + x;
				if (memdiff(p, q, src->w))
					return true;
			}
			break;
		case 2:
			x *= 2;
			for(int dy = 0 ; dy < src->h ; ++dy)
			{
				const uint16 *p = (uint16*)((byte*)src->pixels + dy * src->pitch);
				const uint16 *q = (uint16*)((byte*)dst->pixels + (dy + y) * dst->pitch + x);
				if (memdiff(p, q, src->w << 1))
					return true;
			}
			break;
		case 3:
			for(int dy = 0 ; dy < src->h ; ++dy)
			{
				const byte *p = (byte*)src->pixels + dy * src->pitch;
				const byte *q = (byte*)dst->pixels + (dy + y) * dst->pitch + x * 3;
				if (memdiff(p, q, src->w * 3))
					return true;
			}
			break;
		case 4:
			x *= 4;
			for(int dy = 0 ; dy < src->h ; ++dy)
			{
				const uint32 *p = (uint32*)((byte*)src->pixels + dy * src->pitch);
				const uint32 *q = (uint32*)((byte*)dst->pixels + (dy + y) * dst->pitch + x);
				if (memdiff(p, q, src->w << 2))
					return true;
			}
			break;
		};

		return false;
	}

	void line(SDL_Surface *dst, int x0, int y0, int x1, int y1, uint32 col)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
#undef RENDER
#define RENDER(BPP)\
		if (abs(x0 - x1) > abs(y0 - y1))\
		{\
			if (x0 > x1)\
			{\
				x0 ^= x1;	x1 ^= x0;	x0 ^= x1;\
				y0 ^= y1;	y1 ^= y0;	y0 ^= y1;\
			}\
			const int dx = x1 - x0;\
			const int hdx = dx / 2;\
			for(int x = x0 ; x <= x1 ; ++x)\
			{\
				if (x < _X0(dst) || x >= _X1(dst))\
					continue;\
				const int y = y0 + ((y1 - y0) * (x - x0) + hdx) / dx;\
				if (y < _Y0(dst) || y >= _Y1(dst))\
					continue;\
				Font::setPixel##BPP(dst, x, y, col);\
			}\
		}\
		else if (y0 != y1)\
		{\
			if (y0 > y1)\
			{\
				x0 ^= x1;	x1 ^= x0;	x0 ^= x1;\
				y0 ^= y1;	y1 ^= y0;	y0 ^= y1;\
			}\
			const int dy = y1 - y0;\
			const int hdy = dy / 2;\
			for(int y = y0 ; y <= y1 ; ++y)\
			{\
				if (y < _Y0(dst) || y >= _Y1(dst))\
					continue;\
				const int x = x0 + ((x1 - x0) * (y - y0) + hdy) / dy;\
				if (x < _X0(dst) || x >= _X1(dst))\
					continue;\
				Font::setPixel##BPP(dst, x, y, col);\
			}\
		}\
		else\
		{\
			if (y0 < _Y0(dst) || y0 >= _Y1(dst) || x0 < _X0(dst) || x0 >= _X1(dst))\
				return;\
			Font::setPixel##BPP(dst, x0, y0, col);\
		}

		RENDERALL();
	}

	void vline(SDL_Surface *dst, int x, int y0, int y1, uint32 col)
	{
		if (x < _X0(dst) || x >= _X1(dst) || dst->w <= 0 || dst->h <= 0)
			return;
		y0 = max<int>(_Y0(dst), y0);
		y1 = min<int>(_Y1(dst) - 1, y1);
		if (y0 < _Y1(dst) && y1 >= _Y0(dst) && y0 <= y1)
		{
			byte *p = (byte*)dst->pixels + y0 * dst->pitch + x * dst->format->BytesPerPixel;
			switch(dst->format->BytesPerPixel)
			{
			case 1:
				{
					const size_t step = dst->pitch;
					byte* const end = p + (y1 - y0 + 1) * step;
					for( ; p != end ; p += step)
						*p = col;
				}
				break;
			case 2:
				{
					uint16 *_p = (uint16*)p;
					const size_t step = dst->pitch >> 1;
					uint16* const end = _p + (y1 - y0 + 1) * step;
					for( ; _p != end ; _p += step)
						*_p = col;
				}
				break;
			case 3:
				{
					const byte r = col >> 16;
					const byte g = col >> 8;
					const byte b = col;
					const size_t step = dst->pitch;
					for(byte *end = p + (y1 - y0 + 1) * step ; p != end ; p += step)
					{
						p[0] = r;
						p[1] = g;
						p[2] = b;
					}
				}
				break;
			case 4:
				{
					const size_t step = dst->pitch >> 2;
					for(uint32 *_p = (uint32*)p, *end = (uint32*)(p + (y1 - y0 + 1) * dst->pitch) ; _p != end ; _p += step)
						*_p = col;
				}
				break;
			}
		}
	}

	void hline(SDL_Surface *dst, int y, int x0, int x1, uint32 col)
	{
		if (y < _Y0(dst) || y >= _Y1(dst) || x1 < x0 || dst->w <= 0 || dst->h <= 0)
			return;
		x0 = max<int>(_X0(dst), x0);
		x1 = min<int>(_X1(dst) - 1, x1);
		if (x0 < _X1(dst) && x1 >= _X0(dst) && x0 <= x1)
		{
			byte *p = (byte*)dst->pixels + y * dst->pitch + x0 * dst->format->BytesPerPixel;
			switch(dst->format->BytesPerPixel)
			{
			case 1:
				memset(p, col, x1 - x0 + 1);
				return;
			case 2:
				{
					uint32 *_pl = (uint32*)p;
					uint32* const lend = _pl + (x1 - x0 + 1 >> 1);
					col = (col & 0xFFFFU) | (col << 16);
					for( ; _pl != lend ; ++_pl)
						*_pl = col;
					uint16 *_p = (uint16*)_pl;
					uint16* const end = (uint16*)p + x1 - x0 + 1;
					for( ; _p != end ; ++_p)
						*_p = col;
				}
				break;
			case 3:
				{
					const byte r = col >> 16;
					const byte g = col >> 8;
					const byte b = col;
					for(byte *end = p + (x1 - x0 + 1) * 3 ; p != end ; ++p)
					{
						*p = r; ++p;
						*p = g; ++p;
						*p = b;
					}
				}
				break;
			case 4:
				for(uint32 *_p = (uint32*)p, *end = (uint32*)(p + (x1 - x0 + 1) * 4) ; _p != end ; ++_p)
					*_p = col;
				break;
			}
		}
	}

	void box(SDL_Surface *dst, int x0, int y0, int x1, int y1, uint32 col)
	{
		hline(dst, y0, x0, x1, col);
		hline(dst, y1, x0, x1, col);
		vline(dst, x0, y0, y1, col);
		vline(dst, x1, y0, y1, col);
	}

	void fillbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, uint32 col)
	{
		if (x0 > x1 || y0 > y1 || dst->w <= 0 || dst->h <= 0)
			return;
		SDL_Rect rect;
		rect.x = x0;
		rect.y = y0;
		rect.w = x1 - x0 + 1;
		rect.h = y1 - y0 + 1;
		SDL_FillRect(dst, &rect, col);
	}

	void roundedbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, int r, uint32 col)
	{
		hline(dst, y0, x0 + r, x1 - r, col);
		hline(dst, y1, x0 + r, x1 - r, col);
		vline(dst, x0, y0 + r, y1 - r, col);
		vline(dst, x1, y0 + r, y1 - r, col);
		arc(dst, x0 + r, y0 + r, r, 180, 270, col);
		arc(dst, x1 - r, y0 + r, r, 270, 360, col);
		arc(dst, x0 + r, y1 - r, r, 90, 180, col);
		arc(dst, x1 - r, y1 - r, r, 0, 90, col);
	}

	void fillroundedbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, int r, uint32 col)
	{
		fillbox(dst, x0, y0 + r, x1, y1 - r, col);
		fillbox(dst, x0 + r, y0, x1 - r, y0 + r, col);
		fillbox(dst, x0 + r, y1 - r, x1 - r, y1, col);
		fillcircle(dst, x0 + r, y0 + r, r, col);
		fillcircle(dst, x1 - r, y0 + r, r, col);
		fillcircle(dst, x0 + r, y1 - r, r, col);
		fillcircle(dst, x1 - r, y1 - r, r, col);
	}

	void circle(SDL_Surface *dst, int x, int y, int r, uint32 col)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
		const int r2 = r * r;
		const int hr = int(r * M_SQRT2 * 0.5f + 0.5f);
		for(int _r = 0 ; _r <= hr ; ++_r)
		{
			const int dx = int(sqrt(float(r2 - _r * _r)) + 0.5f);
			putpixel(dst, x - dx, y - _r, col);
			putpixel(dst, x + dx, y - _r, col);
			putpixel(dst, x - dx, y + _r, col);
			putpixel(dst, x + dx, y + _r, col);
			putpixel(dst, x - _r, y - dx, col);
			putpixel(dst, x + _r, y - dx, col);
			putpixel(dst, x - _r, y + dx, col);
			putpixel(dst, x + _r, y + dx, col);
		}
	}

	void fillcircle(SDL_Surface *dst, int x, int y, int r, uint32 col)
	{
		const int r2 = r * r;
		const int hr = int(r * M_SQRT2 * 0.5f + 0.5f);
		for(int _r = 0 ; _r <= hr ; ++_r)
		{
			const int dx = int(sqrt(float(r2 - _r * _r)) + 0.5f);
			hline(dst, y - _r, x - dx, x + dx, col);
			hline(dst, y + _r, x - dx, x + dx, col);
			hline(dst, y - dx, x - _r, x + _r, col);
			hline(dst, y + dx, x - _r, x + _r, col);
		}
	}

	void gradientcircle(SDL_Surface *dst, int x, int y, int radius, float dx, float dy, uint32 col, uint32 dcol)
	{
		uint32 r, g, b;
		uint32 dr, dg, db;
		getRGB(col, dst->format, r, g, b);
		getRGB(dcol, dst->format, dr, dg, db);

		const int r2 = radius * radius;
		const int hr = int(radius * M_SQRT2 * 0.5f + 0.5f);
		const int idx = int(0x1000 * dx);
		const int idy = int(0x1000 * dy);
		for(int _r = 0 ; _r <= hr ; ++_r)
		{
			const int _dx = int(sqrt(float(r2 - _r * _r)) + 0.5f);
#undef DRAW_LINE
#define DRAW_LINE()\
			for(int _x = _x0 ; _x <= _x1 ; ++_x)\
			{\
				const int d = (x - _x) * idx + (_y - y) * idy;\
				const uint32 nr = std::min<uint32>(r + d * dr, 0xFFU);\
				const uint32 ng = std::min<uint32>(g + d * dg, 0xFFU);\
				const uint32 nb = std::min<uint32>(b + d * db, 0xFFU);\
				putpixel(dst, _x, _y, mapRGB(dst->format, nr, ng, nb));\
			}
			int _y = y - _r, _x0 = x - _dx, _x1 = x + _dx;
			DRAW_LINE();
			_y = y + _r;
			DRAW_LINE();
			_y = y - _dx, _x0 = x - _r, _x1 = x + _r;
			DRAW_LINE();
			_y = y + _dx;
			DRAW_LINE();
		}
	}

	void arc(SDL_Surface *dst, int x, int y, int r, int start, int end, uint32 col)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
		const int r2 = r * r;
		const int hr = int(r * M_SQRT2 * 0.5f + 0.5f);
		const float sx = sin(start * M_PI / 180.0);
		const float sy = -cos(start * M_PI / 180.0);
		const float ex = -sin(end * M_PI / 180.0);
		const float ey = cos(end * M_PI / 180.0);
		for(int _r = 0 ; _r <= hr ; ++_r)
		{
			const int dx = int(sqrt(float(r2 - _r * _r)) + 0.5f);
#define DRAW_PIXEL(X, Y)\
			{\
				const int _x = X;\
				const int _y = Y;\
				if (sx * (_x - x) + sy * (_y - y) <= 0.0f\
					&& ex * (_x - x) + ey * (_y - y) <= 0.0f)\
					putpixel(dst, _x, _y, col);\
			}

			DRAW_PIXEL(x - dx, y - _r);
			DRAW_PIXEL(x + dx, y - _r);
			DRAW_PIXEL(x - dx, y + _r);
			DRAW_PIXEL(x + dx, y + _r);
			DRAW_PIXEL(x - _r, y - dx);
			DRAW_PIXEL(x + _r, y - dx);
			DRAW_PIXEL(x - _r, y + dx);
			DRAW_PIXEL(x + _r, y + dx);
		}
	}

	void putpixel(SDL_Surface *dst, int x, int y, uint32 col)
	{
		if (x < _X0(dst) || x >= _X1(dst) || y < _Y0(dst) || y >= _Y1(dst))
			return;
		Font::setPixel(dst, x, y, col);
	}

	uint32 getpixel(SDL_Surface *dst, int x, int y)
	{
		if (x < _X0(dst) || x >= _X1(dst) || y < _Y0(dst) || y >= _Y1(dst))
			return 0;
		return Font::getPixel(dst, x, y);
	}

	void gradientbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, float dx, float dy, uint32 col, uint32 dcol)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
		const int mx = (x0 + x1 + 1) >> 1;
		const int my = (y0 + y1 + 1) >> 1;
		const int idy = int(0x1000 * dy);
		const int idx = int(0x1000 * dx);
		const SDL_PixelFormat *format = dst->format;
		sint32 r, g, b;
		sint32 dr, dg, db;
		getRGB(col, format, r, g, b);
		getRGB(dcol, format, dr, dg, db);
		if (dx == 0.0f)
		{
			for(int y = y0 ; y <= y1 ; ++y)
			{
				const int d = (y - my) * idy;
				const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
				const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
				const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
				const uint32 c = mapRGB(format, nr, ng, nb);
				hline(dst, y, x0, x1, c);
			}
			return;
		}
		if (dy == 0.0f)
		{
			for(int x = x0 ; x <= x1 ; ++x)
			{
				const int d = (x - mx) * idx;
				const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
				const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
				const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
				const uint32 c = mapRGB(format, nr, ng, nb);
				vline(dst, x, y0, y1, c);
			}
			return;
		}
		y0 = std::max<int>(y0, _Y0(dst));
		y1 = std::min<int>(y1, _Y1(dst) - 1);
		x0 = std::max<int>(x0, _X0(dst));
		x1 = std::min<int>(x1, _X1(dst) - 1);
		switch(format->BytesPerPixel)
		{
		case 1:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				byte *p = (byte*)dst->pixels + y * dst->pitch + x0;
				for(int x = x0 ; x <= x1 ; ++x, ++p)
				{
					const int d = (x - mx) * idx + (y - my) * idy;
					const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
					const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
					const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
					*p = mapRGB(format, nr, ng, nb);
				}
			}
			break;
		case 2:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				uint16 *p = (uint16*)dst->pixels + y * (dst->pitch >> 1) + x0;
				for(int x = x0 ; x <= x1 ; ++x, ++p)
				{
					const int d = (x - mx) * idx + (y - my) * idy;
					const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
					const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
					const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
					*p = mapRGB(format, nr, ng, nb);
				}
			}
			break;
		case 3:
			r = (col >> 16) & 0xFFU;
			g = (col >> 8) & 0xFFU;
			b = col & 0xFFU;
			dr = (dcol >> 16) & 0xFFU;
			dg = (dcol >> 8) & 0xFFU;
			db = dcol & 0xFFU;
			for(int y = y0 ; y <= y1 ; ++y)
			{
				byte *p = (byte*)dst->pixels + y * dst->pitch + x0 * 3;
				for(int x = x0 ; x <= x1 ; ++x, p += 3)
				{
					const int d = (x - mx) * idx + (y - my) * idy;
					const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
					const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
					const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
					p[0] = nr;
					p[1] = ng;
					p[2] = nb;
				}
			}
			break;
		case 4:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				uint32 *p = (uint32*)dst->pixels + (y * dst->pitch >> 2) + x0;
				for(int x = x0 ; x <= x1 ; ++x, ++p)
				{
					const int d = (x - mx) * idx + (y - my) * idy;
					const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
					const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
					const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
					*p = mapRGB(format, nr,ng,nb);
				}
			}
			break;
		}
	}

	void roundedgradientbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, int radius, float dx, float dy, uint32 col, uint32 dcol)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
		const int mx = (x0 + x1 + 1) >> 1;
		const int my = (y0 + y1 + 1) >> 1;
		const int idy = int(0x1000 * dy);
		const int idx = int(0x1000 * dx);
		const SDL_PixelFormat *format = dst->format;
		sint32 r, g, b, a;
		sint32 dr, dg, db, da;
		getRGBA(col, format, r, g, b, a);
		getRGBA(dcol, format, dr, dg, db, da);
		const int bx0 = max<int>(x0, _X0(dst));
		const int bx1 = min<int>(x1, _X1(dst) - 1);
		if (bx1 >= _X0(dst) && bx0 < _X1(dst))
		{
			switch(dst->format->BytesPerPixel)
			{
			case 1:
				for(int y = y0 + radius ; y <= y1 - radius ; ++y)
				{
					if (y < _Y0(dst) || y >= _Y1(dst))
						continue;
					byte *p = (byte*)dst->pixels + y * dst->pitch + bx0;
					if (dx == 0.0f)
					{
						const int d = (y - my) * idy;
						const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
						const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
						const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
						hline(dst, y, bx0, bx1, mapRGB(format, nr, ng, nb));
					}
					else
						for(int x = bx0 ; x <= bx1 ; ++x, ++p)
						{
							const int d = (x - mx) * idx + (y - my) * idy;
							const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
							const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
							const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
							*p = mapRGB(dst->format, nr, ng, nb);
						}
				}
				break;
			case 2:
				for(int y = y0 + radius ; y <= y1 - radius ; ++y)
				{
					if (y < _Y0(dst) || y >= _Y1(dst))
						continue;
					uint16 *p = (uint16*)dst->pixels + (y * dst->pitch >> 1) + bx0;
					if (dx == 0.0f)
					{
						const int d = (y - my) * idy;
						const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
						const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
						const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
						hline(dst, y, bx0, bx1, mapRGB(dst->format, nr, ng, nb));
					}
					else
						for(int x = bx0 ; x <= bx1 ; ++x, ++p)
						{
							const int d = (x - mx) * idx + (y - my) * idy;
							const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
							const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
							const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
							*p = mapRGB(format, nr, ng, nb);
						}
				}
				break;
			case 3:
				for(int y = y0 + radius ; y <= y1 - radius ; ++y)
				{
					if (y < _Y0(dst) || y >= _Y1(dst))
						continue;
					if (dx == 0.0f)
					{
						const int d = (y - my) * idy;
						const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
						const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
						const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
						hline(dst, y, bx0, bx1, mapRGB(dst->format, nr, ng, nb));
					}
					else
						for(int x = bx0 ; x <= bx1 ; ++x)
						{
							const int d = (x - mx) * idx + (y - my) * idy;
							const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
							const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
							const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
							Font::setPixel24(dst, x, y, mapRGB(dst->format, nr, ng, nb));
						}
				}
				break;
			case 4:
				for(int y = y0 + radius ; y <= y1 - radius ; ++y)
				{
					if (y < _Y0(dst) || y >= _Y1(dst))
						continue;
					uint32 *p = (uint32*)dst->pixels + (y * dst->pitch >> 2) + bx0;
					if (dx == 0.0f)
					{
						const int d = (y - my) * idy;
						const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
						const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
						const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
						const uint32 na = min<uint32>(a + (d * da >> 12), 0xFFU);
						hline(dst, y, bx0, bx1, mapRGBA(format, nr, ng, nb, na));
					}
					else
						for(int x = bx0 ; x <= bx1 ; ++x, ++p)
						{
							const int d = (x - mx) * idx + (y - my) * idy;
							const uint32 nr = min<uint32>(r + (d * dr >> 12), 0xFFU);
							const uint32 ng = min<uint32>(g + (d * dg >> 12), 0xFFU);
							const uint32 nb = min<uint32>(b + (d * db >> 12), 0xFFU);
							const uint32 na = min<uint32>(a + (d * da >> 12), 0xFFU);
							*p = mapRGBA(format, nr, ng, nb, na);
						}
				}
				getRGBA(col, dst->format, r, g, b, a);
				getRGBA(dcol, dst->format, dr, dg, db, da);
				break;
			};
		}
		const int r2 = radius * radius;
		const int hr = int(radius * M_SQRT2 * 0.5f + 0.5f);
		for(int _r = 0 ; _r <= hr ; ++_r)
		{
			const int _dr = int(sqrt(float(r2 - _r * _r)) + 0.5f);
			int _x0 = x0 + radius - _dr;
			int _x1 = x1 - radius + _dr;
			int y = y0 + radius - _r;
#undef DRAW_LINE
#define DRAW_LINE()\
			for(int x = _x0 ; x <= _x1 ; ++x)\
			{\
				const int d = (x - mx) * idx + (y - my) * idy;\
				const uint32 nr = std::min<uint32>(r + (d * dr >> 12), 0xFFU);\
				const uint32 ng = std::min<uint32>(g + (d * dg >> 12), 0xFFU);\
				const uint32 nb = std::min<uint32>(b + (d * db >> 12), 0xFFU);\
				putpixel(dst, x, y, mapRGB(dst->format, nr, ng, nb));\
			}
			DRAW_LINE();
			y = y1 - radius + _r;
			DRAW_LINE();
			_x0 = x0 + radius - _r;
			_x1 = x1 - radius + _r;
			y = y0 + radius - _dr;
			DRAW_LINE();
			y = y1 - radius + _dr;
			DRAW_LINE();
		}
	}

	void fill(SDL_Surface *dst, uint32 col)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
		SDL_FillRect(dst, NULL, col);
	}

	void initCursors()
	{
		if (cursor_arrow || SDL_GetVideoSurface() == NULL)
			return;

		static const char *arrow =
				"X                               "
				"XX                              "
				"X.X                             "
				"X..X                            "
				"X...X                           "
				"X....X                          "
				"X.....X                         "
				"X......X                        "
				"X.......X                       "
				"X........X                      "
				"X.....XXXXX                     "
				"X..X..X                         "
				"X.X X..X                        "
				"XX  X..X                        "
				"X    X..X                       "
				"     X..X                       "
				"      X..X                      "
				"      X..X                      "
				"       XX                       "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                ";
		cursor_arrow = loadCursor(arrow, 0, 0);

		static const char *edit =
				"   XXXXX                        "
				"   X...X                        "
				"   XX.XX                        "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"    X.X                         "
				"   XX.XX                        "
				"   X...X                        "
				"   XXXXX                        "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                "
				"                                ";
		cursor_edit = loadCursor(edit, 5, 7);

		SDL_SetCursor(cursor_arrow);
	}

	SDL_Cursor *loadCursor(const char *image, int hot_x, int hot_y)
	{
		Uint8 data[4*32];
		Uint8 mask[4*32];

		int i = -1;
		for (int row = 0 ; row < 32 ; ++row)
		{
			for (int col = 0 ; col < 32 ; ++col, ++image)
			{
				if (col % 8)
				{
					data[i] <<= 1;
					mask[i] <<= 1;
				}
				else
				{
					++i;
					data[i] = mask[i] = 0;
				}
				switch (*image)
				{
				case 'X':
					data[i] |= 0x01;
					mask[i] |= 0x01;
					break;
				case '.':
					mask[i] |= 0x01;
					break;
				case ' ':
					break;
				}
			}
		}
		return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
	}

	void vwhitealphagradientbox(SDL_Surface *dst, int x0, int y0, int x1, int y1)
	{
		if (dst->w <= 0 || dst->h <= 0)
			return;
		SDL_PixelFormat *format = dst->format;
		const int h = y1 - y0 + 1;
		const int ph = 0x10000 / h;
		x0 = max<int>(x0, _X0(dst));
		x1 = min<int>(x1, _X1(dst) - 1);
		switch(format->BytesPerPixel)
		{
		case 1:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				if (y < _Y0(dst) || y >= _Y1(dst))	continue;
				const int bkg = 0xFF + ((0xCF - 0xFF) * y * ph >> 16);
				const int iy = (h - y) * bkg;
				byte *p = (byte*)dst->pixels + y * dst->pitch + x0;
				int prevp = -1;
				for(int x = x0 ; x <= x1 ; ++x, ++p)
				{
					if (*p == prevp)
					{
						*p = *(p - 1);
						continue;
					}
					prevp = *p;
					sint32 _r, _g, _b;
					getRGB(*p, format, _r, _g, _b);
					_r = (_r * y + iy) * ph >> 16;
					_g = (_g * y + iy) * ph >> 16;
					_b = (_b * y + iy) * ph >> 16;
					*p = mapRGB(format, _r, _g, _b);
				}
			}
			break;
		case 2:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				if (y < _Y0(dst) || y >= _Y1(dst))	continue;
				const int bkg = 0xFF + ((0xCF - 0xFF) * y * ph >> 16);
				const int iy = (h - y) * bkg;
				uint16 *p = (uint16*)((byte*)dst->pixels + y * dst->pitch + x0 * 2);
				int prevp = -1;
				for(int x = x0 ; x <= x1 ; ++x, ++p)
				{
					if (*p == prevp)
					{
						*p = *(p - 1);
						continue;
					}
					prevp = *p;
					sint32 _r, _g, _b;
					getRGB(*p, format, _r, _g, _b);
					_r = (_r * y + iy) * ph >> 16;
					_g = (_g * y + iy) * ph >> 16;
					_b = (_b * y + iy) * ph >> 16;
					*p = mapRGB(format, _r, _g, _b);
				}
			}
			break;
		case 3:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				if (y < _Y0(dst) || y >= _Y1(dst))	continue;
				const int bkg = 0xFF + ((0xCF - 0xFF) * y * ph >> 16);
				const int iy = (h - y) * bkg;
				byte *p = (byte*)dst->pixels + y * dst->pitch + x0 * 3;
				uint32 prev = 0xFFFFFF;
				for(int x = x0 ; x <= x1 ; ++x)
				{
					if (prev == (*((uint32*)p) >> 8))
					{
						*p = *(p - 3);	++p;
						*p = *(p - 3);	++p;
						*p = *(p - 3);	++p;
						continue;
					}
					prev = (*((uint32*)p) >> 8);
					for(int i = 0 ; i < 3 ; ++i, ++p)
						*p = (*p * y + iy) * ph >> 16;
				}
			}
			break;
		case 4:
			for(int y = y0 ; y <= y1 ; ++y)
			{
				if (y < _Y0(dst) || y >= _Y1(dst))	continue;
				const int bkg = 0xFF + ((0xCF - 0xFF) * y * ph >> 16);
				const int iy = (h - y) * bkg;
				byte *p = (byte*)dst->pixels + y * dst->pitch + x0 * 4;
				uint32 prev = 0;
				for(int x = x0 ; x <= x1 ; ++x)
				{
					if (x > x0 && prev == *(uint32*)p)
					{
						*(uint32*)p = *((uint32*)p - 1);
						p += 4;
						continue;
					}
					prev = *(uint32*)p;
					for(int i = 0 ; i < 4 ; ++i, ++p)
						*p = (*p * y + iy) * ph >> 16;
				}
			}
			break;
		}
	}

}
