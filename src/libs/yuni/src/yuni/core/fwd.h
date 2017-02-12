#ifndef __YUNI_CORE_FWD_H__
# define __YUNI_CORE_FWD_H__

# include <iosfwd> // STL
# ifndef YUNI_OS_MSVC
#	include <bits/stringfwd.h>
# endif


namespace Yuni
{


	//! Type for a default behavior / policy
	struct Default {};

	//! Absence Option
	struct None {};


	// Will be removed as soon as the class CustomString is ready for use
	template<class C = char, int ChunkSizeT = 128> class StringBase;

	// Forward declaration for the base class CustomString<>
	template<unsigned int ChunkSizeT = 128, bool ExpandableT = true, bool ZeroTerminatedT = true>
	class CustomString;


} // namespace Yuni


#endif // __YUNI_CORE_FWD_H__
