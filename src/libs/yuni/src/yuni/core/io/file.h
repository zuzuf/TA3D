#ifndef __YUNI_CORE_FS_FILES_H__
# define __YUNI_CORE_FS_FILES_H__

# include "io.h"
# include "file/openmode.h"



namespace Yuni
{
namespace Core
{
namespace IO
{

/*!
** \brief File manipulation functions
*/
namespace File
{

	// constants
	enum
	{
		//! The maximum allowed size for a file in memory (Default: 80Mo)
		sizeHardLimit = 83886080u,  // 80Mo = 80 * 1024 * 1024
	};



	/*!
	** \brief Test if a node exists and is actually a file
	**
	** To test if a node exists whatever its nature, you should use
	** Core::IO::Exists() instead.
	**
	** \param filename The folder/filename to test
	** \return True if it exists, false otherwise
	*/
	template<class StringT> bool Exists(const StringT& filename);


	/*!
	** \brief Get the size (in bytes) of a file
	**
	** \param filename The filename
	** \param[out] size The size (in bytes) of the file. The value is guaranteed to be set (null) is an error has occured
	** \return True if the operation succeeded, False otherwise
	*/
	template<class StringT> bool Size(const StringT& filename, uint64& size);

	/*!
	** \brief Get the size (in bytes) of a file
	**
	** \code
	** #include <yuni/yuni.h>
	** #include <yuni/core/unit/data.h>
	** #include <yuni/core/io/file.h>
	** #include <iostream>
	**
	** using namespace Yuni;
	** int main()
	** {
	**	Unit::Data::Octet::SIBaseUnit<uint64> size = Core::IO::File::Size("/path/to/my/file");
	**	std::cout << Unit::Data::Octet::Megaoctet<double>(size) << std::endl;
	**	return 0;
	** }
	** \endcode
	**
	** \param filename The filename
	** \return The size (in bytes) of the file. The returned value is guaranteed to be null is an error has occured
	*/
	template<class StringT> uint64 Size(const StringT& filename);



	//! \name Load the content of a file
	//@{
	/*!
	** \brief Load the entire content of a file into memory
	**
	** If the file size is greater than @hardlimit, the content will be truncated (see 'ioErrMemoryLimit').
	** \param[out] out The content of the file
	** \param filename The filename to open
	** \param hardlimit If the size of the file exceeds this limit, it will not be loaded
	** \return ioErrNone if successful
	*/
	template<class StringT1, class StringT2>
	IOError LoadFromFile(StringT1& out, const StringT2& filename, const uint64 hardlimit = sizeHardLimit);


	/*!
	** \brief Save the content of a string into a file
	**
	** \param filename The filename to create/overwrite
	** \param content The new content of the file
	** \return True if the operation succeeded, false otherwise
	*/
	template<class StringT, class U> bool SaveToFile(const StringT& filename, const U& content);
	//@}


	/*!
	** \brief Copy a single file
	**
	** \param from The source file
	** \param to The target file
	** \param overwrite Overwrite the target file if already exists
	** \return ioErrNone if the operation succeeded
	*/
	template<class StringT1, class StringT2>
	IOError Copy(const StringT1& from, const StringT2& to, bool overwrite = true);


	/*!
	** \brief Delete a file
	**
	** \param filename The file to delete
	*/
	template<class StringT> IOError Delete(const StringT& filename);


	/*!
	** \brief Create or erase a file
	**
	** \param filename An UTF8 filename
	** \return True if the file has been created or truncated
	*/
	template<class StringT> bool CreateEmptyFile(const StringT& filename);


	/*!
	** \brief Set the content of a file with an arbitrary string
	**
	** \code
	** Core::IO::File::SetContent("/tmp/anyfile.txt", "Hello world !\n");
	** \endcode
	*/
	template<class StringT, class U> bool SetContent(const StringT& filename, const U& content);

	/*!
	** \brief Append the content of an arbitrary string to a file
	**
	** \code
	** Core::IO::File::AppendContent("/tmp/anyfile.txt", "lorem ipsumi\n");
	** \endcode
	*/
	template<class StringT, class U> bool AppendContent(const StringT& filename, const U& content);






} // namespace File
} // namespace IO
} // namespace Core
} // namespace Yuni

# include "io.h"
# include "file/stream.h"
# include "file/file.hxx"

#endif // __YUNI_CORE_FS_FILES_H__
