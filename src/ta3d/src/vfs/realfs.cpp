#include "../misc/files.h"
#include "../misc/paths.h"
#include "../misc/math.h"
#include "../misc/string.h"
#include "realfs.h"
#include <cstdio>

namespace TA3D
{
    namespace UTILS
    {
        //! Magic autoregistration
        REGISTER_ARCHIVE_TYPE(RealFS);

        void RealFS::finder(String::List &fileList, const String &path)
        {
            fileList.push_back(path);       // We consider a path to a directory as an archive of the real filesystem
        }

        Archive* RealFS::loader(const String &filename)
        {
            return new RealFS(filename);
        }

        RealFS::RealFS(const String &filename)
        {
            open(filename);
        }

        RealFS::~RealFS()
        {
            // Nothing to do
        }

        void RealFS::open(const String& filename)
        {
            Archive::name = filename;
        }

        void RealFS::close()
        {
            Archive::name.clear();
        }

        void RealFS::getFileList(std::list<File*> &lFiles)
        {
            String::List dirs;
            dirs.push_back(name);
            String::List files;
            while(!dirs.empty())
            {
                String current = dirs.front() + Paths::Separator + "*";
                dirs.pop_front();

                Paths::GlobFiles(files, current, false, false);

                Paths::GlobDirs(dirs, current, false, false);
            }

            for(String::List::iterator i = files.begin() ; i != files.end() ; ++i)
            {
                RealFile *file = new RealFile;
                file->setName(*i);
                file->setParent(this);
                lFiles.push_back(file);
            }
        }

        byte* RealFS::readFile(const String& filename, uint32* file_length)
        {
            FILE *file = fopen(filename.c_str(), "rb");
            if (file == NULL)
                return NULL;
            uint64 filesize(0);
            if (!Paths::Files::Size(filename, filesize))
            {
                fclose(file);
                return NULL;
            }
            if (file_length)
                *file_length = (uint32)filesize;
            byte *data = new byte[filesize + 1];
            fread(data, filesize, 1, file);
            data[filesize] = 0;
            fclose(file);
            return data;
        }

        byte* RealFS::readFile(const File *file, uint32* file_length)
        {
            return readFile(file->getName(), file_length);
        }

        byte* RealFS::readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length)
        {
            FILE *file = fopen(filename.c_str(), "rb");
            if (file == NULL)
                return NULL;
            uint64 filesize(0);
            if (!Paths::Files::Size(filename, filesize))
            {
                fclose(file);
                return NULL;
            }
            if (file_length)
                *file_length = (uint32)filesize;
            byte *data = new byte[filesize + 1];
            fseek(file, start, SEEK_SET);
            fread(data + start, Math::Min((uint32)(filesize - start), length), 1, file);
            data[filesize] = 0;
            fclose(file);
            return data;
        }

        byte* RealFS::readFileRange(const File *file, const uint32 start, const uint32 length, uint32 *file_length)
        {
            return readFileRange(file->getName(), start, length, file_length);
        }

        bool RealFS::needsCaching()
        {
            return false;
        }

    }
}
