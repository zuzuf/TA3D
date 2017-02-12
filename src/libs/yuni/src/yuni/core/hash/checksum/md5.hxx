#ifndef __YUNI_CORE_HASH_CHECKSUM_MD5_HXX__
# define __YUNI_CORE_HASH_CHECKSUM_MD5_HXX__


namespace Yuni
{
namespace Hash
{
namespace Checksum
{



	inline String MD5::FromString(const String& s)
	{
		return MD5().fromString(s);
	}



	inline String MD5::FromRawData(const void* rawdata, uint64 size)
	{
		return MD5().fromRawData(rawdata, size);
	}




} // namespace Checksum
} // namespace Hash
} // namespace Yuni

#endif // __YUNI_CORE_HASH_CHECKSUM_MD5_HXX__
