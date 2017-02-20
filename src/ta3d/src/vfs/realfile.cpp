#include "realfile.h"

namespace TA3D
{
	namespace UTILS
	{
		RealFile::RealFile() : buffer(NULL)
		{
		}

		RealFile::RealFile(const QString &filename) : buffer(NULL)
		{
			open(filename);
		}

		RealFile::~RealFile()
		{
			sFile.close();
			if (buffer)
				delete[] buffer;
		}

		void RealFile::open(const QString &filename)
		{
			sFile.close();
			if (buffer)
				delete[] buffer;
			buffer = NULL;
			sFile.open(filename, Yuni::Core::IO::OpenMode::read);
			realFilename = filename;
		}

		bool RealFile::isOpen()
		{
			return sFile.opened();
		}

		bool RealFile::eof()
		{
			if (!sFile.opened())
				return true;
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
			if (!sFile.opened())
				return 0;
			return int(sFile.tell());
		}

		void RealFile::seek(int pos)
		{
			sFile.seekFromBeginning(pos);
		}

		int RealFile::read(void *p, int s)
		{
			if (!sFile.opened())
				return 0;
			return int(sFile.read((char*)p, s));
		}

		bool RealFile::readLine(QString &line)
		{
			if (sFile.eof())
				return false;

			line.clear();
			for(int c = getc() ; c != 0 && c != 13 && c != 10 && c != -1 ; c = getc())
                line.push_back(char(c));

			return true;
		}

		const char *RealFile::data()
		{
			if (buffer)
				return buffer;

			buffer = new char[size() + 1];
			int pos = tell();
			seek(0);
			read(buffer, size());
			buffer[size()] = 0;
			seek(pos);

			return buffer;
		}

		void RealFile::close()
		{
			sFile.close();
			if (buffer)
				delete[] buffer;
			buffer = NULL;
		}

		bool RealFile::isReal() const
		{
			return true;
		}

		const QString &RealFile::getRealFilename() const
		{
			return realFilename;
		}
	}
}
