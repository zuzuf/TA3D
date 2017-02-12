
#include <yuni/yuni.h>
#include <yuni/device/display/list.h>
#include <iostream>


using namespace Yuni;


void title(const String& t)
{
	std::cout << std::endl << "--- " << t << " ---" << std::endl << std::endl;
}


void printMonitor(Device::Display::Monitor::Ptr monitor)
{
	if (!monitor)
		return;

	// The name of the monitor
	std::cout << "Product name: `"
		<< (monitor->productName().empty() ? "<Unknown>" : monitor->productName())
		<< "`\n";

	// Its GUID
	std::cout << "guid:" << monitor->guid();
	if (monitor->primary())
		std::cout << ", primary";
	if (monitor->hardwareAcceleration())
		std::cout << ", Hardware-Accelerated";
	std::cout << std::endl;

	// All resolutions
	Device::Display::Resolution::Vector::const_iterator it;
	for (it = monitor->resolutions().begin(); it != monitor->resolutions().end(); ++it)
	{
		if (!(*it))
			std::cout << "ERROR" << std::endl;
		else
			std::cout << "  . " << (*it)->toString() << "\n";
	}

	// Space
	std::cout << std::endl;
}



int main(void)
{
	// A list of monitors
	Device::Display::List monitors;

	// The list must be refreshed according the current Operating System settings.
	// Without any parameters, the resolutions are filtered to be higher than
	// a minimal resolution (640x480x8 by default).
	//
	// To be sure to have exactly all resolutions, call `refresh()` like this :
	// monitors.refresh(0, 0, 0);
	//
	monitors.refresh();


	// Displaying information about all available monitors
	title("All available displays, the primary display included");
	Device::Display::List::const_iterator it;
	for (it = monitors.begin(); it != monitors.end(); ++it)
	{
		// Print information about a single monitor
		printMonitor(*it);
	}

	return 0;
}
