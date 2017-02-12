#ifndef FONT_H
#define FONT_H

#include <map>
#include <string>
#include "sdl-headers.h"

namespace Gui
{

class Font
{
private:
	inline Font()	{}

private:
	inline static int getGlyphID(wchar_t c);
public:
	static void drawGlyph(const int glyph, SDL_Surface *dst, int x, int y, unsigned int l = 0xFFFFFFFF);
	static void drawGlyph(SDL_Surface *glyph, SDL_Surface *dst, int x, int y, unsigned int l = 0xFFFFFFFF);
	inline static unsigned int getPixel(SDL_Surface *bmp, int x, int y);
	inline static unsigned int getPixel8(SDL_Surface *bmp, int x, int y);
	inline static unsigned int getPixel16(SDL_Surface *bmp, int x, int y);
	inline static unsigned int getPixel24(SDL_Surface *bmp, int x, int y);
	inline static unsigned int getPixel32(SDL_Surface *bmp, int x, int y);
	inline static void setPixel(SDL_Surface *bmp, int x, int y, unsigned int c);
	inline static void setPixel8(SDL_Surface *bmp, int x, int y, unsigned int c);
	inline static void setPixel16(SDL_Surface *bmp, int x, int y, unsigned int c);
	inline static void setPixel24(SDL_Surface *bmp, int x, int y, unsigned int c);
	inline static void setPixel32(SDL_Surface *bmp, int x, int y, unsigned int c);
	static bool hasGlyph(wchar_t c);

public:
	static void print(SDL_Surface *bmp, int x, int y, const std::string &str, unsigned int l = 0xFFFFFFFF);
	static void print(SDL_Surface *bmp, int x, int y, const std::wstring &str, unsigned int l = 0xFFFFFFFF);
};

inline unsigned int Font::getPixel8(SDL_Surface *bmp, int x, int y)
{
	return *((unsigned char*)bmp->pixels + x + y * bmp->pitch);
}

inline void Font::setPixel8(SDL_Surface *bmp, int x, int y, unsigned int c)
{
	*((unsigned char*)bmp->pixels + x + y * bmp->pitch) = (unsigned char)c;
}

inline unsigned int Font::getPixel16(SDL_Surface *bmp, int x, int y)
{
	return *((unsigned short*)bmp->pixels + x + (y * bmp->pitch >> 1));
}

inline void Font::setPixel16(SDL_Surface *bmp, int x, int y, unsigned int c)
{
	*((unsigned short*)bmp->pixels + x + (y * bmp->pitch >> 1)) = (unsigned short)c;
}

inline unsigned int Font::getPixel24(SDL_Surface *bmp, int x, int y)
{
	return (*((unsigned int*)((unsigned char*)bmp->pixels + x * 3 + y * bmp->pitch)) >> 8) & 0xFFFFFFU;
}

inline void Font::setPixel24(SDL_Surface *bmp, int x, int y, unsigned int c)
{
	unsigned char* const p = (unsigned char*)bmp->pixels + x * 3 + y * bmp->pitch;
	p[0] = (c >> 16) & 0xFF;
	p[1] = (c >> 8) & 0xFF;
	p[2] = c & 0xFF;
}

inline unsigned int Font::getPixel32(SDL_Surface *bmp, int x, int y)
{
	return *((unsigned int*)bmp->pixels + x + (y * bmp->pitch >> 2));
}

inline void Font::setPixel32(SDL_Surface *bmp, int x, int y, unsigned int c)
{
	*((unsigned int*)bmp->pixels + x + (y * bmp->pitch >> 2)) = c;
}

inline unsigned int Font::getPixel(SDL_Surface *bmp, int x, int y)
{
	switch(bmp->format->BytesPerPixel)
	{
	case 1:
		return *((unsigned char*)bmp->pixels + x + y * bmp->pitch);
	case 2:
		return *((unsigned short*)bmp->pixels + x + (y * bmp->pitch >> 1));
	case 3:
		return (*((unsigned int*)((unsigned char*)bmp->pixels + x * 3 + y * bmp->pitch)) >> 8) & 0xFFFFFF;
	case 4:
		return *((unsigned int*)bmp->pixels + x + (y * bmp->pitch >> 2));
	}
	return 0;
}

inline void Font::setPixel(SDL_Surface *bmp, int x, int y, unsigned int c)
{
	switch(bmp->format->BytesPerPixel)
	{
	case 1:
		*((unsigned char*)bmp->pixels + x + y * bmp->pitch) = (unsigned char)c;
		return;
	case 2:
		*((unsigned short*)bmp->pixels + x + (y * bmp->pitch >> 1)) = (unsigned short)c;
		return;
	case 3:
		{
			unsigned char* const p = (unsigned char*)bmp->pixels + x * 3 + y * bmp->pitch;
			p[0] = (c >> 16) & 0xFF;
			p[1] = (c >> 8) & 0xFF;
			p[2] = c & 0xFF;
		}
		return;
	case 4:
		*((unsigned int*)bmp->pixels + x + (y * bmp->pitch >> 2)) = c;
		return;
	}
}

}

#endif // FONT_H
