#include "hpi.h"

namespace TA3D
{
    namespace UTILS
    {
        //! Magic autoregistration
        REGISTER_ARCHIVE_TYPE(Hpi);

        void Hpi::finder(String::List &fileList, const String &path)
        {
        }

        Archive* Hpi::loader(const String &filename)
        {
            return new Hpi(filename);
        }

        Hpi::Hpi(const String &filename)
        {
            open(filename);
        }

        Hpi::~Hpi()
        {
            close();
        }

        void Hpi::open(const String& filename)
        {
            Archive::name = filename;
        }

        void Hpi::close()
        {
            Archive::name.clear();
        }

        void Hpi::getFileList(std::list<File*> &lFiles)
        {
        }

        byte* Hpi::readFile(const String& filename, uint32* file_length)
        {
        }

        byte* Hpi::readFile(const File *file, uint32* file_length)
        {
        }

        byte* Hpi::readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length)
        {
        }

        byte* Hpi::readFileRange(const File *file, const uint32 start, const uint32 length, uint32 *file_length)
        {
        }

        bool Hpi::needsCaching()
        {
            return true;
        }

    }
}
