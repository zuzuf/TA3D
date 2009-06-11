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
**  File: hpi.h
** Notes:
**   Zuzuf:  This module implements the HPI interface through the Archive layer
*/

#ifndef __TA3D_UTILS_VFS_HPI_H__
# define __TA3D_UTILS_VFS_HPI_H__

# include "archive.h"
# include <map>

# define HEX_HAPI 0x49504148
# define HPI_V1 0x00010000

namespace TA3D
{
    namespace UTILS
    {
        /*! \class Hpi
        **
        ** \brief abstract class defining the interface required to manipulate archives
        */
        class Hpi : public Archive
        {
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


# pragma pack()
        public:
            class HpiFile : public Archive::File
            {
            public:
                //!
                HPIENTRY  entry;
                //!
                uint64  size;
            public:
                inline void setName(const String &name)   {  Archive::File::name = name; }
                inline void setParent(Archive *parent)   {  Archive::File::parent = parent; }
            };
        public:
            //! Constructor
            Hpi(const String &filename);
            //! Destructor
            virtual ~Hpi();

            /*!
            ** \brief Loads an archive
            */
            virtual void open(const String& filename);

            /*!
            ** \brief Just close the opened archive
            */
            virtual void close();

            /*!
            ** \brief Return the list of all files in the archive
            */
            virtual void getFileList(std::list<File*> &lFiles);

            /*!
            ** \brief
            */
            virtual byte* readFile(const String& filename, uint32* file_length = NULL);
            virtual byte* readFile(const File *file, uint32* file_length = NULL);

            /*!
            ** \brief
            ** \param filename
            ** \param start
            ** \param length
            ** \return
            */
            virtual byte* readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length = NULL);
            virtual byte* readFileRange(const File *file, const uint32 start, const uint32 length, uint32 *file_length = NULL);

            /*!
            ** \brief returns true if using the cache is a good idea (real FS will return false)
            ** \return
            */
            virtual bool needsCaching();

        public:
            static void finder(String::List &fileList, const String &path);
            static Archive* loader(const String &filename);

        private:
            //!
            String m_cDir;
            //!
            HPIHEADER header;
            //!
            sint8 key;
            //!
            sint8* directory;
            //!
            FILE* HPIFile;
            //!
            std::map<String, HpiFile*> files;
        private:
            /*!
        ** \brief
        **
        ** \param hfd
        ** \param startPath
        ** \param offset
        */
            void processRoot(const String& startPath, const sint32 offset);

            /*!
        ** \brief
        */
            void processSubDir(HPIENTRY *base);

            /*!
        ** \brief
        **
        ** \param fpos
        ** \param buff
        ** \param buffsize
        ** \return
        */
            sint32 readAndDecrypt(const sint32 fpos, byte *buff, sint32 buffsize);

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
        ** \return
        */
            sint32 LZ77Decompress(byte *out, byte *in);

            /*!
        ** \brief
        **
        ** \param out
        ** \param in
        ** \param Chunk
        ** \return
        */
            sint32 decompress(byte *out, byte *in, HPICHUNK *Chunk);
        }; // class Hpi
    } // namespace utils
} // namespace TA3D


#endif // __TA3D_UTILS_VFS_HPI_H__
