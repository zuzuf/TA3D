
#include "platform.h"
#include "../../../slist.h"
#include <cassert>
#include "../../directory.h"
#include "../info.h"

#ifdef YUNI_OS_WINDOWS
# include "../../../system/windows.hdr.h"
# include <wchar.h>
# include <io.h>
#else
# include <errno.h>
# include <dirent.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
#endif



namespace Yuni
{
namespace Private
{
namespace Core
{
namespace IO
{
namespace Directory
{

	enum
	{
		wbufferMax = 6192,
	};



	class DirInfo
	{
	public:
		DirInfo()
			# ifndef YUNI_OS_WINDOWS
			:pdir(NULL)
			# else
			:h(-1)
			# endif
		{}

		~DirInfo()
		{
			# ifdef YUNI_OS_WINDOWS
			if (h >= 0)
				_findclose(h);
			# else
			closedir(pdir);
			# endif
		}

		# ifdef YUNI_OS_WINDOWS
		void open(wchar_t* wbuffer)
		# else
		void open()
		# endif
		{
			# ifdef YUNI_OS_WINDOWS
			// Convert the filename
			wbuffer[0] = L'\\';
			wbuffer[1] = L'\\';
			wbuffer[2] = L'?';
			wbuffer[3] = L'\\';
			int n = MultiByteToWideChar(CP_UTF8, 0, parent.c_str(), parent.size(), wbuffer + 4, wbufferMax - 10);
			if (!n)
			{
				h = -1;
				return;
			}

			// Making sure that our string is zero-terminated
			wbuffer[n + 4] = L'\\';
			wbuffer[n + 5] = L'*';
			wbuffer[n + 6] = L'.';
			wbuffer[n + 7] = L'*';
			wbuffer[n + 8] = L'\0';

			// Opening the folder
			h = _wfindfirsti64(wbuffer, &data);
			callNext = false;

			# else

			pdir = ::opendir(parent.c_str());

			# endif

			// This variable must be reseted to avoid recursive calls at startup
			isFolder = false;
		}

		bool next(unsigned int flags)
		{
			# ifndef YUNI_OS_WINDOWS

			if (!pdir)
				return false;
			while ((pent = readdir(pdir)))
			{
				// Avoid `.` and `..`
				if (*(pent->d_name) == '.')
				{
					if ((pent->d_name[1] == '.' && pent->d_name[2] == '\0') || (pent->d_name[1] == '\0'))
						continue;
				}

				name = (const char* const) pent->d_name;
				filename.clear();
				filename << parent << Yuni::Core::IO::Separator << name;
				if (stat(filename.c_str(), &s) != 0)
					continue;

				if (S_ISDIR(s.st_mode))
				{
					if (0 != (flags & Yuni::Core::IO::Directory::Info::itFolder)
						|| (0 != (flags & Yuni::Core::IO::Directory::Info::itRecursive)))
					{
						isFolder = true;
						size = 0;
						return true;
					}
				}
				else
				{
					if (0 != (flags & Yuni::Core::IO::Directory::Info::itFile))
					{
						isFolder = false;
						size = static_cast<uint64>(s.st_size);
						return true;
					}
				}
			}
			return false;

			# else // WINDOWS

			if (h < 0)
				return false;
			do
			{
				if (callNext && 0 != _wfindnexti64(h, &data))
					return false;
				callNext = true;

				// Avoid `.` and `..`
				if (*(data.name) == L'.')
				{
					if ((data.name[1] == L'.' && data.name[2] == L'\0') || (data.name[1] == L'\0'))
						continue;
				}

				const int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, data.name, -1, NULL, 0,  NULL, NULL);
				if (sizeRequired <= 0)
					continue;
				name.reserve((unsigned int) sizeRequired);
				WideCharToMultiByte(CP_UTF8, 0, data.name, -1, (char*)name.data(), sizeRequired,  NULL, NULL);
				name.resize(((unsigned int) sizeRequired) - 1);

				filename.clear();
				filename << parent << '\\' << name;

				if ((data.attrib & _A_SUBDIR))
				{
					if (0 != (flags & Yuni::Core::IO::Directory::Info::itFolder)
						|| (0 != (flags & Yuni::Core::IO::Directory::Info::itRecursive)))
					{
						isFolder = true;
						size = 0;
						return true;
					}
				}
				else
				{
					if (0 != (flags & Yuni::Core::IO::Directory::Info::itFile))
					{
						isFolder = false;
						size = (uint64) data.size;
						return true;
					}
				}

			}
			while (true);
			return false;
			# endif
		}

	public:
		//! Parent folder
		String parent;
		//! Name of the current node
		String name;
		//! The complete filename of the current node
		String filename;
		uint64 size;
		bool isFolder;


	private:
		# ifndef YUNI_OS_WINDOWS
		DIR* pdir;
		struct dirent *pent;
		struct stat s;
		# else
		struct _wfinddatai64_t data;
		long h;
		bool callNext;
		# endif

	};


	class IteratorData
	{
	public:
		IteratorData()
		{}

		IteratorData(const IteratorData& rhs)
		{
			if (rhs.dirinfo.notEmpty())
				push(rhs.dirinfo.front().parent);
		}

		template<class StringT> void push(const StringT& v)
		{
			dirinfo.push_front();
			dirinfo.front().parent = v;
			# ifdef YUNI_OS_WINDOWS
			dirinfo.front().open(wbuffer);
			# else
			dirinfo.front().open();
			# endif

		}

		void push(const char* const str, uint64 length)
		{
			dirinfo.push_front();
			dirinfo.front().parent.assign(str, static_cast<String::Size>(length));
			# ifdef YUNI_OS_WINDOWS
			dirinfo.front().open(wbuffer);
			# else
			dirinfo.front().open();
			# endif
		}

		void pop()
		{
			dirinfo.pop_front();
		}

		bool next()
		{
			// Entering the sub-folder if required
			if (dirinfo.front().isFolder && (0 != (Yuni::Core::IO::Directory::Info::itRecursive & flags)))
			{
				// Starting a new state
				push(dirinfo.front().filename);
			}

			// Next node
			//
			// Infinite loop to not use a recursive function
			do
			{
				if (!dirinfo.front().next(flags))
				{
					// Parent folder
					pop();
					if (dirinfo.empty())
						return false;
				}
				else
				{
					// We must loop when we have to recursively find all files
					if ((0 == (Yuni::Core::IO::Directory::Info::itFolder & flags))
						&& dirinfo.front().isFolder
						&& (0 != (Yuni::Core::IO::Directory::Info::itRecursive & flags)))
					{
						// Starting a new state
						push(dirinfo.front().filename);
						continue;
					}
					return true;
				}
			}
			while (true);
			return false;
		}


	public:
		unsigned int flags;
		LinkedList<DirInfo> dirinfo;
		# ifdef YUNI_OS_WINDOWS
		wchar_t wbuffer[wbufferMax];
		# endif

	};



	IteratorData* IteratorDataCreate(const char* folder, uint64 length, unsigned int flags)
	{
		if (length)
		{
			IteratorData* data = new IteratorData();
			data->flags = flags;
			data->push(folder, length);
			return data;
		}
		return NULL;
	}


	IteratorData* IteratorDataCopy(const IteratorData* data)
	{
		return (data) ? (new IteratorData(*data)) : NULL;
	}

	void IteratorDataFree(const IteratorData* data)
	{
		assert(data != NULL);
		delete data;
	}

	IteratorData* IteratorDataNext(IteratorData* data)
	{
		assert(data != NULL);
		if (data->next())
			return data;
		delete data;
		return NULL;
	}


	const String& IteratorDataFilename(const IteratorData* data)
	{
		assert(data != NULL);
		return data->dirinfo.front().filename;
	}


	const String& IteratorDataParentName(const IteratorData* data)
	{
		assert(data != NULL);
		return data->dirinfo.front().parent;
	}


	const String& IteratorDataName(const IteratorData* data)
	{
		assert(data != NULL);
		return data->dirinfo.front().name;
	}


	uint64 IteratorDataSize(const IteratorData* data)
	{
		assert(data != NULL);
		return data->dirinfo.front().size;
	}


	bool IteratorDataIsFolder(const IteratorData* data)
	{
		assert(data != NULL);
		return data->dirinfo.front().isFolder;
	}


	bool IteratorDataIsFile(const IteratorData* data)
	{
		assert(data != NULL);
		return ! data->dirinfo.front().isFolder;
	}





} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni

