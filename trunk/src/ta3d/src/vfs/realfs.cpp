#include "../misc/files.h"
#include "../misc/paths.h"
#include "../misc/math.h"
#include "../misc/string.h"
#include "../logs/logs.h"
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
            	for (std::map<String, File*>::iterator i = files.begin() ; i != files.end() ; ++i)
               		delete i->second;
            	files.clear();
			}
        }

        void RealFS::getFileList(std::list<File*> &lFiles)
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
                    if (i->last() == '~' || Paths::ExtractFileExt(*i).toLower() == ".bak")      // Don't take useless files into account
                        continue;
                    i->erase(0, root.size() + 1);   // Remove root path + path separator since we don't need them in VFS name and we can add them when accessing the files
                    while(!i->empty() && (i->first() == '/' || i->first() == '\\'))
                        i->erase(0, 1);

                    RealFile *file = new RealFile;
                    file->pathToFile = *i;      // Store full path here
                    // make VFS path
                    i->convertSlashesIntoBackslashes();
                    i->toLower();
                    if (i->first() == '.' || i->find("cache") != String::npos)        // Don't include SVN and cache folders (they are huge and useless to us here)
                    {
                        delete file;
                        continue;
                    }
                    file->setName(*i);
                    file->setParent(this);
                    file->setPriority(0xFFFF);
                    std::map<String, File*>::iterator it = files.find(*i);
                    if (it != files.end())          // On some platform we can have files with the same VFS name (because of different cases resulting in different file names)
                    {
                        delete it->second;
                        files.erase(it);
                    }
                    files.insert( std::pair<String, File*>(*i, file) );
                }
            }
            for(std::map<String, File*>::iterator i = files.begin() ; i != files.end() ; ++i)
                lFiles.push_back(i->second);
        }

		byte* RealFS::readFile(const String& filename, uint32* file_length)
		{
			if (!files.empty())
			{
				std::map<String, File*>::iterator file = files.find(filename);
				if (file != files.end())
					return readFile(file->second, file_length);
			}
			return NULL;
		}

		byte* RealFS::readFile(const File *file, uint32* file_length)
        {
            String unixFilename = ((const RealFile*)file)->pathToFile;
            unixFilename.convertBackslashesIntoSlashes();

            String root = name;
            root.removeTrailingSlash();

            unixFilename = root + Paths::SeparatorAsString + unixFilename;

            FILE *pFile = fopen(unixFilename.c_str(), "rb");
            if (pFile == NULL)
                return NULL;
            uint64 filesize(0);
            if (!Paths::Files::Size(unixFilename, filesize))
            {
                fclose(pFile);
                return NULL;
            }
            if (file_length)
                *file_length = (uint32)filesize;
            byte *data = new byte[filesize + 1];
            fread(data, filesize, 1, pFile);
            data[filesize] = 0;
            fclose(pFile);
            return data;
        }

        byte* RealFS::readFileRange(const String& filename, const uint32 start, const uint32 length, uint32 *file_length)
        {
            std::map<String, File*>::iterator file = files.find(filename);
            if (file != files.end())
                return readFileRange(file->second, start, length, file_length);
            else
                return NULL;
        }

        byte* RealFS::readFileRange(const File *file, const uint32 start, const uint32 length, uint32 *file_length)
        {
            String unixFilename = ((const RealFile*)file)->pathToFile;
            unixFilename.convertBackslashesIntoSlashes();

            String root = name;
            root.removeTrailingSlash();

            unixFilename = root + Paths::SeparatorAsString + unixFilename;

            FILE *pFile = fopen(unixFilename.c_str(), "rb");
            if (pFile == NULL)
                return NULL;
            uint64 filesize(0);
            if (!Paths::Files::Size(unixFilename, filesize))
            {
                fclose(pFile);
                return NULL;
            }
            if (file_length)
                *file_length = (uint32)filesize;
            byte *data = new byte[filesize + 1];
            fseek(pFile, start, SEEK_SET);
            fread(data + start, Math::Min((uint32)(filesize - start), length), 1, pFile);
            data[filesize] = 0;
            fclose(pFile);
            return data;
        }

        bool RealFS::needsCaching()
        {
            return false;
        }

    }
}
