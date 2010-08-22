#include <misc/files.h>
#include <misc/paths.h>
#include <misc/math.h>
#include <misc/string.h>
#include <logs/logs.h>
#include "realfs.h"
#include "realfile.h"
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;

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
            if (!filename.empty() && (filename.last() == '/' || filename.last() == '\\'))
                return new RealFS(filename);
            return NULL;
        }

        RealFS::RealFS(const String &filename)
        {
            open(filename);
        }

        RealFS::~RealFS()
        {
            close();
        }

        void RealFS::open(const String& filename)
        {
            Archive::name = filename;
        }

        void RealFS::close()
        {
            Archive::name.clear();
			if (!files.empty())
			{
				for (HashMap<RealFile*>::Sparse::iterator i = files.begin() ; i != files.end() ; ++i)
					delete i->second;
            	files.clear();
			}
        }

		void RealFS::getFileList(std::deque<FileInfo*> &lFiles)
        {
            if (files.empty())
            {
                String root = name;
                root.removeTrailingSlash();
                String::List dirs;
                dirs.push_back(root);
                String::List fileList;
                while (!dirs.empty())
                {
                    String current = dirs.front() + Paths::Separator + "*";
                    dirs.pop_front();

                    Paths::GlobFiles(fileList, current, false, false);

                    Paths::GlobDirs(dirs, current, false, false);
                }

                for (String::List::iterator i = fileList.begin() ; i != fileList.end() ; ++i)
                {
					if (i->size() <= root.size() + 1
						|| i->last() == '~'
						|| Paths::ExtractFileExt(*i).toLower() == ".bak")      // Don't take useless files into account
                        continue;
                    i->erase(0, root.size() + 1);   // Remove root path + path separator since we don't need them in VFS name and we can add them when accessing the files
                    while(!i->empty() && (i->first() == '/' || i->first() == '\\'))
                        i->erase(0, 1);

                    if (i->find("/.svn/") != String::npos
                        || i->find("\\.svn\\") != String::npos
						|| (i->size() >= 5 && i->substr(0, 5) == ".svn/")
						|| (i->size() >= 5 && i->substr(0, 5) == ".svn\\")
                        || i->find("cache") != String::npos)        // Don't include SVN and cache folders (they are huge and useless to us here)
                        continue;

                    RealFile *file = new RealFile;
                    file->pathToFile = *i;      // Store full path here
                    // make VFS path
                    i->convertSlashesIntoBackslashes();
                    i->toLower();
                    file->setName(*i);
                    file->setParent(this);
                    file->setPriority(0xFFFF);
					HashMap<RealFile*>::Sparse::iterator it = files.find(*i);
                    if (it != files.end())          // On some platform we can have files with the same VFS name (because of different cases resulting in different file names)
						delete it->second;
					files[*i] = file;
                }
            }
			for(HashMap<RealFile*>::Sparse::iterator i = files.begin() ; i != files.end() ; ++i)
                lFiles.push_back(i->second);
        }

		File* RealFS::readFile(const String& filename)
		{
			if (!files.empty())
			{
				HashMap<RealFile*>::Sparse::iterator file = files.find(filename);
				if (file != files.end())
					return readFile(file->second);
			}
			return NULL;
		}

		File* RealFS::readFile(const FileInfo *file)
		{
			String unixFilename = ((const RealFile*)file)->pathToFile;
			unixFilename.convertBackslashesIntoSlashes();

			String root = name;
			root.removeTrailingSlash();

			unixFilename = root + Paths::SeparatorAsString + unixFilename;

			return new UTILS::RealFile(unixFilename);
		}

		File* RealFS::readFileRange(const String& filename, const uint32 start, const uint32 length)
        {
			HashMap<RealFile*>::Sparse::iterator file = files.find(filename);
            if (file != files.end())
				return readFileRange(file->second, start, length);
            else
                return NULL;
        }

		File* RealFS::readFileRange(const FileInfo *file, const uint32, const uint32)
        {
            String unixFilename = ((const RealFile*)file)->pathToFile;
            unixFilename.convertBackslashesIntoSlashes();

            String root = name;
            root.removeTrailingSlash();

            unixFilename = root + Paths::SeparatorAsString + unixFilename;

			return new UTILS::RealFile(unixFilename);
        }

        bool RealFS::needsCaching()
        {
            return false;
        }

    }
}
