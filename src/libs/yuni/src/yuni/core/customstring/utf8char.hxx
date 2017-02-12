#ifndef __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_UTF8_CHAR_HXX__
# define __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_UTF8_CHAR_HXX__



namespace Yuni
{
namespace UTF8
{

	inline unsigned char Char::Mask8Bits(const void* p)
	{
		return static_cast<unsigned char>(0xFF & *(static_cast<const char*>(p)));
	}

	inline unsigned char Char::Mask8Bits(const char p)
	{
		return static_cast<unsigned char>(0xFF & p);
	}


	inline unsigned int Char::Size(const void* p)
	{
		// Char. number range  |        UTF-8 octet sequence
		//    (hexadecimal)    |              (binary)
		// --------------------+------------------------------------
		// 0000 0000-0000 007F | 0xxxxxxx
		// 0000 0080-0000 07FF | 110xxxxx 10xxxxxx
		// 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
		// 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

		const unsigned char lead = Mask8Bits(p);
		if (lead < 0x80) // the leas byte is zero, ascii
			return 1;
		if ((lead >> 5) == 0x6)  // 110x xxxx
			return 2;
		if ((lead >> 4) == 0xe)  // 1110 xxxx
			return 3;
		if ((lead >> 3) == 0x1e) // 1111 0xxx
			return 4;
		return 0;
	}


	inline unsigned int Char::size() const
	{
		if (pValue < 0x80)
			return 1;
		if (pValue < 0x800)
			return 2;
		if (pValue < 0x10000)
			return 3;
		return 4;
	}


	inline bool Char::IsASCII(unsigned char c)
	{
		return c < 0x80;
	}


	inline bool Char::IsTrail(unsigned char c)
	{
		return ((c >> 6) == 0x2);
	}


	template<class StreamT>
	void Char::write(StreamT& out) const
	{
		if (pValue < 0x80)
			out.put(static_cast<char>(static_cast<unsigned char>(pValue)));
		else
		{
			if (pValue < 0x800)
			{
				out.put(static_cast<char>(static_cast<unsigned char>((pValue >> 6)   | 0xc0)));
				out.put(static_cast<char>(static_cast<unsigned char>((pValue & 0x3f) | 0x80)));
			}
			else
			{
				if (pValue < 0x10000)
				{
					out.put(static_cast<char>(static_cast<unsigned char>((pValue >> 12)         | 0xe0)));
					out.put(static_cast<char>(static_cast<unsigned char>(((pValue >> 6) & 0x3f) | 0x80)));
					out.put(static_cast<char>(static_cast<unsigned char>((pValue & 0x3f)        | 0x80)));
				}
				else
				{                                // four bytes
					out.put(static_cast<char>(static_cast<unsigned char>((pValue >> 18)         | 0xf0)));
					out.put(static_cast<char>(static_cast<unsigned char>(((pValue >> 12)& 0x3f) | 0x80)));
					out.put(static_cast<char>(static_cast<unsigned char>(((pValue >> 6) & 0x3f) | 0x80)));
					out.put(static_cast<char>(static_cast<unsigned char>((pValue & 0x3f)        | 0x80)));
				}
			}
		}
	}





} // namespace UTF8
} // namespace Yuni

#endif // __YUNI_CORE_CUSTOMSTRING_CUSTOMSTRING_UTF8_CHAR_HXX__
