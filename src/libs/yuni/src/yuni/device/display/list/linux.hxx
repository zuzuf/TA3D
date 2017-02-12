
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>



namespace Yuni
{
namespace Device
{
namespace Display
{


	static void refreshForX11(MonitorsFound& lst)
	{
		::Display* display = XOpenDisplay(NULL);
		if (display)
		{
			const Monitor::Handle scCount = ScreenCount(display);

			for (Monitor::Handle i = 0; i != scCount; ++i)
			{
				int depCount;
				int* depths = XListDepths(display, i, &depCount);

				Monitor::Ptr newMonitor(new Monitor(String(), i, (0 == i), true, false));
				SmartPtr<OrderedResolutions> res(new OrderedResolutions());

				// All resolutions
				int count;
				XRRScreenSize* list = XRRSizes(display, i, &count);
				for (int i = 0; i < count; ++i)
				{
					XRRScreenSize* it = list + i;
					for (int j = 0; j < depCount; ++j)
					{
						const int d = *(depths + j);
						if (d == 8 || d == 16 || d == 24 || d == 32)
							(*res)[it->width][it->height][(uint8) d] = true;
					}
				}

				lst.push_back(SingleMonitorFound(newMonitor, res));
				XFree(depths);
			}
			XCloseDisplay(display);
		}
	}


	static void refreshOSSpecific(MonitorsFound& lst)
	{
		refreshForX11(lst);
	}





} // namespace Display
} // namespace Device
} // namespace Yuni

