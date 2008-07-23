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



    std::vector< GLuint > read_gaf_imgs( const String &filename, const String &imgname, int *w, int *h,bool truecol)		// Read a gaf image and put it in an OpenGL texture
    {
        std::vector< GLuint >	textures;
        textures.clear();

        byte *data = HPIManager->PullFromHPI( filename );
        if( data )
        {
            int idx = get_gaf_entry_index( data, (char*)imgname.c_str() );
            if( idx == -1 )
            {
                delete[] data;
                return textures;
            }

            set_palette(pal);      // Activate the palette

            int nb_img = get_gaf_nb_img( data, idx );
            if( nb_img <= 0 )
            {
#ifdef GAF_DEBUG_MODE
                Console->AddEntry("WARNING: error while reading %s!", filename.c_str() );
#endif
                delete[] data;
                return textures;
            }

            textures.resize( nb_img );

            for (unsigned int i = 0 ; i < textures.size() ; ++i)
            {
                uint32 fw, fh;
                String cache_filename = filename + "-" + imgname + format("-%d.bin", i );
                textures[ i ] = gfx->load_texture_from_cache( cache_filename, FILTER_TRILINEAR, &fw, &fh );

                if( !textures[ i ] )
                {
                    BITMAP *img = read_gaf_img( data, idx, i, NULL, NULL, truecol );

                    if( !img )
                    {
#ifdef GAF_DEBUG_MODE
                        Console->AddEntry("WARNING: could not read %s image from %s!", imgname.c_str(), filename.c_str() );
#endif
                        delete[] data;
                        return textures;
                    }

                    if( w )	w[i] = img->w;
                    if( h )	h[i] = img->h;

                    bool with_alpha = false;
                    for( int y = 0 ; y < img->h && !with_alpha ; y++ )
                        for( int x = 0 ; x < img->w && !with_alpha ; x++ )
                            with_alpha |= img->line[y][(x<<2)+3] != 255;
                    if(g_useTextureCompression)
                        allegro_gl_set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
                    else
                        allegro_gl_set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );

                    allegro_gl_use_alpha_channel( with_alpha );

                    textures[ i ] = gfx->make_texture( img );

                    allegro_gl_use_alpha_channel( false );

                    gfx->save_texture_to_cache( cache_filename, textures[ i ], img->w, img->h );

                    destroy_bitmap( img );
                }
                else {
                    if( w )	w[i] = fw;
                    if( h )	h[i] = fh;
                }
            }

            delete[] data;

            return textures;
        }
#ifdef GAF_DEBUG_MODE
        else
            Console->AddEntry("WARNING: could not open file %s", filename.c_str() );
#endif
        return textures;
    }

    GLuint	read_gaf_img( const String &filename, const String &imgname, int *w, int *h, bool truecol)		// Read a gaf image and put it in an OpenGL texture
    {
        String cache_filename = filename + "-" + imgname + ".bin";
        uint32 fw, fh;
        GLuint first_try = gfx->load_texture_from_cache( cache_filename, FILTER_TRILINEAR, &fw, &fh );

        if( first_try )
        {
            if( w )	*w = fw;
            if( h )	*h = fh;
            return first_try;
        }
        else
        {
            byte *data = HPIManager->PullFromHPI( filename );
            if( data )
            {
                int idx = get_gaf_entry_index( data, (char*)imgname.c_str() );
                if( idx == -1 ) {
                    delete[] data;
                    return 0;
                }

                set_palette(pal);      // Activate the palette

                BITMAP *img = read_gaf_img( data, idx, 0, NULL, NULL, truecol );

                if( !img )
                {
#ifdef GAF_DEBUG_MODE
                    Console->AddEntry("WARNING: could read %s image from %s!", imgname.c_str(), filename.c_str() );
#endif
                    delete[] data;
                    return 0;
                }

                if( w )	*w = img->w;
                if( h )	*h = img->h;

                bool with_alpha = false;
                for( int y = 0 ; y < img->h && !with_alpha ; y++ )
                    for( int x = 0 ; x < img->w && !with_alpha ; x++ )
                        with_alpha |= img->line[y][(x<<2)+3] != 255;
                if(g_useTextureCompression)
                    allegro_gl_set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
                else
                    allegro_gl_set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );

                allegro_gl_use_alpha_channel( with_alpha );

                GLuint gl_img = gfx->make_texture( img );

                allegro_gl_use_alpha_channel( false );

                gfx->save_texture_to_cache( cache_filename, gl_img, img->w, img->h );

                destroy_bitmap( img );
                delete[] data;

                return gl_img;
            }
#ifdef GAF_DEBUG_MODE
            else
                Console->AddEntry("WARNING: could not open file %s", filename.c_str() );
#endif
        }
        return 0;
    }

    int get_gaf_nb_entry(byte *buf)
    {
        GAFHEADER header;		// En-tête du fichier

        header.IDVersion=((int*)buf)[0];		// Lit l'en-tête
        header.Entries=((int*)buf)[1];
        header.Unknown1=((int*)buf)[2];

        return header.Entries;
    }

    char *get_gaf_entry_name(byte *buf,int entry_idx)
    {
        GAFHEADER header;		// En-tête du fichier

        header.IDVersion=((int*)buf)[0];		// Lit l'en-tête
        header.Entries=((int*)buf)[1];
        header.Unknown1=((int*)buf)[2];

        if(entry_idx>=header.Entries)		// Si le fichier contient moins d'images que img_idx, il y a erreur
            return NULL;

#ifdef GAF_DEBUG_MODE
        printf("nombre d'entrées dans le GAF: %d\n",header.Entries);
#endif

        int i;
        int f_pos=12;
        int *pointers = new int[header.Entries];

        for(i=0;i<header.Entries;i++)			// Lit la liste de pointeurs vers les objets du fichier
            pointers[i]=((int*)buf)[3+i];
        f_pos+=header.Entries*4;

        GAFENTRY entry;
        entry.Frames=((short*)(buf+pointers[entry_idx]))[0];		// Lit l'en-tête de l'entrée
        entry.Unknown1=((short*)(buf+pointers[entry_idx]))[1];
        entry.Unknown2=((int*)(buf+pointers[entry_idx]))[1];
        memcpy(entry.Name,buf+pointers[entry_idx]+8,32);
        f_pos=pointers[entry_idx]+40;

#ifdef GAF_DEBUG_MODE
        printf("nombre d'images dans l'entrée: %d\n",entry.Frames);
        printf("nom de l'entrée: %s\n",entry.Name);
#endif

        delete[] pointers;
        return strdup(entry.Name);
    }

    int get_gaf_entry_index(byte *buf,const char *name)
    {
        int nb_entry=get_gaf_nb_entry(buf);

        for (int i = 0; i < nb_entry; ++i)
        {
            char *entry_name=get_gaf_entry_name(buf,i);
            if(strcasecmp(entry_name,name)==0)
            {
                free(entry_name);
                return i;
            }
            free(entry_name);
        }
        return -1;
    }

    int get_gaf_nb_img(byte *buf,int entry_idx)
    {
        GAFHEADER header;		// En-tête du fichier

        header.IDVersion=((int*)buf)[0];		// Lit l'en-tête
        header.Entries=((int*)buf)[1];
        header.Unknown1=((int*)buf)[2];

        if(entry_idx>=header.Entries)		// Si le fichier contient moins d'images que img_idx, il y a erreur
            return 0;

#ifdef GAF_DEBUG_MODE
        printf("nombre d'entrées dans le GAF: %d\n",header.Entries);
#endif

        int i;
        int f_pos=12;
        int *pointers=new int[header.Entries];

        for(i=0;i<header.Entries;i++)			// Lit la liste de pointeurs vers les objets du fichier
            pointers[i]=((int*)buf)[3+i];
        f_pos+=header.Entries*4;

        GAFENTRY entry;
        entry.Frames=((short*)(buf+pointers[entry_idx]))[0];		// Lit l'en-tête de l'entrée
        entry.Unknown1=((short*)(buf+pointers[entry_idx]))[1];
        entry.Unknown2=((int*)(buf+pointers[entry_idx]))[1];
        memcpy(entry.Name,buf+pointers[entry_idx]+8,32);
        f_pos=pointers[entry_idx]+40;

#ifdef GAF_DEBUG_MODE
        printf("nombre d'images dans l'entrée: %d\n",entry.Frames);
        printf("nom de l'entrée: %s\n",entry.Name);
#endif

        delete[] pointers;
        return entry.Frames;
    }

    BITMAP *read_gaf_img(byte *buf,int entry_idx,int img_idx,short *ofs_x,short *ofs_y,bool truecol)			// Lit une image d'un fichier gaf en mémoire
    {
        GAFHEADER header;		// En-tête du fichier

        header.IDVersion=((int*)buf)[0];		// Lit l'en-tête
        header.Entries=((int*)buf)[1];
        header.Unknown1=((int*)buf)[2];

        if(entry_idx>=header.Entries)		// Si le fichier contient moins d'images que img_idx, il y a erreur
            return NULL;

#ifdef GAF_DEBUG_MODE
        printf("nombre d'entrées dans le GAF: %d\n",header.Entries);
#endif

        int i;
        int f_pos=12;
        int *pointers = new int[header.Entries];

        for(i=0;i<header.Entries;i++)			// Lit la liste de pointeurs vers les objets du fichier
            pointers[i]=((int*)buf)[3+i];
        f_pos+=header.Entries*4;

        GAFENTRY entry;
        entry.Frames=((short*)(buf+pointers[entry_idx]))[0];		// Lit l'en-tête de l'entrée
        entry.Unknown1=((short*)(buf+pointers[entry_idx]))[1];
        entry.Unknown2=((int*)(buf+pointers[entry_idx]))[1];
        memcpy(entry.Name,buf+pointers[entry_idx]+8,32);
        f_pos=pointers[entry_idx]+40;

#ifdef GAF_DEBUG_MODE
        printf("nombre d'images dans l'entrée: %d\n",entry.Frames);
        printf("nom de l'entrée: %s\n",entry.Name);
#endif

        GAFFRAMEENTRY *frame = new GAFFRAMEENTRY[entry.Frames];

        for (i=0;i<entry.Frames;i++)
        {
            frame[i].PtrFrameTable=*((int*)(buf+f_pos));
            f_pos+=4;
            frame[i].Unknown1=*((int*)(buf+f_pos));
            f_pos+=4;
        }

        f_pos=frame[img_idx].PtrFrameTable;

        GAFFRAMEDATA framedata;								// Lit les informations sur l'image
        framedata.Width=*((short*)(buf+f_pos));			f_pos+=2;
        framedata.Height=*((short*)(buf+f_pos));		f_pos+=2;
        framedata.XPos=*((short*)(buf+f_pos));			f_pos+=2;
        framedata.YPos=*((short*)(buf+f_pos));			f_pos+=2;
        framedata.Transparency=*((char*)(buf+f_pos));	f_pos+=1;
        framedata.Compressed=*((char*)(buf+f_pos));		f_pos+=1;
        framedata.FramePointers=*((short*)(buf+f_pos));	f_pos+=2;
        framedata.Unknown2=*((int*)(buf+f_pos));		f_pos+=4;
        framedata.PtrFrameData=*((int*)(buf+f_pos));	f_pos+=4;
        framedata.Unknown3=*((int*)(buf+f_pos));		f_pos+=4;

#ifdef GAF_DEBUG_MODE
        printf("taille de l'image: %dx%d\n",framedata.Width,framedata.Height);
        printf("position de l'image: (%d,%d)\n",framedata.XPos,framedata.YPos);
        printf("nombre de sous-images: %d\n",framedata.FramePointers);
        printf("framedata.Compressed=%d\n",framedata.Compressed);
#endif

        uint32 *frames = (uint32*) (buf + framedata.PtrFrameData);

        if(ofs_x)
            *ofs_x=framedata.XPos;
        if(ofs_y)
            *ofs_y=framedata.YPos;

        int nb_subframe = framedata.FramePointers;
        int frame_x = framedata.XPos;
        int frame_y = framedata.YPos;
        int frame_w = framedata.Width;
        int frame_h = framedata.Height;

        BITMAP *frame_img = NULL;

        if( header.IDVersion == GAF_TRUECOLOR )
        {
            f_pos = framedata.PtrFrameData;
            int img_size = 0;
            img_size = *((int*)(buf+f_pos));		f_pos+=4;

            set_color_depth( 32 );
            frame_img = load_memory_jpg( buf + f_pos, img_size, NULL );
            f_pos += img_size;

            if( framedata.Transparency != 0 && frame_img != NULL ) // Read alpha channel
            {
                img_size = *((int*)(buf+f_pos));		f_pos+=4;
                BITMAP *img_alpha = load_memory_jpg( buf + f_pos, img_size, NULL );	f_pos += img_size;
                if( img_alpha )
                {
                    if( bitmap_color_depth( frame_img ) != 32 )
                    {
                        BITMAP *tmp = create_bitmap_ex( 32, frame_img->w, frame_img->h );
                        blit( frame_img, tmp, 0, 0, 0, 0, frame_img->w, frame_img->h );
                        destroy_bitmap( frame_img );
                        frame_img = tmp;
                    }
                    set_color_depth( 32 );
                    for( int y = 0 ; y < frame_img->h ; y++ )
                        for( int x = 0 ; x < frame_img->w ; x++ )
                        {
                            int c = getpixel( frame_img, x, y );
                            putpixel( frame_img, x, y, makeacol( getr(c), getg(c), getb(c), img_alpha->line[y][x<<2] ) );
                        }
                    destroy_bitmap( img_alpha );
                }
            }
        }
        else
        {
            for( int subframe = 0 ; subframe < nb_subframe || subframe < 1 ; subframe++ )
            {
                if( nb_subframe ) {
                    f_pos = frames[ subframe ];
                    framedata.Width=*((short*)(buf+f_pos));			f_pos+=2;
                    framedata.Height=*((short*)(buf+f_pos));		f_pos+=2;
                    framedata.XPos=*((short*)(buf+f_pos));			f_pos+=2;
                    framedata.YPos=*((short*)(buf+f_pos));			f_pos+=2;
                    framedata.Transparency=*((char*)(buf+f_pos));	f_pos+=1;
                    framedata.Compressed=*((char*)(buf+f_pos));		f_pos+=1;
                    framedata.FramePointers=*((short*)(buf+f_pos));	f_pos+=2;
                    framedata.Unknown2=*((int*)(buf+f_pos));		f_pos+=4;
                    framedata.PtrFrameData=*((int*)(buf+f_pos));	f_pos+=4;
                    framedata.Unknown3=*((int*)(buf+f_pos));		f_pos+=4;
                }

                BITMAP *img = NULL;

                if (framedata.Compressed) // Si l'image est comprimée
                {
                    if(!truecol)
                    {
                        img = create_bitmap_ex(8,framedata.Width,framedata.Height);
                        clear(img);
                    }
                    else
                    {
                        img = create_bitmap_ex(32,framedata.Width,framedata.Height);
                        clear_to_color(img,0);
                    }

                    short length;
                    f_pos=framedata.PtrFrameData;
                    for(i=0;i<img->h;i++) // Décode les lignes les unes après les autres
                    {
                        length=*((short*)(buf+f_pos));
                        f_pos+=2;
                        int x=0,e=0;
                        do
                        {
                            byte mask=buf[f_pos++];
                            e++;
                            if(mask&0x01)
                            {
                                if(!truecol)
                                    x+=mask>>1;
                                else {
                                    int l=mask>>1;
                                    while(l>0) {
                                        putpixel(img,x++,i,0x00000000);
                                        l--;
                                    }
                                }
                            }
                            else
                            {
                                if(mask&0x02)
                                {
                                    int l=(mask>>2)+1;
                                    while(l>0)
                                    {
                                        if(!truecol)
                                            img->line[i][x++]=buf[f_pos];
                                        else
                                            putpixel(img,x++,i,makeacol(pal[buf[f_pos]].r<<2,pal[buf[f_pos]].g<<2,pal[buf[f_pos]].b<<2,0xFF));
                                        l--;
                                    }
                                    f_pos++;
                                    e++;
                                }
                                else
                                {
                                    int l=(mask>>2)+1;
                                    while(l>0)
                                    {
                                        if(truecol)
                                        {
                                            putpixel(img,x++,i,makeacol(pal[buf[f_pos]].r<<2,pal[buf[f_pos]].g<<2,pal[buf[f_pos]].b<<2,0xFF));
                                            f_pos++;
                                        }
                                        else
                                            img->line[i][x++]=buf[f_pos++];
                                        e++;
                                        l--;
                                    }
                                }
                            }
                        } while (e < length && x < img->w);
                        f_pos += length-e;
                    }
                }
                else {								// Si l'image n'est pas comprimée
                    img=create_bitmap_ex(8,framedata.Width,framedata.Height);
                    clear(img);

                    f_pos=framedata.PtrFrameData;
                    for(i=0;i<img->h;i++) // Copie les octets de l'image
                    {
                        memcpy(img->line[i],buf+f_pos,img->w);
                        f_pos+=img->w;
                    }

                    if(truecol)
                    {
                        BITMAP *tmp = create_bitmap_ex(32,framedata.Width,framedata.Height);
                        blit( img, tmp, 0, 0, 0, 0, img->w, img->h );
                        for( int y = 0 ; y < tmp->h ; y++ )
                        {
                            for( int x = 0 ; x < tmp->w ; x++ )
                            {
                                if( img->line[y][x] == framedata.Transparency )
                                    ((uint32*)(tmp->line[y]))[x] = 0x00000000;
                                else
                                    ((uint32*)(tmp->line[y]))[x] |= makeacol( 0,0,0, 0xFF );
                            }
                        }
                        destroy_bitmap( img );
                        img = tmp;
                    }
                }

                if( nb_subframe == 0 )
                    frame_img = img;
                else {
                    if( subframe == 0 )
                    {
                        if(!truecol)
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
                        if( truecol )
                        {
                            for( int y = 0 ; y < img->h ; y++ )
                            {
                                int Y = y + frame_y - framedata.YPos;
                                if( Y < 0 || Y >= frame_img->h )	continue;
                                int X = frame_x - framedata.XPos;
                                for( int x = 0 ; x < img->w ; x++ )
                                {
                                    if( X >= 0 && X < frame_img->w )
                                    {
                                        int r = frame_img->line[Y][(X<<2)];
                                        int g = frame_img->line[Y][(X<<2)+1];
                                        int b = frame_img->line[Y][(X<<2)+2];

                                        int r2 = img->line[y][(x<<2)];
                                        int g2 = img->line[y][(x<<2)+1];
                                        int b2 = img->line[y][(x<<2)+2];
                                        int a2 = img->line[y][(x<<2)+3];

                                        r = r * (255 - a2) + r2 * a2 >> 8;
                                        g = g * (255 - g2) + g2 * a2 >> 8;
                                        b = b * (255 - b2) + b2 * a2 >> 8;

                                        frame_img->line[Y][(X<<2)] = r;
                                        frame_img->line[Y][(X<<2)+1] = g;
                                        frame_img->line[Y][(X<<2)+2] = b;
                                    }
                                    X++;
                                }
                            }
                        }
                        else
                            masked_blit( img, frame_img, 0, 0, frame_x - framedata.XPos, frame_y - framedata.YPos, img->w, img->h );
                    }
                    destroy_bitmap( img );
                }
            }
        }

        delete[] pointers;
        delete[] frame;
        return frame_img;
    }

    void ANIM::load_gaf(byte *buf,int entry_idx,bool truecol,const char *fname)
    {
        if( fname )
            filename = strdup( fname );
        else
            filename = strdup( "" );

        nb_bmp=get_gaf_nb_img(buf,entry_idx);

        bmp=(BITMAP**) malloc(sizeof(BITMAP*)*nb_bmp);
        glbmp=(GLuint*) malloc(sizeof(GLuint)*nb_bmp);
        ofs_x=(short*) malloc(sizeof(short)*nb_bmp);
        ofs_y=(short*) malloc(sizeof(short)*nb_bmp);
        w=(short*) malloc(sizeof(short)*nb_bmp);
        h=(short*) malloc(sizeof(short)*nb_bmp);
        name=get_gaf_entry_name(buf,entry_idx);
        dgl=false;

        int i=0,f=0;
        for(;i<nb_bmp;i++)
        {
            if ((bmp[i-f]=read_gaf_img(buf,entry_idx,i,&(ofs_x[i-f]),&(ofs_y[i-f]),truecol))!=NULL)
            {
                w[i-f]=bmp[i-f]->w;
                h[i-f]=bmp[i-f]->h;
                if(!truecol)
                {
                    BITMAP *tmp=create_bitmap(w[i-f],h[i-f]);
                    blit(bmp[i-f],tmp,0,0,0,0,tmp->w,tmp->h);
                    destroy_bitmap(bmp[i-f]);
                    bmp[i-f]=tmp;
                }
            }
            else 
                f++;
        }
        nb_bmp -= f;
    }

    void ANIM::convert(bool NO_FILTER,bool COMPRESSED)
    {
        if( dgl )	return;			// Already done!!
        dgl=true;
        for (int i=0;i<nb_bmp;i++)
        {
            String cache_filename = filename + format("-%s-%d.bin", name ? name : "none", i );

            if( String( filename ) != "" )
                glbmp[i] = gfx->load_texture_from_cache( cache_filename, NO_FILTER ? FILTER_NONE : FILTER_TRILINEAR );
            else
                glbmp[i] = 0;

            if( !glbmp[i] )
            {
                set_color_depth(32);
                BITMAP *tmp=create_bitmap(bmp[i]->w,bmp[i]->h);
                blit(bmp[i],tmp,0,0,0,0,tmp->w,tmp->h);
                destroy_bitmap(bmp[i]);
                bmp[i]=tmp;
                if(g_useTextureCompression && COMPRESSED)
                    allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
                else
                    allegro_gl_set_texture_format(GL_RGBA8);
                allegro_gl_use_alpha_channel(true);
                glbmp[i]=gfx->make_texture(bmp[i], NO_FILTER ? FILTER_NONE : FILTER_TRILINEAR );
                allegro_gl_use_alpha_channel(false);
                if( filename != "" )
                    gfx->save_texture_to_cache( cache_filename, glbmp[i], bmp[i]->w, bmp[i]->h );
            }
        }
    }


} // namespace TA3D

