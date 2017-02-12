
#include "version.h"

namespace Yuni
{


	void Version::InternalLib(Version& v)
	{
		v.hi = YUNI_VERSION_HI;
		v.lo = YUNI_VERSION_LO;
		v.revision = YUNI_VERSION_REV;
	}


	Version::Version(const Version& c)
		:hi(c.hi), lo(c.lo), revision(c.revision)
	{}


	bool Version::isLessThan(const Version& rhs) const
	{
		if (hi < rhs.hi)
			return true;
		if (hi == rhs.hi)
		{
			if (lo < rhs.lo)
				return true;
			if (lo == rhs.lo)
				return revision < rhs.revision;
		}
		return false;
	}


	bool Version::isGreaterThan(const Version& rhs) const
	{
		if (hi > rhs.hi)
			return true;
		if (hi == rhs.hi)
		{
			if (lo > rhs.lo)
				return true;
			if (lo == rhs.lo)
				return revision > rhs.revision;
		}
		return false;
	}


	String Version::toString() const
	{
		return String() << hi << '.' << lo << '.' << revision;
	}


	void Version::print(std::ostream& out) const
	{
		out << hi << "." << lo << "." << revision;
	}



	Version& Version::operator = (const Version& rhs)
	{
		hi = rhs.hi;
		lo = rhs.lo;
		revision = rhs.revision;
		return *this;
	}


	bool Version::null() const
	{
		return (hi == 0 && lo == 0 && revision == 0);
	}



} // namespace Yuni

