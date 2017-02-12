#ifndef RENDERAPI_H
#define RENDERAPI_H

#include "types.h"

struct SDL_Surface;
struct SDL_Cursor;
struct SDL_PixelFormat;

namespace Gui
{

	SDL_Surface *createNativeSurface(int w, int h);
	SDL_Surface *createSubSurface(SDL_Surface *src, int x, int y, int w, int h);
	SDL_Surface SubSurface(SDL_Surface *src, int x, int y, int w, int h);
	void blit(SDL_Surface *src, SDL_Surface *dst, int x, int y);
	void blit(SDL_Surface *src, SDL_Surface *dst, int x0, int y0, int x1, int y1, int w, int h);
	void line(SDL_Surface *dst, int x0, int y0, int x1, int y1, uint32 col);
	void vline(SDL_Surface *dst, int x, int y0, int y1, uint32 col);
	void hline(SDL_Surface *dst, int y, int x0, int x1, uint32 col);
	void box(SDL_Surface *dst, int x0, int y0, int x1, int y1, uint32 col);
	void fillbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, uint32 col);
	void roundedbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, int r, uint32 col);
	void fillroundedbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, int r, uint32 col);
	void circle(SDL_Surface *dst, int x, int y, int r, uint32 col);
	void fillcircle(SDL_Surface *dst, int x, int y, int r, uint32 col);
	void arc(SDL_Surface *dst, int x, int y, int r, int start, int end, uint32 col);
	void putpixel(SDL_Surface *dst, int x, int y, uint32 col);
	uint32 getpixel(SDL_Surface *dst, int x, int y);
	void gradientbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, float dx, float dy, uint32 col, uint32 dcol);
	void roundedgradientbox(SDL_Surface *dst, int x0, int y0, int x1, int y1, int r, float dx, float dy, uint32 col, uint32 dcol);
	void gradientcircle(SDL_Surface *dst, int x, int y, int r, float dx, float dy, uint32 col, uint32 dcol);
	void fill(SDL_Surface *dst, uint32 col);
	bool compareSurfaces(SDL_Surface *src, SDL_Surface *dst, int x, int y);
	void vwhitealphagradientbox(SDL_Surface *dst, int x0, int y0, int x1, int y1);

	void updateGUIColors();
	void initCursors();
	SDL_Cursor *loadCursor(const char *image, int hot_x, int hot_y);

	extern uint8 colormap[32768U];
	extern uint32 black;
	extern uint32 darkgrey;
	extern uint32 grey;
	extern uint32 lightgrey;
	extern uint32 verylightgrey;
	extern uint32 white;
	extern uint32 blue;
	extern uint32 red;
	extern uint32 green;
	extern uint32 yellow;
	extern uint32 darkblue;
	extern uint32 darkgreen;
	extern uint32 darkred;
	extern uint32 darkyellow;
	extern uint32 lightblue;
	extern uint32 lightgreen;
	extern uint32 lightred;
	extern uint32 lightyellow;
	extern SDL_Cursor *cursor_arrow;
	extern SDL_Cursor *cursor_edit;

	inline uint32 mapRGB(const SDL_PixelFormat* const format, uint32 r, uint32 g, uint32 b)
	{
		if (format->BytesPerPixel == 1)
			return colormap[((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)];
		r >>= format->Rloss;
		g >>= format->Gloss;
		b >>= format->Bloss;
		return (r << format->Rshift) | (g << format->Gshift) | (b << format->Bshift);
	}

	inline uint32 mapRGBA(const SDL_PixelFormat* const format, uint32 r, uint32 g, uint32 b, uint32 a)
	{
		if (format->BytesPerPixel == 1)
			return colormap[((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)];
		r >>= format->Rloss;
		g >>= format->Gloss;
		b >>= format->Bloss;
		a >>= format->Aloss;
		return (r << format->Rshift) | (g << format->Gshift) | (b << format->Bshift) | (a << format->Ashift);
	}

	template<class T>
	inline void getRGB(const uint32 c, const SDL_PixelFormat* const format, T &r, T &g, T &b)
	{
		if (format->BytesPerPixel == 1)
		{
			r = format->palette->colors[c].r;
			g = format->palette->colors[c].g;
			b = format->palette->colors[c].b;
			return;
		}
		r = (c & format->Rmask) >> format->Rshift << format->Rloss;
		g = (c & format->Gmask) >> format->Gshift << format->Gloss;
		b = (c & format->Bmask) >> format->Bshift << format->Bloss;
	}

	template<class T>
	inline void getRGBA(const uint32 c, const SDL_PixelFormat* const format, T &r, T &g, T &b, T &a)
	{
		if (format->BytesPerPixel == 1)
		{
			r = format->palette->colors[c].r;
			g = format->palette->colors[c].g;
			b = format->palette->colors[c].b;
			a = 0;
			return;
		}
		r = (c & format->Rmask) >> format->Rshift << format->Rloss;
		g = (c & format->Gmask) >> format->Gshift << format->Gloss;
		b = (c & format->Bmask) >> format->Bshift << format->Bloss;
		a = (c & format->Amask) >> format->Ashift << format->Aloss;
	}
}

#endif // RENDERAPI_H
