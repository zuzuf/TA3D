
#include "resolution.h"
#include "../../core/hash/checksum/md5.h"
#include "../../core/math.h"


namespace Yuni
{
namespace Device
{
namespace Display
{


	Resolution::Resolution(const uint32 w, const uint32 h, const uint8 b)
	{
		pBitsPerPixel = (32 == b ||  24 == b || 16 == b || 8 == b) ? (uint8)b : (uint8)32;
		pWidth  = Math::MinMax<uint32>(w, minimumWidth,  maximumWidth);
		pHeight = Math::MinMax<uint32>(h, minimumHeight, maximumHeight);
	}


	String Resolution::toString() const
	{
		String ret;
		ret << pWidth << 'x' << pHeight;
		if (pBitsPerPixel)
			ret << " (" << (int)pBitsPerPixel << "Bits)";
		return ret;
	}


	bool Resolution::operator < (const Resolution& rhs) const
	{
		if (pWidth < rhs.pWidth)
			return true;
		if (pWidth == rhs.pWidth)
		{
			if (pHeight < rhs.pHeight)
				return true;
			if (pHeight == rhs.pHeight)
				return (pBitsPerPixel < rhs.pBitsPerPixel);
		}
		return false;
	}


	bool Resolution::operator > (const Resolution& rhs) const
	{
		if (pWidth > rhs.pWidth)
			return true;
		if (pWidth == rhs.pWidth)
		{
			if (pHeight > rhs.pHeight)
				return true;
			if (pHeight == rhs.pHeight)
				return (pBitsPerPixel > rhs.pBitsPerPixel);
		}
		return false;
	}


	std::ostream& Resolution::print(std::ostream& out) const
	{
		out << pWidth << "x" << pHeight;
		if (pBitsPerPixel)
			out << " (" << (int) pBitsPerPixel << "Bits)";
		return out;
	}


	Resolution& Resolution::operator = (const Resolution& p)
	{
		pWidth = p.pWidth;
		pHeight = p.pHeight;
		pBitsPerPixel = p.pBitsPerPixel;
		return (*this);
	}




} // namespace Display
} // namespace Device
} // namespace Yuni

