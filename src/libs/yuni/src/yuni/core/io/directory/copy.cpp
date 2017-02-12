
#include "../directory.h"
#include "../../slist.h"
#include "info.h"
#include "../file.h"



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


	bool DummyCopyUpdateEvent(Yuni::Core::IO::Directory::CopyState, const String&, const String&, uint64, uint64)
	{
		return true;
	}


	struct InfoItem
	{
		bool isFile;
		uint64  size;
		String filename;
	};
	typedef LinkedList<InfoItem> List;



	bool RecursiveCopy(const char* src, unsigned int srclen, const char* dst, unsigned int dstlen, bool recursive,
		bool overwrite, const Yuni::Core::IO::Directory::CopyOnUpdateBind& onUpdate)
	{
		using namespace Yuni::Core::IO::Directory;

		// normalize paths
		String fsrc;
		Yuni::Core::IO::Normalize(fsrc, src, srclen);
		if (fsrc.empty())
			return false;

		String fdst;
		Yuni::Core::IO::Normalize(fdst, dst, dstlen);

		// The list of files to copy
		List list;
		uint64 totalSize = 0;

		// Adding the target folder, to create it if required
		if (!onUpdate(cpsGatheringInformation, fdst, fdst, 0, 1))
			return false;
		Yuni::Core::IO::Directory::Create(fdst);

		{
			Yuni::Core::IO::Directory::Info info(fsrc);
			if (recursive)
			{
				const Yuni::Core::IO::Directory::Info::recursive_iterator& end = info.recursive_end();
				for (Yuni::Core::IO::Directory::Info::recursive_iterator i = info.recursive_begin(); i != end; ++i)
				{
					list.push_back();
					InfoItem& info = list.back();
					info.filename = i.filename();
					info.isFile   = i.isFile();
					totalSize += i.size();
					if (!onUpdate(cpsGatheringInformation, *i, *i, 0, list.size()))
						return false;
				}
			}
			else
			{
				const Yuni::Core::IO::Directory::Info::recursive_iterator& end = info.recursive_end();
				for (Yuni::Core::IO::Directory::Info::recursive_iterator i = info.recursive_begin(); i != end; ++i)
				{
					list.push_back();
					InfoItem& info = list.back();
					info.filename = i.filename();
					info.isFile   = i.isFile();
					totalSize += i.size();

					if (!onUpdate(cpsGatheringInformation, i.filename(), i.filename(), 0, list.size()))
						return false;
				}
			}
		}

		uint64 current = 0;
		// A temporary string
		String tmp;

		// Streams : in the worst scenario, the last file to copy will be closed
		// at the end of this routine
		// Stream on the source file
		Yuni::Core::IO::File::Stream fromFile;
		// Stream on the target file
		Yuni::Core::IO::File::Stream toFile;

		// A temporary buffer for copying files' contents
		// 16k seems to be a good choice (better than smaller block size when used
		// in Virtual Machines)
		enum { bufferSize = 8192 };
		char* buffer = new char[bufferSize];

		// reduce overhead brought by `onUpdate`
		unsigned int skip = 8;

		const List::const_iterator end = list.end();
		for (List::const_iterator i = list.begin(); i != end; ++i)
		{
			// alias to the current information block
			const InfoItem& info = *i;

			// Address of the target file
			tmp = fdst; // without any OS-dependant separator
			if (fsrc.size() < info.filename.size())
				tmp.append(info.filename.c_str() + fsrc.size(), info.filename.size() - fsrc.size());

			if (!info.isFile)
			{
				// The target file is actually a folder
				// We have to create it before copying its content
				if (!onUpdate(cpsCopying, info.filename, tmp, current, totalSize)
					|| !Yuni::Core::IO::Directory::Create(tmp))
				{
					delete[] buffer;
					return false;
				}
			}
			else
			{
				// The target file is a real file (and not a folder)
				// Checking first for overwritting
				if (!overwrite && Yuni::Core::IO::Exists(tmp))
					continue;

				// Try to open the source file
				// The previous opened source file will be closed here
				if (fromFile.open(info.filename, Yuni::Core::IO::OpenMode::read))
				{
					// Try to open for writing the target file
					// The previous opened target file will be closed here
					if (toFile.open(tmp, Yuni::Core::IO::OpenMode::write | Yuni::Core::IO::OpenMode::truncate))
					{
						// reading the whole source file
						size_t numRead;
						while ((numRead = fromFile.read(buffer, bufferSize)) != 0)
						{
							// progression
							current += numRead;

							// Trying to copy the block which has just been read
							if (numRead != toFile.write((const char*)buffer, numRead))
							{
								delete[] buffer;
								return false;
							}

							// Notify the user from time to time about the progression
							if (!--skip)
							{
								if (!onUpdate(cpsCopying, info.filename, tmp, current, totalSize))
								{
									delete[] buffer;
									return false;
								}
								skip = 8;
							}
						} // read
					}
				}
			}
		}

		delete[] buffer;

		return true;
	}




} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni

