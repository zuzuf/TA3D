#ifndef __YUNI_CORE_BIT_BIT_HXX__
# define __YUNI_CORE_BIT_BIT_HXX__


namespace Yuni
{
namespace Core
{
namespace Bit
{


	template<class T>
	unsigned int Count(T data)
	{
		unsigned int c = 0;
		while (data)
		{
			c += c & 0x1u;
			data >>= 1;
		}
		return c;
	}


	inline bool Get(const char* data, unsigned int index)
	{
		# ifdef YUNI_OS_MSVC
		return (YUNI_BIT_GET(data, index)) ? true : false;
		# else
		return YUNI_BIT_GET(data, index);
		# endif
	}


	inline void Set(char* data, unsigned int index)
	{
		//data[index >> 3] |= 1 << ((ENDIANESS) ? (7 - (i) & 7) : (i) & 7);
		YUNI_BIT_SET(data, index);
	}


	inline void Unset(char* data, unsigned int index)
	{
		//data[index >> 3] ~= (1 << ((ENDIANESS) ? (7 - (index) & 7) : (index) & 7));
		YUNI_BIT_UNSET(data, index);
	}




} // namespace Bit
} // namespace Core
} // namespace Yuni

#endif // __YUNI_CORE_BIT_BIT_HXX__
