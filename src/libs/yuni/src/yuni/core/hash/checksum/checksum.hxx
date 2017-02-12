#ifndef __YUNI_CORE_HASH_CHECKSUM_CHECKSUM_HXX__
# define __YUNI_CORE_HASH_CHECKSUM_CHECKSUM_HXX__


namespace Yuni
{
namespace Hash
{
namespace Checksum
{


	inline void IChecksum::reset()
	{
		pValue.clear();
	}


	inline const String& IChecksum::fromString(const String& s)
	{
		return fromRawData(s.data(), s.size());
	}


	inline const String& IChecksum::operator[] (const String& s)
	{
		fromString(s);
		return pValue;
	}


	inline const String& IChecksum::value() const
	{
		return pValue;
	}


	inline const String& IChecksum::operator() () const
	{
		return pValue;
	}



} // namespace Checksum
} // namespace Hash
} // namespace Yuni

#endif // __YUNI_CORE_HASH_CHECKSUM_CHECKSUM_HXX__
