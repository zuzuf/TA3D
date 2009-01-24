/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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

/*
**  File: stdafx.cpp
** Notes:
**   Cire: stdafx.h and stdafx.cpp will generate a pch file
**         (pre compiled headers). Its goal is to include
**         everything that should be global to the project, as well
**         as platform specific setups and configurations.
*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "misc/paths.h"
#include "gfx/gfx.h"
#include <sys/stat.h>
#include <fstream>
#include <math.h>
#include <zlib.h>

using namespace std;

namespace TA3D
{


String
format(const char* fmt, ...)
{
    if( !fmt )
        return String("");

    int result = -1, length = 256;
    char *buffer = 0;

	while(-1 == result || result > length )
	{
	    va_list args;
		va_start(args, fmt);

		length <<= 1;
		if (buffer)
		    delete [] buffer;
		buffer = new char [length + 1];
        memset(buffer, 0, length + 1);
        #if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
		result = _vsnprintf(buffer, length, fmt, args);
        #else
		result = vsnprintf(buffer, length, fmt, args);
        #endif
		va_end(args);
	}
	String s(buffer);
	delete [] buffer;
	return s;
}

sint32
SearchString(const String& s, const String& stringToSearch, const bool ignoreCase)
{
	static const std::basic_string <char>::size_type NotFound = std::string::npos;

	std::basic_string <char>::size_type iFind;
	String sz1, sz2;

	if(ignoreCase)
	{
		sz1 = String::ToUpper(s);
		sz2 = String::ToUpper(stringToSearch);
	}
	else
	{
		sz1 = s;
		sz2 = stringToSearch;
	}
	iFind = sz1.find(sz2);

	return ((NotFound == iFind) ? -1 : (sint32)iFind);
}


String
ReplaceString(const String& s, const String& toSearch, const String& replaceWith, const bool ignoreCase)
{
    String result = s;
	int f = 0;
	while ((f = SearchString(result, toSearch, ignoreCase)) != -1)
		result = result.replace(f, toSearch.size(), replaceWith);
	return result;
}


String
ReplaceChar(const String& s, const char toSearch, const char replaceWith)
{
    String ret(s);
    int l = s.size();
	for( int i = 0 ; i < l; ++i)
    {
		if(toSearch == ret[i])
		    ret[i] = replaceWith;
    }
	return ret;
}


bool
StartsWith(const String& a, const String& b)
{
    String y (a);
	String z (b);
	uint16 ai, bi;

	ai = (uint16)(y.toLower().length());
	bi = (uint16)(z.toLower().length());

	if (ai > bi)
		return  ( (y.compare( 0, bi, z ) == 0 ) ? true : false );
	return ( (z.compare( 0, ai, y ) == 0 ) ? true : false );
}


String
get_path(const String& s)
{
	String path;
	for( int i = s.size() - 1 ; i >= 0 ; i-- )
    {
		if( s[i] == '/' || s[i] == '\\' )
        {
			path = s;
			path.resize(i);
			break;
		}
    }
	return path;
}



#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
void
ExtractPathFile(const String& szFullFileName, String& szFile, String& szDir)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	// Below extracts our path where the application is being run.
	::_splitpath_s( szFullFileName.c_str(), drive, dir, fname, ext );

	szFile = fname;
	szDir = drive;
	szDir += dir;
}
#endif


String
GetClientPath(void)
{
	static bool bName = false;
	static String szName = "";

	if (!bName)
	{
        # if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
		char fPath[ MAX_PATH ];
		String Tmp;
		::GetModuleFileNameA( NULL, fPath, MAX_PATH );
		ExtractPathFile( fPath, Tmp, szName );
        # endif
		bName = true;
	}
	return szName;
}

uint32 file_size(const String &filename)
{
    fstream file(filename.c_str(), fstream::in);

    if (file.is_open())
    {
        file.seekg(0, fstream::end);
        uint32 _size = file.tellg();
        file.close();

        return _size;
    }

    return 0;
}

int ASCIItoUTF8(const byte c, byte *out)
{
    if (c < 0x80)
    {
        *out = c;
        return 1;
    }
    else if(c < 0xC0)
    {
        out[0] = 0xC2;
        out[1] = c;
        return 2;
    }
    out[0] = 0xC3;
    out[1] = c - 0x40;
    return 2;
}

bool exists(const String &filename)
{
    struct stat FileInfo;
    return stat(filename.c_str(),&FileInfo) == 0;
}

SDL_Surface *convert_format(SDL_Surface *bmp)
{
    if (bmp->format->BitsPerPixel != 32
    || bmp->format->Rmask != 0x000000FF
    || bmp->format->Gmask != 0x0000FF00
    || bmp->format->Bmask != 0x00FF0000
    || bmp->format->Amask != 0xFF000000)
    {
        SDL_PixelFormat target_format;
        target_format.palette = NULL;
        target_format.BitsPerPixel = 32;
        target_format.BytesPerPixel = 4;
        target_format.Rloss = target_format.Gloss = target_format.Bloss = target_format.Aloss = 0;
        target_format.Rmask = 0x000000FF;
        target_format.Gmask = 0x0000FF00;
        target_format.Bmask = 0x00FF0000;
        target_format.Amask = 0xFF000000;
        target_format.colorkey = 0x00FF00FF;
        target_format.alpha = 0xFF;
        target_format.Rshift = 0;
        target_format.Gshift = 8;
        target_format.Bshift = 16;
        target_format.Ashift = 24;

        if (bmp->format->BitsPerPixel == 8)
            SDL_SetPalette(bmp, SDL_LOGPAL|SDL_PHYSPAL, TA3D::VARS::pal, 0, 256);

        SDL_Surface *tmp = SDL_ConvertSurface(bmp, &target_format, SDL_SWSURFACE);
        SDL_FreeSurface(bmp);
        bmp = tmp;
    }
    return bmp;
}

SDL_Surface *convert_format_copy(SDL_Surface *bmp)
{
    SDL_PixelFormat target_format;
    target_format.palette = NULL;
    target_format.BitsPerPixel = 32;
    target_format.BytesPerPixel = 4;
    target_format.Rloss = target_format.Gloss = target_format.Bloss = target_format.Aloss = 0;
    target_format.Rmask = 0x000000FF;
    target_format.Gmask = 0x0000FF00;
    target_format.Bmask = 0x00FF0000;
    target_format.Amask = 0xFF000000;
    target_format.colorkey = 0x00FF00FF;
    target_format.alpha = 0xFF;
    target_format.Rshift = 0;
    target_format.Gshift = 8;
    target_format.Bshift = 16;
    target_format.Ashift = 24;

    if (bmp->format->BitsPerPixel == 8)
        SDL_SetPalette(bmp, SDL_LOGPAL|SDL_PHYSPAL, TA3D::VARS::pal, 0, 256);

    SDL_Surface *tmp = SDL_ConvertSurface(bmp, &target_format, SDL_SWSURFACE);

    return tmp;
}

SDL_Surface *convert_format_24(SDL_Surface *bmp)
{
    if (bmp->format->BitsPerPixel != 24
    || bmp->format->Rmask != 0x000000FF
    || bmp->format->Gmask != 0x0000FF00
    || bmp->format->Bmask != 0x00FF0000)
    {
        SDL_PixelFormat target_format;
        target_format.palette = NULL;
        target_format.BitsPerPixel = 24;
        target_format.BytesPerPixel = 3;
        target_format.Rloss = target_format.Gloss = target_format.Bloss = 0;
        target_format.Aloss = 8;
        target_format.Rmask = 0x000000FF;
        target_format.Gmask = 0x0000FF00;
        target_format.Bmask = 0x00FF0000;
        target_format.Amask = 0x00000000;
        target_format.colorkey = 0x00FF00FF;
        target_format.alpha = 0xFF;
        target_format.Rshift = 0;
        target_format.Gshift = 8;
        target_format.Bshift = 16;
        target_format.Ashift = 24;

        if (bmp->format->BitsPerPixel == 8)
            SDL_SetPalette(bmp, SDL_LOGPAL|SDL_PHYSPAL, TA3D::VARS::pal, 0, 256);

        SDL_Surface *tmp = SDL_ConvertSurface(bmp, &target_format, SDL_SWSURFACE);
        SDL_FreeSurface(bmp);
        bmp = tmp;
    }
    return bmp;
}

SDL_Surface *convert_format_16(SDL_Surface *bmp)
{
    if (bmp->format->BitsPerPixel != 16)
    {
        SDL_PixelFormat target_format;
        target_format.palette = NULL;
        target_format.BitsPerPixel = 16;
        target_format.BytesPerPixel = 2;
        target_format.Rloss = 3;
        target_format.Gloss = 2;
        target_format.Bloss = 3;
        target_format.Aloss = 8;
        target_format.Rmask = 0x0000001F;
        target_format.Gmask = 0x000007E0;
        target_format.Bmask = 0x0000F800;
        target_format.Amask = 0x00000000;
        target_format.colorkey = 0x0000F81F;
        target_format.alpha = 0xFF;
        target_format.Rshift = 0;
        target_format.Gshift = 5;
        target_format.Bshift = 11;
        target_format.Ashift = 16;

        if (bmp->format->BitsPerPixel == 8)
            SDL_SetPalette(bmp, SDL_LOGPAL|SDL_PHYSPAL, TA3D::VARS::pal, 0, 256);

        SDL_Surface *tmp = SDL_ConvertSurface(bmp, &target_format, SDL_SWSURFACE);
        SDL_FreeSurface(bmp);
        bmp = tmp;
    }
    return bmp;
}

void blit(SDL_Surface *in, SDL_Surface *out, int x0, int y0, int x1, int y1, int w, int h)
{
    SDL_Surface *tmp = in;
    if (in->format->BitsPerPixel != out->format->BitsPerPixel)
    {
        if (in->format->BitsPerPixel == 8)
            SDL_SetPalette(in, SDL_LOGPAL|SDL_PHYSPAL, TA3D::VARS::pal, 0, 256);

        tmp = SDL_ConvertSurface(in, out->format, SDL_SWSURFACE);
    }

    SDL_LockSurface(tmp);
    SDL_LockSurface(out);

    int sx = x0;
    int dx = x1;
    int cw = w;
    if (sx < 0)
    {
        dx -= sx;
        cw += sx;
        sx = 0;
    }
    if (dx < 0)
    {
        sx -= dx;
        cw += dx;
        dx = 0;
    }

    if (sx < tmp->w && dx < out->w)
    {
        sx *= tmp->format->BytesPerPixel;
        dx *= tmp->format->BytesPerPixel;
        cw *= tmp->format->BytesPerPixel;
        for(int y = 0 ; y < h ; y++)
        {
            int dy = y1 + y;
            int sy = y + y0;
            if (dy < 0 || dy >= out->h || sy < 0 || sy >= tmp->h)    continue;
            dy *= out->pitch;
            sy *= tmp->pitch;

            memcpy( ((byte*)out->pixels) + dy + dx, ((byte*)tmp->pixels) + sy + sx, cw);
        }
    }

    SDL_UnlockSurface(tmp);
    SDL_UnlockSurface(out);

    if (in->format->BitsPerPixel != out->format->BitsPerPixel)
        SDL_FreeSurface(tmp);
}

void masked_blit(SDL_Surface *in, SDL_Surface *out, int x0, int y0, int x1, int y1, int w, int h)
{
    SDL_LockSurface(in);
    SDL_LockSurface(out);
    for(int y = 0 ; y < h ; y++)
    {
        int dy = (y1 + y) * out->pitch;
        int sy = (y + y0) * in->pitch;
        for(int x = 0 ; x < w ; x++)
        {
            int sx = x + x0;
            int dx = x1 + x;
            byte b = ((byte*)in->pixels)[sy + sx];
            if (b)
                ((byte*)out->pixels)[dy + dx] = b;
        }
    }
    SDL_UnlockSurface(in);
    SDL_UnlockSurface(out);
}

void stretch_blit( SDL_Surface *in, SDL_Surface *out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 )
{
    switch(in->format->BitsPerPixel)
    {
    case 8:
        for(int y = 0 ; y < h0 ; y++)
        {
            int dy = (y1 + y * h1 / h0) * out->pitch;
            int sy = (y + y0) * in->pitch;
            for(int x = 0 ; x < w0 ; x++)
            {
                int sx = x + x0;
                int dx = x1 + x * w1 / w0;
                ((byte*)out->pixels)[dy + dx] = ((byte*)in->pixels)[sy + sx];
            }
        }
        break;
    case 16:
        for(int y = 0 ; y < h0 ; y++)
        {
            int dy = (y1 + y * h1 / h0) * out->pitch >> 1;
            int sy = (y + y0) * in->pitch >> 1;
            for(int x = 0 ; x < w0 ; x++)
            {
                int sx = x + x0;
                int dx = x1 + x * w1 / w0;
                ((uint16*)out->pixels)[dy + dx] = ((uint16*)in->pixels)[sy + sx];
            }
        }
        break;
    case 32:
        for(int y = 0 ; y < h0 ; y++)
        {
            int dy = (y1 + y * h1 / h0) * out->pitch >> 2;
            int sy = (y + y0) * in->pitch >> 2;
            for(int x = 0 ; x < w0 ; x++)
            {
                int sx = x + x0;
                int dx = x1 + x * w1 / w0;
                ((uint32*)out->pixels)[dy + dx] = ((uint32*)in->pixels)[sy + sx];
            }
        }
        break;
    };
}

void rest(uint32 msec)
{
    SDL_Delay(msec);
}

void putpixel(SDL_Surface *bmp, int x, int y, uint32 col)
{
    if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)   return;
    switch(bmp->format->BitsPerPixel)
    {
    case 8:
        SurfaceByte(bmp, x, y) = col;
        break;
    case 16:
        (((uint16*)((bmp)->pixels))[(y) * ((bmp)->pitch >> 1) + (x)]) = col;
        break;
    case 24:
        SurfaceByte(bmp, x * 3, y) = getb32(col);
        SurfaceByte(bmp, x * 3 + 1, y) = getg32(col);
        SurfaceByte(bmp, x * 3 + 2, y) = getr32(col);
        break;
    case 32:
        SurfaceInt(bmp, x, y) = col;
        break;
    };
}

uint32 getpixel(SDL_Surface *bmp, int x, int y)
{
    if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)   return 0;
    switch(bmp->format->BitsPerPixel)
    {
    case 8:
        return SurfaceByte(bmp, x, y);
    case 16:
        return (((uint16*)((bmp)->pixels))[(y) * ((bmp)->pitch >> 1) + (x)]);
    case 24:
        {
            int b = SurfaceByte(bmp, x * 3, y);
            int g = SurfaceByte(bmp, x * 3 + 1, y);
            int r = SurfaceByte(bmp, x * 3 + 2, y);
            return makecol24(r,g,b);
        }
    case 32:
        return SurfaceInt(bmp, x, y);
    };
    return 0;
}

void line(SDL_Surface *bmp, int x0, int y0, int x1, int y1, uint32 col)
{
    if (abs(x0 - x1) > abs(y0 - y1))
    {
        if (x0 > x1)
        {
            x0 ^= x1;   x1 ^= x0;   x0 ^= x1;
            y0 ^= y1;   y1 ^= y0;   y0 ^= y1;
        }
        for(int x = x0 ; x <= x1 ; x++)
            putpixel(bmp, x, y0 + (y1 - y0) * (x - x0) / (x1 - x0), col);
    }
    else
    {
        if (y0 > y1)
        {
            x0 ^= x1;   x1 ^= x0;   x0 ^= x1;
            y0 ^= y1;   y1 ^= y0;   y0 ^= y1;
        }
        for(int y = y0 ; y <= y1 ; y++)
            putpixel(bmp, x0 + (x1 - x0) * (y - y0) / (y1 - y0), y, col);
    }
}

void triangle(SDL_Surface *bmp, int x0, int y0, int x1, int y1, int x2, int y2, uint32 col)
{
    if (y0 > y1)
    {
        x0 ^= x1;   x1 ^= x0;   x0 ^= x1;
        y0 ^= y1;   y1 ^= y0;   y0 ^= y1;
    }
    if (y1 > y2)
    {
        x2 ^= x1;   x1 ^= x2;   x2 ^= x1;
        y2 ^= y1;   y1 ^= y2;   y2 ^= y1;
    }
    if (y0 > y1)
    {
        x0 ^= x1;   x1 ^= x0;   x0 ^= x1;
        y0 ^= y1;   y1 ^= y0;   y0 ^= y1;
    }

    if (y0 < y1)
        for(int y = y0 ; y <= y1 ; y++)
        {
            int ax = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int bx = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            if (ax > bx)
            {
                ax ^= bx;   bx ^= ax;   ax ^= bx;
            }
            for(int x = ax ; x <= bx ; x++)
                putpixel(bmp, x, y, col);
        }
    if (y1 < y2)
        for(int y = y1 ; y <= y2 ; y++)
        {
            int ax = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            int bx = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            if (ax > bx)
            {
                ax ^= bx;   bx ^= ax;   ax ^= bx;
            }
            for(int x = ax ; x <= bx ; x++)
                putpixel(bmp, x, y, col);
        }
}

void circlefill(SDL_Surface *bmp, int x, int y, int r, uint32 col)
{
    r *= r;
    for(int sy = -y ; sy <= y ; sy++)
    {
        int dx = sqrtf(r - sy * sy);
        int ax = x - dx;
        int bx = x + dx;
        for(int sx = ax ; sx <= bx ; sx++)
            putpixel(bmp, sx, y + sy, col);
    }
}

void rectfill(SDL_Surface *bmp, int x0, int y0, int x1, int y1, uint32 col)
{
    SDL_Rect rect = {x0, y0, x1 - x0, y1 - y0};
    SDL_FillRect(bmp, &rect, col);
}

void SaveTex(SDL_Surface *bmp, const String &filename)
{
    gzFile file = gzopen(filename.c_str(), "wb");
    if (file)
    {
        SDL_LockSurface(bmp);
        int w = bmp->w;
        int h = bmp->h;
        int bpp = bmp->format->BitsPerPixel;
        gzwrite( file, &w, sizeof(w));
        gzwrite( file, &h, sizeof(h));
        gzwrite( file, &bpp, sizeof(bpp));
        for(int y = 0 ; y < bmp->h ; y++)
            gzwrite( file, ((char*)(bmp->pixels)) + y * bmp->pitch, bmp->w * bmp->format->BytesPerPixel);
        SDL_UnlockSurface(bmp);

        gzclose( file );
    }
    else
        LOG_ERROR("could not save file : " << filename);
}

SDL_Surface *LoadTex(const String &filename)
{
    gzFile file = gzopen(filename.c_str(), "rb");
    if (file)
    {
        int w, h, bpp;
        gzread( file, &w, sizeof(w));
        gzread( file, &h, sizeof(h));
        gzread( file, &bpp, sizeof(bpp));
        SDL_Surface *bmp = gfx->create_surface_ex(bpp, w, h);
        SDL_LockSurface(bmp);
        for(int y = 0 ; y < bmp->h ; y++)
            gzread( file, ((char*)(bmp->pixels)) + y * bmp->pitch, bmp->w * bmp->format->BytesPerPixel);
        SDL_UnlockSurface(bmp);

        gzclose( file );

        return bmp;
    }
    else
        LOG_ERROR("could not load file : " << filename);
    return NULL;
}

void save_bitmap(const String &filename, SDL_Surface* bmp)
{
    String ext = String::ToLower( Paths::ExtractFileExt(filename) );
    if (ext == "bmp")
        SDL_SaveBMP(bmp, filename.c_str());
    else if (ext == "tex")                      // This is for cached texture data
        SaveTex(bmp, filename);
    else
        LOG_WARNING("save_bitmap : file format not supported (" << filename << ")");
}
}
