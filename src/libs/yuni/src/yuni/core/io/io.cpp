
#include "io.h"


namespace Yuni
{
namespace Core
{
namespace IO
{


	# ifdef YUNI_OS_WINDOWS
	const char Separator = '\\';
	# else
	const char Separator = '/';
	# endif

	//! The path-separator character according to the platform (stored in a string instead of a char)
	//! \ingroup PathsAndFiles
	# ifdef YUNI_OS_WINDOWS
	const char* SeparatorAsString = "\\";
	# else
	const char* SeparatorAsString = "/";
	# endif


	# ifdef YUNI_OS_WINDOWS
	const char* Constant<char>::SeparatorAsString = "\\";
	# else
	const char* Constant<char>::SeparatorAsString = "/";
	# endif

	# ifdef YUNI_OS_WINDOWS
	const wchar_t* Constant<wchar_t>::SeparatorAsString = L"\\";
	# else
	const wchar_t* Constant<wchar_t>::SeparatorAsString = L"/";
	# endif


	# ifdef YUNI_OS_WINDOWS
	const char Constant<char>::Separator = '\\';
	# else
	const char Constant<char>::Separator = '/';
	# endif

	# ifdef YUNI_OS_WINDOWS
	const wchar_t Constant<wchar_t>::Separator = L'\\';
	# else
	const wchar_t Constant<wchar_t>::Separator = L'/';
	# endif

	const wchar_t* Constant<wchar_t>::AllSeparators = L"\\/";
	const char*    Constant<char>   ::AllSeparators = "\\/";

	const wchar_t Constant<wchar_t>::Dot = L'.';
	const char    Constant<char>   ::Dot = '.';



} // namespace IO
} // namespace Core
} // namespace Yuni

