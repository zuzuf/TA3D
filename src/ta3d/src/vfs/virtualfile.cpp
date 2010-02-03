#include "virtualfile.h"
#include <misc/math.h>

namespace TA3D
{
	namespace UTILS
	{
		void VirtualFile::setBuffer(byte *buf, int s)
		{
			pos = 0;
			bufferSize = s;
			if (buffer)
				delete[] buffer;
			buffer = buf;
			if (buffer == NULL)
				bufferSize = 0;
		}

		VirtualFile::VirtualFile() : buffer(NULL), bufferSize(0), pos(0)
		{}

		VirtualFile::~VirtualFile()
		{
			setBuffer(NULL, 0);
		}

		bool VirtualFile::eof()
		{
			return pos == bufferSize;
		}

		bool VirtualFile::isOpen()
		{
			return buffer != NULL;
		}

		int VirtualFile::size()
		{
			return bufferSize;
		}

		int VirtualFile::tell()
		{
			return pos;
		}

		void VirtualFile::seek(int pos)
		{
			this->pos = Math::Clamp(pos, 0, bufferSize);
		}

		void VirtualFile::read(void *p, int s)
		{
			if (pos == bufferSize || s <= 0)
				return;
			int n = Math::Min(s, bufferSize - pos);
			memcpy(p, buffer + pos, n);
			pos += n;
		}

		bool VirtualFile::readLine(String &line)
		{
			if (pos == bufferSize)
				return false;

			line.clear();
			char *end = (char*)buffer + bufferSize;
			for(char *p = (char*)buffer + pos ; p != end && *p != 0 && *p != 13 && *p != 10 ; ++pos, ++p)
				line << *p;

			return true;
		}
	}
}
