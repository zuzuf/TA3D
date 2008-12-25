
#include "pcx.h"
#include "../logs/logs.h"
#include "../TA3D_NameSpace.h"


namespace TA3D
{
namespace Converters
{



    BITMAP* PCX::RawDataToBitmap(const byte* data, const RGB* cpal)
    {
        if (!data || !cpal)
            return NULL;
        // Size of the picture
        short width  = *((short*)(data+8))  - *((short*)(data+4)) + 1;
        short height = *((short*)(data+10)) - *((short*)(data+6)) + 1;

        BITMAP* pcx = create_bitmap(width, height);
        LOG_ASSERT(pcx != NULL);
        clear(pcx);

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
                        ((int*)(pcx->line[y]))[x++] = col;
                }
                else
                    ((int*)(pcx->line[y]))[x++] = makecol(cpal[c].r << 2, cpal[c].g << 2, cpal[c].b << 2);
            } while(x < pcx->w);
        }
        return pcx;
    }



    BITMAP* PCX::FromHPIToBitmap(const String& filename)
    {
        if (!filename.empty())
        {
            byte* data = HPIManager->PullFromHPI(filename);
            if (data)
            {
                BITMAP* ret = RawDataToBitmap(data, pal);
                delete[] data;
                return ret;
            }
        }
        return NULL;
    }


} // namespace Converters
} // namespace TA3D


