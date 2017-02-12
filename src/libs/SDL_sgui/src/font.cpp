
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/font.h>
#include "fontdata.h"
#include <SDL/sgui/renderapi.h>

#define _X0(surface)	((surface)->clip_rect.x)
#define _Y0(surface)	((surface)->clip_rect.y)
#define _X1(surface)	((surface)->clip_rect.x + (surface)->clip_rect.w)
#define _Y1(surface)	((surface)->clip_rect.y + (surface)->clip_rect.h)

using namespace std;

namespace Gui
{

	inline int Font::getGlyphID(wchar_t c)
	{
		switch(c)
		{
		case L'a':	return 0;
		case L'b':	return 1;
		case L'c':	return 2;
		case L'd':	return 3;
		case L'e':	return 4;
		case L'f':	return 5;
		case L'g':	return 6;
		case L'h':	return 7;
		case L'i':	return 8;
		case L'j':	return 9;
		case L'k':	return 10;
		case L'l':	return 11;
		case L'm':	return 12;
		case L'n':	return 13;
		case L'o':	return 14;
		case L'p':	return 15;
		case L'q':	return 16;
		case L'r':	return 17;
		case L's':	return 18;
		case L't':	return 19;
		case L'u':	return 20;
		case L'v':	return 21;
		case L'w':	return 22;
		case L'x':	return 23;
		case L'y':	return 24;
		case L'z':	return 25;
		case L'A':	return 26;
		case L'B':	return 27;
		case L'C':	return 28;
		case L'D':	return 29;
		case L'E':	return 30;
		case L'F':	return 31;
		case L'G':	return 32;
		case L'H':	return 33;
		case L'I':	return 34;
		case L'J':	return 35;
		case L'K':	return 36;
		case L'L':	return 37;
		case L'M':	return 38;
		case L'N':	return 39;
		case L'O':	return 40;
		case L'P':	return 41;
		case L'Q':	return 42;
		case L'R':	return 43;
		case L'S':	return 44;
		case L'T':	return 45;
		case L'U':	return 46;
		case L'V':	return 47;
		case L'W':	return 48;
		case L'X':	return 49;
		case L'Y':	return 50;
		case L'Z':	return 51;
		case L'0':	return 52;
		case L'1':	return 53;
		case L'2':	return 54;
		case L'3':	return 55;
		case L'4':	return 56;
		case L'5':	return 57;
		case L'6':	return 58;
		case L'7':	return 59;
		case L'8':	return 60;
		case L'9':	return 61;
		case L',':	return 62;
		case L';':	return 63;
		case L'.':	return 64;
		case L':':	return 65;
		case L'/':	return 66;
		case L'!':	return 67;
		case L'?':	return 68;
		case L'%':	return 69;
		case L'&':	return 70;
		case L'"':	return 71;
		case L'#':	return 72;
		case L'\'':	return 73;
		case L'`':	return 74;
		case L'¨':	return 75;
		case L'^':	return 76;
		case L'@':	return 77;
		case L'é':	return 78;
		case L'è':	return 79;
		case L'ç':	return 80;
		case L'à':	return 81;
		case L'€':	return 82;
		case L'$':	return 83;
		case L'-':	return 84;
		case L'+':	return 85;
		case L'*':	return 86;
		case L'\\':	return 87;
		case L'_':	return 88;
		case L'²':	return 89;
		case L'°':	return 90;
		case L'(':	return 91;
		case L')':	return 92;
		case L'[':	return 93;
		case L']':	return 94;
		case L'{':	return 95;
		case L'}':	return 96;
		case L'ù':	return 97;
		case L'ô':	return 98;
		case L'û':	return 99;
		case L'â':	return 100;
		case L'ê':	return 101;
		case L'î':	return 102;
		case L'<':	return 103;
		case L'>':	return 104;
		case L'~':	return 105;
		case L'|':	return 106;
		case L'ï':	return 107;
		case L'ö':	return 108;
		case L'ü':	return 109;
		case L'ë':	return 110;
		case L'ÿ':	return 111;
		case L'ŷ':	return 112;
		case L'=':	return 113;
		case L' ':	return 114;
		default:
			return -1;
		}
	}

	void Font::print(SDL_Surface *bmp, int x, int y, const string &str, unsigned int l)
	{
		if (y >= _Y1(bmp) || y + 16 <= _Y0(bmp))
			return;
		for(string::const_iterator i = str.begin(), end = str.end() ; i != end ; ++i, x += 8)
		{
			const int glyph_id = getGlyphID(*i);
			if (glyph_id == -1)
				continue;
			if (x >= _X1(bmp))
				return;
			drawGlyph(glyph_id, bmp, x, y, l);
		}
	}

	void Font::print(SDL_Surface *bmp, int x, int y, const wstring &str, unsigned int l)
	{
		if (y >= _Y1(bmp) || y + 16 <= _Y0(bmp))
			return;
		for(wstring::const_iterator i = str.begin(), end = str.end() ; i != end ; ++i, x += 8)
		{
			const int glyph_id = getGlyphID(*i);
			if (glyph_id == -1)
				continue;
			if (x >= _X1(bmp))
				return;
			drawGlyph(glyph_id, bmp, x, y, l);
		}
	}

	void Font::drawGlyph(const int glyph, SDL_Surface *dst, int x, int y, unsigned int l)
	{
		if (dst->h <= 0 || dst->w <= 0)
			return;

		sint32 lr, lg, lb, la;
		const SDL_PixelFormat* const format = dst->format;
		getRGBA(l, format, lr, lg, lb, la);
		const byte *pg = (const byte*)header_data + (glyph % 26) * 8 + (glyph / 26) * 16 * width;
		const int glyph_h = 16;
		const int glyph_w = 8;
		const size_t glyph_step = width - glyph_w;
		switch(format->BytesPerPixel)
		{
		case 1:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += width;
					continue;
				}
				byte *p = (byte*)dst->pixels + sy * dst->pitch + x;
				int	prev = -1;
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg, ++p)
				{
					const int alpha = 0xFF - *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx >= _X1(dst) || sx < 0 || alpha == 0)
					{
						prev = -1;
						continue;
					}
					if (((alpha << 8) | *p) == prev)
					{
						*p = *(p - 1);
						continue;
					}
					prev = (alpha << 8) | *p;
					sint32 r, g, b;
					getRGB(*p, format, r, g, b);

					r = (r * (0xFF - alpha) + alpha * lr) >> 8;
					g = (g * (0xFF - alpha) + alpha * lg) >> 8;
					b = (b * (0xFF - alpha) + alpha * lb) >> 8;

					*p = mapRGB(format, r, g, b);
				}
				pg += glyph_step;
			}
			break;
		case 2:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += width;
					continue;
				}
				uint16 *p = (uint16*)((byte*)dst->pixels + sy * dst->pitch + x * 2);
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg, ++p)
				{
					const int alpha = 0xFF - *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx >= _X1(dst) || sx < 0 || alpha == 0)
						continue;
					sint32 r, g, b;
					getRGB(*p, format, r, g, b);

					r = (r * (0xFF - alpha) + alpha * lr) >> 8;
					g = (g * (0xFF - alpha) + alpha * lg) >> 8;
					b = (b * (0xFF - alpha) + alpha * lb) >> 8;

					*p = mapRGB(dst->format, r, g, b);
				}
				pg += glyph_step;
			}
			break;
		case 3:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += width;
					continue;
				}
				byte *p = (byte*)dst->pixels + sy * dst->pitch + x * 3;
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg)
				{
					const int alpha = 0xFF - *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx < 0 || sx >= _X1(dst) || alpha == 0)
					{
						p += 3;
						continue;
					}
					*p = (*p * (0xFF - alpha) + alpha * lr) >> 8; ++p;
					*p = (*p * (0xFF - alpha) + alpha * lg) >> 8; ++p;
					*p = (*p * (0xFF - alpha) + alpha * lb) >> 8; ++p;
				}
				pg += glyph_step;
			}
			break;
		case 4:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += width;
					continue;
				}
				uint32 *p = (uint32*)dst->pixels + (sy * dst->pitch >> 2) + x;
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg)
				{
					const int alpha = 0xFF - *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx < 0 || sx >= _X1(dst) || alpha == 0)
					{
						++p;
						continue;
					}
					sint32 r, g, b;
					getRGB(*p, format, r, g, b);
					r = (r * (0xFF - alpha) + alpha * lr) >> 8;
					g = (g * (0xFF - alpha) + alpha * lg) >> 8;
					b = (b * (0xFF - alpha) + alpha * lb) >> 8;
					*p = mapRGB(format, r, g, b);
					++p;
				}
				pg += glyph_step;
			}
			break;
		};
	}

	void Font::drawGlyph(SDL_Surface *glyph, SDL_Surface *dst, int x, int y, unsigned int l)
	{
		if (dst->h <= 0 || dst->w <= 0)
			return;

		sint32 lr, lg, lb, la;
		const SDL_PixelFormat* const format = dst->format;
		getRGBA(l, format, lr, lg, lb, la);
		const byte *pg = (const byte*)glyph->pixels;
		const int glyph_h = glyph->h;
		const int glyph_w = glyph->w;
		const size_t glyph_step = glyph->pitch - glyph_w;
		switch(format->BytesPerPixel)
		{
		case 1:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += glyph->pitch;
					continue;
				}
				byte *p = (byte*)dst->pixels + sy * dst->pitch + x;
				int	prev = -1;
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg, ++p)
				{
					const int alpha = *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx >= _X1(dst) || sx < 0 || alpha == 0)
					{
						prev = -1;
						continue;
					}
					if (((alpha << 8) | *p) == prev)
					{
						*p = *(p - 1);
						continue;
					}
					prev = (alpha << 8) | *p;
					sint32 r, g, b;
					getRGB(*p, format, r, g, b);

					r = (r * (0xFF - alpha) + alpha * lr) >> 8;
					g = (g * (0xFF - alpha) + alpha * lg) >> 8;
					b = (b * (0xFF - alpha) + alpha * lb) >> 8;

					*p = mapRGB(format, r, g, b);
				}
				pg += glyph_step;
			}
			break;
		case 2:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += glyph->pitch;
					continue;
				}
				uint16 *p = (uint16*)((byte*)dst->pixels + sy * dst->pitch + x * 2);
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg, ++p)
				{
					const int alpha = *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx >= _X1(dst) || sx < 0 || alpha == 0)
						continue;
					sint32 r, g, b;
					getRGB(*p, format, r, g, b);

					r = (r * (0xFF - alpha) + alpha * lr) >> 8;
					g = (g * (0xFF - alpha) + alpha * lg) >> 8;
					b = (b * (0xFF - alpha) + alpha * lb) >> 8;

					*p = mapRGB(dst->format, r, g, b);
				}
				pg += glyph_step;
			}
			break;
		case 3:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += glyph->pitch;
					continue;
				}
				byte *p = (byte*)dst->pixels + sy * dst->pitch + x * 3;
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg)
				{
					const int alpha = *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx < 0 || sx >= _X1(dst) || alpha == 0)
					{
						p += 3;
						continue;
					}
					*p = (*p * (0xFF - alpha) + alpha * lr) >> 8; ++p;
					*p = (*p * (0xFF - alpha) + alpha * lg) >> 8; ++p;
					*p = (*p * (0xFF - alpha) + alpha * lb) >> 8; ++p;
				}
				pg += glyph_step;
			}
			break;
		case 4:
			for(int dy = 0 ; dy < glyph_h ; ++dy)
			{
				const int sy = y + dy;
				if (sy < _Y0(dst) || sy >= _Y1(dst) || sy < 0)
				{
					pg += glyph->pitch;
					continue;
				}
				uint32 *p = (uint32*)dst->pixels + (sy * dst->pitch >> 2) + x;
				for(int dx = 0 ; dx < glyph_w ; ++dx, ++pg)
				{
					const int alpha = *pg;
					const int sx = x + dx;
					if (sx < _X0(dst) || sx < 0 || sx >= _X1(dst) || alpha == 0)
					{
						++p;
						continue;
					}
					sint32 r, g, b;
					getRGB(*p, format, r, g, b);
					r = (r * (0xFF - alpha) + alpha * lr) >> 8;
					g = (g * (0xFF - alpha) + alpha * lg) >> 8;
					b = (b * (0xFF - alpha) + alpha * lb) >> 8;
					*p = mapRGB(format, r, g, b);
					++p;
				}
				pg += glyph_step;
			}
			break;
		};
	}

	bool Font::hasGlyph(wchar_t c)
	{
		return getGlyphID(c) != -1;
	}

}
