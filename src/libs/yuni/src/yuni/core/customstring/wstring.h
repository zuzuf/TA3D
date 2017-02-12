#ifndef __YUNI_CORE_CUSTOMSTRING_WSTRING_H__
# define __YUNI_CORE_CUSTOMSTRING_WSTRING_H__

# ifdef YUNI_OS_WINDOWS

# include "../../yuni.h"
# include "customstring.h"
# include "../system/windows.hdr.h"
# include "../noncopyable.h"


namespace Yuni
{
namespace Private
{

	/*!
	** \brief Convert a C-String into a Wide String (Windows Only)
	*/
	template<bool UNCPrefix = false>
	class WString : private NonCopyable<WString<UNCPrefix> >
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		*/
		template<class StringT> WString(const StringT& string);
		/*!
		** \brief Constructor with a C-String
		*/
		WString(const char* cstring, unsigned int size);
		//! Destructor
		~WString();
		//@}

		/*!
		** \brief Size of the wide string
		*/
		unsigned int size() const;

		/*!
		** \brief Get if the string is empty
		*/
		bool empty() const;

		/*!
		** \brief Get the wide string
		*/
		const wchar_t* c_str() const;

		/*!
		** \brief Get the wide string
		*/
		wchar_t* c_str();

		/*!
		** \brief Replace all occurences of a single char
		*/ 
		void replace(char from, char to);

		//! \name Operators
		//@{
		//! Cast to wchar_t*
		operator const wchar_t* () const;
		//@}


	private:
		//! Convert a C-String into a Wide String
		void prepareWString(const char* const cstring, unsigned int size);

	private:
		//! Wide string
		wchar_t* pWString;
		//! Size of the wide string
		unsigned int pSize;

	}; // class WString




} // namespace Private
} // namespace Yuni

# include "wstring.hxx"

# endif // YUNI_OS_WINDOWS
#endif // __YUNI_CORE_CUSTOMSTRING_WSTRING_H__
