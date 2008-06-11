/*
 **  File: TA3D_hpi.cpp
 ** Notes:
 **   Cire: See notes in TA3D_hpi.h
 */

#include "stdafx.h"
#include "TA3D_NameSpace.h"


#if defined TA3D_PLATFORM_WINDOWS
#include "tools/win32/include/zlib.h"
#pragma comment(lib, "tools/win32/libs/zlib.lib")
#else
#include <zlib.h>
#endif

using namespace std;
using namespace TA3D::UTILS::HPI;

namespace TA3D
{
    namespace VARS
    {
        cHPIHandler *HPIManager;
    }
}

sint32  cHPIHandler::ReadAndDecrypt( sint32 fpos, byte *buff, sint32 buffsize, HPIFILEDATA *HPIInfo)
{
    sint32 count, tkey, result;

    fseek(HPIInfo->HPIFile, fpos, SEEK_SET);

    result = (sint32)fread(buff, buffsize, 1, HPIInfo->HPIFile);

    if (HPIInfo->Key)
    {
        for (count = 0; count < buffsize; count++)
        {
            tkey = (fpos + count) ^ HPIInfo->Key;
            buff[count] = tkey ^ ~buff[count];
        }
    }

    return result;
}

sint32 cHPIHandler::ZLibDecompress( byte *out, byte *in, HPICHUNK *Chunk)
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
    if (result != Z_OK) {
        //printf("Error on inflateInit %d\nMessage: %s\n", result, zs.msg);
        return 0;
    }

    result = inflate(&zs, Z_FINISH);
    if (result != Z_STREAM_END) {
        //printf("Error on inflate %d\nMessage: %s\n", result, zs.msg);
        zs.total_out = 0;
    }

    result = inflateEnd(&zs);
    if (result != Z_OK) {
        //printf("Error on inflateEnd %d\nMessage: %s\n", result, zs.msg);
        return 0;
    }

    return zs.total_out;
}

sint32 cHPIHandler::LZ77Decompress( byte *out, byte *in, HPICHUNK *Chunk)
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
            inptr++;
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
                    for (x = 0; x < count; x++)
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

sint32 cHPIHandler::Decompress( byte *out, byte *in, HPICHUNK *Chunk )
{
    sint32 x, Checksum;

    Checksum = 0;
    for (x = 0; x < Chunk->CompressedSize; x++) {
        Checksum += (byte) in[x];
        if (Chunk->Encrypt)
            in[x] = (in[x] - x) ^ x;
    }

    if (Chunk->Checksum != Checksum)
    {
        // GlobalDebugger-> Checksum error! Calculated: 0x%X  Actual: 0x%X\n", Checksum, Chunk->Checksum);
        return 0;
    }

    switch (Chunk->CompMethod) {
        case 1 : return LZ77Decompress(out, in, Chunk);
        case 2 : return ZLibDecompress(out, in, Chunk);
        default : return 0;
    }
}

byte *cHPIHandler::DecodeFileToMem(HPIITEM *hi, uint32 *file_length)
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

    if(file_length)
        *file_length=Length;

    WriteBuff = (byte *)GetMem(Length+1, 0);
    if (!WriteBuff)
    {
        // NO NEED to global deubber this, get meme shoulda done it for us.
        return NULL;
    }
    WriteBuff[Length] = 0;

    if (FileFlag) {
        DeCount = Length >> 16;
        if (Length & 0xFFFF)
            DeCount++;
        DeLen = DeCount * sizeof(sint32);

        DeSize = (sint32 *)GetMem(DeLen, 0);

        ReadAndDecrypt(Offset, (byte *) DeSize, DeLen, hi->hfd );

        Offset += DeLen;

        WritePtr = 0;

        for (x = 0; x < DeCount; x++) {
            Chunk = (HPICHUNK *)GetMem(DeSize[x], 0);
            ReadAndDecrypt(Offset, (byte *) Chunk, DeSize[x], hi->hfd);
            Offset += DeSize[x];

            DeBuff = (byte *) (Chunk+1);

            WriteSize = Decompress(WriteBuff+WritePtr, DeBuff, Chunk);
            WritePtr += WriteSize;

            free(Chunk);
        }
        free(DeSize);
    }
    else {
        // file not compressed
        ReadAndDecrypt(Offset, WriteBuff, Length, hi->hfd );
    }

    return WriteBuff;
}

byte *cHPIHandler::DecodeFileToMem_zone(HPIITEM *hi , uint32 start , uint32 length , uint32 *file_length)
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

    if(file_length)
        *file_length=Length;

    WriteBuff = (byte *)GetMem(Length+1, 0);
    if (!WriteBuff)
    {
        // NO NEED to global deubber this, get meme shoulda done it for us.
        return NULL;
    }
    WriteBuff[Length] = 0;

    if (FileFlag) {
        DeCount = Length >> 16;
        if (Length & 0xFFFF)
            DeCount++;
        DeLen = DeCount * sizeof(sint32);

        DeSize = (sint32 *)GetMem(DeLen, 0);

        ReadAndDecrypt(Offset, (byte *) DeSize, DeLen, hi->hfd );

        Offset += DeLen;

        WritePtr = 0;

        for (x = 0; x < DeCount; x++) {
            Chunk = (HPICHUNK *)GetMem(DeSize[x], 0);
            ReadAndDecrypt(Offset, (byte *) Chunk, DeSize[x], hi->hfd);
            Offset += DeSize[x];

            if((uint32)WritePtr>=start || WritePtr+Chunk->DecompressedSize>=(sint32)start) {

                DeBuff = (byte *) (Chunk+1);

                WriteSize = Decompress(WriteBuff+WritePtr, DeBuff, Chunk);
                WritePtr += WriteSize;
            }
            else
                WritePtr += Chunk->DecompressedSize;

            free(Chunk);
            if(WritePtr>=(sint32)(start+length) )   break;
        }
        free(DeSize);
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
        free(hfd->Directory);
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
    HPIITEM *li;

    Base = hi->E1;
    if (Base)
        Entries = (sint32 *) (hi->hfd->Directory + Base->CountOffset);
    else
        Entries = (sint32 *) (hi->hfd->Directory + hi->hfd->H1.Start);

    EntryOffset = Entries + 1;
    Entry = (HPIENTRY *) (hi->hfd->Directory + *EntryOffset);

    for (count = 0; count < *Entries; count++) {
        Name = hi->hfd->Directory + Entry->NameOffset;
        FileCount = (sint32 *) (hi->hfd->Directory + Entry->CountOffset);
        FileLength = FileCount + 1;

        li = new HPIITEM;//(HPIITEM *)GetMem(sizeof(HPIITEM), 1);
        li->hfd = hi->hfd;
        li->Name = Name;
        li->E1 = Entry;

        if (Entry->Flag == 1)   
        {
            std::string sDir = m_cDir; // save directory
            m_cDir += (char *)Name;    // add new path to directory.
            m_cDir += "\\";          // don't forget to end path with /

            ProcessSubDir( li );     // process new directory

            delete li;   // free the item.
            li = NULL;   // really free the item.

            m_cDir = sDir; // restore dir with saved dir
        } else {
            std::string f = Lowercase( m_cDir + (char *)Name );

            li->Size = *FileLength;

            m_Archive->InsertOrUpdate( f, li );
        }

        Entry++;
    }
}

void  cHPIHandler::ProcessRoot(HPIFILEDATA *hfd, const std::string &StartPath, sint32 offset )
{
    sint32 *Entries, *FileCount, *EntryOffset;
    schar *Name;
    std::string MyPath;

    Entries = (sint32 *)(hfd->Directory + offset);
    EntryOffset = Entries + 1;
    HPIENTRY *Entry = (HPIENTRY *)(hfd->Directory + *EntryOffset);

    for( sint32 count = 0; count < *Entries; count++ )
    {
        Name = hfd->Directory + Entry->NameOffset;
        FileCount = (sint32 *) (hfd->Directory + Entry->CountOffset);
        if (Entry->Flag == 1)   
        {
            MyPath = StartPath;
            if( MyPath.length() )
                MyPath += "\\";
            MyPath += (char *)Name;
            m_cDir = MyPath + "\\";

            HPIITEM *hi = new HPIITEM;//(HPIITEM *)GetMem(sizeof(HPIITEM), 1);

            hi->hfd = hfd;
            hi->IsDir = 1;
            hi->Size = 0;
            hi->Name = Name;
            hi->E1 = Entry;

            ProcessSubDir( hi );

            delete hi;
            hi = NULL;
        }
        Entry++;
    }
}

void cHPIHandler::AddArchive( const std::string &FileName, bool priority )
{
    HPIFILEDATA *hfd = (HPIFILEDATA *)GetMem(sizeof(HPIFILEDATA), 1);

    hfd->priority = priority;

    if (!hfd)
        return; // No Need to generate error here as getmem already did it.

    hfd->HPIFile=TA3D_OpenFile( FileName.c_str(), "rb" );
    if( !hfd->HPIFile )
    {
        CloseHPIFile(hfd);
        free(hfd);
        //      GlobalDebugger-> Failed to open hpi file for reading.
        return;
    }

    HPIVERSION hv;
    fread(&hv, sizeof(HPIVERSION), 1, hfd->HPIFile);

    if( hv.Version != HPI_V1 || hv.HPIMarker != HEX_HAPI )
    {
        CloseHPIFile(hfd);
        free(hfd);

        hfd = NULL;

        return;
    }

    fread(&hfd->H1, sizeof(HPIHEADER), 1, hfd->HPIFile);
    if (hfd->H1.Key)
        hfd->Key = ~((hfd->H1.Key * 4)   | (hfd->H1.Key >> 6));
    else
        hfd->Key = 0;

    int start = hfd->H1.Start;
    int size = hfd->H1.DirectorySize;

    hfd->Directory = (sint8 *)GetMem(size, 1);

    ReadAndDecrypt(start, (byte *)hfd->Directory + start, size - start, hfd );
    m_cDir = "";

    m_HPIFiles.push_front( hfd );

    ProcessRoot(hfd, "", start);
}

void cHPIHandler::LocateAndReadFiles( const std::string &Path, const std::string &FileSearch, bool priority )
{
    al_ffblk search;

    if( al_findfirst(FileSearch.c_str(), &search, FA_RDONLY | FA_ARCH ) ==0 )
    {
        do
        {
            AddArchive( Path + search.name, priority || Lowercase( search.name ) == "ta3d.hpi" );
        } while( al_findnext( &search ) == 0 );

        al_findclose(&search);
    }
}

void cHPIHandler::SearchDirForArchives( const std::string &Path )
{
    m_Path = Path;
    schar ext[4][6] = { "*.ufo", "*.hpi", "*.ccx", "*.gp3" };

    for( uint32 i = 0; i < 4; i++ )
        LocateAndReadFiles( Path, Path + (char *)ext[i], false );

    if( TA3D_CURRENT_MOD != "" )
        for( uint32 i = 0; i < 4; i++ )
            LocateAndReadFiles( Path + TA3D_CURRENT_MOD, Path + TA3D_CURRENT_MOD + (char *)ext[i], true );
}

cHPIHandler::~cHPIHandler()
{
    // Cleanup:
    //   First delete the hash, the hash table will erase all values
    // since we created it to manage values for us.
    if( m_Archive )
        delete m_Archive;
    m_Archive = NULL;

    if( m_file_cache ) {
        for( int i = 0 ; i < m_file_cache->size() ; i++ ) {
            free( (*m_file_cache)[ i ].name );
            free( (*m_file_cache)[ i ].data );
        }
        delete m_file_cache;
    }
    m_file_cache = NULL;

    // Now close and free hpi files.
    if( m_HPIFiles.size() == 0 )
        return;

    std::list< HPIFILEDATA * >::iterator cur;
    HPIFILEDATA *hfd;

    do
    {
        cur = m_HPIFiles.begin();
        hfd = (*cur);

        m_HPIFiles.pop_front();

        CloseHPIFile( hfd );
        free( hfd );
        hfd = NULL;
    } while(  m_HPIFiles.size() != 0 );
}

#if defined( DEBUG ) // no need for bloating in release builds.
void cHPIHandler::ShowArchive()
{
    for( vector< list<ELEMENT> >::iterator iter = m_Archive.table.begin() ; iter != m_Archive.table.end() ; iter++ )
        for( list<ELEMENT>::iterator cur = iter->begin() ; cur != iter->end() ; cur++ )
            printf( "(arch)%s\n", cur->key.c_str() );
}
#endif

void cHPIHandler::PutInCache( const String &FileName, uint32 FileSize, byte *data )
{
    if( m_file_cache == NULL )
        m_file_cache = new std::vector< CACHEFILEDATA >;

    String cacheable_filename = Lowercase( FileName );

    for( int i = 0 ; i < cacheable_filename.size() ; i++ )
        if( cacheable_filename[ i ] == '/' )
            cacheable_filename[ i ] = 'S';

    String cache_filename = TA3D_OUTPUT_DIR + "cache/" + cacheable_filename + ".dat";		// Save file in disk cache
    FILE *cache_file = TA3D_OpenFile( cache_filename, "wb" );
    if( cache_file ) {
        fwrite( data, FileSize, 1, cache_file );
        fclose( cache_file );
    }

    if( FileSize >= 0x100000 )		return;			// Don't store big files to prevent filling memory with cache data ;)

    int idx = m_file_cache->size();

    if( m_file_cache->size() >= 10 ) {				// Cycle the data within the vector
        free( (*m_file_cache)[ 0 ].name );
        free( (*m_file_cache)[ 0 ].data );
        for( int i = 0 ; i < 9 ; i++ )
            (*m_file_cache)[ i ] = (*m_file_cache)[ i + 1 ];
        idx = m_file_cache->size() - 1;
    }

    if( m_file_cache->size() <= idx )
        m_file_cache->resize( m_file_cache->size() + 1 );

    (*m_file_cache)[ idx ].name = strdup( Lowercase( FileName ).c_str() );			// Store a copy of the data
    (*m_file_cache)[ idx ].length = FileSize;
    (*m_file_cache)[ idx ].data = new byte[ FileSize ];
    memcpy( (*m_file_cache)[ idx ].data, data, FileSize );
}

byte *cHPIHandler::IsInDiskCache(const String& FileName, uint32 *p_FileSize)
{
    // May be in cache but doesn't use cache (ie: campaign script)
    if(SearchString( FileName, ".lua", true) >= 0)
        return NULL;

    String cacheable_filename = Lowercase( FileName );
    for(String::iterator i = cacheable_filename.begin() ; i != cacheable_filename.end(); ++i)
    {
        if('/' == *i)
            *i = 'S';
    }

    String cache_filename = TA3D_OUTPUT_DIR + "cache/" + cacheable_filename + ".dat";

    if( TA3D::FileExists(cache_filename)) // Check disk cache
    {
        FILE *cache_file = TA3D_OpenFile( cache_filename, "rb" );
        if( cache_file ) // Load file from disk cache (faster than decompressing it)
        {
            uint32 FileSize = FILE_SIZE( cache_filename.c_str() );
            if( p_FileSize )
                *p_FileSize = FileSize;

            byte *data = new byte[ FileSize + 1 ];
            fread( data, FileSize, 1, cache_file );
            data[ FileSize ] = 0;
            fclose( cache_file );

            return data;
        }
    }
    return NULL;
}

TA3D::UTILS::HPI::cHPIHandler::CACHEFILEDATA *cHPIHandler::IsInCache( const String &FileName )
{
    if(!m_file_cache)
        return NULL;

    String key = Lowercase( FileName );
    for( int i = 0 ; i < m_file_cache->size() ; i++ )			// Check RAM Cache
    {
        if( (*m_file_cache)[ i ].name == key )
        {
            if( i < m_file_cache->size() - 1 )
            {
                CACHEFILEDATA tmp = (*m_file_cache)[ i ];					// Keep the last file at the end of the vector
                for( int e = i ; e < m_file_cache->size() - 1 ; e++ )
                    (*m_file_cache)[ e ] = (*m_file_cache)[ e + 1 ];
                i = m_file_cache->size() - 1;
                (*m_file_cache)[ i ] = tmp;
            }

            return &((*m_file_cache)[ i ]);
        }
    }
    return NULL;
}

byte *cHPIHandler::PullFromHPI( const std::string &FileName , uint32 *file_length )
{
    String UNIX_filename = m_Path + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';

    CACHEFILEDATA *cache_result = IsInCache( UNIX_filename );		// Look for it in the cache
    if( cache_result ) {
        byte *data = (byte*) malloc( cache_result->length );
        memcpy( data, cache_result->data, cache_result->length );
        if( file_length )
            *file_length = cache_result->length;
        return data;
    }

    uint32	FileSize;

    UNIX_filename = m_Path + TA3D_CURRENT_MOD + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';

    if( exists( UNIX_filename.c_str() ) ) {			// Current mod has priority
        FILE *src = TA3D_OpenFile( UNIX_filename.c_str(), "rb" );
        if( src ) {
            FileSize = FILE_SIZE( UNIX_filename.c_str() );
            byte *data = (byte*) malloc( FileSize + 1 );
            fread( data, FileSize, 1, src );
            data[ FileSize ] = 0;				// NULL terminated
            fclose( src );
            if( file_length )
                *file_length = FileSize;
            PutInCache( UNIX_filename, FileSize, data );
            return data;
        }
    }

    {
        byte 	*data = IsInDiskCache( UNIX_filename, &FileSize );
        if( data ) {
            if( file_length )	*file_length = FileSize;
            return data;
        }
    }

    HPIITEM *iterFind = m_Archive->Find( Lowercase( FileName ) );
    if( iterFind != NULL && iterFind->hfd->priority ) {		// Prioritary file!!
        byte *data = DecodeFileToMem( iterFind , &FileSize );
        PutInCache( UNIX_filename, FileSize, data );
        if( file_length )
            *file_length = FileSize;
        return data;
    }

    UNIX_filename = m_Path + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';

    if( exists( UNIX_filename.c_str() ) ) {
        FILE *src = TA3D_OpenFile( UNIX_filename.c_str(), "rb" );
        if( src ) {
            FileSize = FILE_SIZE( UNIX_filename.c_str() );
            byte *data = (byte*) malloc( FileSize + 1 );
            fread( data, FileSize, 1, src );
            data[ FileSize ] = 0;				// NULL terminated
            fclose( src );
            if( file_length )
                *file_length = FileSize;
            PutInCache( UNIX_filename, FileSize, data );
            return data;
        }
    }

    {
        byte 	*data = IsInDiskCache( UNIX_filename, &FileSize );
        if( data ) {
            if( file_length )	*file_length = FileSize;
            return data;
        }
    }

    if( iterFind != NULL ) {
        byte *data = DecodeFileToMem( iterFind , &FileSize );
        PutInCache( UNIX_filename, FileSize, data );
        if( file_length )
            *file_length = FileSize;
        return data;
    }

    return NULL;
}

byte *cHPIHandler::PullFromHPI_zone( const std::string &FileName , uint32 start , uint32 length , uint32 *file_length )
{
    String UNIX_filename = m_Path + TA3D_CURRENT_MOD + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';

    if( exists( UNIX_filename.c_str() ) ) {			// Current mod has priority
        FILE *src = TA3D_OpenFile( UNIX_filename.c_str(), "rb" );
        if( src ) {
            byte *data = (byte*) malloc( FILE_SIZE( UNIX_filename.c_str() ) + 1 );
            fread( data, FILE_SIZE( UNIX_filename.c_str() ), 1, src );
            data[ FILE_SIZE( UNIX_filename.c_str() ) ] = 0;				// NULL terminated
            fclose( src );
            if( file_length )
                *file_length = FILE_SIZE( UNIX_filename.c_str() );
            return data;
        }
    }

    HPIITEM* iterFind = m_Archive->Find( Lowercase( FileName ) );
    if( iterFind != NULL && iterFind->hfd->priority )				// Prioritary file!!
        return DecodeFileToMem_zone( iterFind , start , length , file_length );

    UNIX_filename = m_Path + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';

    if( exists( UNIX_filename.c_str() ) ) {
        FILE *src = TA3D_OpenFile( UNIX_filename.c_str(), "rb" );
        if( src ) {
            byte *data = (byte*) malloc( FILE_SIZE( UNIX_filename.c_str() ) + 1 );
            fread( data, FILE_SIZE( UNIX_filename.c_str() ), 1, src );
            data[ FILE_SIZE( UNIX_filename.c_str() ) ] = 0;				// NULL terminated
            fclose( src );
            if( file_length )
                *file_length = FILE_SIZE( UNIX_filename.c_str() );
            return data;
        }
    }

    return ( (iterFind != NULL) ? DecodeFileToMem_zone( iterFind , start , length , file_length ) : NULL );
}

bool cHPIHandler::Exists( const std::string &FileName )
{
    String UNIX_filename = m_Path + TA3D_CURRENT_MOD + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';
    if( exists( UNIX_filename.c_str() ) )	return true;

    UNIX_filename = m_Path + FileName;
    for( uint16 i = 0 ; i < UNIX_filename.size() ; i++ )
        if( UNIX_filename[i] == '\\' )
            UNIX_filename[i] = '/';
    if( exists( UNIX_filename.c_str() ) )	return true;

    HPIITEM *iterFind = m_Archive->Find( Lowercase( FileName ) );

    return (iterFind != NULL);
}

uint32 cHPIHandler::GetFilelist( const std::string &Search, std::list<std::string> *li )
{
    uint32 list_size = m_Archive->WildCardSearch( Search, li );
    al_ffblk info;

    String UNIX_search = m_Path + Search;
    for( uint16 i = 0 ; i < UNIX_search.size() ; i++ )
        if( UNIX_search[i] == '\\' )
            UNIX_search[i] = '/';

    if (al_findfirst(UNIX_search.c_str(), &info, FA_RDONLY | FA_ARCH ) == 0) {
        int last = -1;
        for( uint16 i = 0 ; i < UNIX_search.size() ; i++ )
            if( UNIX_search[i] == '/' ) {
                UNIX_search[i] = '\\';
                last = i;
            }
        if( last >= 0 )
            UNIX_search.resize( last + 1 );
        else
            UNIX_search = "";

        do {
            li->push_back( UNIX_search + info.name );
        } while (al_findnext(&info) == 0);

        al_findclose(&info);

        li->sort();
        li->unique();
        list_size = li->size();
    }

    if( TA3D_CURRENT_MOD != "" ) {
        UNIX_search = m_Path + TA3D_CURRENT_MOD + Search;
        for( uint16 i = 0 ; i < UNIX_search.size() ; i++ )
            if( UNIX_search[i] == '\\' )
                UNIX_search[i] = '/';

        if (al_findfirst( UNIX_search.c_str(), &info, FA_RDONLY | FA_ARCH ) == 0) {
            int last = -1;
            for( uint16 i = 0 ; i < UNIX_search.size() ; i++ )
                if( UNIX_search[i] == '/' ) {
                    UNIX_search[i] = '\\';
                    last = i;
                }
            if( last >= 0 )
                UNIX_search.resize( last + 1 );
            else
                UNIX_search = "";

            do {
                li->push_back( UNIX_search + info.name );
            } while (al_findnext(&info) == 0);

            al_findclose(&info);

            li->sort();
            li->unique();
            list_size = li->size();
        }
    }

    return list_size;
}

void TA3D_FILE::topen( const String &filename )
{
    destroy();

    if( HPIManager == NULL ) {
        Console->AddEntry( "Warning: TA3D_FILE used without HPIManager set!! No file opened" );
        return;
    }

    String win_filename;

    for( uint16 i = 0 ; i < filename.size() ; i++ )
        if( filename[ i ] == '/' )
            win_filename += '\\';
        else
            win_filename += filename[ i ];

    data = HPIManager->PullFromHPI( win_filename, &length );
    pos = 0;
}

char *TA3D_FILE::tgets( void *buf, int size )
{
    if( data == NULL || pos < 0 || pos >= length )	return NULL;
    for( char *out_buf = (char*)buf; size > 1 ; size-- ) {
        *out_buf = tgetc();
        if( !*out_buf || *out_buf == '\n' || *out_buf == '\r' )	{
            out_buf++;
            *out_buf = 0;
            return (char*)buf;
        }
        out_buf++;
    }
    return NULL;
}

TA3D_FILE	*TA3D::UTILS::HPI::ta3d_fopen( String filename )
{
    TA3D_FILE *file = new TA3D_FILE;

    if( file ) {
        file->topen( filename );
        if( !file->isopen() ) {
            delete file;
            file = NULL;
        }
    }

    return file;
}

void		TA3D::UTILS::HPI::ta3d_fclose( TA3D_FILE *file )
{
    if( file )
        delete file;
}

char		TA3D::UTILS::HPI::ta3d_fgetc( TA3D_FILE *file )
{
    if( file )
        return file->tgetc();
    else
        return 0;
}

int		TA3D::UTILS::HPI::ta3d_fread( void *buf, int size, TA3D_FILE *file )
{
    if( file )
        return file->tread( buf, size );
    return 0;
}

int		TA3D::UTILS::HPI::ta3d_fread( void *buf, int size, int repeat, TA3D_FILE *file )
{
    if( file )
        return file->tread( buf, size * repeat );
    return 0;
}

char		*TA3D::UTILS::HPI::ta3d_fgets( void *buf, int size, TA3D_FILE *file )
{
    if( file )
        return file->tgets( buf, size );
    return NULL;
}

void		TA3D::UTILS::HPI::ta3d_fseek( int offset, TA3D_FILE *file )
{
    if( file )
        file->tseek( offset );
}

bool		TA3D::UTILS::HPI::ta3d_feof( TA3D_FILE *file )
{
    return file ? file->teof() : true;
}

int			TA3D::UTILS::HPI::ta3d_fsize( TA3D_FILE *file )
{
    return file ? file->tsize() : 0;
}

