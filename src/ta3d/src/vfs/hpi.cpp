#include "hpi.h"
#include <misc/paths.h>
#include <zlib.h>
#include <logs/logs.h>
#include "virtualfile.h"

namespace TA3D
{
    namespace UTILS
    {
        //! Magic autoregistration
        REGISTER_ARCHIVE_TYPE(Hpi)

        void Hpi::finder(QStringList &fileList, const QString &path)
        {
            QStringList files;
            if (path.endsWith('/'))
                Paths::GlobFiles(files, path + "*", false);
            else
                Paths::GlobFiles(files, path + "/*", false);
            for(const QString &i : files)
            {
                const QString &ext = Paths::ExtractFileExt(i).toLower();
                if (ext == ".hpi" || ext == ".gp3" || ext == ".ccx" || ext == ".ufo")
                    fileList.push_back(i);
            }
        }

        Archive* Hpi::loader(const QString &filename)
        {
            const QString &ext = Paths::ExtractFileExt(filename).toLower();
            if (ext == ".hpi" || ext == ".gp3" || ext == ".ccx" || ext == ".ufo")
                return new Hpi(filename);
            return NULL;
        }

        Hpi::Hpi(const QString &filename)
        {
            directory = NULL;
            open(filename);
        }

        Hpi::~Hpi()
        {
            close();
        }

        void Hpi::open(const QString& filename)
        {
            close();

            Archive::name = filename;
            const QString &ext = Paths::ExtractFileExt(filename).toLower();
            priority = 0;
            if (ext == ".ccx")
                priority = 1;
            else if (ext == ".gp3")
                priority = 2;
            if (Paths::ExtractFileName(filename).toLower() == "ta3d.hpi")
                priority = 3;

            static int archive_count = 0;
            archive_count++;
            HPIFile.setFileName(filename);
            HPIFile.open(QIODevice::ReadOnly);
            if (!HPIFile.isOpen())
            {
                LOG_DEBUG(LOG_PREFIX_VFS << "failed to open hpi file '" << filename << "' for reading with error : '" << HPIFile.errorString() << "'");
                close();
                return;
            }

            HPIVERSION hv;
			HPIFile.read((char*)&hv, sizeof(HPIVERSION));

            if (hv.Version != HPI_V1 || hv.HPIMarker != HEX_HAPI)
            {
                close();
                LOG_DEBUG(LOG_PREFIX_VFS << "failed to load hpi file '" << filename << "' : wrong format or file corrupt");
                return;
            }

			HPIFile.read((char*)&header, sizeof(HPIHEADER));
            if (header.Key)
				key = sint8((header.Key * 4) | (header.Key >> 6));
            else
                key = 0;

			const int start = header.Start;
			const int size = header.DirectorySize;

            directory = new sint8 [size];

            readAndDecrypt(start, (byte *)directory + start, size - start);
        }

        void Hpi::close()
        {
            Archive::name.clear();
			DELETE_ARRAY(directory);

            if (HPIFile.isOpen())
				HPIFile.close();

			for(HashMap<HpiFile*>::Sparse::iterator i = files.begin() ; i != files.end() ; ++i)
				delete *i;
            files.clear();
        }

		void Hpi::getFileList(std::deque<FileInfo*> &lFiles)
        {
            if (files.empty())
            {
                m_cDir.clear();
				processRoot(QString(), header.Start);
            }
			for(HashMap<HpiFile*>::Sparse::iterator i = files.begin() ; i != files.end() ; ++i)
				lFiles.push_back(*i);
        }

		File* Hpi::readFile(const QString& filename)
        {
            const QString &key = QString(filename).replace('\\', '/');
			HashMap<HpiFile*>::Sparse::iterator item = files.find(key);
			if (item == files.end())
				return NULL;
			return readFile(*item);
        }

		File* Hpi::readFile(const FileInfo *file)
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

            WriteBuff = new byte[Length + 1];

            WriteBuff[Length] = 0;

            if (FileFlag)
            {
                DeCount = Length >> 16;
                if (Length & 0xFFFF)
                    DeCount++;

                DeSize = new sint32[DeCount];

				DeLen = DeCount * (int)sizeof(sint32);

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

					DELETE_ARRAY(chunk);
                }
				if (WritePtr > Length)
					LOG_ERROR(LOG_PREFIX_VFS << "HPI : more bytes(" << WritePtr << ") than expected(" << Length << ")");
				DELETE_ARRAY(DeSize);
            }
            else
            {
                // file not compressed
                readAndDecrypt(Offset, WriteBuff, Length);
            }

			return new VirtualFile(WriteBuff, Length);
        }

		File* Hpi::readFileRange(const QString& filename, const uint32 start, const uint32 length)
        {
            const QString &key = QString(filename).replace('\\', '/');
            HashMap<HpiFile*>::Sparse::iterator item = files.find(key);
			if (item == files.end())
				return NULL;
			return readFileRange(*item, start, length);
        }

		File* Hpi::readFileRange(const FileInfo *file, const uint32 start, const uint32 length)
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

            WriteBuff = new byte[Length+1];

            WriteBuff[Length] = 0;

            if (FileFlag)
            {
                DeCount = Length >> 16;
                if (Length & 0xFFFF)
                    DeCount++;

                DeSize = new sint32 [ DeCount ];
				DeLen = DeCount * (int)sizeof(sint32);

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

					DELETE_ARRAY(ChunkBytes);
                    if (WritePtr >= (sint32)(start+length) )   break;
                }
				DELETE_ARRAY(DeSize);
            }
            else
            {
                // file not compressed
                readAndDecrypt(Offset + start, WriteBuff + start, std::min<int>(Length - start, length));
            }

            return new VirtualFile(WriteBuff, std::min<int>(length, Length - start), start, Length);
        }

        sint32  Hpi::readAndDecrypt(sint32 fpos, byte *buff, const sint32 buffsize)
        {
            sint32 result;
            HPIFile.seek(fpos);
			result = (sint32)HPIFile.read((char*)buff, buffsize);
            if (key)
            {
                for (byte *end = buff + buffsize ; buff != end ; ++buff, ++fpos)
					*buff ^= byte(fpos ^ key);
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
            byte *out0 = out;
            sint32 work1, work2, work3;
            schar DBuff[4096];

            work1 = 1;
            work2 = 1;
            work3 = *in++;

            while (true)
            {
                if ((work2 & work3) == 0)
                {
                    *out++ = *in;
                    DBuff[work1] = *in;
                    work1 = (work1 + 1) & 0xFFF;
                    ++in;
                }
                else
                {
                    int count = *((uint16 *) (in));
                    in += 2;
                    int DPtr = count >> 4;
                    if (DPtr == 0)
						return sint32(out - out0);
                    else
                    {
                        count = (count & 0x0f) + 2;
                        if (count >= 0)
                        {
                            for (byte *end = out + count ; out != end ; ++out)
                            {
                                *out = DBuff[work1] = DBuff[DPtr];
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
                    work3 = *in++;
                }
            }
			return sint32(out - out0);
        }

        sint32 Hpi::decompress(byte *out, byte *in, HPICHUNK* Chunk)
        {
            sint32 Checksum(0);
            for (sint32 x = 0; x < Chunk->CompressedSize; ++x)
            {
                Checksum += (byte) in[x];
                if (Chunk->Encrypt)
					in[x] = byte((in[x] - x) ^ x);
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
                    QString sDir = m_cDir // save directory
                            + (char *)Name + "/";

                    processSubDir(Entry);     // process new directory

                    m_cDir = sDir; // restore dir with saved dir
                }
                else
                {
                    HpiFile *li = new HpiFile;
                    const QString &f = ToLower(m_cDir + (char *)Name);
                    li->setName(f);
                    li->setParent(this);
                    li->setPriority(priority);
                    li->entry = *Entry;
                    li->size = *FileLength;
                    files.insert( std::pair<QString, HpiFile*>(f, li) );
                }
                ++Entry;
            }
        }

        void Hpi::processRoot(const QString& startPath, const sint32 offset)
        {
            sint32 *Entries, *FileCount, *FileLength, *EntryOffset;
            schar *Name;
            QString MyPath;

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
                        MyPath += "/";
                    MyPath += (char *)Name;
                    m_cDir = MyPath + "/";

                    processSubDir( Entry );
                }
                else
                {
                    HpiFile *li = new HpiFile;
                    const QString &f = ToLower(m_cDir + (char *)Name);
                    li->setName(f);
                    li->setParent(this);
                    li->setPriority(priority);
                    li->entry = *Entry;
                    li->size = *FileLength;
                    files.insert( std::pair<QString, HpiFile*>(f, li) );
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
