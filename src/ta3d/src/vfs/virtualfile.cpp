#include "virtualfile.h"
#include <misc/math.h>

namespace TA3D
{
	namespace UTILS
	{
		void VirtualFile::setBuffer(byte *buf, int s, int start, int end)
		{
			pos = 0;
			bufferSize = s;
			if (buffer)
				delete[] buffer;
			buffer = buf;
			if (buffer == NULL)
				bufferSize = 0;
			offset = Math::Max(start, 0);
			if (end == -1)
				streamSize = bufferSize + offset;
			else
				streamSize = Math::Max(end, offset + bufferSize);
		}

		void VirtualFile::copyBuffer(byte *buf, int s, int start, int end)
		{
			pos = 0;
			bufferSize = s;
			if (buffer)
				delete[] buffer;
			if (s)
			{
				buffer = new byte[s + 1];
				memcpy(buffer, buf, s);
				buffer[s] = 0;
			}
			else
				buffer = NULL;
			if (buffer == NULL)
				bufferSize = 0;
			offset = Math::Max(start, 0);
			if (end == -1)
				streamSize = bufferSize + offset;
			else
				streamSize = Math::Max(end, offset + bufferSize);
		}

		VirtualFile::VirtualFile() : buffer(NULL), bufferSize(0), pos(0), offset(0), streamSize(0)
		{}

		VirtualFile::VirtualFile(byte *buf, int s, int start, int end) : buffer(NULL), bufferSize(0), pos(0), offset(0), streamSize(0)
		{
			setBuffer(buf, s, start, end);
		}

		VirtualFile::~VirtualFile()
		{
			setBuffer(NULL, 0);
		}

		bool VirtualFile::eof()
		{
			return pos == streamSize;
		}

		bool VirtualFile::isOpen()
		{
			return buffer != NULL;
		}

		int VirtualFile::size()
		{
			return streamSize;
		}

		int VirtualFile::tell()
		{
			return pos;
		}

		void VirtualFile::seek(int pos)
		{
			this->pos = Math::Clamp(pos, 0, streamSize);
		}

		int VirtualFile::read(void *q, int s)
		{
			char *p = (char*)q;
			int k = 0;
			for ( ; s && pos < offset && pos < streamSize ; ++pos, --s, ++p, ++k)
				*p = 0;
			if (pos == streamSize || s <= 0)
				return k;
			int n = Math::Min(s, offset + bufferSize - pos);
			memcpy(p, buffer + pos, n);
			pos += n;
			s -= n;
			if (s)
			{
				p += n;
				for ( ; s && pos < streamSize ; ++pos, --s, ++p, ++k)
					*p = 0;
			}
			return k + n;
		}

		bool VirtualFile::readLine(QString &line)
		{
			if (pos == streamSize)
				return false;

			line.clear();
			if (pos < offset || pos >= offset + bufferSize)
			{
				++pos;
				return true;
			}

			char *end = (char*)buffer + offset + bufferSize;
			for(char *p = (char*)buffer + pos ; p != end && *p != 0 && *p != 13 && *p != 10 ; ++pos, ++p)
                line += *p;

			return true;
		}

		const char *VirtualFile::data()
		{
			return (const char*)buffer;
		}

		void VirtualFile::close()
		{
			if (buffer)
				delete[] buffer;
			buffer = NULL;
			bufferSize = 0;
			pos = 0;
			offset = 0;
			streamSize = 0;
		}

		bool VirtualFile::isReal() const
		{
			return false;
		}

        QString VirtualFile::getRealFilename() const
		{
            return QString();
		}
	}
}
