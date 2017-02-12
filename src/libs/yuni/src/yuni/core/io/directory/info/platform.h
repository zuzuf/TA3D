#ifndef __YUNI_CORE_IO_DIRECTORY_INFO_PLATFORM_H__
# define __YUNI_CORE_IO_DIRECTORY_INFO_PLATFORM_H__

# include "../../../string.h"


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

	//! Platform-dependant data implementation
	class IteratorData;



	IteratorData* IteratorDataCreate(const char* folder, uint64 length, unsigned int flags);

	IteratorData* IteratorDataCopy(const IteratorData*);

	void IteratorDataFree(const IteratorData*);

	bool IteratorDataValid(const IteratorData*);

	IteratorData* IteratorDataNext(IteratorData*);

	const String& IteratorDataFilename(const IteratorData*);

	const String& IteratorDataParentName(const IteratorData*);

	const String& IteratorDataName(const IteratorData*);

	uint64 IteratorDataSize(const IteratorData*);

	bool IteratorDataIsFolder(const IteratorData*);

	bool IteratorDataIsFile(const IteratorData*);




} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_IO_DIRECTORY_INFO_PLATFORM_H__
