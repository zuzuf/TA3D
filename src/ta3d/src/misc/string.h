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
# include <QString>
# include <QList>


namespace TA3D
{
    inline QString Substr(const QString &str, int start, int len = -1)
	{
        if (len != -1)
            len = std::min<int>(len, str.size() - start);
        return str.mid(start, len);
	}

    inline QString ToUpper(const QString &str)
	{
        return str.toUpper();
	}

    inline QString ToLower(const QString &str)
	{
        return str.toLower();
	}

	int ASCIItoUTF8(const byte c, byte *out);

    char* ConvertToUTF8(const char* s);
    char* ConvertToUTF8(const char* s, const size_t len);
    char* ConvertToUTF8(const char* s, size_t size, uint32& newSize);

    sint32 SearchString(const QString& s, const QString& stringToSearch, const bool ignoreCase);

	/*!
	** \brief explode a command string into a vector of parameters : program param0 "parameter 1" param2 => {"program", "param0", "parameter 1", "param2"}
	** \brief s The command string to spli
	** \return The resulting vector of strings
	*/
    QStringList SplitCommand(const QString& s);

	/*!
    ** \brief Escape a QString in order to make it fit nicely between two '"'
	** \param s The string to convert
    ** \return A new QString
	*/
    QString Escape(const QString& s);




	/*!
    ** \brief Convert an UTF-8 QString into a WideChar QString
	**
	** \todo This class is here only to provide compatibility with FTGL 2.1.2 API which doesn't support UTF-8 encoding :/
	**  everyone will agree it's nasty, but it'll remain here until we get something better
	*/
    struct WString
	{
	public:
        WString(const char* s);
        WString(const QString& str);

		const wchar_t* cw_str() const {return pBuffer;}
	private:
		void fromUtf8(const char* s, size_t length);

	private:
		wchar_t pBuffer[5120];

    }; // class WQString






} // namespace TA3D

#endif // __TA3D_TOOLBOX_STRING_H__
