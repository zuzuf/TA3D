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
**  File: TA3D_hpi.h
** Notes:
**   Cire: The original author of most of this code is Joe D, aka Kingbot
**           I modified it to be more complain with C++ as it was mostly
**           C based.  I wrapped it neatly in a class, and exposed only
**           those methods that the game engine need to access.
**         TODO: If this is to be accessed by multiple threads it should be
**           rewritten as it currently can not handle multiple calls.
**               HPIITEM might be expanded to keep a reference track of
**           how many times it gets asked to decode some file, if it
**           reaches some point it might just keep that 'data' in its
**           self and return it instead of having to decode each time.
*/

#ifndef __TA3D_UTILS_HPI_H__
# define __TA3D_UTILS_HPI_H__

# include <list>
# include <vector>
# include "misc/hash_table.h"




namespace TA3D
{
namespace UTILS
{
namespace HPI
{

#define HEX_HAPI 0x49504148
#define HPI_V1 0x00010000

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
    class cHPIHandler
    {
        // Member Declarations:
    private:
        #pragma pack(1) // Byte alignment.

        struct HPIVERSION
        {
            sint32 HPIMarker; /* Must be HEX_HAPI */
            sint32 Version;   /* Must be HPI_V1 */
        };

        struct HPIHEADER
        {
            sint32 DirectorySize; /* Directory size */
            sint32 Key;           /* Decode key */
            sint32 Start;         /* Directory offset */

            HPIHEADER() : DirectorySize(0), Key(0), Start(0) {}
        };

        struct HPIENTRY
        {
            sint32 NameOffset;
            sint32 CountOffset;
            sint8 Flag;
        };

        struct HPICHUNK
        {
            sint32 Marker;           /* always 0x48535153 (SQSH) */
            sint8  Unknown1;         /* I have no idea what these mean */
            sint8  CompMethod;       /* 1 = lz77, 2 = zlib */
            sint8  Encrypt;          /* Is the chunk encrypted? */
            sint32 CompressedSize;   /* the length of the compressed data */
            sint32 DecompressedSize; /* the length of the decompressed data */
            sint32 Checksum;         /* check sum */
        };
        union HPICHUNK_U             /* for strict-aliasing safety */
        {
        	HPICHUNK chunk;
        	byte bytes[sizeof(HPICHUNK)];
        };

        struct HPIFILEDATA
        {
            HPIHEADER   H1;
            sint8      Key;
            sint8      *Directory;
            FILE      *HPIFile;
            bool			priority;

            HPIFILEDATA(bool priority = false) : Key(0), Directory(0), HPIFile(0), priority(priority) {}
        };

        struct CACHEFILEDATA
        {
            uint32		length;
            byte		*data;
            String		name;
        };

        struct HPIITEM
        {
            HPIFILEDATA	*hfd;
            HPIENTRY		*E1;
            sint32		IsDir;
            sint8			*Name;
            sint32		Size;
        };
#pragma pack()

        // Member Functions:
    private:
        void AddArchive( const String &FileName, bool priority );
        void LocateAndReadFiles( const String &Path, const String &FileSearch, bool priority );
        void ProcessRoot( HPIFILEDATA *hfd, const String &StartPath, sint32 offset );
        void ProcessSubDir( HPIITEM *hi );

        sint32  ReadAndDecrypt( sint32 fpos, byte *buff, sint32 buffsize, HPIFILEDATA *HPIInfo );
        sint32 ZLibDecompress( byte *out, byte *in, HPICHUNK *Chunk );
        sint32 LZ77Decompress( byte *out, byte *in, HPICHUNK *Chunk );
        sint32 Decompress( byte *out, byte *in, HPICHUNK *Chunk );
        void CloseHPIFile( HPIFILEDATA *hfd );
        //   void cHPIHandler::CloseCurrentFile( void );
        byte *DecodeFileToMem( HPIITEM *hi , uint32 *file_length=NULL );
        byte *DecodeFileToMem_zone( HPIITEM *hi , uint32 start , uint32 length , uint32 *file_length=NULL );

    public:
        void SearchDirForArchives( const String &Path );

        void PutInCache( const String &FileName, uint32 FileSize, byte *data );
        CACHEFILEDATA *IsInCache( const String &FileName );
        byte *IsInDiskCache( const String &FileName, uint32 *p_FileSize = NULL );

        // constructor:
        cHPIHandler( const String &Path  )
        {
            m_Archive = new TA3D::UTILS::clpHashTable< HPIITEM *>( 16384, true );
            m_file_cache = new std::list< CACHEFILEDATA >;

            SearchDirForArchives( Path );
        }

        uint32 getFilelist(const String& s, std::list<String>& li);
        uint32 getFilelist(const String& s, std::vector<String>& li);

        // DeConstructor
        ~cHPIHandler();

#if defined( DEBUG )
        void ShowArchive(); // for debug only we don't need it.
#endif

        byte *PullFromHPI( const String &FileName , uint32 *file_length=NULL);
        byte *PullFromHPI_zone( const String &FileName , uint32 start , uint32 length , uint32 *file_length=NULL);
        bool Exists( const String &FileName);

        // Member Variables:
    private:
        String m_cDir;     // used when building dir structurs.

        String m_Path;		// used when looking for files in the real file system

        TA3D::UTILS::clpHashTable< HPIITEM * > *m_Archive;

        std::list< CACHEFILEDATA >	*m_file_cache;			// The cache is used to speed up things when a file is loaded multiple times

        // A list of HPIFILEDATA, needed only for cleanup.
        std::list< HPIFILEDATA * > m_HPIFiles;
    }; // class cHPIHandler;

    class TA3D_FILE					// a class to manage files in HPI as easily as if they were normal files (for reading only)
    {
    private:
        byte	*data;
        uint32	pos;
        uint32	length;
    public:

        TA3D_FILE()
            :data(NULL), pos(0), length(0)
        {}

        inline bool isopen()
        {
            return data != NULL;
        }

        inline void destroy()
        {
            if( data )	delete[] data;
            pos = 0;
            length = 0;
        }

        ~TA3D_FILE()
        {
            destroy();
        }

        void topen( const String &filename );

        inline char tgetc()
        {
            if( data == NULL || pos >= length )	return 0;
            return ((char*)data)[ pos++ ];
        }

        inline int tread( void *buf, int size )
        {
            if( data == NULL || pos >= length )	return 0;
            if( pos + size > length )
                size = length - pos;
            memcpy( buf, data+pos, size );
            pos += size;
            return size;
        }

        char *tgets( void *buf, int size );

        inline void tseek( int &offset )	{	pos += offset;	}

        inline bool teof()	{	return pos >= length;	}

        inline int tsize()	{	return (int)length;	}
    };

    TA3D_FILE	*ta3d_fopen( String filename );
    void		ta3d_fclose( TA3D_FILE *file );
    char		ta3d_fgetc( TA3D_FILE *file );
    int			ta3d_fread( void *buf, int size, TA3D_FILE *file );
    int			ta3d_fread( void *buf, int size, int repeat, TA3D_FILE *file );
    char		*ta3d_fgets( void *buf, int size, TA3D_FILE *file );
    void		ta3d_fseek( int offset, TA3D_FILE *file );
    bool		ta3d_feof( TA3D_FILE *file );
    int			ta3d_fsize( TA3D_FILE *file );

    bool        load_palette(RGB *pal, const char *filename = "palettes\\palette.pal");
        
} // namespace HPI


} // namespace utils
} // namespace TA3D


#endif // __TA3D_UTILS_HPI_H__
