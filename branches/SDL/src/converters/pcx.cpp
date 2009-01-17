
#include "../stdafx.h"
#include "pcx.h"
#include "../logs/logs.h"
#include "../TA3D_NameSpace.h"


namespace TA3D
{
namespace Converters
{



    SDL_Surface* PCX::RawDataToBitmap(const byte* data, const SDL_Color* cpal)
    {
        if (!data || !cpal)
            return NULL;
        // Size of the picture
        short width  = *((short*)(data+8))  - *((short*)(data+4)) + 1;
        short height = *((short*)(data+10)) - *((short*)(data+6)) + 1;

        SDL_Surface* pcx = gfx->create_surface(width, height);
        LOG_ASSERT(pcx != NULL);
        SDL_FillRect(pcx, NULL, 0);

        int pos = 128;
        for (int y = 0; y < pcx->h; ++y)
        {
            int x(0);
            do
            {
                int c = data[pos++];
                if (c > 191)
                {
                    int l = c - 192;
                    c = data[pos++];
                    int col = makecol(cpal[c].r << 2, cpal[c].g << 2, cpal[c].b << 2);
                    for(; l > 0 && x < pcx->w; --l)
                        SurfaceInt(pcx, x++, y) = col;
                }
                else
                    SurfaceInt(pcx, x++, y) = makecol(cpal[c].r << 2, cpal[c].g << 2, cpal[c].b << 2);
            } while(x < pcx->w);
        }
        return pcx;
    }



    SDL_Surface* PCX::FromHPIToBitmap(const String& filename)
    {
        if (!filename.empty())
        {
            byte* data = HPIManager->PullFromHPI(filename);
            if (data)
            {
                SDL_Surface* ret = RawDataToBitmap(data, pal);
                delete[] data;
                return ret;
            }
        }
        return NULL;
    }


} // namespace Converters
} // namespace TA3D


