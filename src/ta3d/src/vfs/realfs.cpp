#include "realfs.h"

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
        }

        byte* RealFS::readFile(const String& filename, uint32* file_length)
        {
        }

        byte* RealFS::readFile(const File *file, uint32* file_length)
        {
        }

        byte* RealFS::readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length)
        {
        }

        byte* RealFS::readFileRange(const File *file, const uint32 start, const uint32 length, uint32 *file_length)
        {
        }

        bool RealFS::needsCaching()
        {
            return false;
        }

    }
}
