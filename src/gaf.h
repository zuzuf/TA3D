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
|                                         gaf.h                                      |
|  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
| des fichiers gaf de total annihilation qui sont les fichiers contenant les images  |
| et les animations du jeu.                                                          |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __GAF_CLASSES
#define __GAF_CLASSES

#define GAF_STANDARD	0x00010100
#define GAF_TRUECOLOR	0x00010101

# include "stdafx.h"
# include <vector>


namespace TA3D
{

    struct GAFHEADER
    {
        int		IDVersion;	/* Version stamp - always 0x00010100 */ // 0x00010101 is used for truecolor mode
        int		Entries;	/* Number of items contained in this file */
        int		Unknown1;	/* Always 0 */
    };

    struct GAFENTRY
    {
        short	Frames;		/* Number of frames in this entry */
        short	Unknown1;	/* Unknown - always 1 */
        int		Unknown2;	/* Unknown - always 0 */
        char	Name[32];	/* Name of the entry */
    };

    struct GAFFRAMEENTRY
    {
        int		PtrFrameTable;	/* Pointer to frame data */
        int		Unknown1;		/* Unknown - varies */
    };

    struct GAFFRAMEDATA
    {
        short	Width;			/* Width of the frame */
        short	Height;			/* Height of the frame */
        short	XPos;			/* X offset */
        short	YPos;			/* Y offset */
        char	Transparency;	/* Transparency color for uncompressed images - always 9 */	// In truecolor mode : alpha channel present
        char	Compressed;		/* Compression flag */	// Useless in truecolor mode
        short	FramePointers;	/* Count of subframes */
        int		Unknown2;		/* Unknown - always 0 */
        int		PtrFrameData;	/* Pointer to pixels or subframes */
        int		Unknown3;		/* Unknown - value varies */
    };

    int get_gaf_nb_entry(byte *buf);

    char *get_gaf_entry_name(byte *buf,int entry_idx);

    int get_gaf_entry_index(byte *buf,const char *name);

    int get_gaf_nb_img(byte *buf,int entry_idx);

    BITMAP *read_gaf_img(byte *buf,int entry_idx,int img_idx,short *ofs_x=NULL,short *ofs_y=NULL,bool truecol=true);			// Lit une image d'un fichier gaf en mémoire

    GLuint	read_gaf_img( const String &filename, const String &imgname, int *w=NULL, int *h=NULL,bool truecol=true);		// Read a gaf image and put it in an OpenGL texture

    std::vector< GLuint >	read_gaf_imgs( const String &filename, const String &imgname, int *w=NULL, int *h=NULL,bool truecol=true);		// Read a gaf image and put it in an OpenGL texture

    class ANIM			// Pour la lecture des fichiers GAF animés
    {
    public:
        int		nb_bmp;
        BITMAP	**bmp;
        short	*ofs_x;
        short	*ofs_y;
        short	*w,*h;
        GLuint	*glbmp;
        char	*name;
        bool	dgl;
        char	*filename;

        inline void init()
        {
            nb_bmp=0;
            bmp=NULL;
            ofs_x=ofs_y=NULL;
            glbmp=NULL;
            w=h=NULL;
            name=NULL;
            dgl=false;
            filename = NULL;
        }

        ANIM()
        {
            init();
        }

        inline void destroy()
        {
            if( filename )	free( filename );
            if(nb_bmp>0) {
                for(int i=0;i<nb_bmp;i++) {
                    if(bmp[i])
                        destroy_bitmap(bmp[i]);
                    if(dgl)
                        glDeleteTextures(1,&(glbmp[i]));
                }
                if(w)
                    free(w);
                if(h)
                    free(h);
                if(name)
                    free(name);
                if(ofs_x)
                    free(ofs_x);
                if(ofs_y)
                    free(ofs_y);
                free(bmp);
                free(glbmp);
            }
            init();
        }

        ~ANIM()
        {
            destroy();
        }

        void load_gaf(byte *buf,int entry_idx=0,bool truecol=true,const char *fname=NULL);

        void convert(bool NO_FILTER=false,bool COMPRESSED=false);

        void clean()
        {
            for(int i=0;i<nb_bmp;i++) {		// Fait un peu le ménage
                if(bmp[i])
                    destroy_bitmap(bmp[i]);
                bmp[i]=NULL;
            }
            if(name) {
                free(name);
                name=NULL;
            }
        }
    };

    class ANIMS			// Pour la lecture des fichiers GAF animés
    {
    public:
        int		nb_anim;
        ANIM	*anm;

        inline void init()
        {
            nb_anim=0;
            anm=NULL;
        }

        ANIMS()
        {
            init();
        }

        inline void destroy()
        {
            if(nb_anim>0)
                delete[] anm;
            init();
        }

        ~ANIMS()
        {
            destroy();
        }

        void load_gaf( byte *buf, bool doConvert=false, const char *fname = NULL )
        {
            if( buf == NULL )	return;

            nb_anim=get_gaf_nb_entry(buf);

            anm = new ANIM[nb_anim];

            for( int i = 0 ; i < nb_anim ; i++ )
                anm[i].load_gaf(buf,i,true,fname);

            if( doConvert )
                convert();
        }

        void convert(bool no_filter=false,bool compressed=false)
        {
            for(int i=0;i<nb_anim;i++)
                anm[i].convert(no_filter,compressed);
        }

        void clean()
        {
            for(int i=0;i<nb_anim;i++)
                anm[i].clean();
        }

        int find_entry(const char *name)
        {
            for(int i=0;i<nb_anim;i++)
                if(anm[i].name!=NULL && strcasecmp(anm[i].name,name)==0)
                    return i;
            return -1;
        }
    };


} // namespace TA3D


#endif
