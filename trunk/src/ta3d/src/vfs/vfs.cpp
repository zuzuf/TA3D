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
/*
 **  File: vfs.cpp
 ** Notes:
 **   Zuzuf: See notes in vfs.h
 */

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "../misc/paths.h"
#include "../logs/logs.h"
#include "../misc/resources.h"
#include "../misc/files.h"
#include "vfs.h"
#include <sstream>

#include <zlib.h>
#if defined TA3D_PLATFORM_WINDOWS
#pragma comment(lib, "tools/win32/mingw32/libs/zlib.lib")
#endif



namespace TA3D
{

    namespace UTILS
    {
        VFS *VFS::pInstance = NULL;
        VFS *VFS::instance()
        {
            if (!pInstance)
                pInstance = new VFS();
            return pInstance;
        }


        void VFS::addArchive(const String& filename, const int priority)
        {
            Archive *archive = Archive::load(filename);

            if (archive)
            {
                archives.push_back(archive);
                std::list<Archive::File*> archiveFiles;
                archive->getFileList(archiveFiles);
                for(std::list<Archive::File*>::iterator i = archiveFiles.begin() ; i != archiveFiles.end() ; ++i)
                {
                    (*i)->setPriority((*i)->getPriority() + priority);      // Update file priority

                    Archive::File *file = files->find((*i)->getName());
                    if ((file && file->getPriority() < (*i)->getPriority()) || file == NULL)
                        files->insertOrUpdate((*i)->getName(), *i);
                }
            }
            else
                LOG_ERROR(LOG_PREFIX_VFS << "could not load archive '" << filename << "'");
        }


        void VFS::locateAndReadArchives(const String& path, const int priority)
        {
            String::List fileList;
            Archive::getArchiveList(fileList, path);
            for(String::List::iterator i = fileList.begin() ; i != fileList.end() ; ++i)
                addArchive(*i, priority);
        }


        // constructor:
        VFS::VFS() : fileCache(), archives(), files(NULL), m_Path()
        {
            load();
        }

        VFS::~VFS()
        {
            unload();
        }

        void VFS::unload()
        {
            // Cleanup:
            //   First delete the hash, we don't need to delete the File objects since they are completely
            //   handled by the associated Archive classes
            if (files)
                delete files;
            files = NULL;

            for (std::list<CacheFileData>::iterator i = fileCache.begin() ; i != fileCache.end() ; ++i)
                delete i->data;
            fileCache.clear();

            // Now close and free archives.
            for(std::list<Archive*>::iterator i = archives.begin() ; i != archives.end() ; ++i)
                delete *i;
            archives.clear();
        }

        void VFS::load()
        {
            if (files)
                unload();

            files = new TA3D::UTILS::clpHashTable<Archive::File*>(16384, false);

            m_Path = TA3D::Resources::GetPaths();
            if (m_Path.empty())
                m_Path.push_back("");
            for (String::Vector::iterator i = m_Path.begin(); i != m_Path.end(); ++i)
                locateAndReadArchives(*i, 0);
            if (!TA3D_CURRENT_MOD.empty())
            {
                for (String::Vector::iterator i = m_Path.begin(); i != m_Path.end(); ++i)
                {
                    locateAndReadArchives(*i + TA3D_CURRENT_MOD, 0x10000);
                }
            }
        }

        void VFS::reload()
        {
            unload();
            load();
        }

        void VFS::putInCache(const String& filename, const uint32 filesize, const byte* data)
        {
            String cacheable_filename(filename);
            cacheable_filename.toLower();
            for (String::iterator i = cacheable_filename.begin(); i != cacheable_filename.end(); ++i)
            {
                if ('/' == *i || '\\' == *i)
                    *i = 'S';
            }

            String cache_filename;
            cache_filename << TA3D::Paths::Caches << cacheable_filename << ".dat"; // Save file in disk cache
            FILE* cache_file = TA3D_OpenFile(cache_filename, "wb");
            if (cache_file)
            {
                fwrite(data, filesize, 1, cache_file);
                fclose(cache_file);
            }

            if (filesize >= 0x100000)	// Don't store big files to prevent filling memory with cache data ;)
                return;

            if (fileCache.size() >= 10) // Cycle the data within the list
            {
                delete[] fileCache.front().data;
                fileCache.pop_front();
            }

            CacheFileData newentry;

            newentry.name = String::ToLower(filename);			// Store a copy of the data
            newentry.length = filesize;
            newentry.data = new byte[filesize];
            memcpy(newentry.data, data, filesize);

            fileCache.push_back(newentry);
        }



        byte* VFS::isInDiskCache(const String& filename, uint32 *filesize)
        {
            // May be in cache but doesn't use cache (ie: campaign script)
            if (SearchString(filename, ".lua", true) >= 0)
                return NULL;

            String cacheable_filename(filename);
            cacheable_filename.toLower();
            for (String::iterator i = cacheable_filename.begin() ; i != cacheable_filename.end(); ++i)
            {
                if ('/' == *i || '\\' == *i)
                    *i = 'S';
            }

            String cache_filename;
            cache_filename << TA3D::Paths::Caches << cacheable_filename << ".dat";

            if(TA3D::Paths::Exists(cache_filename)) // Check disk cache
            {
                FILE *cache_file = TA3D_OpenFile( cache_filename, "rb" );
                if (cache_file) // Load file from disk cache (faster than decompressing it)
                {
                    uint64 FileSize;
                    Paths::Files::Size(cache_filename, FileSize);
                    if (filesize)
                        *filesize = (uint32)FileSize;

                    byte *data = new byte[FileSize + 1];
                    fread(data, FileSize, 1, cache_file);
                    data[FileSize] = 0;
                    fclose(cache_file);
                    return data;
                }
            }
            return NULL;
        }

        VFS::CacheFileData* VFS::isInCache(const String& filename)
        {
            if (fileCache.empty())
                return NULL;

            String key = String::ToLower(filename);
            std::list<CacheFileData>::iterator i;
            for (i = fileCache.begin() ; i != fileCache.end() ; ++i) // Check RAM Cache
            {
                if (i->name == key)
                {
                    // Move this file to the end of the list to keep it in the cache longer
                    fileCache.push_back(*i);
                    fileCache.erase(i);
                    return &fileCache.back();
                }
            }
            return NULL;
        }

        byte *VFS::readFile(const String& filename, uint32* fileLength)
        {
            String key = String::ToLower(filename);
            key.convertSlashesIntoBackslashes();
            uint32 FileSize;

            CacheFileData *cache = isInCache(key);
            if (cache)
            {
                if (fileLength)
                    *fileLength = cache->length;
                byte *data = new byte[cache->length + 1];
                memcpy(data, cache->data, cache->length);
                data[cache->length] = 0;
                if (cache->length == 0)
                {
                    delete[] data;
                    return NULL;
                }
                return data;
            }
            else
            {
                byte* data = isInDiskCache( key, &FileSize );
                if( data )
                {
                    if (fileLength)
                        *fileLength = FileSize;
                    if (FileSize == 0)
                    {
                        delete[] data;
                        return NULL;
                    }
                    return data;
                }
            }

            Archive::File *file = files->find(key);

            if (file)
            {
                byte *data = file->read(&FileSize);
                if (file->needsCaching())
                    putInCache( key, FileSize, data );
                if (fileLength)
                    *fileLength = FileSize;
                if (FileSize == 0)
                {
                    delete[] data;
                    return NULL;
                }
                return data;
            }

            return NULL;
        }



        byte* VFS::readFileRange(const String &filename, const uint32 start, const uint32 length, uint32 *fileLength)
        {
            String key = String::ToLower(filename);
            key.convertSlashesIntoBackslashes();
            uint32 FileSize;

            CacheFileData *cache = isInCache(key);
            if (cache)
            {
                if (fileLength)
                    *fileLength = cache->length;
                byte *data = new byte[cache->length + 1];
                memcpy(data, cache->data, cache->length);
                data[cache->length] = 0;
                return data;
            }
            else
            {
                byte* data = isInDiskCache( key, &FileSize );
                if( data )
                {
                    if (fileLength)
                        *fileLength = FileSize;
                    return data;
                }
            }

            Archive::File *file = files->find(key);
            return file ? file->readRange(start, length, fileLength) : NULL;
        }


        bool VFS::fileExists(const String& filename)
        {
            String key = String::ToLower(filename);
            key.convertSlashesIntoBackslashes();

            Archive::File *file = files->find(key);
            return (file != NULL);
        }

        int VFS::filePriority(const String& filename)
        {
            String key = String::ToLower(filename);
            key.convertSlashesIntoBackslashes();

            Archive::File *file = files->find(key);
            return file ? file->getPriority() : -0xFFFFFF;     // If it doesn't exist it has a lower priority than anything else
        }


        uint32 VFS::getFilelist(const String& s, String::Vector& li)
        {
            String::List l;
            uint32 r = getFilelist(s, l);
            for (String::List::const_iterator i = l.begin(); i != l.end(); ++i)
                li.push_back(*i);
            return r;
        }


        uint32 VFS::getFilelist(const String& s, String::List& li)
        {
            String pattern(s);
            pattern.toLower();
            pattern.convertSlashesIntoBackslashes();
            return files->wildCardSearch(pattern, li);
        }




        void TA3D_FILE::topen(const String& filename)
        {
            destroy();

            String win_filename(filename);
            win_filename.convertSlashesIntoBackslashes();

            data = VFS::instance()->readFile(win_filename, &length);
            pos = 0;
        }

        char TA3D_FILE::tgetc()
        {
            if (data == NULL || pos >= length )
                return 0;
            return ((char*)data)[pos++];
        }


        char* TA3D_FILE::tgets(void *buf, int size)
        {
            if (data == NULL || pos >= length)
                return NULL;
            for (char* out_buf = (char*)buf; size > 1; --size)
            {
                *out_buf = tgetc();
                if (!*out_buf || *out_buf == '\n' || *out_buf == '\r')
                {
                    out_buf++;
                    *out_buf = 0;
                    return (char*)buf;
                }
                ++out_buf;
            }
            return NULL;
        }

        int TA3D_FILE::tread(void *buf, int size)
        {
            if (data == NULL || pos >= length)
                return 0;
            if (pos + size > length)
                size = length - pos;
            memcpy(buf, data + pos, size);
            pos += size;
            return size;
        }


        void TA3D_FILE::destroy()
        {
            if (data)
                delete[] data;
            pos = 0;
            length = 0;
        }




        TA3D_FILE* ta3d_fopen(const String& filename)
        {
            TA3D_FILE *file = new TA3D_FILE;

            if (file)
            {
                file->topen(filename);
                if (!file->isopen())
                {
                    delete file;
                    file = NULL;
                }
            }
            return file;
        }



        void fclose( TA3D_FILE *file )
        {
            if (file)
                delete file;
        }


        char fgetc(TA3D_FILE *file)
        {
            return (file) ? file->tgetc() : (char)0;
        }


        int fread(void *buf, int size, TA3D_FILE *file)
        {
            if( file )
                return file->tread( buf, size );
            return 0;
        }

        int fread(void *buf, int size, int repeat, TA3D_FILE *file)
        {
            if( file )
                return file->tread( buf, size * repeat );
            return 0;
        }

        char* fgets( void *buf, int size, TA3D_FILE *file )
        {
            if( file )
                return file->tgets( buf, size );
            return NULL;
        }

        void fseek( int offset, TA3D_FILE *file )
        {
            if (file)
                file->tseek(offset);
        }

        bool feof(TA3D_FILE *file)
        {
            return file ? file->teof() : true;
        }

        int fsize(TA3D_FILE *file)
        {
            return file ? file->tsize() : 0;
        }


        bool load_palette(SDL_Color *pal, const String& filename)
        {
            byte* palette = VFS::instance()->readFile(filename);
            if (palette == NULL)
                return false;

            for (int i = 0; i < 256; ++i)
            {
                pal[i].r = palette[i << 2];
                pal[i].g = palette[(i << 2) + 1];
                pal[i].b = palette[(i << 2) + 2];
            }
            delete[] palette;
            return true;
        }

        template<class T>
                bool TmplLoadFromFile(T& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
        {
            if (emptyListBefore)
                out.clear();

            uint32 file_length(0);
            byte *data = VFS::instance()->readFile(filename, &file_length);
            if (data == NULL)
            {
                LOG_WARNING("Impossible to open the file `" << filename << "`");
                return false;
            }
            if (sizeLimit && file_length > sizeLimit)
            {
                delete[] data;
                LOG_WARNING("Impossible to read the file `" << filename << "` (size > " << sizeLimit << ")");
                return false;
            }
            std::stringstream file;
            file.write((const char*)data, file_length);
            delete[] data;
            std::string line;
            while (std::getline(file, line))
                out.push_back(line);
            return true;
        }


        bool TA3D_FILE::Load(String::List& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
        {
            return TmplLoadFromFile< String::List >(out, filename, sizeLimit, emptyListBefore);
        }

        bool TA3D_FILE::Load(String::Vector& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
        {
            return TmplLoadFromFile< String::Vector >(out, filename, sizeLimit, emptyListBefore);
        }

    } // namespace UTILS
} // namespace TA3D
