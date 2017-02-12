#ifndef __YUNI_CORE_CUSTOMSTRING_WSTRING_HXX__
# define __YUNI_CORE_CUSTOMSTRING_WSTRING_HXX__

# ifdef YUNI_OS_WINDOWS

namespace Yuni
{
namespace Private
{

	template<bool UNCPrefix>
	template<class StringT>
	inline WString<UNCPrefix>::WString(const StringT& string)
	{
		prepareWString(Traits::CString<StringT>::Perform(string),
			Traits::Length<StringT, unsigned int>::Value(string));
	}


	template<bool UNCPrefix>
	inline WString<UNCPrefix>::WString(const char* cstring, unsigned int size)
	{
		prepareWString(cstring, size);
	}


	template<bool UNCPrefix>
	inline WString<UNCPrefix>::~WString()
	{
		delete[] pWString;
	}


	template<bool UNCPrefix>
	inline unsigned int WString<UNCPrefix>::size() const
	{
		return pSize;
	}


	template<bool UNCPrefix>
	inline void WString<UNCPrefix>::replace(char from, char to)
	{
		for (unsigned int i = 0; i != pSize; ++i)
		{
			if (pWString[i] == from)
				pWString[i] = to;
		}
	}


	template<bool UNCPrefix>
	inline bool WString<UNCPrefix>::empty() const
	{
		return !pSize;
	}


	template<bool UNCPrefix>
	inline const wchar_t* WString<UNCPrefix>::c_str() const
	{
		return pWString;
	}


	template<bool UNCPrefix>
	inline wchar_t* WString<UNCPrefix>::c_str()
	{
		return pWString;
	}


	template<bool UNCPrefix>
	inline WString<UNCPrefix>::operator const wchar_t* () const
	{
		return pWString;
	}


	template<bool UNCPrefix>
	void WString<UNCPrefix>::prepareWString(const char* const cstring, unsigned int size)
	{
		if (!size)
		{
			if (UNCPrefix)
			{
				pSize = 4;
				pWString = new wchar_t[5];
				pWString[0] = L'\\';
				pWString[1] = L'\\';
				pWString[2] = L'?';
				pWString[3] = L'\\';
				pWString[4] = L'\0';
			}
			else
			{
				pSize = 0;
				pWString = NULL;
			}
			return;
		}

		// Offset according to the presence of the UNC prefix
		const unsigned int offset = UNCPrefix ? 4 : 0;

		// Size of the wide string. This value may be modified if there is not
		// enough room for converting the C-String
		pSize = size + offset;

		// Allocate and convert the C-String. Several iterations may be required
		// for allocating enough room for the conversion.
		do
		{
			pWString = new wchar_t[pSize + 1];
			if (!pWString)
			{
				// Impossible to allocate the buffer. Aborting.
				pWString = NULL;
				pSize = 0;
				return;
			}

			// Appending the Windows UNC prefix
			if (UNCPrefix)
			{
				pWString[0] = L'\\';
				pWString[1] = L'\\';
				pWString[2] = L'?';
				pWString[3] = L'\\';
			}
			// Converting into Wide String
			const int n = MultiByteToWideChar(CP_UTF8, 0, cstring, size, pWString + offset, pSize + 1);
			if (n <= 0)
			{
				// An error has occured
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// There is not enough room for the conversion
					// Trying again with more rooms
					pSize += (pSize > 2) ? pSize / 2 : 2;
					delete[] pWString;
					continue;
				}
				// This is a real error. Aborting.
				delete[] pWString;
				pWString = NULL;
				pSize = 0;
				return;
			}
			// Adding the final zero
			pWString[n + offset] = L'\0';
			return;
		}
		while (true);
	}





} // namespace Private
} // namespace Yuni

# endif // YUNI_OS_WINDOWS
#endif // __YUNI_CORE_CUSTOMSTRING_WSTRING_HXX__
