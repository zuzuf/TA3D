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
        std::list<Archive::ArchiveFinder> *Archive::listArchiveFinder = NULL;
        std::list<Archive::ArchiveLoader> *Archive::listArchiveLoader = NULL;

        void Archive::getArchiveList(QStringList &fileList, const QString &path)
        {
            if (!listArchiveFinder)
                listArchiveFinder = new std::list<Archive::ArchiveFinder>;
            for(std::list<ArchiveFinder>::iterator i = listArchiveFinder->begin() ; i != listArchiveFinder->end() ; ++i)
                (*i)(fileList, path);
        }

        void Archive::registerArchiveFinder(ArchiveFinder finder)
        {
            if (!listArchiveFinder)
                listArchiveFinder = new std::list<Archive::ArchiveFinder>;
            Archive::listArchiveFinder->push_back(finder);
        }

        void Archive::registerArchiveLoader(ArchiveLoader loader)
        {
            if (!listArchiveLoader)
                listArchiveLoader = new std::list<Archive::ArchiveLoader>;
            Archive::listArchiveLoader->push_back(loader);
        }

        Archive *Archive::load(const QString &filename)
        {
            if (!listArchiveLoader)
                listArchiveLoader = new std::list<Archive::ArchiveLoader>;
            for(std::list<ArchiveLoader>::iterator i = listArchiveLoader->begin() ; i != listArchiveLoader->end() ; ++i)
            {
                Archive *archive = (*i)(filename);
                if (archive)
                    return archive;
            }
            return NULL;
        }

		File* Archive::FileInfo::read()
        {
			return parent->readFile(this);
        }

		File* Archive::FileInfo::readRange(const uint32 start, const uint32 length)
        {
			return parent->readFileRange(this, start, length);
        }

		bool Archive::FileInfo::needsCaching() const
        {
            return parent->needsCaching();
        }
    }
}
