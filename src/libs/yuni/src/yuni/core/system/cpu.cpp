
#include "cpu.h"

#if defined(YUNI_OS_DARWIN) || defined(YUNI_OS_FREEBSD) || defined(YUNI_OS_NETBSD) || defined(YUNI_OS_OPENBSD)
#	include <sys/param.h>
#	include <sys/sysctl.h>
#endif
#if defined(YUNI_OS_LINUX) || defined(YUNI_OS_CYGWIN)
#	include <fstream>
#	include "../string.h"
#endif
#include "windows.hdr.h"


namespace Yuni
{
namespace System
{
namespace CPU
{


#if defined(YUNI_OS_DARWIN) || defined(YUNI_OS_FREEBSD) || defined(YUNI_OS_NETBSD) || defined(YUNI_OS_OPENBSD)
# define YUNI_CPU_COUNT_HAS_IMPLEMENTATION
	unsigned int Count()
	{
		int count;
		size_t size = sizeof(count);

		if (sysctlbyname("hw.ncpu", &count, &size, NULL, 0))
			return 1;
		return (count <= 0) ? 1 : static_cast<unsigned int>(count);
	}
#endif


#ifdef YUNI_OS_WINDOWS
# define YUNI_CPU_COUNT_HAS_IMPLEMENTATION
	unsigned int Count()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors;
	}
#endif


#if defined(YUNI_OS_LINUX) || defined(YUNI_OS_CYGWIN)
# define YUNI_CPU_COUNT_HAS_IMPLEMENTATION
	unsigned int Count()
	{
		/*
		* It seems there's no better way to get this info on Linux systems.
		* If somebody can find it without depending on the location of /proc,
		* please patch this function.
		*/
		int count = 0;
		std::ifstream cpuInfo("/proc/cpuinfo", std::ifstream::in);
		std::string lineBuffer;

		std::getline(cpuInfo, lineBuffer);
		while (cpuInfo.good())
		{
			if (std::string::npos != lineBuffer.find("processor"))
				++count;
			std::getline(cpuInfo, lineBuffer);
		}
		cpuInfo.close();

		return (0 == count ? 1 : count);
	}
#endif




#ifndef YUNI_CPU_COUNT_HAS_IMPLEMENTATION
#  warning "The method Yuni::System::CPU::Count() has not been implemented for the current platform"

	unsigned int Count()
	{
		return 1; // Default value
	}

#endif



} // namespace CPU
} // namespace System
} // namespace Yuni


