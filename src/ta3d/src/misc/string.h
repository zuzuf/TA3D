/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#ifndef __TA3D_TOOLBOX_STRING_H__
# define __TA3D_TOOLBOX_STRING_H__

# include <stdafx.h>
# include <yuni/core/string.h>


namespace TA3D
{
	inline String Substr(const String &str, unsigned int start, unsigned int len = 0xFFFFFFFF)
	{
		len = std::min<unsigned int>(len, str.size() - start);
		return String(str.data() + start, len);
	}

	inline String SubstrUTF8(const String &str, unsigned int start, unsigned int len = 0xFFFFFFFF)
	{
		return String(str.utf8begin() + start, str.utf8begin() + start + std::min<unsigned int>(len, str.utf8size() - start));
	}

	inline String ToUpper(const String &str)
	{
		return String(str).toUpper();
	}

	inline String ToLower(const String &str)
	{
		return String(str).toLower();
	}

	int ASCIItoUTF8(const byte c, byte *out);

	String InttoUTF8(const uint16 c);

	/*!
	** \brief Convert a string from ASCII to UTF8
	** \param s The string to convert
	** \return A new Null-terminated String (must be deleted with the keyword `delete[]`), even if s is NULL
	*/
	char* ConvertToUTF8(const char* s);

	/*!
	** \brief Convert a string from ASCII to UTF8
	** \param s The string to convert
	** \param len The length of the string
	** \param[out] The new size
	** \return A new Null-terminated String (must be deleted with the keyword `delete[]`), even if s is NULL
	*/
	char* ConvertToUTF8(const char* s, const size_t len);
	char* ConvertToUTF8(const char* s, const size_t len, uint32& newSize);

	/*!
	** \brief Convert a string from ASCII to UTF8
	** \param s The string to convert
	** \return A new String
	*/
	String ConvertToUTF8(const String& s);


	sint32 SearchString(const String& s, const String& stringToSearch, const bool ignoreCase);

	/*!
	** \brief explode a command string into a vector of parameters : program param0 "parameter 1" param2 => {"program", "param0", "parameter 1", "param2"}
	** \brief s The command string to spli
	** \return The resulting vector of strings
	*/
	String::Vector SplitCommand(const String& s);

	/*!
	** \brief Escape a String in order to make it fit nicely between two '"'
	** \param s The string to convert
	** \return A new String
	*/
	String Escape(const String& s);

	/*!
	** \brief Convert an UTF-8 String into a WideChar String
	** \todo This class is here only to provide compatibility with FTGL 2.1.2 API which doesn't support UTF-8 encoding :/
	**  everyone will agree it's nasty, but it'll remain here until we get something better
	*/
	struct WString
	{
	public:
		WString(const char* s);
		WString(const String& str);

		const wchar_t* cw_str() const {return pBuffer;}
	private:
		void fromUtf8(const char* s, size_t length);

	private:
		wchar_t pBuffer[5120];

	}; // class WString


} // namespace TA3D

#endif // __TA3D_TOOLBOX_STRING_H__
