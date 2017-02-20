/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005  Roland BROCHARD

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
/*
**  File: vfs.cpp
** Notes:
**   Zuzuf: See notes in vfs.h
*/

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include <misc/paths.h>
#include <logs/logs.h>
#include <misc/resources.h>
#include <misc/files.h>
#include "vfs.h"
#include "file.h"
#include "virtualfile.h"
#include "realfile.h"

#include <QFile>

#include <zlib.h>


namespace TA3D
{
namespace UTILS
{


	
	void VFS::addArchive(const QString& filename, const int priority)
	{
		ThreadingPolicy::MutexLocker locker(*this);

		LOG_DEBUG(LOG_PREFIX_VFS << "loading archive '" << filename << "'");
		Archive *archive = Archive::load(filename);

		if (archive)
		{
			LOG_DEBUG(LOG_PREFIX_VFS << "adding archive to the archive list");
			archives.push_back(archive);
			std::deque<Archive::FileInfo*> archiveFiles;
			LOG_DEBUG(LOG_PREFIX_VFS << "getting file list from archive");
			archive->getFileList(archiveFiles);
			LOG_DEBUG(LOG_PREFIX_VFS << "inserting archive files into file hash table");

			pFiles.rehash(pFiles.size() + archiveFiles.size());
			const std::deque<Archive::FileInfo*>::iterator end = archiveFiles.end();
			for (std::deque<Archive::FileInfo*>::iterator i = archiveFiles.begin() ; i != end; ++i)
			{
				(*i)->setPriority((*i)->getPriority() + priority); // Update file priority

				Archive::FileInfo* file = pFiles.contains((*i)->getName()) ? pFiles[(*i)->getName()] : NULL;
				if (!file || (file->getPriority() < (*i)->getPriority()))
					pFiles[(*i)->getName()] = *i;
			}
		}
		else
			LOG_ERROR(LOG_PREFIX_VFS << "could not load archive '" << filename << "'");
	}


	void VFS::locateAndReadArchives(const QString& path, const int priority)
	{
		ThreadingPolicy::MutexLocker locker(*this);
        QStringList fileList;
		LOG_DEBUG(LOG_PREFIX_VFS << "Getting archive list");
		Archive::getArchiveList(fileList, path);
		LOG_DEBUG(LOG_PREFIX_VFS << "Adding archives to the VFS");

        for (const QString &i : fileList)
            addArchive(i, priority);
	}


	// constructor:
	VFS::VFS()
	{
	}

	VFS::~VFS()
	{
		unload();
	}


	void VFS::unload()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		unloadWL();
	}


	void VFS::unloadWL()
	{
		LOG_DEBUG(LOG_PREFIX_VFS << "unloading VFS");
		// Cleanup:
		//   First delete the hash, we don't need to delete the File objects since they are completely
		//   handled by the associated Archive classes
		pFiles.clear();
		pDirs.clear();


		LOG_DEBUG(LOG_PREFIX_VFS << "freeing VFS cache");
		if (!fileCache.empty())
		{
			const std::list<CacheFileData>::iterator end = fileCache.end();
			for (std::list<CacheFileData>::iterator i = fileCache.begin() ; i != end; ++i)
				DELETE_ARRAY(i->data);
			fileCache.clear();
		}

		// Now close and free archives.
		LOG_DEBUG(LOG_PREFIX_VFS << "closing and freeing archives");
		if (!archives.empty())
		{
			const std::vector<Archive*>::iterator end = archives.end();
			for (std::vector<Archive*>::iterator i = archives.begin() ; i != end; ++i)
				delete *i;
			archives.clear();
		}
		LOG_DEBUG(LOG_PREFIX_VFS << "VFS unloaded.");
	}


	void VFS::load()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		loadWL();
	}


	void VFS::loadWL()
	{
		unloadWL();

		LOG_DEBUG(LOG_PREFIX_VFS << "loading VFS");

		LOG_DEBUG(LOG_PREFIX_VFS << "reading root path");
		pPaths = TA3D::Resources::GetPaths();
		if (pPaths.empty())
            pPaths.push_back(QString());
		LOG_DEBUG(LOG_PREFIX_VFS << "browse archives");
        for (const QString &i : pPaths)
            locateAndReadArchives(i, 0);

        if (!TA3D_CURRENT_MOD.isEmpty())
		{
			LOG_DEBUG(LOG_PREFIX_VFS << "browse mod archives");
            for (const QString &i : pPaths)
                locateAndReadArchives(i + TA3D_CURRENT_MOD, 0x10000);
		}
		buildDirMap();
		LOG_DEBUG(LOG_PREFIX_VFS << "VFS loaded");
	}



	void VFS::reload()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		loadWL();
	}


	void VFS::putInCache(const QString& filename, File* file)
	{
        const QString cacheable_filename(filename.toLower().replace('\\', '/').replace('/', 'S'));

		if (!lp_CONFIG->developerMode)		// Don't fill the cache with files in developer mode, they would not be used anyway
		{
            QString cache_filename = TA3D::Paths::Caches + cacheable_filename + ".dat"; // Save file in disk cache
            QFile cache_file(cache_filename);
            cache_file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            if (cache_file.isOpen())
			{
				file->seek(0);
				char *buf = new char[10240];
				for(int i = 0 ; i < file->size() ; i += 10240)
				{
					int l = Math::Min(10240, file->size() - i);
					file->read(buf, l);
					cache_file.write(buf, l);
				}
				delete[] buf;
				cache_file.close();
			}
		}
		file->seek(0);

		if (file->size() >= 0x100000)	// Don't store big pFiles to prevent filling memory with cache data ;)
			return;

		ThreadingPolicy::MutexLocker locker(*this);

		if (fileCache.size() >= 10) // Cycle the data within the list
		{
			DELETE_ARRAY(fileCache.front().data);
			fileCache.pop_front();
		}

		CacheFileData newentry;

        newentry.name = filename.toLower();			// Store a copy of the data
		newentry.length = file->size();
		newentry.data = new byte[file->size()];
		file->read(newentry.data, file->size());

		fileCache.push_back(newentry);

		file->seek(0);
	}



	File* VFS::isInDiskCacheWL(const QString& filename)
	{
		// In developer mode, the cache is always "empty", we just don't use it
		if (lp_CONFIG->developerMode)
			return NULL;

		// May be in cache but doesn't use cache (ie: campaign script)
        if (filename.contains(".lua", Qt::CaseInsensitive))
			return NULL;

        const QString &cacheable_filename = filename.toLower().replace('\\', '/').replace('/', 'S');

        const QString &cache_filename = TA3D::Paths::Caches + cacheable_filename + ".dat";

		if (TA3D::Paths::Exists(cache_filename)) // Check disk cache
		{
			return new RealFile( cache_filename );
		}
		return NULL;
	}


	VFS::CacheFileData* VFS::isInCache(const QString& filename)
	{
        if (filename.isEmpty())
			return NULL;
		ThreadingPolicy::MutexLocker locker(*this);
		return isInCacheWL(filename);
	}


	VFS::CacheFileData* VFS::isInCacheWL(const QString& filename)
	{
		if (fileCache.empty())
			return NULL;

        const QString &key = filename.toLower();
		std::list<CacheFileData>::iterator i;
		for (i = fileCache.begin() ; i != fileCache.end() ; ++i) // Check RAM Cache
		{
			if (i->name == key)
			{
				// Move this file to the end of the list to keep it in the cache longer
				fileCache.push_back(*i);
				fileCache.erase(i);
				return &fileCache.back();
			}
		}
		return NULL;
	}


	File *VFS::readFile(const QString& filename)
	{
        if (filename.isEmpty())
			return NULL;

        const QString &key = filename.toLower().replace('\\', '/');

		ThreadingPolicy::MutexLocker locker(*this);

		CacheFileData *cache = isInCacheWL(key);
		if (cache)
		{
			if (cache->length == 0)
				return NULL;
			VirtualFile *f = new VirtualFile;
			f->copyBuffer(cache->data, cache->length);
			return f;
		}

		File* cacheFile = isInDiskCacheWL(key);
		if (cacheFile)
			return cacheFile;

		TA3D::UTILS::HashMap<Archive::FileInfo*>::Dense::iterator itFile = pFiles.find(key);

		if (itFile != pFiles.end())
		{
			File *file = itFile.value()->read();
			if (itFile.value()->needsCaching())
				putInCache( key, file );
			return file;
		}

		LOG_DEBUG(LOG_PREFIX_VFS << "file not found (" << filename << ')');

		return NULL;
	}



	File* VFS::readFileRange(const QString &filename, const uint32 start, const uint32 length)
	{
        if (filename.isEmpty())
			return NULL;

        const QString &key = filename.toLower().replace('\\', '/');

		ThreadingPolicy::MutexLocker locker(*this);

        CacheFileData *cache = key.isEmpty() ? NULL : isInCacheWL(key);
		if (cache)
		{
			if (cache->length == 0)
				return NULL;
			VirtualFile *f = new VirtualFile;
			f->copyBuffer(cache->data, cache->length);
			return f;
		}
		else
		{
			File* cacheFile = isInDiskCacheWL( key );
			if (cacheFile)
				return cacheFile;
		}

		TA3D::UTILS::HashMap<Archive::FileInfo*>::Dense::iterator itFile = pFiles.find(key);
		if (itFile == pFiles.end())
			return NULL;
		return *itFile ? itFile.value()->readRange(start, length) : NULL;
	}



	bool VFS::fileExists(QString filename)
	{
        if (filename.isEmpty())
			return false;
        filename = filename.toLower().replace('\\', '/');

		ThreadingPolicy::MutexLocker locker(*this);
		return pFiles.find(filename) != pFiles.end();
	}


	int VFS::filePriority(const QString& filename)
	{
        if (filename.isEmpty())
			return -0xFFFFFF;
        const QString key(filename.toLower().replace('\\', '/'));

		ThreadingPolicy::MutexLocker locker(*this);
		HashMap<Archive::FileInfo*>::Dense::iterator file = pFiles.find(key);
		// If it doesn't exist it has a lower priority than anything else
		return (file != pFiles.end() && *file != NULL) ? file.value()->getPriority() : -0xFFFFFF;
	}


    uint32 VFS::getFilelist(QString pattern, QStringList& li)
	{
        pattern = pattern.toLower().replace('\\', '/');

		ThreadingPolicy::MutexLocker locker(*this);
		return wildCardSearch(pFiles, pattern, li);
	}

	uint32 VFS::getDirlist(QString pattern, QStringList& li)
	{
        pattern = pattern.toLower().replace('\\', '/');

        QString parent = '/' + Paths::ExtractFilePath(pattern);
		pattern = Paths::ExtractFileName(pattern);

		ThreadingPolicy::MutexLocker locker(*this);
		if (pDirs.count(parent) == 0)
			return 0;
		return wildCardSearch(pDirs[parent], pattern, li);
	}

    void VFS::buildDirMap()
	{
		pDirs.clear();
		for(FileInfoMap::iterator i = pFiles.begin() ; i != pFiles.end() ; ++i)
		{
            QString cur = Paths::ExtractFilePath('/' + i.key());
            for ( ; !cur.isEmpty() && cur != "/" ; )
			{
                if (cur.endsWith('/'))
					cur.chop(1);
				QString parent = Paths::ExtractFilePath(cur);
                if (parent.isEmpty())
                    parent = '/';

				cur = Substr(cur, 1);
				pDirs[parent][cur] = true;

				cur = parent;
			}
		}
	}

	QString VFS::extractFile(const QString& filename)
	{
		ThreadingPolicy::MutexLocker locker(*this);
        QString targetName = Paths::Caches + Paths::ExtractFileName(filename);
        QFile file(targetName);
        file.open(QIODevice::Truncate | QIODevice::WriteOnly);
        if (!file.isOpen())
		{
			LOG_ERROR(LOG_PREFIX_VFS << "impossible to create file '" << targetName << "'");
			return targetName;
		}
		File *vfile = readFile(filename);
		if (vfile && vfile->isOpen())
		{
			char *buf = new char[10240];
			for(int i = 0 ; i < vfile->size() ; i += 10240)
			{
				int l = Math::Min(10240, vfile->size() - i);
				vfile->read(buf, l);
				file.write(buf, l);
			}
			delete[] buf;
		}
		else
		{
			LOG_WARNING(LOG_PREFIX_VFS << "could not extract file '" << filename << "'");
		}
		if (vfile)
			delete vfile;
		file.flush();
		file.close();
		return targetName;
	}

    uint32 VFS::getArchivelist(QStringList &li) const
	{
        for(Archive* it : archives)
            li.push_back(it->getName());
		return (uint32)archives.size();
	}

	bool load_palette(SDL_Color *pal, const QString& filename)
	{
		File* palette = VFS::Instance()->readFile(filename);
		if (palette == NULL)
			return false;

		for (int i = 0; i < 256; ++i)
		{
			*palette >> pal[i].r;
			*palette >> pal[i].g;
			*palette >> pal[i].b;
			char c;
			*palette >> c;
		}
		delete palette;
		return true;
	}

	template<class T>
			bool tplLoadFromFile(T& out, const QString& filename, const uint32 sizeLimit, const bool emptyListBefore)
	{
		if (emptyListBefore)
			out.clear();

		File *file = VFS::Instance()->readFile(filename);
		if (file == NULL)
		{
			LOG_WARNING("Impossible to open the file `" << filename << "`");
			return false;
		}
		if (sizeLimit && (uint32)file->size() > sizeLimit)
		{
			delete file;
			LOG_WARNING("Impossible to read the file `" << filename << "` (size > " << sizeLimit << ")");
			return false;
		}
		QString line;
		while (file->readLine(line))
			out.push_back(line);
		delete file;
		return true;
	}

    bool loadFromFile(QStringList& out, const QString& filename, const uint32 sizeLimit, const bool emptyListBefore)
	{
		return tplLoadFromFile(out, filename, sizeLimit, emptyListBefore);
	}
} // namespace UTILS
} // namespace TA3D
