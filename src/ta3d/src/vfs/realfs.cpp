#include <misc/files.h>
#include <misc/paths.h>
#include <misc/math.h>
#include <misc/string.h>
#include <logs/logs.h>
#include "realfs.h"
#include "realfile.h"
#include <QFile>

namespace TA3D
{
    namespace UTILS
    {
        //! Magic autoregistration
        REGISTER_ARCHIVE_TYPE(RealFS)

        void RealFS::finder(QStringList &fileList, const QString &path)
        {
            fileList.push_back(path);       // We consider a path to a directory as an archive of the real filesystem
        }

        Archive* RealFS::loader(const QString &filename)
        {
            if (!filename.isEmpty() && (filename.endsWith('/') || filename.endsWith('\\')))
                return new RealFS(filename);
            return NULL;
        }

        RealFS::RealFS(const QString &filename)
        {
            open(filename);
        }

        RealFS::~RealFS()
        {
            close();
        }

        void RealFS::open(const QString& filename)
        {
            Archive::name = filename;
        }

        void RealFS::close()
        {
            Archive::name.clear();
			if (!files.empty())
			{
				for (HashMap<RealFile*>::Sparse::iterator i = files.begin() ; i != files.end() ; ++i)
					delete *i;
            	files.clear();
			}
        }

		void RealFS::getFileList(std::deque<FileInfo*> &lFiles)
        {
            if (files.empty())
            {
                QString root = name;
                if (root.endsWith('/')) root.chop(1);
                QStringList dirs;
                dirs.push_back(root);
                QStringList fileList;
                while (!dirs.empty())
                {
                    QString current = dirs.front() + "/*";
                    dirs.pop_front();

                    Paths::GlobFiles(fileList, current, false);

                    Paths::GlobDirs(dirs, current, false);
                }

                for (QStringList::iterator i = fileList.begin() ; i != fileList.end() ; ++i)
                {
					if (i->size() <= root.size() + 1
                        || i->endsWith('~')
						|| Paths::ExtractFileExt(*i).toLower() == ".bak")      // Don't take useless files into account
                        continue;
					*i = Substr(*i, root.size() + 1);	// Remove root path + path separator since we don't need them in VFS name and we can add them when accessing the files
                    if (!i->isEmpty())
					{
                        QString::size_type s = 0;
                        while(s < i->size() && ((*i)[s] == '/' || (*i)[s] == '\\')) ++s;
                        if (s == i->size())
                            s = -1;
						if (s != -1)
							*i = Substr(*i, s);
					}

                    if (i->contains("/.svn/")
                            || i->contains("\\.svn\\")
                            || i->contains("\\.git\\")
                            || i->contains("/.git/")
                            || i->startsWith(".svn/")
                            || i->startsWith(".svn\\")
                            || i->startsWith(".git/")
                            || i->startsWith(".git\\")
                            || i->contains("cache"))        // Don't include SVN and cache folders (they are huge and useless to us here)
                        continue;

                    RealFile *file = new RealFile;
                    file->pathToFile = *i;      // Store full path here
                    // make VFS path
                    i->replace('\\','/');
                    *i = i->toLower();
                    file->setName(*i);
                    file->setParent(this);
                    file->setPriority(0xFFFF);
					HashMap<RealFile*>::Sparse::iterator it = files.find(*i);
                    if (it != files.end())          // On some platform we can have files with the same VFS name (because of different cases resulting in different file names)
						delete *it;
					files[*i] = file;
                }
            }
			for(HashMap<RealFile*>::Sparse::iterator i = files.begin() ; i != files.end() ; ++i)
				lFiles.push_back(*i);
        }

		File* RealFS::readFile(const QString& filename)
		{
            if (!files.empty())
			{
				HashMap<RealFile*>::Sparse::iterator file = files.find(filename);
				if (file != files.end())
					return readFile(*file);
			}
			return NULL;
		}

		File* RealFS::readFile(const FileInfo *file)
		{
			QString unixFilename = ((const RealFile*)file)->pathToFile;
            unixFilename.replace('\\', '/');

			QString root = name;
            if (root.endsWith('/'))
                root.chop(1);

            unixFilename = root + '/' + unixFilename;

			return new UTILS::RealFile(unixFilename);
		}

		File* RealFS::readFileRange(const QString& filename, const uint32 start, const uint32 length)
        {
			HashMap<RealFile*>::Sparse::iterator file = files.find(filename);
            if (file != files.end())
				return readFileRange(*file, start, length);
            else
                return NULL;
        }

		File* RealFS::readFileRange(const FileInfo *file, const uint32, const uint32)
        {
            QString unixFilename = ((const RealFile*)file)->pathToFile;
            unixFilename.replace('\\', '/');

            QString root = name;
            if (root.endsWith('/'))
                root.chop(1);

            unixFilename = root + '/' + unixFilename;

			return new UTILS::RealFile(unixFilename);
        }

        bool RealFS::needsCaching()
        {
            return false;
        }

    }
}
