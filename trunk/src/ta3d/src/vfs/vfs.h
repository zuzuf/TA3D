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
**  File: vfs.h
** Notes:
**   Zuzuf:  This module implements an abstraction layer over the filesystem
**           and the archives loaded by the game. It doesn't handle archives itself
**           it only maps a path/filename to the corresponding data.
*/

#ifndef __TA3D_UTILS_VFS_H__
# define __TA3D_UTILS_VFS_H__

# include <list>
# include <vector>
# include "archive.h"
# include "../misc/hash_table.h"
# include "../misc/string.h"
# include "../sdl.h"
# include "../threads/mutex.h"

# ifndef MAX_PATH
#   define MAX_PATH 260
# endif



namespace TA3D
{
    namespace UTILS
    {
        /*! \class VFS
        **
        ** \brief
        */
        class VFS
        {
        private:
            /*! \class CacheFileData
            **
            ** \brief
            */
            struct CacheFileData
            {
                uint32		length;
                byte		*data;
                String		name;
            }; // class CacheFileData

            Mutex mCache;

        private:
            //! \name Constructor & Destructor
            //@{
            // constructor:
            VFS();

            //! Destructor
            ~VFS();
            //@}
        public:

            /*!
            ** \brief reload all archives
            */
            void reload();

            /*!
            ** \brief
            ** \param path
            */
            void searchDirForArchives(const String& path);

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
            byte* readFile(const String& filename, uint32* file_length = NULL);

            /*!
            ** \brief
            ** \param filename
            ** \param start
            ** \param length
            ** \return
            */
            byte* readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length = NULL);

            /*!
            ** \brief
            ** \param filename
            ** \return
            */
            bool fileExists(const String& filename);

            /*!
            ** \brief returns the priority level of a file
            ** \param filename
            ** \return
            */
            int filePriority(const String& filename);

        private:
            /*!
            ** \brief load all archives
            */
            void load();

            /*!
            ** \brief unload all archives
            */
            void unload();

            /*!
            ** \brief
            **
            ** \param filename
            ** \param priority
            */
            void addArchive(const String& filename, const int priority);

            /*!
            ** \brief
            **
            ** \param path
            ** \param priority
            */
            void locateAndReadArchives(const String& path, const int priority);

            /*!
            ** \brief
            ** \param filename
            ** \param filesize
            ** /param data
            */
            void putInCache(const String& filename, const uint32 filesize, const byte* data);

            /*!
            ** \brief
            ** \param filename
            */
            CacheFileData* isInCache(const String& filename);

            /*!
            ** \brief
            ** \param filename
            ** \param filesize
            */
            byte *isInDiskCache(const String& filename, uint32* filesize = NULL);

        private:
            //! used when looking for files in the real file system
            String::Vector m_Path;
            //!
            TA3D::UTILS::clpHashTable< Archive::File* > *files;

            //! The cache is used to speed up things when a file is loaded multiple times
            std::list< CacheFileData >	fileCache;
            //! A list of Archive*, needed only for cleanup.
            std::list< Archive* > archives;

        private:
            static VFS *pInstance;
        public:
            static VFS *instance();
        }; // class VFS;





        /*! \class TA3D_FILE
        **
        ** \brief Manage files in VFS as easily as if they were normal files
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

        public:
            static bool Load(String::List& out, const String& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);
            static bool Load(String::Vector& out, const String& filename, const uint32 sizeLimit = 0, const bool emptyListBefore = true);
        }; // class TA3D_FILE


        TA3D_FILE	*ta3d_fopen(const String& filename);
        void		fclose( TA3D_FILE *file );
        char		fgetc( TA3D_FILE *file );
        int			fread( void *buf, int size, TA3D_FILE *file );
        int			fread( void *buf, int size, int repeat, TA3D_FILE *file );
        char		*fgets( void *buf, int size, TA3D_FILE *file );
        void		fseek( int offset, TA3D_FILE *file );
        bool		feof( TA3D_FILE *file );
        int			fsize( TA3D_FILE *file );

        bool        load_palette(SDL_Color *pal, const String& filename = "palettes\\palette.pal");

    } // namespace utils
} // namespace TA3D


#endif // __TA3D_UTILS_VFS_H__
