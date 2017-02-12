#ifndef __YUNI_CORE_IO_IO_H__
# define __YUNI_CORE_IO_IO_H__

# include "../../yuni.h"
# include "../string.h"


namespace Yuni
{
namespace Core
{
/*
** \brief Low-level routines for file and directory support
** \ingroup IO
*/
namespace IO
{

	//! \name System-dependant variables
	//@{
	/*!
	** \brief The path-separator character according to the platform (ex: `/`)
	**
	** \ingroup IO
	*/
	extern const char Separator; // '/'

	/*!
	** \brief The path-separator character according to the platform (stored in a string instead of a char)
	**
	** \ingroup IO
	*/
	extern const char* SeparatorAsString; // "/"

	/*!
	** \brief Constant acoording a type
	**
	** These variables are identical to SeparatorAsString and Separator
	** but are charactere-type dependant.
	** \ingroup IO
	*/
	template<typename C /* = char*/> struct Constant;
	template<> struct Constant<char>
	{
		// The complete specialization with wchar_t is in directory.hxx

		//! The path-separator character according to the platform (ex: `/`)
		static const char  Separator; // '/';
		//! The path-separator character according to the platform (stored in a string instead of a char)
		static const char* SeparatorAsString; // "/"
		//! All path-separator characters, for all platforms
		static const char* AllSeparators; // "\\/"

		//! Dot
		static const char  Dot; // '.';

	}; // class Constant<char>

	template<> struct Constant<wchar_t>
	{
		//! The path-separator character according to the platform (ex: `/`)
		static const wchar_t  Separator; // L'/';
		//! The path-separator character according to the platform (stored in a string instead of a char)
		static const wchar_t* SeparatorAsString; // = L"/";
		//! All path-separator characters, for all platforms
		static const wchar_t* AllSeparators; // = L"\\/";
		//! Dot
		static const wchar_t  Dot; // L'.';
	};
	//@}



	/*!
	** \brief Flow control used in the IO module
	*/
	enum Flow
	{
		//! Abort the whole process
		flowAbort = 0,
		//! Continue
		flowContinue,
		//! Skip the current item
		flowSkip,
	};


	/*!
	** \brief Type of a single node (bitmask)
	*/
	enum NodeType
	{
		//! The node doest not exist
		typeUnknown = 0,
		//! The node is a folder
		typeFolder = 1,
		//! The node is a file
		typeFile = 2,
	};


	enum IOError
	{
		//! No error
		ioErrNone = 0,
		//! Generic error
		ioErrUnknown,
		//! Bad filename
		ioErrBadFilename,
		//! The file could not be loaded - not found or permission error
		ioErrNotFound,
		//! A hard memory limit has been reached
		ioErrMemoryLimit,
		//! It is impossible to overwrite an eisting file
		ioErrOverwriteNotAllowed,
	};


	/*!
	** \brief Extract the path part of a filename
	** \ingroup IO
	**
	** The path part will be extracted according the system-(un)dependant path-separator
	** \code
	**    String outputString;
	**	  Core::IO::Filename::ExtractFilePath(outputString, "/tmp/foo.txt");
	**    std::cout << outputString << std::endl; // writes `/tmp`
	** \endcode
	**
	** \param[out] out The output string
	** \param[in] p The filename
	** \param[in] systemDependant Consider only the system-dependant path-separator
	*/
	template<class StringT1, class StringT2>
	void ExtractFilePath(StringT1& out, const StringT2& p, const bool systemDependant = false);



	/*!
	** \brief Extract the bare file name
	**
	** \ingroup IO
	**
	** The file name will be extracted according the last occurence
	** of the system-dependant path-separator (if systemDependant = true)
	**
	** \param p The original filename
	** \param systemDependant Consider only the system-dependant path-separator
	** \return The bare filename from the original one
	**
	** \see Paths::Separator
	*/
	template<class StringT1, class StringT2>
	void ExtractFileName(StringT1& out, const StringT2& p, const bool systemDependant = true);


	/*!
	** \brief Extract the bare file name without its extension
	**
	** \ingroup IO
	**
	** The file name will be extracted according the last occurence
	** of the system-dependant path-separator (if systemDependant = true).
	**
	** \param p The original file name
	** \param systemDependant Consider only the system-dependant path-separator
	** \return The bare file name without its extension
	**
	** \see Paths::Separator
	*/
	template<class StringT1, class StringT2>
	void ExtractFileNameWithoutExtension(StringT1& out, const StringT2& p, const bool systemDependant = true);


	/*!
	** \brief Extract the extention of a file name
	**
	** \ingroup IO
	**
	** \param out Variable where the result will be appended
	** \param filename The original filename
	** \param dot True to include the dot when extracting the extension
	** \param clear True to clear `out` before processing
	** \return True if an extension has been found
	*/
	template<class StringT1, class StringT2>
	bool ExtractExtension(StringT1& out, const StringT2& filename, bool dot = true, bool clear = true);


	/*!
	** \brief Get if a path is absolute
	**
	** \ingroup IO
	**
	** \param filename The path or the filename to test
	** \return True if the given filename is an absolute path, false otherwise (or empty)
	*/
	template<class StringT> bool IsAbsolute(const StringT& filename);


	/*!
	** \brief Get if a path is relative
	**
	** \ingroup IO
	**
	** \param filename The path or the filename to test
	** \return True if the given filename is an absolute path, false otherwise (or empty)
	*/
	template<class StringT> bool IsRelative(const StringT& filename);


	/*!
	** \brief Make a path absolute
	**
	** \ingroup IO
	**
	** The current directory will be used when the given path is not absolute.
	** \param[out] out         The variable where to write the result
	** \param      filename    The filename to make absolute
	** \param      clearBefore True to clean @out before
	*/
	template<class StringT1, class StringT2>
	void MakeAbsolute(StringT1& out, const StringT2& path, bool clearBefore = true);

	/*!
	** \brief Make a path absolute
	**
	** \ingroup IO
	**
	** The current directory will be used when the given path is not absolute.
	** \param[out] out         The variable where to write the result
	** \param      filename    The filename to make absolute
	** \param      currentPath A custom current path to use if the filename is not absolute
	** \param      clearBefore True to clean @out before
	*/
	template<class StringT1, class StringT2, class StringT3>
	void MakeAbsolute(StringT1& out, const StringT2& path, const StringT3& currentPath, bool clearBefore = true);


	/*!
	** \brief Replace the extension
	**
	** \ingroup IO
	** \code
	** std::string s = "file.avi";
	** Yuni::Core::IO::ReplaceExtension(s, ".mpeg");
	** std::cout << s << std::endl; // file.mpeg
	** \endcode
	**
	** \param[in,out] filename     The original filename
	** \param         newExtension The new extension (dot included, ex: `.ota`)
	** \return True if the extension has been replaced (means `found and replaced`)
	*/
	template<class StringT1, class StringT2>
	bool ReplaceExtension(StringT1& filename, const StringT2& newExtension);


	/*!
	** \brief Test if a node exists (whatever its type, a folder or a file)
	**
	** \ingroup IO
	** \param filename The file/directory to test
	** \return True if it exists, false otherwise
	*/
	template<class StringT> bool Exists(const StringT& filename);

	/*!
	** \brief Get the type of a node
	**
	** \ingroup IO
	** \param filename The file/directory to test
	** \return True if it exists, false otherwise
	*/
	template<class StringT> NodeType TypeOf(const StringT& filename);



	/*!
	** \brief Normalize a filename
	**
	** The input can be a Windows-style or a Unix-style path, with mixed slasles and anti-slashes.
	** This routine removes dot segments (`.` and `..`) from a given filename (when
	** possible).
	** Any final slash will be removed.
	**
	** \bug The relative filenames like C:..\folder1\folder2 are not handled properly
	**
	** \param[out] out            A string (any class compliant to std::string) where to write the result
	** \param      in             A path/filename to normalize
	** \param      inLength       Length of #in (optional, -1 for autodetection)
	** \param      replaceSlashes True to replace slashes according the local OS conventions. False to keep
	**                            as it.
	*/
	template<class StringT1, class StringT2>
	void Normalize(StringT1& out, const StringT2& in, unsigned int inLength = (unsigned int)-1,
		bool replaceSlashes = true);






} // namespace IO
} // namespace Core
} // namespace Yuni

# include "directory.h"
# include "io.hxx"

#endif // __YUNI_CORE_IO_IO_H__
