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

#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;

#include <zlib.h>
#if defined TA3D_PLATFORM_WINDOWS
# pragma comment(lib, "tools/win32/mingw32/libs/zlib.lib")
#endif



namespace TA3D
{
namespace UTILS
{


	
	void VFS::addArchive(const String& filename, const int priority)
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


	void VFS::locateAndReadArchives(const String& path, const int priority)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		String::List fileList;
		LOG_DEBUG(LOG_PREFIX_VFS << "Getting archive list");
		Archive::getArchiveList(fileList, path);
		LOG_DEBUG(LOG_PREFIX_VFS << "Adding archives to the VFS");

		String::List::const_iterator end = fileList.end();
		for (String::List::const_iterator i = fileList.begin() ; i != end ; ++i)
			addArchive(*i, priority);
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
			pPaths.push_back(nullptr);
		LOG_DEBUG(LOG_PREFIX_VFS << "browse archives");
		for (String::Vector::iterator i = pPaths.begin(); i != pPaths.end(); ++i)
			locateAndReadArchives(*i, 0);

		if (!TA3D_CURRENT_MOD.empty())
		{
			LOG_DEBUG(LOG_PREFIX_VFS << "browse mod archives");
			for (String::Vector::iterator i = pPaths.begin(); i != pPaths.end(); ++i)
				locateAndReadArchives(String(*i) << TA3D_CURRENT_MOD, 0x10000);
		}
		buildDirMap();
		LOG_DEBUG(LOG_PREFIX_VFS << "VFS loaded");
	}



	void VFS::reload()
	{
		ThreadingPolicy::MutexLocker locker(*this);
		loadWL();
	}


	void VFS::putInCache(const String& filename, File* file)
	{
		String cacheable_filename(filename);
		cacheable_filename.toLower();
		for (String::iterator i = cacheable_filename.begin(); i != cacheable_filename.end(); ++i)
		{
			if ('/' == *i || '\\' == *i)
				*i = 'S';
		}

		if (!lp_CONFIG->developerMode)		// Don't fill the cache with files in developer mode, they would not be used anyway
		{
			String cache_filename;
			cache_filename << TA3D::Paths::Caches << cacheable_filename << ".dat"; // Save file in disk cache
			Stream cache_file(cache_filename, Yuni::Core::IO::OpenMode::write);
			if (cache_file.opened())
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

		newentry.name = ToLower(filename);			// Store a copy of the data
		newentry.length = file->size();
		newentry.data = new byte[file->size()];
		file->read(newentry.data, file->size());

		fileCache.push_back(newentry);

		file->seek(0);
	}



	File* VFS::isInDiskCacheWL(const String& filename)
	{
		// In developer mode, the cache is always "empty", we just don't use it
		if (lp_CONFIG->developerMode)
			return NULL;

		// May be in cache but doesn't use cache (ie: campaign script)
		if (SearchString(filename, ".lua", true) >= 0)
			return NULL;

		String cacheable_filename(filename);
		cacheable_filename.toLower();
		for (String::iterator i = cacheable_filename.begin() ; i != cacheable_filename.end(); ++i)
		{
			if ('/' == *i || '\\' == *i)
				*i = 'S';
		}

		String cache_filename;
		cache_filename << TA3D::Paths::Caches << cacheable_filename << ".dat";

		if (TA3D::Paths::Exists(cache_filename)) // Check disk cache
		{
			return new RealFile( cache_filename );
		}
		return NULL;
	}


	VFS::CacheFileData* VFS::isInCache(const String& filename)
	{
		if (!filename)
			return NULL;
		ThreadingPolicy::MutexLocker locker(*this);
		return isInCacheWL(filename);
	}


	VFS::CacheFileData* VFS::isInCacheWL(const String& filename)
	{
		if (fileCache.empty())
			return NULL;

		String key(filename);
		key.toLower();
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


	File *VFS::readFile(const String& filename)
	{
		if (filename.empty())
			return NULL;

		String key(filename);
		key.toLower();
		key.convertSlashesIntoBackslashes();

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



	File* VFS::readFileRange(const String &filename, const uint32 start, const uint32 length)
	{
		if (filename.empty())
			return NULL;

		String key(filename);
		key.toLower();
		key.convertSlashesIntoBackslashes();

		ThreadingPolicy::MutexLocker locker(*this);

		CacheFileData *cache = (key.notEmpty()) ? isInCacheWL(key) : NULL;
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



	bool VFS::fileExists(String filename)
	{
		if (filename.empty())
			return false;
		filename.toLower();
		filename.convertSlashesIntoBackslashes();

		ThreadingPolicy::MutexLocker locker(*this);
		return pFiles.find(filename) != pFiles.end();
	}


	int VFS::filePriority(const String& filename)
	{
		if (filename.empty())
			return -0xFFFFFF;
		String key(filename);
		key.toLower();
		key.convertSlashesIntoBackslashes();

		ThreadingPolicy::MutexLocker locker(*this);
		HashMap<Archive::FileInfo*>::Dense::iterator file = pFiles.find(key);
		// If it doesn't exist it has a lower priority than anything else
		return (file != pFiles.end() && *file != NULL) ? file.value()->getPriority() : -0xFFFFFF;
	}


	uint32 VFS::getFilelist(String pattern, String::Vector& li)
	{
		pattern.toLower();
		pattern.convertSlashesIntoBackslashes();

		ThreadingPolicy::MutexLocker locker(*this);
		return wildCardSearch(pFiles, pattern, li);
	}


	uint32 VFS::getFilelist(String pattern, String::List& li)
	{
		pattern.toLower();
		pattern.convertSlashesIntoBackslashes();

		ThreadingPolicy::MutexLocker locker(*this);
		return wildCardSearch(pFiles, pattern, li);
	}

	uint32 VFS::getDirlist(String pattern, String::Vector& li)
	{
		pattern.toLower();
		pattern.convertSlashesIntoBackslashes();

		String parent = '\\' + Paths::ExtractFilePath(pattern);
		pattern = Paths::ExtractFileName(pattern);

		ThreadingPolicy::MutexLocker locker(*this);
		if (pDirs.count(parent) == 0)
			return 0;
		return wildCardSearch(pDirs[parent], pattern, li);
	}


	uint32 VFS::getDirlist(String pattern, String::List& li)
	{
		pattern.toLower();
		pattern.convertSlashesIntoBackslashes();

		String parent = '\\' + Paths::ExtractFilePath(pattern);
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
			String cur = Paths::ExtractFilePath('\\' + i.key());
			for ( ; !cur.empty() && cur != "\\" ; )
			{
				if (cur.last() == '\\')
					cur.removeLast();
				String parent = Paths::ExtractFilePath(cur);
				if (parent.empty())
					parent = '\\';

				cur = Substr(cur, 1);
				pDirs[parent][cur] = true;

				cur = parent;
			}
		}
	}

	String VFS::extractFile(const String& filename)
	{
		ThreadingPolicy::MutexLocker locker(*this);
		String targetName = String(Paths::Caches) << Paths::ExtractFileName(filename);
		Stream file(targetName, Yuni::Core::IO::OpenMode::write);
		if (!file.opened())
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

	uint32 VFS::getArchivelist(String::Vector& li) const
	{
		for(std::vector<Archive*>::const_iterator it = archives.begin() ; it != archives.end() ; ++it)
			li.push_back((*it)->getName());
		return (uint32)archives.size();
	}

	bool load_palette(SDL_Color *pal, const String& filename)
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
			bool tplLoadFromFile(T& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
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
		String line;
		while (file->readLine(line))
			out.push_back(line);
		delete file;
		return true;
	}

	bool loadFromFile(String::List& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
	{
		return tplLoadFromFile(out, filename, sizeLimit, emptyListBefore);
	}

	bool loadFromFile(String::Vector& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
	{
		return tplLoadFromFile(out, filename, sizeLimit, emptyListBefore);
	}
} // namespace UTILS
} // namespace TA3D
