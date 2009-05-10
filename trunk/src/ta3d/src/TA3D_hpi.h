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
# include "misc/string.h"


# define HEX_HAPI 0x49504148
# define HPI_V1 0x00010000

# ifndef MAX_PATH
#   define MAX_PATH 260
# endif



namespace TA3D
{
namespace UTILS
{
namespace HPI
{

    /*! \class cHPIHandler
    **
    ** \brief
    */
    class cHPIHandler
    {
    public:
        /*! \class CACHEFILEDATA
        **
        ** \brief
        */
        struct CACHEFILEDATA
        {
            uint32		length;
            byte		*data;
            String		name;
        }; // class CACHEFILEDATA

    public:
        //! \name Constructor & Destructor
        //@{
        // constructor:
        cHPIHandler();
        cHPIHandler(const String &archiveName);

        //! Destructor
        ~cHPIHandler();
        //@}

        /*!
        ** \brief
        ** \param path
        */
        void SearchDirForArchives(const String& path);

        /*!
        ** \brief
        ** \param filename
        ** \param filesize
        ** /param data
        */
        void PutInCache(const String& filename, const uint32 filesize, const byte* data);

        /*!
        ** \brief
        ** \param filename
        */
        CACHEFILEDATA* IsInCache(const String& filename);

        /*!
        ** \brief
        ** \param filename
        ** \param filesize
        */
        byte *IsInDiskCache(const String& filename, uint32* filesize = NULL);

        /*!
        ** \brief
        ** \param s
        ** \param[out] li
        */
        uint32 getFilelist(const String& s, String::List& li);
        uint32 getFilelist(const String& s, String::Vector& li);

        /*!
        ** \brief
        */
        byte* PullFromHPI(const String& filename, uint32* file_length = NULL);

        /*!
        ** \brief
        ** \param filename
        ** \param start
        ** \param length
        ** \return
        */
        byte* PullFromHPI_zone(const String& filename, const uint32 start, const uint32 length, uint32 *file_length = NULL);

        /*!
        ** \brief
        ** \param filename
        ** \return
        */
        bool Exists(const String& filename);

        # if defined(DEBUG)
        /*!
        ** \brief
        */
        void ShowArchive(); // for debug only we don't need it.
        # endif


    private:
        # pragma pack(1) // Byte alignment.

        /*! \class HPIVERSION
        **
        ** \brief
        */
        struct HPIVERSION
        {
            //!
            sint32 HPIMarker; /* Must be HEX_HAPI */
            //!
            sint32 Version;   /* Must be HPI_V1 */
        };

        /*! \class HPIHEADER
        **
        ** \brief
        */
        struct HPIHEADER
        {
            //!
            HPIHEADER() :DirectorySize(0), Key(0), Start(0) {}

            //!
            sint32 DirectorySize; /* Directory size */
            //!
            sint32 Key;           /* Decode key */
            //!
            sint32 Start;         /* Directory offset */

        }; // class HPIHEADER


        /*! \class HPIENTRY
        **
        ** \brief
        */
        struct HPIENTRY
        {
            //!
            sint32 NameOffset;
            //!
            sint32 CountOffset;
            //!
            sint8 Flag;

        }; // class HPIENTRY


        /*! \class HPICHUNK
        **
        ** \brief
        */
        struct HPICHUNK
        {
            //! always 0x48535153 (SQSH)
            sint32 Marker;
            //! I have no idea what these mean
            sint8  Unknown1;
            //! 1 = lz77, 2 = zlib
            sint8  CompMethod;
            //! Is the chunk encrypted ?
            sint8  Encrypt;
            //! The length of the compressed data
            sint32 CompressedSize;
            //! The length of the decompressed data
            sint32 DecompressedSize;
            //! Checksum
            sint32 Checksum;

        }; // class HPICHUNK


        /*!
        ** \brief Used for strict-aliasing safety
        */
        union HPICHUNK_U
        {
            //!
            HPICHUNK chunk;
            //!
            byte bytes[sizeof(HPICHUNK)];
        };


        /*! \class HPIFILEDATA
        **
        ** \brief
        */
        struct HPIFILEDATA
        {
            //! Constructor
            HPIFILEDATA(bool priority = false) :Key(0), Directory(0), HPIFile(0), priority(priority) {}

            //!
            HPIHEADER H1;
            //!
            sint8 Key;
            //!
            sint8* Directory;
            //!
            FILE* HPIFile;
            //!
            bool priority;

        }; // class HPIFILEDATA


        /*! \class HPIITEM
        **
        ** \brief
        */
        struct HPIITEM
        {
            //! Pointer to HPI data structure, if NULL then we have a real file here
            HPIFILEDATA*  hfd;
            //!
            HPIENTRY*  E1;
            //!
            sint32  IsDir;
            //!
            String  Name;
            //!
            uint64  Size;

        }; // class HPIITEM

        # pragma pack()

    private:
        /*!
        ** \brief Add files from the real file system
        */
        void AddRealFS();

        /*!
        ** \brief
        **
        ** \param filename
        ** \param priority
        */
        void AddArchive(const String& filename, const bool priority);

        /*!
        ** \brief
        **
        ** \param path
        ** \param filesearch
        ** \param priority
        */
        void LocateAndReadFiles(const String& path, const String &fileSearch, const bool priority);

        /*!
        ** \brief
        **
        ** \param hfd
        ** \param startPath
        ** \param offset
        */
        void ProcessRoot(HPIFILEDATA *hfd, const String& startPath, const sint32 offset);

        /*!
        ** \brief
        */
        void ProcessSubDir(HPIITEM* hi);

        /*!
        ** \brief
        **
        ** \param fpos
        ** \param buff
        ** \param buffsize
        ** \param HPIInfo
        ** \return
        */
        sint32 ReadAndDecrypt(const sint32 fpos, byte *buff, sint32 buffsize, HPIFILEDATA *HPIInfo);

        /*!
        ** \brief
        **
        ** \param[out] out
        ** \param[in] in
        ** \param Chunk
        ** \return
        */
        sint32 ZLibDecompress(byte *out, byte *in, HPICHUNK *Chunk);

        /*!
        ** \brief
        **
        ** \param out
        ** \param in
        ** \param Chunk
        ** \return
        */
        sint32 LZ77Decompress(byte *out, byte *in, HPICHUNK* Chunk);

        /*!
        ** \brief
        **
        ** \param out
        ** \param in
        ** \param Chunk
        ** \return
        */
        sint32 Decompress(byte *out, byte *in, HPICHUNK *Chunk);

        /*!
        ** \brief
        **
        ** \param hfd
        */
        void CloseHPIFile(HPIFILEDATA* hfd);

        /*!
        ** \brief
        **
        ** \param hi
        ** \param fileLength
        ** \return
        */
        byte* DecodeFileToMem(HPIITEM* hi, uint32* fileLength = NULL);

        /*!
        ** \brief
        **
        ** \param hi
        ** \param start
        ** \param length
        ** \param fileLength
        ** \return
        */
        byte* DecodeFileToMem_zone(HPIITEM* hi, const uint32 start, const uint32 length, uint32* fileLength = NULL);


    private:
        //! used when building dir structurs
        String m_cDir;
        //! used when looking for files in the real file system
        String::Vector m_Path;
        //!
        TA3D::UTILS::clpHashTable< HPIITEM * > *m_Archive;

        //! The cache is used to speed up things when a file is loaded multiple times
        std::list< CACHEFILEDATA >	*m_file_cache;
        //! A list of HPIFILEDATA, needed only for cleanup.
        std::list< HPIFILEDATA * > m_HPIFiles;

    }; // class cHPIHandler;





    /*! \class TA3D_FILE
    **
    ** \brief Manage files in HPI as easily as if they were normal files
    ** (read-only)
    */
    class TA3D_FILE
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Constructor
        TA3D_FILE() :data(NULL), pos(0), length(0) {}
        //! Destructor
        ~TA3D_FILE() { destroy(); }
        //@}

        /*!
        ** \brief
        */
        bool isopen() const { return data != NULL; }

        /*!
        ** \brief
        */
        void destroy();

        /*!
        ** \brief
        **
        ** \param filename
        */
        void topen(const String& filename);

        /*!
        ** \brief
        */
        char tgetc();

        /*!
        ** \brief
        **
        ** \param buf
        ** \param size
        */
        int tread(void *buf, int size);

        /*!
        ** \brief
        **
        ** \param buf
        ** \param size
        */
        char* tgets(void *buf, int size);

        /*!
        ** \brief
        ** \param offset
        */
        void tseek(const int offset) { pos += offset; }

        /*!
        ** \brief
        ** \return
        */
        bool teof() const { return (pos >= length); }

        /*!
        ** \brief
        ** \return
        */
        int tsize()	const { return (int)length; }

    private:
        //!
        byte* data;
        //!
        uint32 pos;
        //!
        uint32 length;

    }; // class TA3D_FILE


    TA3D_FILE	*ta3d_fopen(const String& filename );
    void		ta3d_fclose( TA3D_FILE *file );
    char		ta3d_fgetc( TA3D_FILE *file );
    int			ta3d_fread( void *buf, int size, TA3D_FILE *file );
    int			ta3d_fread( void *buf, int size, int repeat, TA3D_FILE *file );
    char		*ta3d_fgets( void *buf, int size, TA3D_FILE *file );
    void		ta3d_fseek( int offset, TA3D_FILE *file );
    bool		ta3d_feof( TA3D_FILE *file );
    int			ta3d_fsize( TA3D_FILE *file );

    bool        load_palette(SDL_Color *pal, const String& filename = "palettes\\palette.pal");

} // namespace HPI


} // namespace utils
} // namespace TA3D


#endif // __TA3D_UTILS_HPI_H__
