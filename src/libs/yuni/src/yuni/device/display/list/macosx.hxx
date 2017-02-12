
# include <CoreFoundation/CoreFoundation.h>
# include <ApplicationServices/ApplicationServices.h>
# include <IOKit/IOKitLib.h>
# include <IOKit/graphics/IOGraphicsLib.h>



# define GET_MODE_WIDTH(mode) GetDictionaryLong((mode), kCGDisplayWidth)
# define GET_MODE_HEIGHT(mode) GetDictionaryLong((mode), kCGDisplayHeight)
# define GET_MODE_REFRESH_RATE(mode) GetDictionaryLong((mode), kCGDisplayRefreshRate)
# define GET_MODE_BITS_PER_PIXEL(mode) GetDictionaryLong((mode), kCGDisplayBitsPerPixel)

# define GET_MODE_SAFE_FOR_HARDWARE(mode) GetDictionaryBoolean((mode), kCGDisplayModeIsSafeForHardware)
# define GET_MODE_STRETCHED(mode) GetDictionaryBoolean((mode), kCGDisplayModeIsStretched)

# define GET_MODE_TELEVISION(mode) GetDictionaryBoolean((mode), kCGDisplayModeIsTelevisionOutput)


namespace Yuni
{
namespace Device
{
namespace Display
{


	/*!
	** \brief Get a boolean from the dictionary
	**
	** \param theDict The dictionary
	** \param key The key to read its value
	*/
	static Boolean GetDictionaryBoolean(CFDictionaryRef theDict, const void* key)
	{
		Boolean value(false);
		CFBooleanRef boolRef = (CFBooleanRef) CFDictionaryGetValue(theDict, key);
		if (NULL != boolRef)
			value = CFBooleanGetValue(boolRef);
		return value;
	}



	/*!
	** \brief Get a long from the dictionary
	**
	** \param theDict The dictionary
	** \param key The key to read its value
	*/
	static long GetDictionaryLong(CFDictionaryRef theDict, const void* key)
	{
		// get a long from the dictionary
		long value(0);
		const CFNumberRef numRef = (CFNumberRef) CFDictionaryGetValue(theDict, key);
		if (NULL != numRef)
			CFNumberGetValue(numRef, kCFNumberLongType, &value);
		return value;
	}


	static void cocoaGetAllAvailableModesUseful(CGDirectDisplayID display, SmartPtr<OrderedResolutions>& res)
	{
		// get a list of all possible display modes for this system.
		CFArrayRef availableModes = CGDisplayAvailableModes(display);
		const unsigned int numberOfAvailableModes = CFArrayGetCount(availableModes);
		if (!numberOfAvailableModes)
			return;

		// get the current bits per pixel.
		const long currentModeBitsPerPixel = GET_MODE_BITS_PER_PIXEL(CGDisplayCurrentMode(display));

		for (unsigned int i = 0; i < numberOfAvailableModes; ++i)
		{
			// look at each mode in the available list
			CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(availableModes, i);

			// we are only interested in modes with the same bits per pixel as current.
			// to allow for switching from fullscreen to windowed modes.
			// that are safe for this hardward
			// that are not stretched.
			Boolean safeForHardware = GET_MODE_SAFE_FOR_HARDWARE(mode);
			Boolean stretched = GET_MODE_STRETCHED(mode);
			int bitsPerPixel = GET_MODE_BITS_PER_PIXEL(mode);

			if ((bitsPerPixel == currentModeBitsPerPixel) && safeForHardware && !stretched)
				(*res)[GET_MODE_WIDTH(mode)][GET_MODE_HEIGHT(mode)][(uint8) bitsPerPixel] = true;
		}
	}


	static void DictionaryValueToString(String& out, CFStringRef formatString, ...)
	{
		CFStringRef resultString;
		CFDataRef data;
		va_list argList;

		va_start(argList, formatString);
		resultString = CFStringCreateWithFormatAndArguments(NULL, NULL, formatString, argList);
		va_end(argList);

		data = CFStringCreateExternalRepresentation(NULL, resultString, CFStringGetSystemEncoding(), '?');

		if (data != NULL)
		{
			char buffer[51];
			snprintf (buffer, 50, "%.*s", (int)CFDataGetLength(data), CFDataGetBytePtr(data));
			out.append(buffer, strlen(buffer));
			CFRelease(data);
		}

		CFRelease(resultString);
	}




	/*!
	** \brief Get all monitor and their resolutions from the Cocoa Framework
	**
	** \see http://developer.apple.com/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/Reference/reference.html#//apple_ref/doc/uid/TP30001070-CH202-F17085
	** \see http://developer.apple.com/documentation/GraphicsImaging/Conceptual/QuartzDisplayServicesConceptual/Articles/DisplayInfo.html#//apple_ref/doc/uid/TP40004272
	** \see http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_quartz_services/chapter_952_section_2.html
	** \see http://www.cocoabuilder.com/archive/message/cocoa/2006/9/7/170773
	*/
	static void refreshForCocoa(MonitorsFound& lst)
	{
		// All displays
		CGDirectDisplayID displayArray [YUNI_DEVICE_MONITOR_COUNT_HARD_LIMIT];
		// The count of display
		CGDisplayCount numDisplays;

		// Grab all available displays
		CGGetOnlineDisplayList(YUNI_DEVICE_MONITOR_COUNT_HARD_LIMIT, displayArray, &numDisplays);
		if (!numDisplays)
			return;

		// Product name
		String monitorProductName;

		// Browse all displays
		for (unsigned int i = 0; i < numDisplays; ++i)
		{
			const CGDirectDisplayID display = displayArray[i];

			monitorProductName.clear();

			// Informations about the display, such as its product name
			io_connect_t displayPort = CGDisplayIOServicePort(display);
			if (displayPort != MACH_PORT_NULL)
			{
				CFDictionaryRef dict = IODisplayCreateInfoDictionary(displayPort, 0);

				CFDictionaryRef names = (CFDictionaryRef)CFDictionaryGetValue(dict, CFSTR(kDisplayProductName));
				// Count items in the dictionary
				CFIndex count = CFDictionaryGetCount(names);

				if (count)
				{
					CFTypeRef* keys   = (CFTypeRef*) ::malloc(count * sizeof(CFTypeRef));
					CFTypeRef* values = (CFTypeRef*) ::malloc(count * sizeof(CFTypeRef));
					CFDictionaryGetKeysAndValues(names, (const void **) keys, (const void **) values);

					DictionaryValueToString(monitorProductName, CFSTR("%@"), values[0]);
					monitorProductName.trim(" \r\n\t");

					::free(keys);
					::free(values);
				}
				CFRelease(dict);
			}

			// int width  = CGDisplayPixelsWide(display);
			// int height = CGDisplayPixelsHigh(display);
			// int bpp    = CGDisplayBitsPerPixel(display);
			const bool mainDisplay = CGDisplayIsMain(display);
			const bool builtin     = CGDisplayIsBuiltin(display);
			const bool ha          = CGDisplayUsesOpenGLAcceleration(display);
			// uint32_t modelNumber = CGDisplayModelNumber(display);
			// uint32_t serialNumer = CGDisplaySerialNumber(display);

			Monitor::Ptr newMonitor(new Monitor(monitorProductName,
				(Monitor::Handle)display, mainDisplay, ha, builtin));

			SmartPtr<OrderedResolutions> res(new OrderedResolutions());
			cocoaGetAllAvailableModesUseful(display, res);

			// Add it to the list
			lst.push_back(SingleMonitorFound(newMonitor, res));
		}
	}


	static void refreshOSSpecific(MonitorsFound& lst)
	{
		refreshForCocoa(lst);
	}





} // namespace Display
} // namespace Device
} // namespace Yuni

