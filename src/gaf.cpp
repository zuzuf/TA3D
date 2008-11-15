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

/*-----------------------------------------------------------------------------------\
  |                                         gaf.cpp                                    |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers gaf de total annihilation qui sont les fichiers contenant les images  |
  | et les animations du jeu.                                                          |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "gaf.h"
#include <vector>
#include "jpeg/ta3d_jpg.h"
#include "gfx/glfunc.h"


namespace TA3D
{

    namespace
    {

        /*!
         * \brief Get the name of a GAF entry
         */
        void convertGAFCharToString(const byte* data, String& out)
        {
            char dt[33];
            memcpy(dt, data, 32);
            dt[32] = '\0';
            out = dt;
        }

    }


    Gaf::Frame::Data::Data()
        :Width(0), Height(0), XPos(0), YPos(0), Transparency(0), Compressed(0),
        FramePointers(0), Unknown2(0), PtrFrameData(0), Unknown3(0)
    {}

    Gaf::Frame::Data::Data(const byte* data, int pos)
    {
        LOG_ASSERT(data != NULL);
        Width         = *((sint16*)(data + pos)); pos += 2;
        Height        = *((sint16*)(data + pos)); pos += 2;
        XPos          = *((sint16*)(data + pos)); pos += 2;
        YPos          = *((sint16*)(data + pos)); pos += 2;
        Transparency  = *((sint8*) (data + pos)); pos += 1;
        Compressed    = *((sint8*) (data + pos)); pos += 1;
        FramePointers = *((sint16*)(data + pos)); pos += 2;
        Unknown2      = *((sint32*)(data + pos)); pos += 4;
        PtrFrameData  = *((sint32*)(data + pos)); pos += 4;
        Unknown3      = *((sint32*)(data + pos)); // pos += 4;
    }


    void Gaf::ToTexturesList(std::vector<GLuint>& out, const String& filename, const String& imgname,
                             int* w, int* h, const bool truecolor, const int filter)
    {
        out.clear();

        const byte* data = HPIManager->PullFromHPI(filename);
        if (data)
        {
            sint32 idx = RawDataGetEntryIndex(data, imgname);
            if (idx == -1)
            {
                delete[] data;
                return;
            }

            set_palette(pal);      // Activate the palette

            sint32 nb_img = RawDataImageCount(data, idx);
            if (nb_img <= 0)
            {
                delete[] data;
                return;
            }

            out.resize(nb_img);

            int indx(0);
            for (std::vector<GLuint>::iterator i = out.begin(); i != out.end(); ++i, ++indx)
            {
                uint32 fw, fh;
                String cache_filename;
                cache_filename << filename << "-" << imgname << format("-%d.bin", indx);
                *i = gfx->load_texture_from_cache(cache_filename, filter, &fw, &fh);

                if (!(*i))
                {
                    BITMAP* img = Gaf::RawDataToBitmap(data, idx, indx, NULL, NULL, truecolor);

                    if (!img)
                    {
                        delete[] data;
                        return;
                    }

                    if (w) w[indx] = img->w;
                    if (h) h[indx] = img->h;

                    bool with_alpha = false;
                    for (int y = 0; y < img->h && !with_alpha; ++y)
                    {
                        for (int x = 0; x < img->w && !with_alpha; ++x)
                            with_alpha |= img->line[y][(x << 2) + 3] != 255;
                    }
                    if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
                        allegro_gl_set_texture_format(with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB);
                    else
                        allegro_gl_set_texture_format(with_alpha ? GL_RGBA8 : GL_RGB8);

                    allegro_gl_use_alpha_channel(with_alpha);
                    *i = gfx->make_texture(img,filter);
                    allegro_gl_use_alpha_channel(false);
                    gfx->save_texture_to_cache(cache_filename, *i, img->w, img->h);
                    destroy_bitmap(img);
                }
                else
                {
                    if (w) w[indx] = fw;
                    if (h) h[indx] = fh;
                }
            }
            delete[] data;
        }
    }



    GLuint Gaf::ToTexture(const String& filename, const String& imgname, int* w, int* h, const bool truecolor, const int filter)
    {
        String cache_filename;
        cache_filename << filename << "-" << imgname << ".bin";
        uint32 fw;
        uint32 fh;
        GLuint first_try = gfx->load_texture_from_cache(cache_filename, filter, &fw, &fh);

        if (first_try)
        {
            if (w)  *w = fw;
            if (h)  *h = fh;
            return first_try;
        }

        byte *data = HPIManager->PullFromHPI(filename);
        if (data)
        {
            sint32 idx = RawDataGetEntryIndex(data, imgname);
            if (idx != -1)
            {
                set_palette(pal); // Activate the palette
                BITMAP *img = Gaf::RawDataToBitmap(data, idx, 0, NULL, NULL, truecolor);
                if (img)
                {
                    if (w) *w = img->w;
                    if (h) *h = img->h;
                    bool with_alpha = false;

                    for (int y = 0; y < img->h && !with_alpha; ++y)
                    {
                        for (int x = 0; x < img->w && !with_alpha; ++x)
                            with_alpha |= img->line[y][(x << 2) + 3] != 255;
                    }
                    if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
                        allegro_gl_set_texture_format(with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB);
                    else
                        allegro_gl_set_texture_format(with_alpha ? GL_RGBA8 : GL_RGB8);

                    allegro_gl_use_alpha_channel(with_alpha);
                    GLuint gl_img = gfx->make_texture(img,filter);
                    allegro_gl_use_alpha_channel(false);
                    gfx->save_texture_to_cache(cache_filename, gl_img, img->w, img->h);

                    destroy_bitmap(img);
                    delete[] data;
                    return gl_img;
                }
            }
            delete[] data;
        }
        return 0;
    }



    String Gaf::RawDataGetEntryName(const byte *buf, int entry_idx)
    {
        LOG_ASSERT(buf != NULL);
        if (entry_idx < 0)
            return "";
        Gaf::Header header(buf);
        if (entry_idx >= header.Entries)
            return "";

        sint32 f_pos = 12;
        sint32 *pointers = new sint32[header.Entries];

        for (sint32 i = 0; i < header.Entries; ++i)
            pointers[i] = ((sint32*)buf)[3 + i];
        f_pos += header.Entries * 4;

        Gaf::Entry entry;
        entry.Frames   = ((sint16*)(buf + pointers[entry_idx]))[0];
        entry.Unknown1 = ((sint16*)(buf + pointers[entry_idx]))[1];
        entry.Unknown2 = ((sint32*)(buf + pointers[entry_idx]))[1];
        convertGAFCharToString(buf + pointers[entry_idx] + 8, entry.name);
        f_pos = pointers[entry_idx] + 40;

        delete[] pointers;
        return entry.name;
    }


    sint32 Gaf::RawDataGetEntryIndex(const byte* buf, const String& name)
    {
        LOG_ASSERT(buf != NULL);
        sint32 nb_entry = RawDataEntriesCount(buf);
        String cmpString = name;
        cmpString.toUpper();
        for (sint32 i = 0; i < nb_entry; ++i)
        {
            if (Gaf::RawDataGetEntryName(buf, i).toUpper() == cmpString)
                return i;
        }
        return -1;
    }



    sint32 Gaf::RawDataImageCount(const byte* buf, const int entry_idx)
    {
        LOG_ASSERT(buf != NULL);
        if (entry_idx < 0)
            return 0;
        Gaf::Header header(buf);
        if (entry_idx >= header.Entries)		// Si le fichier contient moins d'images que img_idx, il y a erreur
            return 0;

        sint32 f_pos = 12;
        sint32* pointers = new sint32[header.Entries];

        for (sint32 i = 0; i < header.Entries; ++i) // Lit la liste de pointeurs vers les objets du fichier
            pointers[i] = ((sint32*)buf)[3 + i];
        f_pos += header.Entries * 4;

        Gaf::Entry entry;
        entry.Frames   = ((sint16*)(buf + pointers[entry_idx]))[0];		// Lit l'en-tête de l'entrée
        entry.Unknown1 = ((sint16*)(buf + pointers[entry_idx]))[1];
        entry.Unknown2 = ((sint32*)(buf + pointers[entry_idx]))[1];
        convertGAFCharToString(buf + pointers[entry_idx] + 8, entry.name);
        f_pos = pointers[entry_idx] + 40;

        delete[] pointers;
        return entry.Frames;
    }



    BITMAP* Gaf::RawDataToBitmap(const byte* buf, const sint32 entry_idx, const sint32 img_idx, short* ofs_x, short* ofs_y, const bool truecolor)
    {
        LOG_ASSERT(buf != NULL);
        if (entry_idx < 0)
            return NULL;
        Gaf::Header header(buf);
        if (entry_idx >= header.Entries) // Si le fichier contient moins d'images que img_idx, il y a erreur
            return NULL;

        sint32 f_pos(12);
        sint32 *pointers = new sint32[header.Entries];

        for (sint32 i = 0; i < header.Entries; ++i) // Lit la liste de pointeurs vers les objets du fichier
            pointers[i] = ((sint32*)buf)[3 + i];
        f_pos += header.Entries * 4;

        Gaf::Entry entry;
        entry.Frames   = ((sint16*)(buf + pointers[entry_idx]))[0];		// Lit l'en-tête de l'entrée
        entry.Unknown1 = ((sint16*)(buf + pointers[entry_idx]))[1];
        entry.Unknown2 = ((sint32*)(buf + pointers[entry_idx]))[1];
        convertGAFCharToString(buf + pointers[entry_idx] + 8, entry.name);
        f_pos = pointers[entry_idx]+40;

        Gaf::Frame::Entry* frame = new Gaf::Frame::Entry[entry.Frames];

        for (sint32 i = 0; i < entry.Frames; ++i)
        {
            frame[i].PtrFrameTable = *((sint32*)(buf+f_pos));
            f_pos += 4;
            frame[i].Unknown1 = *((sint32*)(buf+f_pos));
            f_pos += 4;
        }

        f_pos = frame[img_idx].PtrFrameTable;
        Gaf::Frame::Data framedata(buf, f_pos);
        uint32 *frames = (uint32*) (buf + framedata.PtrFrameData);

        if (ofs_x)
            *ofs_x = framedata.XPos;
        if (ofs_y)
            *ofs_y = framedata.YPos;

        sint32 nb_subframe = framedata.FramePointers;
        sint32 frame_x = framedata.XPos;
        sint32 frame_y = framedata.YPos;
        sint32 frame_w = framedata.Width;
        sint32 frame_h = framedata.Height;

        BITMAP *frame_img = NULL;

        if (header.IDVersion == TA3D_GAF_TRUECOLOR)
        {
            f_pos = framedata.PtrFrameData;
            sint32 img_size = 0;
            img_size = *((sint32*)(buf+f_pos));
            f_pos += 4;

            set_color_depth(32);
            frame_img = load_memory_jpg(buf + f_pos, img_size, NULL);
            f_pos += img_size;

            if (framedata.Transparency != 0 && frame_img != NULL) // Read alpha channel
            {
                img_size = *((sint32*)(buf+f_pos));
                f_pos += 4;
                BITMAP* img_alpha = load_memory_jpg(buf + f_pos, img_size, NULL);
                f_pos += img_size;
                if (img_alpha)
                {
                    if (bitmap_color_depth(frame_img) != 32)
                    {
                        BITMAP* tmp = create_bitmap_ex(32, frame_img->w, frame_img->h);
                        blit(frame_img, tmp, 0, 0, 0, 0, frame_img->w, frame_img->h);
                        destroy_bitmap(frame_img);
                        frame_img = tmp;
                    }
                    for (int y = 0; y < frame_img->h; ++y)
                    {
                        for (int x = 0; x < frame_img->w; ++x)
                        {
                            int c = getpixel(frame_img, x, y);
                            putpixel( frame_img, x, y, makeacol( getr(c), getg(c), getb(c), img_alpha->line[y][x<<2] ) );
                        }
                    }
                    destroy_bitmap(img_alpha);
                }
            }
        }
        else
        {
            for (int subframe = 0; subframe < nb_subframe || subframe < 1 ; ++subframe)
            {
                if (nb_subframe)
                {
                    f_pos = frames[subframe];
                    
                    framedata.Width  = *((sint16*)(buf+f_pos));	f_pos += 2;
                    framedata.Height = *((sint16*)(buf+f_pos));	f_pos += 2;
                    framedata.XPos   = *((sint16*)(buf+f_pos));	f_pos += 2;
                    framedata.YPos   = *((sint16*)(buf+f_pos));	f_pos += 2;

                    framedata.Transparency  = *((char*)(buf+f_pos));    f_pos += 1;
                    framedata.Compressed    = *((char*)(buf+f_pos));	f_pos += 1;
                    framedata.FramePointers = *((sint16*)(buf+f_pos));	f_pos += 2;
                    framedata.Unknown2      = *((sint32*)(buf+f_pos));	f_pos += 4;
                    framedata.PtrFrameData  = *((sint32*)(buf+f_pos));	f_pos += 4;
                    framedata.Unknown3      = *((sint32*)(buf+f_pos));	f_pos += 4;
                }

                BITMAP *img = NULL;

                if (framedata.Compressed) // Si l'image est comprimée
                {
                    LOG_ASSERT(framedata.Width  >= 0 && framedata.Width  < 4096);
                    LOG_ASSERT(framedata.Height >= 0 && framedata.Height < 4096);
                    if (!truecolor)
                    {
                        img = create_bitmap_ex(8, framedata.Width, framedata.Height);
                        clear(img);
                    }
                    else
                    {
                        img = create_bitmap_ex(32, framedata.Width, framedata.Height);
                        clear_to_color(img, 0);
                    }

                    sint16 length;
                    f_pos = framedata.PtrFrameData;
                    for (int i = 0; i < img->h; ++i) // Décode les lignes les unes après les autres
                    {
                        length = *((sint16*)(buf+f_pos));
                        f_pos += 2;
                        int x(0);
                        int e(0);
                        do
                        {
                            byte mask = buf[f_pos++];
                            ++e;
                            if (mask & 0x01)
                            {
                                if (!truecolor)
                                    x += mask >> 1;
                                else
                                {
                                    int l = mask >> 1;
                                    while (l > 0)
                                    {
                                        putpixel(img, x++, i, 0x00000000);
                                        --l;
                                    }
                                }
                            }
                            else
                            {
                                if (mask & 0x02)
                                {
                                    int l = (mask >> 2) + 1;
                                    while (l > 0)
                                    {
                                        if (!truecolor)
                                            img->line[i][x++] = buf[f_pos];
                                        else
                                            putpixel(img,x++,i,makeacol(pal[buf[f_pos]].r<<2,pal[buf[f_pos]].g<<2,pal[buf[f_pos]].b<<2,0xFF));
                                        --l;
                                    }
                                    ++f_pos;
                                    ++e;
                                }
                                else
                                {
                                    int l = (mask >> 2) + 1;
                                    while (l > 0)
                                    {
                                        if (truecolor)
                                        {
                                            putpixel(img, x++, i,
                                                     makeacol(pal[buf[f_pos]].r << 2, pal[buf[f_pos]].g << 2, pal[buf[f_pos]].b << 2, 0xFF));
                                            ++f_pos;
                                        }
                                        else
                                            img->line[i][x++]=buf[f_pos++];
                                        ++e;
                                        --l;
                                    }
                                }
                            }
                        } while (e < length && x < img->w);
                        f_pos += length-e;
                    }
                }
                else
                { 
                    // Si l'image n'est pas comprimée
                    img = create_bitmap_ex(8, framedata.Width, framedata.Height);
                    clear(img);

                    f_pos = framedata.PtrFrameData;
                    for (int i = 0; i < img->h; ++i) // Copie les octets de l'image
                    {
                        memcpy(img->line[i], buf + f_pos, img->w);
                        f_pos += img->w;
                    }

                    if (truecolor)
                    {
                        BITMAP* tmp = create_bitmap_ex(32, framedata.Width, framedata.Height);
                        blit(img, tmp, 0, 0, 0, 0, img->w, img->h);
                        for (int y = 0 ; y < tmp->h; ++y)
                        {
                            for (int x = 0; x < tmp->w; ++x)
                            {
                                if (img->line[y][x] == framedata.Transparency)
                                    ((uint32*)(tmp->line[y]))[x] = 0x00000000;
                                else
                                    ((uint32*)(tmp->line[y]))[x] |= makeacol(0,0,0, 0xFF);
                            }
                        }
                        destroy_bitmap(img);
                        img = tmp;
                    }
                }

                if (nb_subframe == 0)
                    frame_img = img;
                else
                {
                    if (subframe == 0)
                    {
                        if (!truecolor)
                        {
                            frame_img = create_bitmap_ex(8,frame_w,frame_h);
                            clear(frame_img);
                        }
                        else
                        {
                            frame_img = create_bitmap_ex(32,frame_w,frame_h);
                            clear_to_color(frame_img,0);
                        }
                        draw_sprite( frame_img, img, frame_x - framedata.XPos, frame_y - framedata.YPos );
                    }
                    else
                    {
                        if (truecolor)
                        {
                            for (int y = 0; y < img->h; ++y)
                            {
                                int Y = y + frame_y - framedata.YPos;
                                if (Y < 0 || Y >= frame_img->h)
                                    continue;
                                int X = frame_x - framedata.XPos;
                                for (int x = 0; x < img->w; ++x)
                                {
                                    if (X >= 0 && X < frame_img->w)
                                    {
                                        int r = frame_img->line[Y][(X << 2)];
                                        int g = frame_img->line[Y][(X << 2) + 1];
                                        int b = frame_img->line[Y][(X << 2) + 2];

                                        int r2 = img->line[y][(x << 2)];
                                        int g2 = img->line[y][(x << 2) + 1];
                                        int b2 = img->line[y][(x << 2) + 2];
                                        int a2 = img->line[y][(x << 2) + 3];

                                        r = (r * (255 - a2) + r2 * a2) >> 8;
                                        g = (g * (255 - g2) + g2 * a2) >> 8;
                                        b = (b * (255 - b2) + b2 * a2) >> 8;

                                        frame_img->line[Y][(X << 2)] = r;
                                        frame_img->line[Y][(X << 2) + 1] = g;
                                        frame_img->line[Y][(X << 2) + 2] = b;
                                    }
                                    ++X;
                                }
                            }
                        }
                        else
                            masked_blit(img, frame_img, 0, 0, frame_x - framedata.XPos, frame_y - framedata.YPos, img->w, img->h );
                    }
                    destroy_bitmap(img);
                }
            }
        }

        delete[] pointers;
        delete[] frame;
        return frame_img;
    }



    void Gaf::Animation::loadGAFFromRawData(const byte *buf, const int entry_idx, const bool truecolor, const String& fname)
    {
        LOG_ASSERT(buf != NULL);
        if (entry_idx < 0 || !buf)
            return;
        filename = fname;

        nb_bmp = Gaf::RawDataImageCount(buf,entry_idx);

        bmp   = new BITMAP*[nb_bmp];
        glbmp = new GLuint[nb_bmp];
        ofs_x = new short[nb_bmp];
        ofs_y = new short[nb_bmp];
        w     = new short[nb_bmp];
        h     = new short[nb_bmp];
        name  = Gaf::RawDataGetEntryName(buf, entry_idx);
        pAnimationConverted = false;

        int i(0);
        int f(0);
        for (; i < nb_bmp; ++i)
        {
            if ((bmp[i-f] = Gaf::RawDataToBitmap(buf, entry_idx, i, &(ofs_x[i-f]), &(ofs_y[i-f]), truecolor)) != NULL)
            {
                w[i-f] = bmp[i-f]->w;
                h[i-f] = bmp[i-f]->h;
                if (!truecolor)
                {
                    BITMAP* tmp = create_bitmap(w[i-f], h[i-f]);
                    blit(bmp[i-f], tmp, 0,0,0,0, tmp->w, tmp->h);
                    destroy_bitmap(bmp[i-f]);
                    bmp[i-f] = tmp;
                }
            }
            else 
                ++f;
        }
        nb_bmp -= f;
    }



    void Gaf::Animation::init()
    {
        nb_bmp = 0;
        bmp = NULL;
        ofs_x = ofs_y = NULL;
        glbmp = NULL;
        w = h = NULL;
        pAnimationConverted = false;
        filename.clear();
        name.clear();
    }


    void Gaf::Animation::destroy()
    {
        filename.clear();
        name.clear();
        if (nb_bmp > 0)
        {
            for (int i = 0; i < nb_bmp; ++i)
            {
                if (bmp[i])
                    destroy_bitmap(bmp[i]);
                if (pAnimationConverted)
                    gfx->destroy_texture(glbmp[i]);
            }
        }
        if (w) delete[] w;
        if (h) delete[] h;
        if (ofs_x) delete[] ofs_x;
        if (ofs_y) delete[] ofs_y;
        delete[] bmp;
        delete[] glbmp;
        init();
    }

    void Gaf::Animation::clean()
    {
        for (int i = 0; i < nb_bmp; ++i) // Fait un peu le ménage
        {
            if (bmp[i])
                destroy_bitmap(bmp[i]);
            bmp[i] = NULL;
        }
        name.clear();
    }


    void Gaf::Animation::convert(bool NO_FILTER, bool COMPRESSED)
    {
        if (pAnimationConverted)
            return;
        pAnimationConverted = true;
        for (int i = 0; i < nb_bmp; ++i)
        {
            String cache_filename;
            cache_filename << filename << format("-%s-%d.bin", (name.empty() ? "none" : name.c_str()), i);

            if (!filename.empty())
                glbmp[i] = gfx->load_texture_from_cache(cache_filename, NO_FILTER ? FILTER_NONE : FILTER_TRILINEAR );
            else
                glbmp[i] = 0;

            if (!glbmp[i])
            {
                set_color_depth(32);
                BITMAP *tmp = create_bitmap(bmp[i]->w,bmp[i]->h);
                blit(bmp[i], tmp, 0,0,0,0, tmp->w,tmp->h);
                destroy_bitmap(bmp[i]);
                bmp[i] = tmp;
                if (g_useTextureCompression && COMPRESSED && lp_CONFIG->use_texture_compression)
                    allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
                else
                    allegro_gl_set_texture_format(GL_RGBA8);
                allegro_gl_use_alpha_channel(true);
                glbmp[i] = gfx->make_texture(bmp[i], NO_FILTER ? FILTER_NONE : FILTER_TRILINEAR );
                allegro_gl_use_alpha_channel(false);
                if (!filename.empty())
                    gfx->save_texture_to_cache(cache_filename, glbmp[i], bmp[i]->w, bmp[i]->h);
            }
        }
    }

    void Gaf::AnimationList::clear()
    {
        if (pList && pSize > 0)
            delete[] pList;
        pList = NULL;
        pSize = 0;
    }

    Gaf::AnimationList::~AnimationList()
    {
        if (pList && pSize > 0)
            delete[] pList;
    }


    sint32 Gaf::AnimationList::loadGAFFromRawData(const byte* buf, const bool doConvert, const String& fname)
    {
        if (buf != NULL)
        {
            pSize = Gaf::RawDataEntriesCount(buf);
            pList = new Gaf::Animation[pSize];
            for (int i = 0; i < pSize; ++i)
                pList[i].loadGAFFromRawData(buf, i, true, fname);
            if (doConvert)
                convert();
            return pSize;
        }
        return 0;
    }

    sint32 Gaf::AnimationList::findByName(const String& name) const
    {
        for (int i = 0; i < pSize; ++i)
        {
            if (pList[i].name == name)
                return i;
        }
        return -1;
    }


    void Gaf::AnimationList::clean()
    {
        for (int i = 0; i < pSize; ++i)
            pList[i].clean();
    }
    

    void Gaf::AnimationList::convert(const bool no_filter, const bool compressed)
    {
        for (int i = 0; i < pSize; ++i)
            pList[i].convert(no_filter, compressed);
    }


} // namespace TA3D

