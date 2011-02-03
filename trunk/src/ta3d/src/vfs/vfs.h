/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2006  Roland BROCHARD

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
**  File: vfs.h
** Notes:
**   Zuzuf:  This module implements an abstraction layer over the filesystem
**           and the archives loaded by the game. It doesn't handle archives itself
**           it only maps a path/filename to the corresponding data.
*/

#ifndef __TA3D_UTILS_VFS_H__
# define __TA3D_UTILS_VFS_H__

# include <yuni/yuni.h>
# include <yuni/thread/policy.h>
# include <list>
# include <vector>
# include "archive.h"
# include <misc/hash_table.h>
# include <misc/string.h>
# include <sdl.h>
# include <threads/mutex.h>
# include "file.h"

# ifndef MAX_PATH
#   define MAX_PATH 260
# endif



namespace TA3D
{
namespace UTILS
{

	/*!
	** \brief
	*/
	class VFS : public Yuni::Policy::ObjectLevelLockable<VFS>
	{
	private:
		//! The threading policy
		typedef Yuni::Policy::ObjectLevelLockable<VFS>  ThreadingPolicy;

		/*! \class CacheFileData
		**
		** \brief
		*/
		struct CacheFileData
		{
			uint32		length;
			byte		*data;
			String		name;
		}; // class CacheFileData

	public:
		/*!
		** \brief Get the unique instance of the Virtual File system
		*/
		static VFS* Instance();

	public:
		/*!
		** \brief reload all archives
		*/
		void reload();

		/*!
		** \brief
		** \param path
		*/
		void searchDirForArchives(const String& path);

		/*!
		** \brief
		** \param s
		** \param[out] li
		*/
		uint32 getFilelist(String pattern, String::List& li);
		uint32 getFilelist(String pattern, String::Vector& li);

		/*!
		** \brief
		** \param s
		** \param[out] li
		*/
		uint32 getDirlist(String pattern, String::List& li);
		uint32 getDirlist(String pattern, String::Vector& li);

		/*!
		** \brief Get the list of all loaded archives
		** \param[out] li
		*/
		uint32 getArchivelist(String::Vector& li) const;

		/*!
		** \brief
		*/
		File* readFile(const String& filename);

		/*!
		** \brief
		** \param filename
		** \param start
		** \param length
		** \return
		*/
		File* readFileRange(const String& filename, const uint32 start, const uint32 length);

		/*!
		** \brief
		** \param filename
		** \return
		*/
		bool fileExists(String filename);

		/*!
		** \brief returns the priority level of a file
		** \param filename
		** \return
		*/
		int filePriority(const String& filename);

		/*!
		** \brief extract the given file and return an absolute path to it
		** \param filename
		** \return
		*/
		String extractFile(const String& filename);


	private:
		//! \name Constructor & Destructor
		//@{
		// constructor:
		VFS();

		//! Destructor
		~VFS();
		//@}

		/*!
		** \brief load all archives
		*/
		void load();

		/*!
		** \brief unload all archives
		*/
		void unload();

		
		/*!
		** \brief
		**
		** \param filename
		** \param priority
		*/
		void addArchive(const String& filename, const int priority);

		/*!
		** \brief
		**
		** \param path
		** \param priority
		*/
		void locateAndReadArchives(const String& path, const int priority);

		/*!
		** \brief
		** \param filename
		** \param filesize
		** /param data
		*/
		void putInCache(const String& filename, File* file);

		/*!
		** \brief
		** \param filename
		*/
		CacheFileData* isInCache(const String& filename);

		/*!
		** \brief build the table of all dirs from the list of all files
		*/
		void buildDirMap();

	protected:
		void loadWL();
		void unloadWL();

		CacheFileData* isInCacheWL(const String& filename);

		/*!
		** \brief
		** \param filename
		** \param filesize
		*/
		File *isInDiskCacheWL(const String& filename);


	private:
		//! used when looking for files in the real file system
		String::Vector pPaths;
		//!
		typedef TA3D::UTILS::HashMap<Archive::FileInfo*>::Dense FileInfoMap;
		FileInfoMap pFiles;
		//!
		typedef TA3D::UTILS::HashMap< TA3D::UTILS::HashMap<bool>::Sparse >::Dense DirMap;
		DirMap pDirs;

		//! The cache is used to speed up things when a file is loaded multiple times
		std::list<CacheFileData>  fileCache;
		//! A list of Archive*, needed only for cleanup.
		std::vector<Archive*> archives;

	
	}; // class VFS;


	bool load_palette(SDL_Color *pal, const String& filename = "palettes\\palette.pal");

	bool loadFromFile(String::List& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore);
	bool loadFromFile(String::Vector& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore);

} // namespace utils
} // namespace TA3D

# include "vfs.hxx"

#endif // __TA3D_UTILS_VFS_H__
