#ifndef __YUNI_DEVICE_DISPLAY_MONITOR_HXX__
# define __YUNI_DEVICE_DISPLAY_MONITOR_HXX__


namespace Yuni
{
namespace Device
{
namespace Display
{


	inline Monitor::Handle Monitor::handle() const
	{
		return pHandle;
	}


	inline bool Monitor::valid() const
	{
		return pHandle != Monitor::InvalidHandle;
	}


	inline const String& Monitor::productName() const
	{
		return pProductName;
	}


	inline const Resolution::Vector& Monitor::resolutions() const
	{
		return pResolutions;
	}


	inline bool Monitor::primary() const
	{
		return pPrimary;
	}


	inline bool Monitor::hardwareAcceleration() const
	{
		return pHardwareAcceleration;
	}


	inline bool Monitor::builtin() const
	{
		return pBuiltin;
	}



} // namespace Display
} // namespace Device
} // namespace Yuni

#endif // __YUNI_DEVICE_DISPLAY_MONITOR_HXX__
