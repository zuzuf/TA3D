#ifndef __YUNI_CORE_SYSTEM_MEMORY_HXX__
# define __YUNI_CORE_SYSTEM_MEMORY_HXX__


namespace Yuni
{
namespace System
{
namespace Memory
{


	inline Usage::Usage()
	{
		update();
	}


	inline Usage::Usage(const Usage& copy)
		:available(copy.available), total(copy.total)
	{}



} // namespace Memory
} // namespace System
} // namespace Yuni

#endif // __YUNI_CORE_SYSTEM_MEMORY_HXX__
