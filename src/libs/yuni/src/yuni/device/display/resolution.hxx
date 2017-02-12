#ifndef __YUNI_DEVICE_DISPLAY_RESOLUTION_HXX__
# define __YUNI_DEVICE_DISPLAY_RESOLUTION_HXX__



namespace Yuni
{
namespace Device
{
namespace Display
{

	inline Resolution::Resolution(const Resolution& c)
		:pWidth(c.pWidth), pHeight(c.pHeight), pBitsPerPixel(c.pBitsPerPixel)
	{}


	inline uint32 Resolution::width() const
	{
		return pWidth;
	}


	inline uint32 Resolution::height() const
	{
		return pHeight;
	}

	inline uint8 Resolution::bitPerPixel() const
	{
		return pBitsPerPixel;
	}


	inline bool Resolution::operator == (const Resolution& rhs) const
	{
		return rhs.pWidth == pWidth && rhs.pHeight == pHeight
			&& rhs.pBitsPerPixel == pBitsPerPixel;
	}


	inline bool Resolution::operator != (const Resolution& rhs) const
	{
		return !(*this == rhs);
	}


	inline bool Resolution::operator <= (const Resolution& rhs) const
	{
		return pWidth <= rhs.pWidth && pHeight <= rhs.pHeight
			&& pBitsPerPixel <= rhs.pBitsPerPixel;
	}


	inline bool Resolution::operator >= (const Resolution& rhs) const
	{
		return pWidth >= rhs.pWidth && pHeight >= rhs.pHeight
			&& pBitsPerPixel >= rhs.pBitsPerPixel;
	}




} // namespace Display
} // namespace Device
} // namespace Yuni

#endif // __YUNI_DEVICE_DISPLAY_RESOLUTION_H__
