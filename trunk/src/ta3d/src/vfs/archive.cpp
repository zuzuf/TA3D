#include "archive.h"

namespace TA3D
{
    namespace UTILS
    {
        bool Archive::needsCaching()
        {
            return false;
        }

        //! The getArchiveList mechanism
        std::list<Archive::ArchiveFinder> Archive::listArchiveFinder;
        std::list<Archive::ArchiveLoader> Archive::listArchiveLoader;

        void Archive::getArchiveList(String::List &fileList, const String &path)
        {
            for(std::list<ArchiveFinder>::iterator i = listArchiveFinder.begin() ; i != listArchiveFinder.end() ; ++i)
                (*i)(fileList, path);
        }

        void Archive::registerArchiveFinder(ArchiveFinder finder)
        {
            Archive::listArchiveFinder.push_back(finder);
        }

        void Archive::registerArchiveLoader(ArchiveLoader loader)
        {
            Archive::listArchiveLoader.push_back(loader);
        }

        Archive *Archive::load(const String &filename)
        {
            for(std::list<ArchiveLoader>::iterator i = listArchiveLoader.begin() ; i != listArchiveLoader.end() ; ++i)
            {
                Archive *archive = (*i)(filename);
                if (archive)
                    return archive;
            }
            return NULL;
        }

        byte* Archive::File::read(uint32* file_length)
        {
            return parent->readFile(this, file_length);
        }

        byte* Archive::File::readRange(const uint32 start, const uint32 length, uint32* file_length)
        {
            return parent->readFileRange(this, start, length, file_length);
        }

        bool Archive::File::needsCaching()
        {
            return parent->needsCaching();
        }
    }
}
