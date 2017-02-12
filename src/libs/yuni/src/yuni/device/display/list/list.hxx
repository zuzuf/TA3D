#ifndef __YUNI_DEVICE_DISPLAY_LIST_HXX__
# define __YUNI_DEVICE_DISPLAY_LIST_HXX__


namespace Yuni
{
namespace Device
{
namespace Display
{


	inline List::List(const List& c)
		:pMonitors(c.pMonitors), pPrimary(c.pPrimary), pNullMonitor(c.pNullMonitor)
	{}


	inline size_t List::size() const
	{
		return pMonitors.size();
	}


	inline Monitor::Ptr List::primary() const
	{
		return pPrimary;
	}


	inline List::iterator List::begin()
	{
		return pMonitors.begin();
	}


	inline List::iterator List::end()
	{
		return pMonitors.end();
	}

	inline List::const_iterator List::begin() const
	{
		return pMonitors.begin();
	}


	inline List::const_iterator List::end() const
	{
		return pMonitors.end();
	}


} // namespace Display
} // namespace Device
} // namespace Yuni


#endif // __YUNI_DEVICE_DISPLAY_LIST_HXX__
