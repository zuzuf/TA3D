/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2006  Roland BROCHARD

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

/*
**  File: file.h
** Notes:
**   Zuzuf:  This module defines the File interface which can represent a real file
**           or a memory buffer
*/

#ifndef __TA3D_UTILS_FILE_H__
# define __TA3D_UTILS_FILE_H__

# include <misc/string.h>

namespace TA3D
{
	namespace UTILS
	{
		class File
		{
		public:
			//! This is an abstract class, its destructor must be virtual
			virtual ~File()	{}

			//! Read s bytes and write them in memory pointed by p
			virtual int read(void *p, int s) = 0;
			//! Returns the size of the file
			virtual int size() = 0;
			//! Returns true if end of file has been reached
			virtual bool eof() = 0;
			//! Returns current position in file
			virtual int tell() = 0;
			//! Set absolute position in file
			virtual void seek(int pos) = 0;
			//! Read a single line from the file, returns true while end of file has not been reached
			virtual bool readLine(QString &line) = 0;
			//! Returns true if the file is opened
			virtual bool isOpen() = 0;
			//! Returns a pointer to a memory buffer containing the file
			virtual const char *data() = 0;
			//! Close the file
			virtual void close() = 0;
			//! Tell if this is a real file (useful for use with external libraries)
			virtual bool isReal() const = 0;
			//! Returns the real filename (if it's a real file)
			virtual const QString &getRealFilename() const = 0;

			virtual File &operator=(const File &f) = 0;

            inline QString getString()
			{
				if (eof())
					return QString();

				QString str;
				for(int c = getc() ; c != 0 && c != -1 ; c = getc())
                    str.push_back(char(c));

				return str;
			}

			inline int getc()
			{
				if (eof())
					return -1;
				char c(0);
				read(&c, 1);
				return c;
			}

			//! Read binary data to fill a variable of arbitrary type
			template<class T>
					inline void read(T &a)	{	read((char*)&a, sizeof(a));	}
			template<class T>
					inline File &operator>>(T &a)	{	read(a);	return *this;	}

			template<class T>
					void load(T &out)
			{
				QString line;
				while(this->readLine(line))
					out.push_back(line);
			}
		};
	}
}
#endif
