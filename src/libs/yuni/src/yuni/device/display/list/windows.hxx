
#include "../../../core/system/windows.hdr.h"


namespace Yuni
{
namespace Device
{
namespace Display
{


	// Use the following structure rather than DISPLAY_DEVICE, since some old
	// versions of DISPLAY_DEVICE are missing the last two fields and this can
	// cause problems with EnumDisplayDevices on Windows 2000.
	struct DISPLAY_DEVICE_FULL
	{
		DWORD  cb;
		TCHAR  DeviceName[32];
		TCHAR  DeviceString[128];
		DWORD  StateFlags;
		TCHAR  DeviceID[128];
		TCHAR  DeviceKey[128];
	};


	static SingleMonitorFound* findMonitor(const Yuni::String& monitorID, MonitorsFound& lst)
	{
		unsigned int i;
		for (i = 0; i < lst.size() && lst[i].first->guid() != monitorID; ++i)
			;
		return (i >= lst.size()) ? NULL : &lst[i];
	}

	static void addResolutions(DISPLAY_DEVICE_FULL& device, SmartPtr<OrderedResolutions> res)
	{
		DEVMODE devMode;
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmDriverExtra = 32;

		for (unsigned int i = 0; EnumDisplaySettings(device.DeviceName, i, &devMode); ++i)
		{
			(*res)[devMode.dmPelsWidth][devMode.dmPelsHeight][(uint8)devMode.dmBitsPerPel] = true;
		}
	}

	/*!
	** \brief Windows-specific implementation for the monitor / resolution list refresh
	*/
	static void refreshForWindows(MonitorsFound& lst)
	{
		DISPLAY_DEVICE_FULL displayDevice;
		displayDevice.cb = sizeof (DISPLAY_DEVICE_FULL);
		// Loop on all display devices
		for (unsigned int countDevices = 0; EnumDisplayDevices(NULL, countDevices, (DISPLAY_DEVICE*)&displayDevice, 0); ++countDevices)
		{
			// Ignore mirrored displays
			if (!(displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
			{
				DISPLAY_DEVICE_FULL monitorDisplayDevice;
				monitorDisplayDevice.cb = sizeof (DISPLAY_DEVICE_FULL);
				// A second call is necessary to get the monitor name associated with the display
				EnumDisplayDevices(displayDevice.DeviceName, 0, (DISPLAY_DEVICE*)&monitorDisplayDevice, 0);
				bool mainDisplay = (0 != (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE));

				// Check if we have already stored info on this monitor
				SingleMonitorFound* monitorWithRes = findMonitor(monitorDisplayDevice.DeviceID, lst);
				bool newMonitor = (NULL == monitorWithRes);
				Monitor::Ptr monitor;
				SmartPtr<OrderedResolutions> res;
				if (newMonitor)
				{
					// Create the new monitor
					monitor = new Monitor(monitorDisplayDevice.DeviceString, (Monitor::Handle)monitorDisplayDevice.DeviceID, mainDisplay, true, true);
					res = new OrderedResolutions();
				}
				else
				{
					monitor = monitorWithRes->first;
					res = monitorWithRes->second;
				}

				// Add associated resolutions
				addResolutions(displayDevice, res);

				// Add the monitor and its resolutions to the list if necessary
				if (newMonitor)
					lst.push_back(SingleMonitorFound(monitor, res));
				if (countDevices > YUNI_DEVICE_MONITOR_COUNT_HARD_LIMIT)
					break;
			}
		}
	}



	static void refreshOSSpecific(MonitorsFound& lst)
	{
		refreshForWindows(lst);
	}



} // namespace Display
} // namespace Device
} // namespace Yuni

