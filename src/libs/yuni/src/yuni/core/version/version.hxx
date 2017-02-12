#ifndef __YUNI_CORE_VERSION_HXX__
# define __YUNI_CORE_VERSION_HXX__


namespace Yuni
{

	inline Version::Version()
		:hi(0u), lo(0u), revision(0u)
	{}


	inline Version::Version(unsigned int h)
		:hi(h), lo(0u), revision(0u)
	{}


	inline Version::Version(unsigned int h, unsigned int l)
		:hi(h), lo(l), revision(0u)
	{}


	inline Version::Version(unsigned int h, unsigned int l, unsigned int r)
		:hi(h), lo(l), revision(r)
	{}


	inline bool Version::isEqualTo(const Version& rhs) const
	{
		return (rhs.hi == hi) && (rhs.lo == lo) && (rhs.revision == revision);
	}


	inline bool Version::operator <  (const Version& rhs) const
	{
		return isLessThan(rhs);
	}

	inline bool Version::operator <= (const Version& rhs) const
	{
		return isEqualTo(rhs) || isLessThan(rhs);
	}

	inline bool Version::operator >  (const Version& rhs) const
	{
		return isGreaterThan(rhs);
	}

	inline bool Version::operator >= (const Version& rhs) const
	{
		return isEqualTo(rhs) || isGreaterThan(rhs);
	}

	inline bool Version::operator == (const Version& rhs) const
	{
		return isEqualTo(rhs);
	}

	inline bool Version::operator != (const Version& rhs) const
	{
		return !isEqualTo(rhs);
	}



} // namespace Yuni



inline std::ostream& operator << (std::ostream& out, const Yuni::Version& rhs)
{
	rhs.print(out);
	return out;
}


#endif // __YUNI_CORE_VERSION_HXX__
