
#include "monitor.h"
#include "../../core/hash/checksum/md5.h"


namespace Yuni
{
namespace Device
{
namespace Display
{

	void Monitor::addSafeResolutions()
	{
		// Default (and assumed safe) resolutions
		pResolutions.push_back(new Resolution(800, 600, 32));
		pResolutions.push_back(new Resolution(640, 480, 32));
	}


	Monitor::Monitor()
		:pHandle(Monitor::InvalidHandle),
		pPrimary(false), pHardwareAcceleration(false), pBuiltin(false)
	{
		addSafeResolutions();
	}


	Monitor::Monitor(const String& nm, const Monitor::Handle hwn, const bool p, const bool a, const bool b)
		:pHandle(hwn), pProductName(nm), pResolutions(),
		pPrimary(p), pHardwareAcceleration(a), pBuiltin(b)
	{
		addSafeResolutions();
	}


	Monitor::Monitor(const Monitor& c)
		:pHandle(c.pHandle), pProductName(c.pProductName), pResolutions(c.pResolutions), pPrimary(c.pPrimary),
		pHardwareAcceleration(c.pHardwareAcceleration), pBuiltin(c.pBuiltin)
	{}


	Monitor::~Monitor()
	{
		pResolutions.clear();
	}


	Resolution::Ptr Monitor::recommendedResolution() const
	{
		return (!pResolutions.empty())
			? *(pResolutions.begin())
			: new Resolution(640, 480);
	}


	void Monitor::clear()
	{
		pResolutions.clear();
	}


	const String& Monitor::guid()
	{
		// We assume that `pResolutions` is not empty
		if (pMD5Cache.empty())
		{
			String bld;
			bld << this->pProductName << '|';
			if (!pResolutions.empty())
				bld << (*pResolutions.begin())->toString();
			bld << '|' << pPrimary << '|' << pBuiltin << '|' << pHardwareAcceleration;

			Hash::Checksum::MD5 md5;
			pMD5Cache = md5.fromString(bld);
		}
		return pMD5Cache;
	}


	void Monitor::add(const Resolution::Ptr& r)
	{
		pResolutions.push_back(r);
	}


	bool Monitor::resolutionIsValid(const Resolution::Ptr& rhs) const
	{
		if (NULL != rhs) // The pointer must be valid
		{
			// Browse all available resolutions
			// The lookup should be done in the usual way since it is a sorted descendant list
			for (Resolution::Vector::const_iterator it = pResolutions.begin(); it != pResolutions.end(); ++it)
			{
				if (NULL != (*it) && *(*it) == *rhs)
					return true;
			}
		}
		return false;
	}


	Monitor& Monitor::operator += (Resolution* rhs)
	{
		pResolutions.push_back(rhs);
		return *this;
	}

	Monitor& Monitor::operator += (const Resolution::Ptr& rhs)
	{
		pResolutions.push_back(rhs);
		return *this;
	}

	Monitor& Monitor::operator << (Resolution* rhs)
	{
		pResolutions.push_back(rhs);
		return *this;
	}

	Monitor& Monitor::operator << (const Resolution::Ptr& rhs)
	{
		pResolutions.push_back(rhs);
		return *this;
	}




} // namespace Display
} // namespace Device
} // namespace Yuni


