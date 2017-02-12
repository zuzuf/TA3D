#ifndef __YUNI_CORE_SYSTEM_MEMORY_H__
# define __YUNI_CORE_SYSTEM_MEMORY_H__

# include "../../yuni.h"


namespace Yuni
{
namespace System
{
namespace Memory
{


	/*!
	** \brief Get the amount of total physical memory
	**
	** When information about the memory usage can not be retrieved, a default
	** value will be used. (see defaultAvailable)
	** \return A value in bytes
	*/
	uint64 Total();


	/*!
	** \brief Get the amount of available physical memory
	**
	** When information about the memory usage can not be retrieved, a default
	** value will be used. (see defaultAvailable)
	** \return A value in bytes
	*/
	uint64 Available();



	/*!
	** \brief Information about the current memory usage
	**
	** When information about the current memory usage could not be
	** retrieved, some default values are used (see defaultTotal and defaultAvailable).
	** This is the case for example when the implementation is missing for the
	** current operating system. This method ensures that the values are always valid.
	**
	** Performance tip: If you're interrested in a single value, you should use
	**   the corresponding global routine instead, which will be at least as
	**   fast as this this class, sometimes better.
	*/
	class Usage
	{
	public:
		//! \name Constructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Usage();
		/*!
		** \brief Copy constructor
		**
		** The values will remain untouched.
		*/
		Usage(const Usage& copy);
		//@}

		/*!
		** \brief Update information about the current memory usage
		**
		** This routine is automatically called by the constructor.
		*/
		bool update();

	public:
		//! The amount of the available physical memory (in bytes)
		uint64 available;
		//! The amount of the total physical memory (in bytes)
		uint64 total;

	}; // class Usage






	/*!
	** \brief Constants to use when information about the memory usage could not
	**   be retrieved
	*/
	enum
	{
		//! The default amount of available physical memory
		defaultAvailable = 1024 * 1024 * 512,  // 512Mo
		//! The default amount of total physical memory
		defaultTotal     = 1024 * 1024 * 1024, // 1Go
	};




} // namespace Memory
} // namespace System
} // namespace Yuni

# include "memory.hxx"

#endif // __YUNI_CORE_SYSTEM_MEMORY_H__
