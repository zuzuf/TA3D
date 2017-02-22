#include "realfile.h"

namespace TA3D
{
	namespace UTILS
	{
        RealFile::RealFile()
		{
		}

        RealFile::RealFile(const QString &filename)
		{
			open(filename);
		}

		RealFile::~RealFile()
		{
			sFile.close();
		}

		void RealFile::open(const QString &filename)
		{
			sFile.close();
            buffer.clear();
            sFile.setFileName(filename);
            sFile.open(QIODevice::ReadOnly);
		}

		bool RealFile::isOpen()
		{
            return sFile.isOpen();
		}

		bool RealFile::eof()
		{
            if (!sFile.isOpen())
				return true;
            return sFile.atEnd();
		}

		int RealFile::size()
		{
            if (!sFile.isOpen())
				return 0;
            return sFile.size();
        }

		int RealFile::tell()
		{
            if (!sFile.isOpen())
				return 0;
            return sFile.pos();
		}

		void RealFile::seek(int pos)
		{
            sFile.seek(pos);
		}

		int RealFile::read(void *p, int s)
		{
            if (!sFile.isOpen())
				return 0;
			return int(sFile.read((char*)p, s));
		}

		bool RealFile::readLine(QString &line)
		{
            if (sFile.atEnd())
				return false;

			line.clear();
			for(int c = getc() ; c != 0 && c != 13 && c != 10 && c != -1 ; c = getc())
                line.push_back(char(c));

			return true;
		}

		const char *RealFile::data()
		{
            if (!buffer.isEmpty())
                return buffer.data();

			int pos = tell();
			seek(0);
            buffer = sFile.readAll();
			seek(pos);

            return buffer.data();
		}

		void RealFile::close()
		{
			sFile.close();
            buffer.clear();
		}

		bool RealFile::isReal() const
		{
			return true;
		}

        QString RealFile::getRealFilename() const
		{
            return sFile.fileName();
		}
	}
}
