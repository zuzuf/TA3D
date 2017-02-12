#ifndef __YUI_CORE_IO_FILE_OPEN_MODE_H__
# define __YUI_CORE_IO_FILE_OPEN_MODE_H__


namespace Yuni
{
namespace Core
{
namespace IO
{
namespace OpenMode
{


	enum Type
	{
		none = 0,
		//! Open the file for reading
		read = 1,
		//! Open the file for writing. It will be created if it does not exist.
		write = 2,
		//! (write mode) The stream is positioned at the end of the file
		append = 4,
		//! (write mode) Truncte the file if it already exists
		truncate = 8,
	};



	/*!
	** \brief Convert an open mode into a C-String for 'fopen'
	*/
	const char* ToCString(const int mode);

	/*!
	** \brief Convert an open mode into a C-String for 'fwopen'
	*/
	const wchar_t*  ToWCString(const int mode);





} // namespace OpenMode
} // namespace IO
} // namespace Core
} // namespace Yuni

#endif // __YUI_CORE_IO_FILE_OPEN_MODE_H__
