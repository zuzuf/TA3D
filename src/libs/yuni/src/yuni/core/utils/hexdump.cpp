#include "hexdump.h"

namespace Yuni
{
namespace Core
{
namespace Utils
{


	void Hexdump::dumpHexadecimal(String& line, const char* buffer, unsigned int size) const
	{
		for (unsigned int printed = 0; printed < 0x10; ++printed)
		{
			if (printed < size)
				line.appendFormat("%02x", (unsigned int) (*(unsigned char *)(buffer + printed)));
			else
				line.append("  ");

			if (0 != printed % 2)
				line.append(' ');
		}
	}


	void Hexdump::dumpPrintable(String& line, const char* buffer, unsigned int size) const
	{
		unsigned int printed;

		line.append('|');
		for (printed = 0; printed < size; ++printed)
		{
			char c = *(buffer + printed);
			if (c < 0x20 || c > 0x7E) // c is not printable ASCII, replace it by a dot '.'
				c = '.';
			line.append(c);
		}
		line.append('|');
		while (printed++ < 0x10)
			line.append(' ');
	}


	Hexdump& Hexdump::operator = (const Hexdump& rhs)
	{
		if (this != &rhs)
		{
			this->pBuffer = rhs.pBuffer;
			this->pSize = rhs.pSize;
		}
		return *this;
	}




} // namespace Utils
} // namespace Core
} // namespace Yuni




