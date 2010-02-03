#include "realfile.h"

namespace TA3D
{
	namespace UTILS
	{
		RealFile::~RealFile()
		{
			sFile.close();
		}

		void RealFile::open(const String &filename)
		{
			sFile.close();
			sFile.open(filename, Yuni::Core::IO::File::OpenMode::read);
		}

		bool RealFile::isOpen()
		{
			return sFile.opened();
		}

		bool RealFile::eof()
		{
			return sFile.eof();
		}

		int RealFile::size()
		{
			if (!sFile.opened())
				return 0;
			ssize_t pos = sFile.tell();
			sFile.seekFromEndOfFile(0);
			ssize_t s = sFile.tell();
			sFile.seekFromBeginning(pos);
			return int(s);
		}

		int RealFile::tell()
		{
			return int(sFile.tell());
		}

		void RealFile::seek(int pos)
		{
			sFile.seekFromBeginning(pos);
		}

		void RealFile::read(void *p, int s)
		{
			sFile.read((char*)p, s);
		}

		bool RealFile::readLine(String &line)
		{
			if (sFile.eof())
				return false;

			line.clear();
			for(int c = getc() ; c != 0 && c != 13 && c != 10 && c != -1 ; c = getc())
				line << char(c);

			return true;
		}
	}
}
