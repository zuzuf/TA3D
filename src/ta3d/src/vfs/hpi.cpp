#include "hpi.h"
#include "../misc/paths.h"
#include <zlib.h>

namespace TA3D
{
    namespace UTILS
    {
        //! Magic autoregistration
        REGISTER_ARCHIVE_TYPE(Hpi);

        void Hpi::finder(String::List &fileList, const String &path)
        {
            String::List files;
            Paths::GlobFiles(files, path + Paths::Separator + "*", false, true);
            for(String::List::iterator i = files.begin() ; i != files.end() ; ++i)
            {
                String ext = Paths::ExtractFileExt(*i).toLower();
                if (ext == ".hpi" || ext == ".gp3" || ext == ".ccx" || ext == ".ufo")
                    fileList.push_back(*i);
            }
        }

        Archive* Hpi::loader(const String &filename)
        {
            return new Hpi(filename);
        }

        Hpi::Hpi(const String &filename)
        {
            HPIFile = NULL;
            directory = NULL;
            open(filename);
        }

        Hpi::~Hpi()
        {
            close();
        }

        void Hpi::open(const String& filename)
        {
            close();

            Archive::name = filename;

            HPIFile = fopen(filename.c_str(), "rb");
            if (!HPIFile)
            {
                close();
                //      GlobalDebugger-> Failed to open hpi file for reading.
                return;
            }

            HPIVERSION hv;
            fread(&hv, sizeof(HPIVERSION), 1, HPIFile);

            if (hv.Version != HPI_V1 || hv.HPIMarker != HEX_HAPI)
            {
                close();
                return;
            }

            fread(&header, sizeof(HPIHEADER), 1, HPIFile);
            if (header.Key)
                key = (header.Key * 4) | (header.Key >> 6);
            else
                key = 0;

            int start = header.Start;
            int size = header.DirectorySize;

            directory = new sint8 [size];

            readAndDecrypt(start, (byte *)directory + start, size - start);
        }

        void Hpi::close()
        {
            Archive::name.clear();
            if (directory)
            {
                delete[] directory;
                directory = NULL;
            }

            if (HPIFile)
                fclose(HPIFile);

            for(std::map<String, HpiFile*>::iterator i = files.begin() ; i != files.end() ; ++i)
                delete i->second;
            files.clear();

            HPIFile = NULL;
        }

        void Hpi::getFileList(std::list<File*> &lFiles)
        {
            if (files.empty())
            {
                m_cDir.clear();
                processRoot("", header.Start);
            }
            for(std::map<String, HpiFile*>::iterator i = files.begin() ; i != files.end() ; ++i)
                lFiles.push_back(i->second);
        }

        byte* Hpi::readFile(const String& filename, uint32* file_length)
        {
            String key = String::ToLower(filename);
            key.convertSlashesIntoBackslashes();
            return readFile(files[key], file_length);
        }

        byte* Hpi::readFile(const File *file, uint32* file_length)
        {
            const HpiFile *hi = (const HpiFile*)file;

            if (!hi)
                return NULL;

            sint32 DeCount,DeLen, x, WriteSize, WritePtr, Offset, Length, FileFlag, *DeSize;
            byte *DeBuff, *WriteBuff;
            const HPIENTRY *entry;
            HPICHUNK *chunk;

            entry = &(hi->entry);

            Offset = *((sint32 *) (directory + entry->CountOffset));
            Length = *((sint32 *) (directory + entry->CountOffset + 4));
            FileFlag = *(directory + entry->CountOffset + 8);

            if(file_length)
                *file_length = Length;

            WriteBuff = new byte[Length + 1];

            WriteBuff[Length] = 0;

            if (FileFlag)
            {
                DeCount = Length >> 16;
                if (Length & 0xFFFF)
                    DeCount++;

                DeSize = new sint32[DeCount];

                DeLen = DeCount * sizeof(sint32);

                readAndDecrypt(Offset, (byte *) DeSize, DeLen);

                Offset += DeLen;

                WritePtr = 0;

                for (x = 0; x < DeCount; ++x)
                {
                    chunk = (HPICHUNK *) new byte[ DeSize[x] ];
                    readAndDecrypt(Offset, (byte *) chunk, DeSize[x]);
                    Offset += DeSize[x];

                    DeBuff = (byte *) (chunk+1);

                    WriteSize = decompress(WriteBuff + WritePtr, DeBuff, chunk);
                    WritePtr += WriteSize;

                    delete[] chunk;
                }
                delete[] DeSize;
            }
            else
            {
                // file not compressed
                readAndDecrypt(Offset, WriteBuff, Length);
            }

            return WriteBuff;
        }

        byte* Hpi::readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length)
        {
            String key = String::ToLower(filename);
            key.convertSlashesIntoBackslashes();
            return readFileRange(files[key], start, length, file_length);
        }

        byte* Hpi::readFileRange(const File *file, const uint32 start, const uint32 length, uint32 *file_length)
        {
            const HpiFile *hi = (const HpiFile*)file;
            if (!hi)
                return NULL;
            sint32 DeCount,DeLen, x, WriteSize, WritePtr, Offset, Length, FileFlag, *DeSize;
            byte *DeBuff, *WriteBuff;
            const HPIENTRY *entry;

            entry = &(hi->entry);

            Offset = *((sint32 *) (directory + entry->CountOffset));
            Length = *((sint32 *) (directory + entry->CountOffset + 4));
            FileFlag = *(directory + entry->CountOffset + 8);

            if(file_length)
                *file_length = Length;

            WriteBuff = new byte[Length+1];

            WriteBuff[Length] = 0;

            if (FileFlag)
            {
                DeCount = Length >> 16;
                if (Length & 0xFFFF)
                    DeCount++;

                DeSize = new sint32 [ DeCount ];
                DeLen = DeCount * sizeof(sint32);

                readAndDecrypt(Offset, (byte *) DeSize, DeLen);
                Offset += DeLen;
                WritePtr = 0;

                for (x = 0; x < DeCount; ++x)
                {
                    byte *ChunkBytes = new byte[ DeSize[x] ];
                    readAndDecrypt(Offset, ChunkBytes, DeSize[x]);
                    Offset += DeSize[x];

                    HPICHUNK *Chunk = &((HPICHUNK_U*)ChunkBytes)->chunk; // strict-aliasing safe
                    if ((uint32)WritePtr >= start || WritePtr + Chunk->DecompressedSize >= (sint32)start)
                    {

                        DeBuff = (byte *) (Chunk+1);

                        WriteSize = decompress(WriteBuff+WritePtr, DeBuff, Chunk);
                        WritePtr += WriteSize;
                    }
                    else
                        WritePtr += Chunk->DecompressedSize;

                    delete[] ChunkBytes;
                    if (WritePtr >= (sint32)(start+length) )   break;
                }
                delete[] DeSize;
            }
            else
            {
                // file not compressed
                readAndDecrypt(Offset+start, WriteBuff+start, length);
            }

            return WriteBuff;
        }

        sint32  Hpi::readAndDecrypt(const sint32 fpos, byte *buff, const sint32 buffsize)
        {
            sint32 count, result;
            fseek(HPIFile, fpos, SEEK_SET);
            result = (sint32)fread(buff, buffsize, 1, HPIFile);
            if (key)
            {
                for (count = 0; count < buffsize; ++count)
                    buff[count] ^= (fpos + count) ^ key;
            }
            return result;
        }

        sint32 Hpi::ZLibDecompress(byte *out, byte *in, HPICHUNK *Chunk)
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
            return (sint32)zs.total_out;
        }

        sint32 Hpi::LZ77Decompress(byte *out, byte *in)
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
                {
                    int count = *((uint16 *) (in+inptr));
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

        sint32 Hpi::decompress(byte *out, byte *in, HPICHUNK* Chunk)
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
            case 1 : return LZ77Decompress(out, in);
            case 2 : return ZLibDecompress(out, in, Chunk);
            default : return 0;
            };
        }

        void Hpi::processSubDir( HPIENTRY *base )
        {
            sint32 *FileCount, *FileLength, *EntryOffset, *Entries, count;
            schar *Name;
            HPIENTRY *Entry;

            if (base)
                Entries = (sint32 *) (directory + base->CountOffset);
            else
                Entries = (sint32 *) (directory + header.Start);

            EntryOffset = Entries + 1;
            Entry = (HPIENTRY *) (directory + *EntryOffset);

            for (count = 0; count < *Entries; ++count)
            {
                Name = (schar*) (directory + Entry->NameOffset);
                FileCount = (sint32 *) (directory + Entry->CountOffset);
                FileLength = FileCount + 1;

                if (Entry->Flag == 1)
                {
                    String sDir = m_cDir; // save directory
                    m_cDir << (char *)Name << "\\";

                    processSubDir(Entry);     // process new directory

                    m_cDir = sDir; // restore dir with saved dir
                }
                else
                {
                    HpiFile *li = new HpiFile;
                    li->setName(String((char*)Name));
                    li->setParent(this);
                    li->entry = *Entry;
                    String f(String::ToLower(m_cDir + (char *)Name));
                    li->size = *FileLength;
                    files[m_cDir + li->getName()] = li;
                }
                ++Entry;
            }
        }

        void Hpi::processRoot(const String& startPath, const sint32 offset)
        {
            sint32 *Entries, *FileCount, *FileLength, *EntryOffset;
            schar *Name;
            String MyPath;

            Entries = (sint32 *)(directory + offset);
            EntryOffset = Entries + 1;
            HPIENTRY *Entry = (HPIENTRY *)(directory + *EntryOffset);

            for (sint32 count = 0; count < *Entries; ++count)
            {
                Name = (schar*)(directory + Entry->NameOffset);
                FileCount = (sint32 *) (directory + Entry->CountOffset);
                FileLength = FileCount + 1;
                if (Entry->Flag == 1)
                {
                    MyPath = startPath;
                    if (MyPath.length())
                        MyPath += "\\";
                    MyPath += (char *)Name;
                    m_cDir = MyPath + "\\";

                    processSubDir( Entry );
                }
                else
                {
                    HpiFile *li = new HpiFile;
                    li->setName(String((char*)Name));
                    li->setParent(this);
                    li->entry = *Entry;
                    String f(String::ToLower(m_cDir + (char *)Name));
                    li->size = *FileLength;
                    files[m_cDir + li->getName()] = li;
                }
                ++Entry;
            }
        }

        bool Hpi::needsCaching()
        {
            return true;
        }

    }
}
