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
 **  File: TA3D_hpi.cpp
 ** Notes:
 **   Cire: See notes in TA3D_hpi.h
 */

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "misc/paths.h"
#include "logs/logs.h"
#include "misc/resources.h"

#include <zlib.h>
#if defined TA3D_PLATFORM_WINDOWS
#pragma comment(lib, "tools/win32/mingw32/libs/zlib.lib")
#endif



namespace TA3D
{

    namespace VARS
    {
        UTILS::HPI::cHPIHandler* HPIManager = NULL;
    }

namespace UTILS
{
namespace HPI
{


    sint32  cHPIHandler::ReadAndDecrypt(const sint32 fpos, byte *buff, const sint32 buffsize, HPIFILEDATA *HPIInfo)
    {
        sint32 count, result;
        fseek(HPIInfo->HPIFile, fpos, SEEK_SET);
        result = (sint32)fread(buff, buffsize, 1, HPIInfo->HPIFile);
        if (HPIInfo->Key)
        {
            for (count = 0; count < buffsize; ++count)
                buff[count] ^= (fpos + count) ^ HPIInfo->Key;
        }
        return result;
    }

    sint32 cHPIHandler::ZLibDecompress(byte *out, byte *in, HPICHUNK *Chunk)
    {
        z_stream zs;
        sint32 result;

        zs.next_in = in;
        zs.avail_in = Chunk->CompressedSize;
        zs.total_in = 0;

        zs.next_out = out;
        zs.avail_out = Chunk->DecompressedSize;
        zs.total_out = 0;

        zs.msg = NULL;
        zs.state = NULL;
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = NULL;

        zs.data_type = Z_BINARY;
        zs.adler = 0;
        zs.reserved = 0;

        result = inflateInit(&zs);
        if (result != Z_OK)
        {
            //printf("Error on inflateInit %d\nMessage: %s\n", result, zs.msg);
            return 0;
        }

        result = inflate(&zs, Z_FINISH);
        if (result != Z_STREAM_END)
        {
            //printf("Error on inflate %d\nMessage: %s\n", result, zs.msg);
            zs.total_out = 0;
        }

        result = inflateEnd(&zs);
        if (result != Z_OK)
        {
            //printf("Error on inflateEnd %d\nMessage: %s\n", result, zs.msg);
            return 0;
        }
        return zs.total_out;
    }

    sint32 cHPIHandler::LZ77Decompress(byte *out, byte *in, HPICHUNK* Chunk)
    {
        sint32 x, work1, work2, work3, inptr, outptr, done, DPtr;
        schar DBuff[4096];

        done = 0;
        inptr = 0;
        outptr = 0;
        work1 = 1;
        work2 = 1;
        work3 = in[inptr++];

        while (!done)
        {
            if ((work2 & work3) == 0)
            {
                out[outptr++] = in[inptr];
                DBuff[work1] = in[inptr];
                work1 = (work1 + 1) & 0xFFF;
                ++inptr;
            }
            else
            { int
                count = *((uint16 *) (in+inptr));
                inptr += 2;
                DPtr = count >> 4;
                if (DPtr == 0)
                    return outptr;
                else
                {
                    count = (count & 0x0f) + 2;
                    if (count >= 0)
                    {
                        for (x = 0; x < count; ++x)
                        {
                            out[outptr++] = DBuff[DPtr];
                            DBuff[work1] = DBuff[DPtr];
                            DPtr = (DPtr + 1) & 0xFFF;
                            work1 = (work1 + 1) & 0xFFF;
                        }

                    }
                }
            }
            work2 <<= 1;
            if (work2 & 0x0100)
            {
                work2 = 1;
                work3 = in[inptr++];
            }
        }
        return outptr;
    }

    sint32 cHPIHandler::Decompress(byte *out, byte *in, HPICHUNK* Chunk)
    {
        sint32 Checksum(0);
        for (sint32 x = 0; x < Chunk->CompressedSize; ++x)
        {
            Checksum += (byte) in[x];
            if (Chunk->Encrypt)
                in[x] = (in[x] - x) ^ x;
        }

        if (Chunk->Checksum != Checksum)
        {
            // GlobalDebugger-> Checksum error! Calculated: 0x%X  Actual: 0x%X\n", Checksum, Chunk->Checksum);
            return 0;
        }

        switch (Chunk->CompMethod)
        {
            case 1 : return LZ77Decompress(out, in, Chunk);
            case 2 : return ZLibDecompress(out, in, Chunk);
            default : return 0;
        }
    }

    byte *cHPIHandler::DecodeFileToMem(HPIITEM *hi, uint32 *fileLength)
    {
        sint32 DeCount,DeLen, x, WriteSize, WritePtr, Offset, Length, FileFlag, *DeSize;
        byte *DeBuff, *WriteBuff;
        HPIENTRY *Entry;
        HPICHUNK *Chunk;

        if (!hi)
            return NULL;

        Entry = hi->E1;

        Offset = *((sint32 *) (hi->hfd->Directory + Entry->CountOffset));
        Length = *((sint32 *) (hi->hfd->Directory + Entry->CountOffset + 4));
        FileFlag = *(hi->hfd->Directory + Entry->CountOffset + 8);

        if(fileLength)
            *fileLength=Length;

        WriteBuff = new byte[Length+1];

        WriteBuff[Length] = 0;

        if (FileFlag)
        {
            DeCount = Length >> 16;
            if (Length & 0xFFFF)
                DeCount++;

            DeSize = new sint32[DeCount];

            DeLen = DeCount * sizeof(sint32);

            ReadAndDecrypt(Offset, (byte *) DeSize, DeLen, hi->hfd );

            Offset += DeLen;

            WritePtr = 0;

            for (x = 0; x < DeCount; ++x)
            {
                Chunk = (HPICHUNK *) new byte[ DeSize[x] ];
                ReadAndDecrypt(Offset, (byte *) Chunk, DeSize[x], hi->hfd);
                Offset += DeSize[x];

                DeBuff = (byte *) (Chunk+1);

                WriteSize = Decompress(WriteBuff+WritePtr, DeBuff, Chunk);
                WritePtr += WriteSize;

                delete[] Chunk;
            }
            delete[] DeSize;
        }
        else
        {
            // file not compressed
            ReadAndDecrypt(Offset, WriteBuff, Length, hi->hfd );
        }

        return WriteBuff;
    }

    byte *cHPIHandler::DecodeFileToMem_zone(HPIITEM *hi, const uint32 start, const uint32 length, uint32* fileLength)
    {
        sint32 DeCount,DeLen, x, WriteSize, WritePtr, Offset, Length, FileFlag, *DeSize;
        byte *DeBuff, *WriteBuff;
        HPIENTRY *Entry;

        if (!hi)
            return NULL;

        Entry = hi->E1;

        Offset = *((sint32 *) (hi->hfd->Directory + Entry->CountOffset));
        Length = *((sint32 *) (hi->hfd->Directory + Entry->CountOffset + 4));
        FileFlag = *(hi->hfd->Directory + Entry->CountOffset + 8);

        if(fileLength)
            *fileLength = Length;

        WriteBuff = new byte[Length+1];

        WriteBuff[Length] = 0;

        if (FileFlag)
        {
            DeCount = Length >> 16;
            if (Length & 0xFFFF)
                DeCount++;

            DeSize = new sint32 [ DeCount ];
            DeLen = DeCount * sizeof(sint32);

            ReadAndDecrypt(Offset, (byte *) DeSize, DeLen, hi->hfd );
            Offset += DeLen;
            WritePtr = 0;

            for (x = 0; x < DeCount; ++x)
            {
                byte *ChunkBytes = new byte[ DeSize[x] ];
                ReadAndDecrypt(Offset, ChunkBytes, DeSize[x], hi->hfd);
                Offset += DeSize[x];

                HPICHUNK *Chunk = &((HPICHUNK_U*)ChunkBytes)->chunk; // strict-aliasing safe
                if((uint32)WritePtr>=start || WritePtr+Chunk->DecompressedSize>=(sint32)start) {

                    DeBuff = (byte *) (Chunk+1);

                    WriteSize = Decompress(WriteBuff+WritePtr, DeBuff, Chunk);
                    WritePtr += WriteSize;
                }
                else
                    WritePtr += Chunk->DecompressedSize;

                delete[] ChunkBytes;
                if(WritePtr>=(sint32)(start+length) )   break;
            }
            delete[] DeSize;
        }
        else {
            // file not compressed
            ReadAndDecrypt(Offset+start, WriteBuff+start, length, hi->hfd );
        }

        return WriteBuff;
    }

    void cHPIHandler::CloseHPIFile(HPIFILEDATA *hfd)
    {
        if (!hfd)
            return;

        if (hfd->Directory)
        {
            delete[] hfd->Directory;
            hfd->Directory = NULL;
        }

        if (hfd->HPIFile)
            fclose(hfd->HPIFile);

        hfd->HPIFile = NULL;
    }

    void cHPIHandler::ProcessSubDir( HPIITEM *hi )
    {
        sint32 *FileCount, *FileLength, *EntryOffset, *Entries, count;
        schar *Name;
        HPIENTRY *Base, *Entry;

        Base = hi->E1;
        if (Base)
            Entries = (sint32 *) (hi->hfd->Directory + Base->CountOffset);
        else
            Entries = (sint32 *) (hi->hfd->Directory + hi->hfd->H1.Start);

        EntryOffset = Entries + 1;
        Entry = (HPIENTRY *) (hi->hfd->Directory + *EntryOffset);

        for (count = 0; count < *Entries; ++count)
        {
            Name = hi->hfd->Directory + Entry->NameOffset;
            FileCount = (sint32 *) (hi->hfd->Directory + Entry->CountOffset);
            FileLength = FileCount + 1;

            HPIITEM *li = new HPIITEM;
            li->hfd = hi->hfd;
            li->Name = (char*)Name;
            li->E1 = Entry;

            if (Entry->Flag == 1)
            {
                String sDir = m_cDir; // save directory
                m_cDir << (char *)Name << "\\";

                ProcessSubDir(li);     // process new directory

                delete li;   // free the item.
                li = NULL;

                m_cDir = sDir; // restore dir with saved dir
            }
            else
            {
                String f(String::ToLower(m_cDir + (char *)Name));
                li->Size = *FileLength;
                m_Archive->insertOrUpdate(f, li);
            }
            ++Entry;
        }
    }

    void cHPIHandler::ProcessRoot(HPIFILEDATA *hfd, const String& startPath, const sint32 offset)
    {
        sint32 *Entries, *FileCount, *EntryOffset;
        schar *Name;
        String MyPath;

        Entries = (sint32 *)(hfd->Directory + offset);
        EntryOffset = Entries + 1;
        HPIENTRY *Entry = (HPIENTRY *)(hfd->Directory + *EntryOffset);

        for (sint32 count = 0; count < *Entries; ++count)
        {
            Name = hfd->Directory + Entry->NameOffset;
            FileCount = (sint32 *) (hfd->Directory + Entry->CountOffset);
            if (Entry->Flag == 1)
            {
                MyPath = startPath;
                if( MyPath.length() )
                    MyPath += "\\";
                MyPath += (char *)Name;
                m_cDir = MyPath + "\\";

                HPIITEM *hi = new HPIITEM;

                hi->hfd = hfd;
                hi->IsDir = 1;
                hi->Size = 0;
                hi->Name = (char*)Name;
                hi->E1 = Entry;

                ProcessSubDir( hi );

                delete hi;
                hi = NULL;
            }
            ++Entry;
        }
    }

    void cHPIHandler::AddArchive(const String& filename, const bool priority)
    {
        HPIFILEDATA *hfd = new HPIFILEDATA(priority);

        hfd->HPIFile = TA3D_OpenFile(filename, "rb");
        if (!hfd->HPIFile)
        {
            CloseHPIFile(hfd);
            delete hfd;
            //      GlobalDebugger-> Failed to open hpi file for reading.
            return;
        }

        HPIVERSION hv;
        fread(&hv, sizeof(HPIVERSION), 1, hfd->HPIFile);

        if (hv.Version != HPI_V1 || hv.HPIMarker != HEX_HAPI)
        {
            CloseHPIFile(hfd);
            delete hfd;
            return;
        }

        fread(&hfd->H1, sizeof(HPIHEADER), 1, hfd->HPIFile);
        if (hfd->H1.Key)
            hfd->Key = (hfd->H1.Key * 4) | (hfd->H1.Key >> 6);
        else
            hfd->Key = 0;

        int start = hfd->H1.Start;
        int size = hfd->H1.DirectorySize;

        hfd->Directory = new sint8 [size];

        ReadAndDecrypt(start, (byte *)hfd->Directory + start, size - start, hfd );
        m_cDir = "";
        m_HPIFiles.push_front( hfd );
        ProcessRoot(hfd, "", start);
    }


    void cHPIHandler::LocateAndReadFiles(const String& path, const String& filesearch, const bool priority)
    {
        String::List file_list;
        Paths::GlobFiles(file_list, filesearch);
        for(String::List::iterator i = file_list.begin() ; i != file_list.end() ; ++i)
            AddArchive( *i, priority || String::ToLower(Paths::ExtractFileName(*i)) == "ta3d.hpi" );
    }


    void cHPIHandler::SearchDirForArchives(const String& path)
    {
        schar ext[4][6] = { "*.ufo", "*.hpi", "*.ccx", "*.gp3" };

        for (short i = 0; i < 4; ++i)
            LocateAndReadFiles(path, path + (char *)ext[i], false );

        if (!TA3D_CURRENT_MOD.empty())
        {
            for (short i = 0; i < 4; ++i)
                LocateAndReadFiles(path + TA3D_CURRENT_MOD, path + TA3D_CURRENT_MOD + (char *)ext[i], true);
        }
    }


    // constructor:
    cHPIHandler::cHPIHandler()
    {
        m_Archive = new TA3D::UTILS::clpHashTable<HPIITEM*>(16384, true);
        m_file_cache = new std::list<CACHEFILEDATA>;

        m_Path = TA3D::Resources::GetPaths();
        if (!m_Path.empty())
        {
            for (String::Vector::iterator i = m_Path.begin(); i != m_Path.end(); ++i)
                SearchDirForArchives(*i);
        }
        else
            SearchDirForArchives("");
        AddRealFS();
    }

    // constructor used by the installer
    cHPIHandler::cHPIHandler(const String &path)
    {
        m_Archive = new TA3D::UTILS::clpHashTable<HPIITEM*>(16384, true);
        m_file_cache = new std::list<CACHEFILEDATA>;

        SearchDirForArchives(path);
        AddRealFS();
    }

    cHPIHandler::~cHPIHandler()
    {
        // Cleanup:
        //   First delete the hash, the hash table will erase all values
        // since we created it to manage values for us.
        if (m_Archive)
            delete m_Archive;
        m_Archive = NULL;

        if (m_file_cache)
        {
            std::list<CACHEFILEDATA>::iterator i;
            for (i = m_file_cache->begin() ; i != m_file_cache->end() ; ++i)
                delete i->data;
            delete m_file_cache;
        }
        m_file_cache = NULL;

        // Now close and free hpi files.
        if (m_HPIFiles.size() == 0)
            return;

        std::list< HPIFILEDATA * >::iterator cur;
        HPIFILEDATA *hfd;

        do
        {
            cur = m_HPIFiles.begin();
            hfd = (*cur);

            m_HPIFiles.pop_front();

            CloseHPIFile(hfd);
            delete hfd;
            hfd = NULL;
        } while (m_HPIFiles.size() != 0);
    }

    # if defined( DEBUG ) // no need for bloating in release builds.
    void cHPIHandler::ShowArchive()
    {
        for (vector< list<ELEMENT> >::iterator iter = m_Archive.table.begin() ; iter != m_Archive.table.end(); ++iter)
            for (list<ELEMENT>::iterator cur = iter->begin(); cur != iter->end() ; ++cur)
                printf( "(arch)%s\n", cur->key.c_str() );
    }
    # endif

    void cHPIHandler::PutInCache(const String& filename, const uint32 filesize, const byte* data)
    {
        if (!m_file_cache)
            m_file_cache = new std::list< CACHEFILEDATA >;

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

        if (m_file_cache->size() >= 10) // Cycle the data within the list
        {
            delete[] m_file_cache->front().data;
            m_file_cache->pop_front();
        }

        CACHEFILEDATA newentry;

        newentry.name = String::ToLower(filename);			// Store a copy of the data
        newentry.length = filesize;
        newentry.data = new byte[filesize];
        memcpy(newentry.data, data, filesize);

        m_file_cache->push_back(newentry);
    }



    byte* cHPIHandler::IsInDiskCache(const String& filename, uint32 *filesize)
    {
        // May be in cache but doesn't use cache (ie: campaign script)
        if (SearchString(filename, ".lua", true) >= 0)
            return NULL;

        String cacheable_filename = String::ToLower(filename);
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
                uint32 FileSize = FILE_SIZE(cache_filename.c_str());
                if (filesize)
                    *filesize = FileSize;

                byte *data = new byte[FileSize + 1];
                fread(data, FileSize, 1, cache_file);
                data[FileSize] = 0;
                fclose(cache_file);
                return data;
            }
        }
        return NULL;
    }

    cHPIHandler::CACHEFILEDATA* cHPIHandler::IsInCache(const String& filename)
    {
        if (!m_file_cache)
            return NULL;

        String key = String::ToLower(filename);
        std::list<CACHEFILEDATA>::iterator i;
        for (i = m_file_cache->begin() ; i != m_file_cache->end() ; ++i) // Check RAM Cache
        {
            if (i->name == key)
            {
                // Move this file to the end of the list to keep it in the cache longer
                m_file_cache->push_back(*i);
                m_file_cache->erase(i);
                return &m_file_cache->back();
            }
        }
        return NULL;
    }

    byte *cHPIHandler::PullFromHPI(const String& filename, uint32* fileLength)
    {
        String key = String::ToLower(filename);
        key.convertSlashesIntoAntiSlashes();
        uint32 FileSize;

        CACHEFILEDATA *cache = IsInCache(key);
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
            byte* data = IsInDiskCache( key, &FileSize );
            if( data )
            {
                if (fileLength)
                    *fileLength = FileSize;
                return data;
            }
        }

        HPIITEM *iterFind = m_Archive->find(key);

        if (iterFind != NULL)
        {
            if (iterFind->hfd == NULL)  // This is a real file
            {
                FILE* src = fopen(iterFind->Name.c_str(), "rb");
                if (src)
                {
                    FileSize = iterFind->Size;
                    byte *data = new byte[ FileSize + 1 ];
                    fread( data, FileSize, 1, src );
                    data[FileSize] = 0;				// NULL terminated
                    FileSize++;
                    fclose(src);
                    if (fileLength)
                        *fileLength = FileSize;
                    return data;
                }
            }
            else                        // The file is in an archive
            {
                byte *data = DecodeFileToMem( iterFind , &FileSize );
                PutInCache( key, FileSize, data );
                if (fileLength)
                    *fileLength = FileSize;
                return data;
            }
        }

        return NULL;
    }



    byte* cHPIHandler::PullFromHPI_zone(const String &filename, const uint32 start, const uint32 length, uint32 *fileLength)
    {
        String key = String::ToLower(filename);
        key.convertSlashesIntoAntiSlashes();
        uint32 FileSize;

        CACHEFILEDATA *cache = IsInCache(key);
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
            byte* data = IsInDiskCache( key, &FileSize );
            if( data )
            {
                if (fileLength)
                    *fileLength = FileSize;
                return data;
            }
        }

        HPIITEM* iterFind = m_Archive->find(key);
        if (iterFind && iterFind->hfd == NULL)
        {
            FILE* src = fopen(iterFind->Name.c_str(), "rb");
            if (src)
            {
                FileSize = iterFind->Size;
                byte* data = new byte [FileSize + 1];
                fread( data, FileSize, 1, src );
                data[ FileSize ] = 0;				// NULL terminated
                FileSize++;
                fclose( src );
                if (fileLength)
                    *fileLength = FileSize;
                return data;
            }
        }

        return ((iterFind != NULL) ? DecodeFileToMem_zone(iterFind, start, length, fileLength) : NULL);
    }

    void cHPIHandler::AddRealFS()
    {
        for(int e = 0 ; e < m_Path.size() ; e++)
        {
            String::Vector pathQueue;
            String root = m_Path[e];
            pathQueue.push_back( root );
            while(!pathQueue.empty())
            {
                String cur_Path = pathQueue.back();
                pathQueue.pop_back();

                if (cur_Path.empty())   continue;
                if (Paths::ExtractFileName(cur_Path).match(".*"))   continue;
                if (cur_Path[cur_Path.size() - 1] != '\\' && cur_Path[cur_Path.size() - 1] != '/')
                    cur_Path << '/';

                String::Vector fileList;
                Paths::GlobFiles(fileList,cur_Path + "*",true,true);
                Paths::GlobDirs(pathQueue,cur_Path + "*",false,false);

                for(int i = 0 ; i < fileList.size() ; i++)
                {
                    String filename = cur_Path + fileList[i];
                    String key = String::ToLower(cur_Path.substr(root.size(), cur_Path.size() - root.size()) + fileList[i]);
                    key.convertSlashesIntoAntiSlashes();

                    HPIITEM *iterFind = m_Archive->find(key);
                    if (iterFind && iterFind->hfd && iterFind->hfd->priority)   continue;
                    iterFind = new HPIITEM;
                    iterFind->E1 = NULL;
                    iterFind->hfd = NULL;
                    iterFind->IsDir = 0;
                    iterFind->Size = FILE_SIZE(filename);
                    iterFind->Name = filename;

                    m_Archive->insertOrUpdate(key, iterFind);
                }
            }
        }

        if (!TA3D_CURRENT_MOD.empty())
        {
            for(int e = 0 ; e < m_Path.size() ; e++)
            {
                String::Vector pathQueue;
                String root = m_Path[e] + TA3D_CURRENT_MOD;
                pathQueue.push_back( root );
                while(!pathQueue.empty())
                {
                    String cur_Path = pathQueue.back();
                    pathQueue.pop_back();

                    String::Vector fileList;
                    Paths::GlobFiles(fileList, cur_Path + "*", true, true);
                    Paths::GlobDirs(pathQueue, cur_Path + "*", false, false);

                    for(int i = 0 ; i < fileList.size() ; i++)
                    {
                        String filename = cur_Path + fileList[i];
                        String key = String::ToLower(cur_Path.substr(root.size(), cur_Path.size() - root.size()) + fileList[i]);
                        key.convertSlashesIntoAntiSlashes();
                        HPIITEM *iterFind;
                        iterFind = new HPIITEM;
                        iterFind->E1 = NULL;
                        iterFind->hfd = NULL;
                        iterFind->IsDir = 0;
                        iterFind->Size = FILE_SIZE(filename);
                        iterFind->Name = filename;

                        m_Archive->insertOrUpdate(filename, iterFind);
                    }
                }
            }
        }
    }


    bool cHPIHandler::Exists(const String& filename)
    {
        String key = String::ToLower(filename);
        key.convertSlashesIntoAntiSlashes();

        HPIITEM* iterFind = m_Archive->find(key);
        return (iterFind != NULL);
    }



    uint32 cHPIHandler::getFilelist(const String& s, String::Vector& li)
    {
        String::List l;
        uint32 r = getFilelist(s, l);
        for (String::List::const_iterator i = l.begin(); i != l.end(); ++i)
            li.push_back(*i);
        return r;
    }


    uint32 cHPIHandler::getFilelist(const String& s, String::List& li)
    {
        String pattern = String::ToLower(s).convertSlashesIntoAntiSlashes();
        return m_Archive->wildCardSearch(pattern, li);
    }




    void TA3D_FILE::topen(const String& filename)
    {
        destroy();

        if (NULL == HPIManager)
        {
            LOG_WARNING("TA3D_FILE used without HPIManager set ! No file opened.");
            return;
        }

        String win_filename(filename);
        win_filename.convertSlashesIntoAntiSlashes();

        data = HPIManager->PullFromHPI(win_filename, &length);
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



    void ta3d_fclose( TA3D_FILE *file )
    {
        if (file)
            delete file;
    }


    char ta3d_fgetc(TA3D_FILE *file)
    {
        return (file) ? file->tgetc() : 0;
    }


    int ta3d_fread(void *buf, int size, TA3D_FILE *file)
    {
        if( file )
            return file->tread( buf, size );
        return 0;
    }

    int ta3d_fread(void *buf, int size, int repeat, TA3D_FILE *file)
    {
        if( file )
            return file->tread( buf, size * repeat );
        return 0;
    }

    char* ta3d_fgets( void *buf, int size, TA3D_FILE *file )
    {
        if( file )
            return file->tgets( buf, size );
        return NULL;
    }

    void ta3d_fseek( int offset, TA3D_FILE *file )
    {
        if (file)
            file->tseek(offset);
    }

    bool ta3d_feof(TA3D_FILE *file)
    {
        return file ? file->teof() : true;
    }

    int ta3d_fsize(TA3D_FILE *file)
    {
        return file ? file->tsize() : 0;
    }


    bool load_palette(SDL_Color *pal, const String& filename)
    {
        byte* palette = HPIManager->PullFromHPI(filename);
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


} // namespace HPI
} // namespace UTILS
} // namespace TA3D
